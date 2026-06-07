#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ElleQR {

enum class Ecc { L = 0, M = 1, Q = 2, H = 3 };

struct Code {
    int               size = 0;
    std::vector<bool> modules;
    bool Get(int x, int y) const {
        return modules[(size_t)y * (size_t)size + (size_t)x];
    }
    void Set(int x, int y, bool v) {
        modules[(size_t)y * (size_t)size + (size_t)x] = v;
    }
};

Code Encode(const std::string& data, Ecc ecc = Ecc::M);

std::string ToSvg(const Code& code, int scale = 6, int quietZone = 4);

}
