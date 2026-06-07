#include "ElleEmbedding.h"

#include <cassert>
#include <cmath>
#include <cstdio>
#include <string>

static int g_pass = 0, g_fail = 0;

#define CHECK(cond, msg) do {                                                \
    if (cond) { g_pass++; std::printf("  PASS  %s\n", msg); }                 \
    else       { g_fail++; std::printf("  FAIL  %s\n", msg); }                \
} while (0)

static float cosine_of(const std::string& a, const std::string& b) {
    ElleEmbedding va{}, vb{};
    ElleEmbeddings::Encode(a, va);
    ElleEmbeddings::Encode(b, vb);
    return ElleEmbeddings::Cosine(va, vb);
}

int main() {
    std::printf("ElleEmbedding novelty signal — unit test\n");

    ElleEmbedding v1{}, v2{};
    ElleEmbeddings::Encode("The cat sat on the mat.", v1);
    ElleEmbeddings::Encode("The cat sat on the mat.", v2);
    bool same = true;
    for (size_t i = 0; i < ELLE_EMBEDDING_DIM; i++) if (v1[i] != v2[i]) { same = false; break; }
    CHECK(same, "Encode is deterministic");

    float self = ElleEmbeddings::Cosine(v1, v1);
    CHECK(std::fabs(self - 1.0f) < 1e-5f, "cosine(x, x) == 1.0 for non-empty x");

    ElleEmbedding vz{};
    ElleEmbeddings::Encode("", vz);
    float zs = ElleEmbeddings::Cosine(vz, vz);
    CHECK(zs == 0.0f, "Empty text → zero vector → cosine 0");

    float near1 = cosine_of("I love my cat",       "I love cats");
    float near2 = cosine_of("I love my cat",       "My cat is sleeping");
    float far1  = cosine_of("I love my cat",       "Quantum chromodynamics");
    float far2  = cosine_of("I love my cat",       "The stock market closed up today");
    std::printf("    near1 = %.3f  near2 = %.3f  far1 = %.3f  far2 = %.3f\n",
                near1, near2, far1, far2);
    CHECK(near1 > far1,  "related cat-talk > unrelated physics");
    CHECK(near1 > far2,  "related cat-talk > unrelated finance");
    CHECK(near2 > far1,  "related cat-talk #2 > unrelated physics");

    float disjoint = cosine_of("aaaaaaaaaaaaaaaaaaaa",
                               "The quick brown fox jumps over the lazy dog");
    CHECK(std::fabs(disjoint) < 0.4f, "disjoint content → low cosine magnitude");

    std::printf("\nResults: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
