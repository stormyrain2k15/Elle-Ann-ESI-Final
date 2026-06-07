#include "ElleEmbedding.h"

#include <cctype>
#include <cmath>
#include <cstring>

namespace {

constexpr uint64_t FNV_OFFSET = 0xcbf29ce484222325ULL;
constexpr uint64_t FNV_PRIME  = 0x100000001b3ULL;

inline uint64_t fnv1a(const char* data, size_t len) {
    uint64_t h = FNV_OFFSET;
    for (size_t i = 0; i < len; i++) {
        h ^= (uint8_t)data[i];
        h *= FNV_PRIME;
    }
    return h;
}

inline std::string Normalize(const std::string& in) {
    std::string out;
    out.reserve(in.size() + 2);
    out.push_back(' ');
    bool lastSpace = true;
    for (char ch : in) {
        unsigned char uc = (unsigned char)ch;
        if (std::isalnum(uc)) {
            out.push_back((char)std::tolower(uc));
            lastSpace = false;
        } else if (!lastSpace) {
            out.push_back(' ');
            lastSpace = true;
        }
    }
    if (lastSpace && !out.empty() && out.back() == ' ') {

    } else {
        out.push_back(' ');
    }
    return out;
}

}

namespace ElleEmbeddings {

void Encode(const std::string& text, ElleEmbedding& out) {

    out.fill(0.0f);
    if (text.empty()) return;

    std::string norm = Normalize(text);
    if (norm.size() < 3) return;

    size_t ngrams = 0;
    for (size_t i = 0; i + 3 <= norm.size(); i++) {
        uint64_t h = fnv1a(norm.data() + i, 3);
        size_t   dim  = (size_t)(h % ELLE_EMBEDDING_DIM);

        float    sign = (h >> 63) ? -1.0f : 1.0f;
        out[dim] += sign;
        ngrams++;
    }
    if (ngrams == 0) return;

    float sumSq = 0.0f;
    for (size_t i = 0; i < ELLE_EMBEDDING_DIM; i++) sumSq += out[i] * out[i];
    if (sumSq <= 1e-20f) { out.fill(0.0f); return; }
    float inv = 1.0f / std::sqrt(sumSq);
    for (size_t i = 0; i < ELLE_EMBEDDING_DIM; i++) out[i] *= inv;
}

float Cosine(const ElleEmbedding& a, const ElleEmbedding& b) {
    float dot = 0.0f;

    for (size_t i = 0; i < ELLE_EMBEDDING_DIM; i++) dot += a[i] * b[i];
    return dot;
}

}
