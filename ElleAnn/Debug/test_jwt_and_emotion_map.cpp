#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cctype>

namespace refsha {

static inline uint32_t rotr(uint32_t x, uint32_t n) { return (x >> n) | (x << (32 - n)); }
static const uint32_t K[64] = {
    0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
    0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
    0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
    0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
    0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
    0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
    0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
    0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

void sha256(const uint8_t* data, size_t len, uint8_t out[32]) {
    uint32_t h[8] = {0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,
                    0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19};
    uint64_t bitlen = (uint64_t)len * 8;
    std::vector<uint8_t> buf(data, data + len);
    buf.push_back(0x80);
    while (buf.size() % 64 != 56) buf.push_back(0);
    for (int i = 7; i >= 0; --i) buf.push_back((uint8_t)(bitlen >> (i * 8)));
    for (size_t off = 0; off < buf.size(); off += 64) {
        uint32_t w[64];
        for (int i = 0; i < 16; i++) {
            w[i] = ((uint32_t)buf[off+i*4]   << 24) |
                   ((uint32_t)buf[off+i*4+1] << 16) |
                   ((uint32_t)buf[off+i*4+2] <<  8) |
                   ((uint32_t)buf[off+i*4+3]);
        }
        for (int i = 16; i < 64; i++) {
            uint32_t s0 = rotr(w[i-15],7) ^ rotr(w[i-15],18) ^ (w[i-15] >> 3);
            uint32_t s1 = rotr(w[i-2],17) ^ rotr(w[i-2],19) ^ (w[i-2] >> 10);
            w[i] = w[i-16] + s0 + w[i-7] + s1;
        }
        uint32_t a=h[0],b=h[1],c=h[2],d=h[3],e=h[4],f=h[5],g=h[6],hh=h[7];
        for (int i = 0; i < 64; i++) {
            uint32_t S1 = rotr(e,6) ^ rotr(e,11) ^ rotr(e,25);
            uint32_t ch = (e & f) ^ ((~e) & g);
            uint32_t t1 = hh + S1 + ch + K[i] + w[i];
            uint32_t S0 = rotr(a,2) ^ rotr(a,13) ^ rotr(a,22);
            uint32_t mj = (a & b) ^ (a & c) ^ (b & c);
            uint32_t t2 = S0 + mj;
            hh=g; g=f; f=e; e=d+t1; d=c; c=b; b=a; a=t1+t2;
        }
        h[0]+=a; h[1]+=b; h[2]+=c; h[3]+=d; h[4]+=e; h[5]+=f; h[6]+=g; h[7]+=hh;
    }
    for (int i = 0; i < 8; i++) {
        out[i*4]   = (uint8_t)(h[i] >> 24);
        out[i*4+1] = (uint8_t)(h[i] >> 16);
        out[i*4+2] = (uint8_t)(h[i] >>  8);
        out[i*4+3] = (uint8_t)(h[i]);
    }
}

void hmac_sha256(const uint8_t* key, size_t klen,
                 const uint8_t* msg, size_t mlen,
                 uint8_t out[32]) {
    uint8_t k0[64] = {0};
    if (klen > 64) { sha256(key, klen, k0); }
    else           { std::memcpy(k0, key, klen); }
    uint8_t ipad[64], opad[64];
    for (int i = 0; i < 64; i++) { ipad[i] = k0[i] ^ 0x36; opad[i] = k0[i] ^ 0x5c; }
    std::vector<uint8_t> inner(ipad, ipad + 64);
    inner.insert(inner.end(), msg, msg + mlen);
    uint8_t ih[32];
    sha256(inner.data(), inner.size(), ih);
    std::vector<uint8_t> outer(opad, opad + 64);
    outer.insert(outer.end(), ih, ih + 32);
    sha256(outer.data(), outer.size(), out);
}

}

static const char kB64[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
static std::string b64url(const uint8_t* data, size_t len) {
    std::string out; out.reserve(((len + 2) / 3) * 4);
    size_t i = 0;
    while (i + 3 <= len) {
        uint32_t v = (uint32_t)data[i]<<16 | (uint32_t)data[i+1]<<8 | data[i+2];
        out.push_back(kB64[(v>>18)&0x3F]);
        out.push_back(kB64[(v>>12)&0x3F]);
        out.push_back(kB64[(v>> 6)&0x3F]);
        out.push_back(kB64[ v     &0x3F]);
        i += 3;
    }
    if (i < len) {
        uint32_t v = (uint32_t)data[i] << 16;
        if (i + 1 < len) v |= (uint32_t)data[i+1] << 8;
        out.push_back(kB64[(v>>18)&0x3F]);
        out.push_back(kB64[(v>>12)&0x3F]);
        if (i + 1 < len) out.push_back(kB64[(v>>6)&0x3F]);
    }
    return out;
}

static std::string mint_jwt(const std::string& deviceId,
                            const std::string& deviceName,
                            uint64_t iat, uint64_t exp,
                            const std::string& secret) {
    const std::string header = R"({"alg":"HS256","typ":"JWT"})";
    std::ostringstream ps;
    ps << R"({"sub":")" << deviceId
       << R"(","name":")" << deviceName
       << R"(","iat":)" << iat
       << R"(,"exp":)" << exp << "}";
    const std::string payload = ps.str();
    std::string h64 = b64url((const uint8_t*)header.data(), header.size());
    std::string p64 = b64url((const uint8_t*)payload.data(), payload.size());
    std::string signing = h64 + "." + p64;
    uint8_t mac[32];
    refsha::hmac_sha256((const uint8_t*)secret.data(), secret.size(),
                         (const uint8_t*)signing.data(), signing.size(),
                         mac);
    return signing + "." + b64url(mac, 32);
}

