#include "ElleQR.h"

#include <array>
#include <cstring>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace ElleQR {
namespace {

static const int kByteCapacity[10][4] = {

      { 17,  14,  11,   7 },
      { 32,  26,  20,  14 },
      { 53,  42,  32,  24 },
      { 78,  62,  46,  34 },
      {106,  84,  60,  44 },
      {134, 106,  74,  58 },
      {154, 122,  86,  64 },
      {192, 152, 108,  84 },
      {230, 180, 130,  98 },
     {271, 213, 151, 119 }
};

static const int kRawCodewords[10] = {
    26, 44, 70, 100, 134, 172, 196, 242, 292, 346
};

struct EccCfg { int g1_blocks; int g1_dataCw; int g2_blocks; int g2_dataCw; int eccCw; };
static const EccCfg kEcc[10][4] = {
     {{1,19,0,0,7 },{1,16,0,0,10},{1,13,0,0,13},{1, 9,0,0,17}},
     {{1,34,0,0,10},{1,28,0,0,16},{1,22,0,0,22},{1,16,0,0,28}},
     {{1,55,0,0,15},{1,44,0,0,26},{2,17,0,0,18},{2,13,0,0,22}},
     {{1,80,0,0,20},{2,32,0,0,18},{2,24,0,0,26},{4, 9,0,0,16}},
     {{1,108,0,0,26},{2,43,0,0,24},{2,15,2,16,18},{2,11,2,12,22}},
     {{2,68,0,0,18},{4,27,0,0,16},{4,19,0,0,24},{4,15,0,0,28}},
     {{2,78,0,0,20},{4,31,0,0,18},{2,14,4,15,18},{4,13,1,14,26}},
     {{2,97,0,0,24},{2,38,2,39,22},{4,18,2,19,22},{4,14,2,15,26}},
     {{2,116,0,0,30},{3,36,2,37,22},{4,16,4,17,20},{4,12,4,13,24}},
     {{2,68,2,69,18},{4,43,1,44,26},{6,19,2,20,24},{6,15,2,16,28}}
};

static uint8_t gfExp[512];
static uint8_t gfLog[256];
static bool    gfInit = false;
static void initGf() {
    if (gfInit) return;
    int x = 1;
    for (int i = 0; i < 255; i++) {
        gfExp[i] = (uint8_t)x;
        gfLog[x] = (uint8_t)i;
        x <<= 1;
        if (x & 0x100) x ^= 0x11D;
    }
    for (int i = 255; i < 512; i++) gfExp[i] = gfExp[i - 255];
    gfInit = true;
}
static uint8_t gfMul(uint8_t a, uint8_t b) {
    if (a == 0 || b == 0) return 0;
    return gfExp[gfLog[a] + gfLog[b]];
}

static std::vector<uint8_t> rsGenerator(int degree) {
    initGf();
    std::vector<uint8_t> g(1, 1);
    for (int i = 0; i < degree; i++) {
        std::vector<uint8_t> n(g.size() + 1, 0);
        for (size_t j = 0; j < g.size(); j++) {
            n[j]     ^= gfMul(g[j], 1);
            n[j + 1] ^= gfMul(g[j], gfExp[i]);
        }
        g = std::move(n);
    }
    return g;
}

static std::vector<uint8_t> rsCompute(const std::vector<uint8_t>& data, int eccLen) {
    auto gen = rsGenerator(eccLen);
    std::vector<uint8_t> out((size_t)eccLen, 0);
    for (uint8_t d : data) {
        uint8_t factor = d ^ out[0];
        out.erase(out.begin());
        out.push_back(0);
        for (size_t j = 0; j < out.size(); j++) {
            out[j] ^= gfMul(gen[j + 1], factor);
        }
    }
    return out;
}

struct BitBuf {
    std::vector<bool> bits;
    void append(uint32_t value, int numBits) {
        for (int i = numBits - 1; i >= 0; i--) {
            bits.push_back(((value >> i) & 1) != 0);
        }
    }
};

static std::vector<int> alignPositions(int version) {
    if (version == 1) return {};
    int n;
    switch (version) {
        case  2: case  3: case  4: case  5: case  6:
            n = 2;
            break;
        case 7: case 8: case 9: case 10:
            n = 3;
            break;
        default:
            return {};
    }

    static const int t[11][3] = {
         { 0, 0, 0 },
         { 6,18, 0 },
         { 6,22, 0 },
         { 6,26, 0 },
         { 6,30, 0 },
         { 6,34, 0 },
         { 6,22,38 },
         { 6,24,42 },
         { 6,26,46 },
         { 6,28,50 },
        { 0, 0, 0 }
    };
    std::vector<int> out;
    for (int i = 0; i < n; i++) out.push_back(t[version - 1][i]);
    return out;
}

class Matrix {
public:
    int size;
    std::vector<uint8_t> mod;
    std::vector<uint8_t> res;
    Matrix(int s) : size(s), mod((size_t)s*s,0), res((size_t)s*s,0) {}
    void set(int x, int y, bool v, bool reserved=false) {
        mod[(size_t)y*size + (size_t)x] = v ? 1 : 0;
        if (reserved) res[(size_t)y*size + (size_t)x] = 1;
    }
    bool get(int x, int y) const { return mod[(size_t)y*size + (size_t)x] != 0; }
    bool reserved(int x, int y) const { return res[(size_t)y*size + (size_t)x] != 0; }
};

static void drawFinder(Matrix& m, int cx, int cy) {
    for (int dy = -4; dy <= 4; dy++) {
        for (int dx = -4; dx <= 4; dx++) {
            int x = cx + dx, y = cy + dy;
            if (x < 0 || x >= m.size || y < 0 || y >= m.size) continue;
            int ad = std::max(std::abs(dx), std::abs(dy));
            bool dark = (ad != 2 && ad <= 3);
            m.set(x, y, dark, true);
        }
    }
}
static void drawAlignment(Matrix& m, int cx, int cy) {
    for (int dy = -2; dy <= 2; dy++) {
        for (int dx = -2; dx <= 2; dx++) {
            int ad = std::max(std::abs(dx), std::abs(dy));
            bool dark = (ad != 1);
            m.set(cx + dx, cy + dy, dark, true);
        }
    }
}

static const uint16_t kFormatBits[4][8] = {
     {0x77C4,0x72F3,0x7DAA,0x789D,0x662F,0x6318,0x6C41,0x6976},
     {0x5412,0x5125,0x5E7C,0x5B4B,0x45F9,0x40CE,0x4F97,0x4AA0},
     {0x355F,0x3068,0x3F31,0x3A06,0x24B4,0x2183,0x2EDA,0x2BED},
     {0x1689,0x13BE,0x1CE7,0x19D0,0x0762,0x0255,0x0D0C,0x083B}
};

static const uint32_t kVersionBits[4] = {
     0x07C94,  0x085BC,  0x09A99,  0x0A4D3
};

static void drawFunctionPatterns(Matrix& m, int version) {
    int s = m.size;

    drawFinder(m, 3, 3);
    drawFinder(m, s - 4, 3);
    drawFinder(m, 3, s - 4);

    for (int i = 0; i < 8; i++) {
        if (i < s) {
            m.set(7, i, false, true); m.set(i, 7, false, true);
            m.set(s - 8, i, false, true); m.set(s - 1 - i, 7, false, true);
            m.set(7, s - 1 - i, false, true); m.set(i, s - 8, false, true);
        }
    }

    for (int i = 8; i < s - 8; i++) {
        bool dark = (i % 2 == 0);
        m.set(6, i, dark, true);
        m.set(i, 6, dark, true);
    }

    auto pos = alignPositions(version);
    for (size_t i = 0; i < pos.size(); i++) {
        for (size_t j = 0; j < pos.size(); j++) {
            int cx = pos[j], cy = pos[i];

            if ((cx == 6 && cy == 6) ||
                (cx == 6 && cy == s - 7) ||
                (cx == s - 7 && cy == 6)) continue;
            drawAlignment(m, cx, cy);
        }
    }

    m.set(8, 4 * version + 9, true, true);

    for (int i = 0; i < 9; i++) m.set(8, i, false, true);
    for (int i = 0; i < 8; i++) m.set(i, 8, false, true);
    for (int i = 0; i < 7; i++) m.set(8, s - 1 - i, false, true);
    for (int i = 0; i < 8; i++) m.set(s - 1 - i, 8, false, true);

    if (version >= 7) {
        for (int y = 0; y < 6; y++)
            for (int x = 0; x < 3; x++) {
                m.set(s - 11 + x, y, false, true);
                m.set(y, s - 11 + x, false, true);
            }
    }
}

static void drawFormat(Matrix& m, Ecc ecc, int mask) {
    uint16_t bits = kFormatBits[(int)ecc][mask];
    int s = m.size;

    for (int i = 0; i <= 5; i++) m.set(8, i, ((bits >> i) & 1) != 0);
    m.set(8, 7, ((bits >> 6) & 1) != 0);
    m.set(8, 8, ((bits >> 7) & 1) != 0);
    m.set(7, 8, ((bits >> 8) & 1) != 0);
    for (int i = 9; i < 15; i++) m.set(14 - i, 8, ((bits >> i) & 1) != 0);

    for (int i = 0; i < 8; i++)  m.set(s - 1 - i, 8, ((bits >> i) & 1) != 0);
    for (int i = 8; i < 15; i++) m.set(8, s - 15 + i, ((bits >> i) & 1) != 0);
    m.set(8, s - 8, true);
}

static void drawVersion(Matrix& m, int version) {
    if (version < 7) return;
    uint32_t bits = kVersionBits[version - 7];
    int s = m.size;
    for (int i = 0; i < 18; i++) {
        bool b = ((bits >> i) & 1) != 0;
        int a = i / 3, c = i % 3;
        m.set(s - 11 + c, a, b);
        m.set(a, s - 11 + c, b);
    }
}

static void drawData(Matrix& m, const std::vector<uint8_t>& codewords) {
    int s = m.size;
    size_t bitIdx = 0;
    int totalBits = (int)codewords.size() * 8;
    int x = s - 1;
    while (x > 0) {
        if (x == 6) x--;
        for (int step = 0; step < s; step++) {
            for (int dx = 0; dx < 2; dx++) {
                int cx = x - dx;

                bool upward = (((x + 1) & 2) == 0);
                int cy = upward ? (s - 1 - step) : step;
                if (m.reserved(cx, cy)) continue;
                bool bit = false;
                if ((int)bitIdx < totalBits) {
                    uint8_t byte = codewords[bitIdx / 8];
                    bit = ((byte >> (7 - (bitIdx % 8))) & 1) != 0;
                    bitIdx++;
                }
                m.set(cx, cy, bit);
            }
        }
        x -= 2;
    }
}

static bool maskFn(int mask, int x, int y) {
    switch (mask) {
        case 0: return (x + y) % 2 == 0;
        case 1: return y % 2 == 0;
        case 2: return x % 3 == 0;
        case 3: return (x + y) % 3 == 0;
        case 4: return (y / 2 + x / 3) % 2 == 0;
        case 5: return (x * y) % 2 + (x * y) % 3 == 0;
        case 6: return ((x * y) % 2 + (x * y) % 3) % 2 == 0;
        case 7: return ((x + y) % 2 + (x * y) % 3) % 2 == 0;
    }
    return false;
}

static void applyMask(Matrix& m, int mask) {
    for (int y = 0; y < m.size; y++) {
        for (int x = 0; x < m.size; x++) {
            if (m.reserved(x, y)) continue;
            if (maskFn(mask, x, y)) {
                m.set(x, y, !m.get(x, y));
            }
        }
    }
}

static int penalty(const Matrix& m) {
    int s = m.size;
    int p = 0;

    for (int y = 0; y < s; y++) {
        int run = 1; bool cur = m.get(0, y);
        for (int x = 1; x < s; x++) {
            bool b = m.get(x, y);
            if (b == cur) { run++; if (run == 5) p += 3; else if (run > 5) p += 1; }
            else          { run = 1; cur = b; }
        }
    }
    for (int x = 0; x < s; x++) {
        int run = 1; bool cur = m.get(x, 0);
        for (int y = 1; y < s; y++) {
            bool b = m.get(x, y);
            if (b == cur) { run++; if (run == 5) p += 3; else if (run > 5) p += 1; }
            else          { run = 1; cur = b; }
        }
    }

    for (int y = 0; y < s - 1; y++) {
        for (int x = 0; x < s - 1; x++) {
            bool c = m.get(x, y);
            if (c == m.get(x + 1, y) && c == m.get(x, y + 1) && c == m.get(x + 1, y + 1))
                p += 3;
        }
    }

    auto match = [&](int x, int y, int dx, int dy) {

        const int pat[5] = {1,1,3,1,1};

        int total = 11;
        int endx = x + dx * (total - 1), endy = y + dy * (total - 1);
        if (endx < 0 || endx >= s || endy < 0 || endy >= s) return false;

        int idx = 0;
        for (int seg = 0; seg < 5; seg++) {
            bool dark = (seg % 2 == 0);
            for (int k = 0; k < pat[seg]; k++) {
                int cx = x + dx * idx;
                int cy = y + dy * idx;
                if (m.get(cx, cy) != dark) return false;
                idx++;
            }
        }

        for (int k = 0; k < 4; k++) {
            int cx = x + dx * idx;
            int cy = y + dy * idx;
            if (m.get(cx, cy)) return false;
            idx++;
        }
        return true;
    };
    for (int y = 0; y < s; y++)
        for (int x = 0; x < s; x++) {
            if (match(x, y, 1, 0)) p += 40;
            if (match(x, y, 0, 1)) p += 40;
        }

    int dark = 0;
    for (int i = 0; i < s * s; i++) if (m.mod[i]) dark++;
    int pct = (dark * 100) / (s * s);
    int k = 0;
    while (std::abs(pct - 50) > 5 * (k + 1)) k++;
    p += k * 10;
    return p;
}

static Matrix copyMatrix(const Matrix& src) { return src; }

}

