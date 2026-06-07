#pragma once

#include <array>
#include <cstdint>
#include <string>

constexpr size_t ELLE_EMBEDDING_DIM = 256;

using ElleEmbedding = std::array<float, ELLE_EMBEDDING_DIM>;

namespace ElleEmbeddings {

    void Encode(const std::string& text, ElleEmbedding& out);

    [[nodiscard]] float Cosine(const ElleEmbedding& a, const ElleEmbedding& b);

}