static int g_pass = 0, g_fail = 0;
#define CHECK(cond) do { \
    if (cond) { g_pass++; } \
    else { g_fail++; std::printf("  FAIL at line %d: %s\n", __LINE__, #cond); } \
} while (0)

static void test_hmac_sha256_vector() {

    uint8_t key[20]; std::memset(key, 0x0b, 20);
    const char* msg = "Hi There";
    uint8_t mac[32];
    refsha::hmac_sha256(key, 20, (const uint8_t*)msg, std::strlen(msg), mac);
    static const uint8_t exp[32] = {
        0xb0,0x34,0x4c,0x61,0xd8,0xdb,0x38,0x53,0x5c,0xa8,0xaf,0xce,0xaf,0x0b,0xf1,0x2b,
        0x88,0x1d,0xc2,0x00,0xc9,0x83,0x3d,0xa7,0x26,0xe9,0x37,0x6c,0x2e,0x32,0xcf,0xf7
    };
    CHECK(std::memcmp(mac, exp, 32) == 0);
}

static void test_jwt_roundtrip() {
    std::string tok = mint_jwt("dev-1234", "Pixel 9",
                               1700000000000ull, 1707776000000ull, "shh-secret");

    int dots = 0; for (char c : tok) if (c == '.') dots++;
    CHECK(dots == 2);

    size_t d1 = tok.find('.');
    CHECK(d1 != std::string::npos);
    std::string h = tok.substr(0, d1);

    CHECK(h.size() == 36);

    std::string tok2 = mint_jwt("dev-1234", "Pixel 9",
                                1700000000000ull, 1707776000000ull, "shh-secret");
    CHECK(tok == tok2);

    std::string tok3 = mint_jwt("dev-1234", "Pixel 9",
                                1700000000000ull, 1707776000000ull, "other");
    CHECK(tok != tok3);
}

static void test_emotion_map_lookup() {
    static const char* names[] = {
        "Joy","Sadness","Anger","Fear","Disgust","Surprise","Contempt","Trust",
        "Love","Anticipation","Empathy","Compassion","Hope","Despair"
    };
    const size_t N = sizeof(names) / sizeof(names[0]);
    std::unordered_map<std::string, uint32_t> m;
    m.reserve(N);
    for (size_t i = 0; i < N; i++) {
        std::string key = names[i];
        std::transform(key.begin(), key.end(), key.begin(),
                       [](unsigned char c){ return (char)std::tolower(c); });
        m.emplace(std::move(key), (uint32_t)i);
    }

    CHECK(m.at("joy") == 0);
    CHECK(m.at("empathy") == 10);
    CHECK(m.at("compassion") == 11);

    CHECK(m.find("not-a-real-emotion") == m.end());

    CHECK(m.find("Joy") == m.end());
}

static std::vector<uint8_t> b64url_decode(const std::string& s) {
    static int8_t rev[256];
    static bool init = false;
    if (!init) {
        for (int i = 0; i < 256; i++) rev[i] = -1;
        for (int i = 0; i < 64; i++) rev[(uint8_t)kB64[i]] = (int8_t)i;
        init = true;
    }
    std::vector<uint8_t> out; out.reserve((s.size() * 3) / 4);
    uint32_t buf = 0; int bits = 0;
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

struct JwtV { bool ok=false; std::string sub; uint64_t exp=0; std::string why; };

static JwtV verify_jwt(const std::string& tok, const std::string& secret, uint64_t now) {
    JwtV r;
    if (tok.empty() || secret.empty()) { r.why="empty"; return r; }
    size_t d1 = tok.find('.');
    if (d1 == std::string::npos) { r.why="no dots"; return r; }
    size_t d2 = tok.find('.', d1+1);
    if (d2 == std::string::npos) { r.why="one dot"; return r; }
    if (tok.find('.', d2+1) != std::string::npos) { r.why="too many dots"; return r; }
    std::string h64 = tok.substr(0, d1);
    std::string p64 = tok.substr(d1+1, d2-d1-1);
    std::string s64 = tok.substr(d2+1);
    auto h = b64url_decode(h64);
    if (h.empty()) { r.why="bad header b64"; return r; }
    std::string hs(h.begin(), h.end());

    if (hs.find("\"alg\":\"HS256\"") == std::string::npos) { r.why="bad alg"; return r; }
    uint8_t mac[32];
    std::string signing = h64 + "." + p64;
    refsha::hmac_sha256((const uint8_t*)secret.data(), secret.size(),
                         (const uint8_t*)signing.data(), signing.size(), mac);
    auto sig = b64url_decode(s64);
    if (sig.size() != 32) { r.why="bad sig size"; return r; }
    uint8_t diff = 0;
    for (int i = 0; i < 32; i++) diff |= mac[i] ^ sig[i];
    if (diff != 0) { r.why="sig mismatch"; return r; }
    auto p = b64url_decode(p64);
    if (p.empty()) { r.why="bad payload b64"; return r; }
    std::string ps(p.begin(), p.end());

    auto subStart = ps.find("\"sub\":\"");
    if (subStart == std::string::npos) { r.why="no sub"; return r; }
    subStart += 7;
    auto subEnd = ps.find('"', subStart);
    if (subEnd == std::string::npos) { r.why="bad sub"; return r; }
    r.sub = ps.substr(subStart, subEnd-subStart);
    auto expStart = ps.find("\"exp\":");
    if (expStart == std::string::npos) { r.why="no exp"; return r; }
    expStart += 6;
    uint64_t exp = 0;
    while (expStart < ps.size() && ps[expStart] >= '0' && ps[expStart] <= '9') {
        exp = exp * 10 + (uint64_t)(ps[expStart]-'0');
        expStart++;
    }
    r.exp = exp;
    if (exp <= now) { r.why="expired"; return r; }
    r.ok = true;
    return r;
}

static void test_jwt_verify_paths() {
    const std::string secret = "shh-secret";
    uint64_t now = 1700000000000ull;
    uint64_t exp = now + 5000;
    std::string tok = mint_jwt("dev-1234", "Pixel 9", now, exp, secret);

    auto v = verify_jwt(tok, secret, now);
    CHECK(v.ok);
    CHECK(v.sub == "dev-1234");
    CHECK(v.exp == exp);

    auto v2 = verify_jwt(tok, "other-secret", now);
    CHECK(!v2.ok);

    auto v3 = verify_jwt(tok, secret, exp + 1);
    CHECK(!v3.ok);

    std::string tampered = tok;

    auto d1 = tampered.find('.');
    tampered[d1 + 3] = (tampered[d1 + 3] == 'A') ? 'B' : 'A';
    auto v4 = verify_jwt(tampered, secret, now);
    CHECK(!v4.ok);

    auto v5 = verify_jwt("just-some-bearer-token", secret, now);
    CHECK(!v5.ok);
}

int main() {
    test_hmac_sha256_vector();
    test_jwt_roundtrip();
    test_emotion_map_lookup();
    test_jwt_verify_paths();
    std::printf("passed=%d failed=%d\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
