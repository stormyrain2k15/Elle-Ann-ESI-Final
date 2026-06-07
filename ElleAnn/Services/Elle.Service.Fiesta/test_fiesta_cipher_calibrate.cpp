#include "FiestaProtoBase.h"
#include "FiestaCipher.h"

#include <cstdio>
#include <cstdint>
#include <cstring>
#include <vector>

static const uint8_t kEncrypted[] = { 0x51, 0x01, 0xE1, 0x36, 0xCB, 0x3A, 0x8B };
static const uint8_t kPlaintext[] = { 0x0C, 0x01, 0xD6, 0x07, 0x04, 0x0A, 0x00 };
static_assert(sizeof(kEncrypted) == sizeof(kPlaintext), "");
static constexpr std::size_t kLen = sizeof(kEncrypted);

static const uint8_t kExpectedKeystream[kLen] = {
    0x5D, 0x00, 0x37, 0x31, 0xCF, 0x30, 0x8B
};

static bool TryCipher(Fiesta::CipherKind kind, uint16_t seed,
                      uint8_t out_keystream[kLen]) {
    Fiesta::Cipher c;
    c.SetKind(kind);
    c.Reset(seed);

    uint8_t buf[kLen] = {0};
    c.EncryptOut(buf, kLen);

    std::memcpy(out_keystream, buf, kLen);
    return std::memcmp(buf, kExpectedKeystream, kLen) == 0;
}

int main() {
    std::printf("──── Phase 6c step 0 — cipher calibration ────\n");
    std::printf("Wire enc bytes : ");
    for (auto b : kEncrypted) { std::printf("%02X ", b); }
    std::printf("\n");
    std::printf("Plaintext      : ");
    for (auto b : kPlaintext) { std::printf("%02X ", b); }
    std::printf("\n");
    std::printf("Expected mask  : ");
    for (auto b : kExpectedKeystream) { std::printf("%02X ", b); }
    std::printf("\n\n");

    int total_matches = 0;

    std::printf("Scanning LCG seeds 0..0xFFFF...\n");
    for (uint32_t seed = 0; seed <= 0xFFFF; ++seed) {
        uint8_t ks[kLen];
        if (TryCipher(Fiesta::CipherKind::LCG, (uint16_t)seed, ks)) {
            std::printf("  ★ MATCH  LCG    seed=0x%04X\n", seed);
            ++total_matches;
        }
    }

    std::printf("Scanning XOR499 table positions 0..498...\n");
    Fiesta::Cipher c;
    c.SetKind(Fiesta::CipherKind::XOR499);
    for (uint16_t pos = 0; pos < 499; ++pos) {
        c.Reset(0);

        uint8_t pad[499] = {0};
        if (pos > 0) c.EncryptOut(pad, pos);

        uint8_t buf[kLen] = {0};
        c.EncryptOut(buf, kLen);
        if (std::memcmp(buf, kExpectedKeystream, kLen) == 0) {
            std::printf("  ★ MATCH  XOR499 starting position = %u (0x%X)\n",
                        pos, pos);
            ++total_matches;
        }
    }

    std::printf("\nDiagnostic: first 8 keystream bytes per cipher (seed=0):\n");
    for (auto kind : {Fiesta::CipherKind::LCG, Fiesta::CipherKind::XOR499}) {
        Fiesta::Cipher c;
        c.SetKind(kind);
        c.Reset(0);
        uint8_t buf[8] = {0};
        c.EncryptOut(buf, 8);
        std::printf("  %s : ",
                    kind == Fiesta::CipherKind::LCG ? "LCG   " : "XOR499");
        for (auto b : buf) { std::printf("%02X ", b); }
        std::printf("\n");
    }

    std::printf("\nResult: %d cipher/seed combinations match the rosetta stone.\n",
                total_matches);
    if (total_matches == 0) {
        std::printf("→ Neither in-tree cipher matches.  The user's server\n"
                    "  uses a different family — see\n"
                    "  _re_artifacts/cipher/README.md for IDA hunt steps.\n");
        return 1;
    }
    return 0;
}