Code Encode(const std::string& data, Ecc ecc) {

    int bytes = (int)data.size();
    int version = 0;
    for (int v = 1; v <= 10; v++) {
        if (bytes <= kByteCapacity[v - 1][(int)ecc]) { version = v; break; }
    }
    if (version == 0) return {};

    int size = 17 + 4 * version;
    int totalDataCw = kRawCodewords[version - 1];
    const EccCfg& cfg = kEcc[version - 1][(int)ecc];
    int totalBlocks = cfg.g1_blocks + cfg.g2_blocks;
    int dataCwTotal = cfg.g1_blocks * cfg.g1_dataCw + cfg.g2_blocks * cfg.g2_dataCw;

    BitBuf bb;
    bb.append(0b0100, 4);
    int cciBits = (version <= 9) ? 8 : 16;
    bb.append((uint32_t)bytes, cciBits);
    for (int i = 0; i < bytes; i++) bb.append((uint8_t)data[i], 8);
    int cap = dataCwTotal * 8;

    int term = std::min(4, cap - (int)bb.bits.size());
    if (term > 0) bb.append(0, term);

    while (bb.bits.size() % 8 != 0) bb.bits.push_back(false);

    uint8_t pad[2] = {0xEC, 0x11};
    int padIdx = 0;
    while ((int)bb.bits.size() < cap) {
        bb.append(pad[padIdx], 8);
        padIdx = 1 - padIdx;
    }

    std::vector<uint8_t> raw(dataCwTotal, 0);
    for (size_t i = 0; i < bb.bits.size(); i++) {
        if (bb.bits[i]) raw[i / 8] |= (uint8_t)(1 << (7 - (i % 8)));
    }

    std::vector<std::vector<uint8_t>> dataBlocks, eccBlocks;
    int off = 0;
    for (int b = 0; b < totalBlocks; b++) {
        int thisDataCw = (b < cfg.g1_blocks) ? cfg.g1_dataCw : cfg.g2_dataCw;
        std::vector<uint8_t> db(raw.begin() + off, raw.begin() + off + thisDataCw);
        off += thisDataCw;
        dataBlocks.push_back(db);
        eccBlocks.push_back(rsCompute(db, cfg.eccCw));
    }

    std::vector<uint8_t> interleaved;
    interleaved.reserve((size_t)totalDataCw);
    int maxDataCw = std::max(cfg.g1_dataCw,
                             cfg.g2_blocks > 0 ? cfg.g2_dataCw : cfg.g1_dataCw);
    for (int i = 0; i < maxDataCw; i++) {
        for (int b = 0; b < totalBlocks; b++) {
            if (i < (int)dataBlocks[b].size()) interleaved.push_back(dataBlocks[b][i]);
        }
    }
    for (int i = 0; i < cfg.eccCw; i++) {
        for (int b = 0; b < totalBlocks; b++) {
            interleaved.push_back(eccBlocks[b][i]);
        }
    }

    Matrix base(size);
    drawFunctionPatterns(base, version);
    if (version >= 7) drawVersion(base, version);
    drawData(base, interleaved);

    int bestMask = 0;
    int bestPenalty = INT32_MAX;
    Matrix bestM(size);
    for (int mask = 0; mask < 8; mask++) {
        Matrix m = copyMatrix(base);
        applyMask(m, mask);
        drawFormat(m, ecc, mask);
        int p = penalty(m);
        if (p < bestPenalty) {
            bestPenalty = p;
            bestMask = mask;
            bestM = m;
        }
    }
    (void)bestMask;

    Code c;
    c.size = size;
    c.modules.resize((size_t)size * size);
    for (int i = 0; i < size * size; i++) c.modules[i] = bestM.mod[i] != 0;
    return c;
}

std::string ToSvg(const Code& code, int scale, int quietZone) {
    if (code.size == 0) return {};
    if (scale < 1)     scale = 1;
    if (quietZone < 0) quietZone = 0;
    int total = (code.size + quietZone * 2) * scale;

    std::ostringstream s;

    s << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
      << "<svg xmlns=\"http://www.w3.org/2000/svg\""
         " viewBox=\"0 0 "
      << total << " " << total << "\" width=\"" << total
      << "\" height=\"" << total
      << "\" shape-rendering=\"crispEdges\">"
      << "<rect width=\"100%\" height=\"100%\" fill=\"#ffffff\"/>"
      << "<path fill=\"#000000\" d=\"";
    for (int y = 0; y < code.size; y++) {

        int x = 0;
        while (x < code.size) {
            if (!code.Get(x, y)) { x++; continue; }
            int start = x;
            while (x < code.size && code.Get(x, y)) x++;
            int len = x - start;
            int px = (start + quietZone) * scale;
            int py = (y + quietZone) * scale;
            s << "M" << px << "," << py
              << "h" << (len * scale)
              << "v" << scale
              << "h-" << (len * scale) << "z";
        }
    }
    s << "\"/></svg>";
    return s.str();
}

}
