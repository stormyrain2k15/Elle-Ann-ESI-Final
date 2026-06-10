#pragma once
#ifndef ELLE_SHN_VERSION_STORE_H
#define ELLE_SHN_VERSION_STORE_H

#include <cstdint>
#include <string>
#include <vector>

namespace ElleShnVersionStore {

struct WriteResult {
    bool        ok              = false;
    bool        previous_existed = false;
    std::string previous_hash;
    std::string new_hash;
    std::size_t previous_bytes  = 0;
    std::size_t new_bytes       = 0;
    std::string version_path;
    std::string error;
};

WriteResult AtomicWriteWithVersioning(const std::string& absDir,
                                      const std::string& name,
                                      const std::string& bytes,
                                      std::uint32_t      keepVersions = 10);

struct VersionEntry {
    std::string filename;
    std::string path;
    std::uint64_t ms = 0;
    std::uint64_t bytes = 0;
};

std::vector<VersionEntry> ListVersions(const std::string& absDir,
                                       const std::string& name);

}

#endif
