#include <doctest/doctest.h>

#include "ElleShnVersionStore.h"

#include <chrono>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <thread>

namespace fs = std::filesystem;

namespace {

std::string TempDir(const std::string& tag) {
    auto p = fs::temp_directory_path() /
        ("elle_shn_ver_" + tag + "_" + std::to_string(::getpid()));
    std::error_code ec;
    fs::remove_all(p, ec);
    fs::create_directories(p, ec);
    return p.string();
}

std::string ReadAll(const std::string& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f.is_open()) return {};
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

}

TEST_CASE("ElleShnVersionStore: first write — no previous, no version snapshot") {
    auto dir = TempDir("first");
    std::string bytes = "0123456789ABCDEF";
    auto r = ElleShnVersionStore::AtomicWriteWithVersioning(dir, "hero.shn", bytes);
    REQUIRE(r.ok);
    CHECK_FALSE(r.previous_existed);
    CHECK(r.previous_bytes == 0);
    CHECK(r.previous_hash.empty());
    CHECK(r.new_bytes == 16);
    CHECK(r.new_hash.size() == 40);
    CHECK(r.version_path.empty());
    CHECK(ReadAll((fs::path(dir) / "hero.shn").string()) == bytes);
}

TEST_CASE("ElleShnVersionStore: identical re-write — short-circuit, no version dir created") {
    auto dir = TempDir("idem");
    std::string bytes = "same-bytes-twice";
    ElleShnVersionStore::AtomicWriteWithVersioning(dir, "hero.shn", bytes);
    auto r = ElleShnVersionStore::AtomicWriteWithVersioning(dir, "hero.shn", bytes);
    REQUIRE(r.ok);
    CHECK(r.previous_existed);
    CHECK(r.previous_hash == r.new_hash);
    CHECK(r.version_path.empty());
    std::error_code ec;
    CHECK_FALSE(fs::exists(fs::path(dir) / ".shn_versions", ec));
}

TEST_CASE("ElleShnVersionStore: real change — old bytes snapshotted into .shn_versions") {
    auto dir = TempDir("change");
    std::string v1 = "version-one-payload";
    std::string v2 = "version-two-different";
    ElleShnVersionStore::AtomicWriteWithVersioning(dir, "hero.shn", v1);
    auto r = ElleShnVersionStore::AtomicWriteWithVersioning(dir, "hero.shn", v2);
    REQUIRE(r.ok);
    REQUIRE(r.previous_existed);
    CHECK(r.previous_bytes == v1.size());
    CHECK(r.new_bytes      == v2.size());
    CHECK(r.previous_hash  != r.new_hash);
    REQUIRE(!r.version_path.empty());
    CHECK(ReadAll(r.version_path) == v1);
    CHECK(ReadAll((fs::path(dir) / "hero.shn").string()) == v2);
}

TEST_CASE("ElleShnVersionStore: pruning keeps only last N versions") {
    auto dir = TempDir("prune");
    for (int i = 0; i < 8; ++i) {
        std::string b = "payload-" + std::to_string(i);
        ElleShnVersionStore::AtomicWriteWithVersioning(dir, "hero.shn", b, 3);
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    auto versions = ElleShnVersionStore::ListVersions(dir, "hero.shn");
    CHECK(versions.size() <= 3);
    for (auto& v : versions) CHECK(v.ms > 0);
}

TEST_CASE("ElleShnVersionStore: ListVersions returns newest-first") {
    auto dir = TempDir("list");
    ElleShnVersionStore::AtomicWriteWithVersioning(dir, "hero.shn", "a", 10);
    std::this_thread::sleep_for(std::chrono::milliseconds(3));
    ElleShnVersionStore::AtomicWriteWithVersioning(dir, "hero.shn", "ab", 10);
    std::this_thread::sleep_for(std::chrono::milliseconds(3));
    ElleShnVersionStore::AtomicWriteWithVersioning(dir, "hero.shn", "abc", 10);
    auto versions = ElleShnVersionStore::ListVersions(dir, "hero.shn");
    REQUIRE(versions.size() == 2);
    CHECK(versions[0].ms >= versions[1].ms);
}

TEST_CASE("ElleShnVersionStore: SHA1 hex is 40 chars, deterministic, distinct for different bytes") {
    auto dir = TempDir("hash");
    auto r1 = ElleShnVersionStore::AtomicWriteWithVersioning(dir, "h.shn", "hello");
    auto dir2 = TempDir("hash2");
    auto r2 = ElleShnVersionStore::AtomicWriteWithVersioning(dir2, "h.shn", "hello");
    auto dir3 = TempDir("hash3");
    auto r3 = ElleShnVersionStore::AtomicWriteWithVersioning(dir3, "h.shn", "world");
    CHECK(r1.new_hash.size() == 40);
    CHECK(r1.new_hash == r2.new_hash);
    CHECK(r1.new_hash != r3.new_hash);
    CHECK(r1.new_hash == "aaf4c61ddcc5e8a2dabede0f3b482cd9aea9434d");
}
