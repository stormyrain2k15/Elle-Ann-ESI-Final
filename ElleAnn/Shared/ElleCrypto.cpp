#include "ElleCrypto.h"
#include "ElleLogger.h"

#include <windows.h>
#include <bcrypt.h>
#include <mutex>
#include <cstring>

#pragma comment(lib, "bcrypt.lib")

namespace {

BCRYPT_ALG_HANDLE g_sha256    = nullptr;
BCRYPT_ALG_HANDLE g_hmacSha256= nullptr;
BCRYPT_ALG_HANDLE g_rng       = nullptr;
std::once_flag    g_initFlag;

void InitAlgorithmsOnce() {
    std::call_once(g_initFlag, []{
        NTSTATUS st;
        st = BCryptOpenAlgorithmProvider(&g_sha256,
                                          BCRYPT_SHA256_ALGORITHM,
                                          nullptr, 0);
        if (!BCRYPT_SUCCESS(st))
            ELLE_WARN("BCryptOpenAlgorithmProvider(SHA256) NTSTATUS=0x%08X", (unsigned)st);

        st = BCryptOpenAlgorithmProvider(&g_hmacSha256,
                                          BCRYPT_SHA256_ALGORITHM,
                                          nullptr,
                                          BCRYPT_ALG_HANDLE_HMAC_FLAG);
        if (!BCRYPT_SUCCESS(st))
            ELLE_WARN("BCryptOpenAlgorithmProvider(HMAC-SHA256) NTSTATUS=0x%08X", (unsigned)st);

        st = BCryptOpenAlgorithmProvider(&g_rng,
                                          BCRYPT_RNG_ALGORITHM,
                                          nullptr, 0);
        if (!BCRYPT_SUCCESS(st))
            ELLE_WARN("BCryptOpenAlgorithmProvider(RNG) NTSTATUS=0x%08X", (unsigned)st);
    });
}

constexpr char kB64UrlAlphabet[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

}

namespace ElleCrypto {

bool Sha256(const void* data, size_t len, uint8_t out[32]) {
    InitAlgorithmsOnce();
    if (!g_sha256) return false;

    BCRYPT_HASH_HANDLE h = nullptr;
    NTSTATUS st = BCryptCreateHash(g_sha256, &h, nullptr, 0, nullptr, 0, 0);
    if (!BCRYPT_SUCCESS(st)) {
        ELLE_WARN("BCryptCreateHash NTSTATUS=0x%08X", (unsigned)st);
        return false;
    }
    bool ok = false;
    do {
        st = BCryptHashData(h, (PUCHAR)data, (ULONG)len, 0);
        if (!BCRYPT_SUCCESS(st)) break;
        st = BCryptFinishHash(h, out, 32, 0);
        if (!BCRYPT_SUCCESS(st)) break;
        ok = true;
    } while (0);
    BCryptDestroyHash(h);
    if (!ok) ELLE_WARN("BCryptHashData/FinishHash NTSTATUS=0x%08X", (unsigned)st);
    return ok;
}

std::string Sha256Hex(const std::string& data) {
    uint8_t digest[32];
    if (!Sha256(data.data(), data.size(), digest)) return {};
    static const char hex[] = "0123456789abcdef";
    std::string out(64, '0');
    for (size_t i = 0; i < 32; i++) {
        out[i*2]   = hex[(digest[i] >> 4) & 0xF];
        out[i*2+1] = hex[digest[i] & 0xF];
    }
    return out;
}

bool HmacSha256(const void* key, size_t keyLen,
                const void* data, size_t dataLen,
                uint8_t out[32]) {
    InitAlgorithmsOnce();
    if (!g_hmacSha256) return false;

    BCRYPT_HASH_HANDLE h = nullptr;
    NTSTATUS st = BCryptCreateHash(g_hmacSha256, &h, nullptr, 0,
                                    (PUCHAR)key, (ULONG)keyLen, 0);
    if (!BCRYPT_SUCCESS(st)) {
        ELLE_WARN("BCryptCreateHash(HMAC) NTSTATUS=0x%08X", (unsigned)st);
        return false;
    }
    bool ok = false;
    do {
        st = BCryptHashData(h, (PUCHAR)data, (ULONG)dataLen, 0);
        if (!BCRYPT_SUCCESS(st)) break;
        st = BCryptFinishHash(h, out, 32, 0);
        if (!BCRYPT_SUCCESS(st)) break;
        ok = true;
    } while (0);
    BCryptDestroyHash(h);
    if (!ok) ELLE_WARN("BCryptHashData/FinishHash(HMAC) NTSTATUS=0x%08X", (unsigned)st);
    return ok;
}

bool RandomBytes(void* out, size_t len) {
    InitAlgorithmsOnce();
    if (!g_rng || !out || len == 0) return false;
    NTSTATUS st = BCryptGenRandom(g_rng, (PUCHAR)out, (ULONG)len, 0);
    if (!BCRYPT_SUCCESS(st)) {
        ELLE_WARN("BCryptGenRandom NTSTATUS=0x%08X", (unsigned)st);
        return false;
    }
    return true;
}

std::string RandomDigits(uint32_t digits) {

    if (digits == 0 || digits > 18) return {};

    uint64_t modulus = 1;
    for (uint32_t i = 0; i < digits; i++) modulus *= 10;

    const uint64_t threshold = (uint64_t)(-1) - ((uint64_t)(-1) % modulus) - 1;

    uint64_t r = 0;
    for (int attempts = 0; attempts < 64; attempts++) {
        if (!RandomBytes(&r, sizeof(r))) return {};
        if (r <= threshold) break;
    }
    r %= modulus;

    std::string out(digits, '0');
    for (int i = (int)digits - 1; i >= 0; i--) {
        out[i] = (char)('0' + (r % 10));
        r /= 10;
    }
    return out;
}

std::string RandomUrlToken(size_t bytes) {
    std::vector<uint8_t> buf(bytes);
    if (!RandomBytes(buf.data(), bytes)) return {};
    return Base64UrlEncode(buf.data(), bytes);
}

std::string RandomHex(size_t bytes) {

    std::vector<uint8_t> buf(bytes);
    if (!RandomBytes(buf.data(), bytes)) return {};
    static const char* kHex = "0123456789abcdef";
    std::string out;
    out.resize(bytes * 2);
    for (size_t i = 0; i < bytes; i++) {
        out[i * 2 + 0] = kHex[(buf[i] >> 4) & 0xF];
        out[i * 2 + 1] = kHex[buf[i] & 0xF];
    }
    return out;
}

bool ConstantTimeEquals(const void* a, const void* b, size_t len) {
    const uint8_t* pa = (const uint8_t*)a;
    const uint8_t* pb = (const uint8_t*)b;
    uint8_t diff = 0;
    for (size_t i = 0; i < len; i++) diff |= (uint8_t)(pa[i] ^ pb[i]);
    return diff == 0;
}

std::string Base64UrlEncode(const void* data, size_t len) {
    const uint8_t* in = (const uint8_t*)data;
    std::string out;
    out.reserve(((len + 2) / 3) * 4);
    size_t i = 0;
    while (i + 3 <= len) {
        uint32_t v = (uint32_t)in[i] << 16 | (uint32_t)in[i+1] << 8 | in[i+2];
        out.push_back(kB64UrlAlphabet[(v >> 18) & 0x3F]);
        out.push_back(kB64UrlAlphabet[(v >> 12) & 0x3F]);
        out.push_back(kB64UrlAlphabet[(v >>  6) & 0x3F]);
        out.push_back(kB64UrlAlphabet[v         & 0x3F]);
        i += 3;
    }
    if (i < len) {
        uint32_t v = (uint32_t)in[i] << 16;
        if (i + 1 < len) v |= (uint32_t)in[i+1] << 8;
        out.push_back(kB64UrlAlphabet[(v >> 18) & 0x3F]);
        out.push_back(kB64UrlAlphabet[(v >> 12) & 0x3F]);
        if (i + 1 < len) out.push_back(kB64UrlAlphabet[(v >> 6) & 0x3F]);

    }
    return out;
}

std::vector<uint8_t> Base64UrlDecode(const std::string& s) {

    static int8_t rev[256];
    static std::once_flag revInit;
    std::call_once(revInit, []{
        for (int i = 0; i < 256; i++) rev[i] = -1;
        for (int i = 0; i < 64; i++) rev[(uint8_t)kB64UrlAlphabet[i]] = (int8_t)i;
    });

    std::vector<uint8_t> out;
    out.reserve((s.size() * 3) / 4);
    uint32_t buf = 0;
    int bits = 0;
    for (char ch : s) {
        int v = rev[(uint8_t)ch];
        if (v < 0) return {};
        buf = (buf << 6) | (uint32_t)v;
        bits += 6;
        if (bits >= 8) {
            bits -= 8;
            out.push_back((uint8_t)((buf >> bits) & 0xFF));
        }
    }

    if (bits > 0 && (buf & ((1u << bits) - 1)) != 0) return {};
    return out;
}

}
