#include "/app/ElleAnn/Services/Elle.Service.Fiesta/FiestaCipher.h"
#include <cassert>
#include <cstdio>
#include <vector>
#include <cstring>

int main() {
    using namespace Fiesta;
    int fails = 0;

    {
        Cipher enc, dec;
        enc.SetKind(CipherKind::LCG); dec.SetKind(CipherKind::LCG);
        enc.Reset(0x1234); dec.Reset(0x1234);
        std::vector<uint8_t> a(64); for (auto& b : a) b = 0x42;
        auto orig = a;
        enc.EncryptOut(a.data(), a.size());
        if (a == orig) { fails++; std::fprintf(stderr, "T1: LCG didn't mutate\n"); }
        dec.DecryptIn(a.data(), a.size());
        if (a != orig) { fails++; std::fprintf(stderr, "T1: LCG roundtrip\n"); }
    }

    {
        Cipher enc, dec;
        enc.SetKind(CipherKind::XOR499); dec.SetKind(CipherKind::XOR499);
        enc.Reset(0); dec.Reset(0);
        const char* msg = "DragonFiesta-Rewrite session payload";
        std::vector<uint8_t> a(msg, msg + std::strlen(msg));
        auto orig = a;
        enc.EncryptOut(a.data(), a.size());
        if (a == orig) { fails++; std::fprintf(stderr, "T2: XOR499 didn't mutate\n"); }
        dec.DecryptIn(a.data(), a.size());
        if (a != orig) { fails++; std::fprintf(stderr, "T2: XOR499 roundtrip\n"); }
    }

    {
        Cipher enc;
        enc.SetKind(CipherKind::XOR499);
        enc.Reset(0);
        std::vector<uint8_t> a = {0x00};
        enc.EncryptOut(a.data(), 1);
        if (a[0] != 0x07) { fails++; std::fprintf(stderr,
                            "T3: first byte != 0x07 (got 0x%02X)\n", a[0]); }
    }

    {
        Cipher c;
        c.SetKind(CipherKind::LCG); c.Reset(0x77);
        std::vector<uint8_t> a(8, 0); c.EncryptOut(a.data(), a.size());
        c.SetKind(CipherKind::XOR499); c.Reset(0);
        std::vector<uint8_t> b = {0x00};
        c.EncryptOut(b.data(), 1);
        if (b[0] != 0x07) { fails++; std::fprintf(stderr,
            "T4: switch+Reset(0) → first XOR499 byte != 0x07\n"); }
    }

    if (fails == 0) std::printf("ALL CIPHER TESTS PASS\n");
    else            std::printf("%d test(s) FAILED\n", fails);
    return fails;
}
