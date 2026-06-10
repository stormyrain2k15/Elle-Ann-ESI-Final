#include "ElleShnVersionStore.h"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace ElleShnVersionStore {

namespace {

constexpr const char* kVersionsDir = ".shn_versions";

std::uint64_t NowMs() {
    return (std::uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
}

std::string Sha1HexLite(const std::string& bytes) {
    std::uint32_t h[5] = {0x67452301u, 0xEFCDAB89u, 0x98BADCFEu, 0x10325476u, 0xC3D2E1F0u};
    auto rol = [](std::uint32_t v, std::uint32_t n) {
        return (v << n) | (v >> (32 - n));
    };
    std::uint64_t bitlen = (std::uint64_t)bytes.size() * 8ull;
    std::string padded = bytes;
    padded.push_back((char)0x80);
    while ((padded.size() % 64) != 56) padded.push_back('\0');
    for (int i = 7; i >= 0; --i)
        padded.push_back((char)((bitlen >> (i * 8)) & 0xFF));

    for (std::size_t off = 0; off < padded.size(); off += 64) {
        std::uint32_t w[80];
        for (int i = 0; i < 16; ++i) {
            const auto* p = reinterpret_cast<const unsigned char*>(padded.data() + off + i*4);
            w[i] = (std::uint32_t(p[0]) << 24) | (std::uint32_t(p[1]) << 16) |
                   (std::uint32_t(p[2]) << 8)  |  std::uint32_t(p[3]);
        }
        for (int i = 16; i < 80; ++i)
            w[i] = rol(w[i-3] ^ w[i-8] ^ w[i-14] ^ w[i-16], 1);
        std::uint32_t a=h[0], b=h[1], c=h[2], d=h[3], e=h[4];
        for (int i = 0; i < 80; ++i) {
            std::uint32_t f, k;
            if (i < 20)      { f = (b & c) | ((~b) & d);          k = 0x5A827999u; }
            else if (i < 40) { f = b ^ c ^ d;                     k = 0x6ED9EBA1u; }
            else if (i < 60) { f = (b & c) | (b & d) | (c & d);   k = 0x8F1BBCDCu; }
            else             { f = b ^ c ^ d;                     k = 0xCA62C1D6u; }
            std::uint32_t t = rol(a, 5) + f + e + k + w[i];
            e = d; d = c; c = rol(b, 30); b = a; a = t;
        }
        h[0] += a; h[1] += b; h[2] += c; h[3] += d; h[4] += e;
    }
    char buf[41];
    for (int i = 0; i < 5; ++i)
        std::snprintf(buf + i*8, 9, "%08x", h[i]);
    buf[40] = 0;
    return buf;
}

std::string ReadAllBytes(const fs::path& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f.is_open()) return {};
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

bool AtomicWriteFile(const fs::path& path, const std::string& bytes,
                     std::string& error) {
    fs::path tmp = path; tmp += ".tmp";
    {
        std::ofstream f(tmp, std::ios::binary | std::ios::trunc);
        if (!f.is_open()) { error = "cannot open " + tmp.string(); return false; }
        f.write(bytes.data(), (std::streamsize)bytes.size());
        if (!f.good())   { error = "write to " + tmp.string() + " failed"; return false; }
    }
    std::error_code ec;
    fs::rename(tmp, path, ec);
    if (ec) {
        fs::copy_file(tmp, path, fs::copy_options::overwrite_existing, ec);
        if (ec) { error = "atomic rename + copy fallback failed: " + ec.message(); return false; }
        fs::remove(tmp, ec);
    }
    return true;
}

void PruneVersions(const fs::path& versionsDir, std::uint32_t keep) {
    std::vector<fs::path> files;
    std::error_code ec;
    for (const auto& ent : fs::directory_iterator(versionsDir, ec)) {
        if (!ent.is_regular_file()) continue;
        if (ent.path().extension() == ".shn") files.push_back(ent.path());
    }
    if (files.size() <= keep) return;
    std::sort(files.begin(), files.end(),
              [](const fs::path& a, const fs::path& b) {
                  return a.filename() < b.filename();
              });
    for (std::size_t i = 0; i + keep < files.size(); ++i) {
        fs::remove(files[i], ec);
    }
}

}

WriteResult AtomicWriteWithVersioning(const std::string& absDir,
                                      const std::string& name,
                                      const std::string& bytes,
                                      std::uint32_t      keepVersions) {
    WriteResult r;
    r.new_bytes = bytes.size();
    r.new_hash  = Sha1HexLite(bytes);

    fs::path target = fs::path(absDir) / name;

    std::error_code ec;
    if (fs::exists(target, ec)) {
        r.previous_existed = true;
        std::string old_bytes = ReadAllBytes(target);
        r.previous_bytes = old_bytes.size();
        r.previous_hash  = Sha1HexLite(old_bytes);

        if (r.previous_hash == r.new_hash) {
            r.ok = true;
            r.version_path.clear();
            return r;
        }

        fs::path versionsDir = fs::path(absDir) / kVersionsDir /
                               fs::path(name).stem().string();
        fs::create_directories(versionsDir, ec);

        char tsbuf[24];
        std::snprintf(tsbuf, sizeof(tsbuf), "%020llu",
                      (unsigned long long)NowMs());
        std::string base = fs::path(name).stem().string();
        std::string ext  = fs::path(name).extension().string();
        if (ext.empty()) ext = ".shn";
        fs::path version_path = versionsDir /
                                (base + "." + tsbuf + ext);

        std::string ver_err;
        if (!AtomicWriteFile(version_path, old_bytes, ver_err)) {
            r.error = "version snapshot failed: " + ver_err;
            return r;
        }
        r.version_path = version_path.string();

        if (keepVersions > 0) PruneVersions(versionsDir, keepVersions);
    }

    std::string write_err;
    if (!AtomicWriteFile(target, bytes, write_err)) {
        r.error = write_err;
        return r;
    }
    r.ok = true;
    return r;
}

std::vector<VersionEntry> ListVersions(const std::string& absDir,
                                       const std::string& name) {
    std::vector<VersionEntry> out;
    std::error_code ec;
    fs::path versionsDir = fs::path(absDir) / kVersionsDir /
                           fs::path(name).stem().string();
    if (!fs::exists(versionsDir, ec)) return out;

    for (const auto& ent : fs::directory_iterator(versionsDir, ec)) {
        if (!ent.is_regular_file()) continue;
        if (ent.path().extension() != ".shn") continue;
        VersionEntry e;
        e.filename = ent.path().filename().string();
        e.path     = ent.path().string();
        e.bytes    = (std::uint64_t)ent.file_size(ec);

        std::string stem = ent.path().stem().string();
        auto dot = stem.rfind('.');
        if (dot != std::string::npos && dot + 1 < stem.size()) {
            try { e.ms = (std::uint64_t)std::stoull(stem.substr(dot + 1)); }
            catch (...) { e.ms = 0; }
        }
        out.push_back(std::move(e));
    }
    std::sort(out.begin(), out.end(),
              [](const VersionEntry& a, const VersionEntry& b) {
                  return a.ms > b.ms;
              });
    return out;
}

}
