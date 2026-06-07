#include "FiestaProtoBase.h"
#include "FiestaPacket.h"
#include "FiestaDecoders.h"

#include <cassert>
#include <cstdio>
#include <cstdint>
#include <cstring>

static const uint8_t kChat_EllaAnn_hi[] = {

    0x45, 0x6C, 0x6C, 0x65, 0x41, 0x6E, 0x6E,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

    0x00, 0x02, 0x68, 0x69
};

static const uint8_t kChat_Crystal_hi[] = {
    0x43, 0x72, 0x79, 0x73, 0x74, 0x61, 0x6C,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x02, 0x68, 0x69
};

static const uint8_t kCharBase_EllaAnn[] = {

    0x05, 0x00, 0x00, 0x00,

    0x45, 0x6C, 0x6C, 0x65, 0x41, 0x6E, 0x6E,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

    0x00, 0x96,

    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

    0x0E, 0x27, 0x09, 0x27, 0xC5, 0x0F, 0x00, 0x00, 0xD1, 0x0D, 0x00, 0x00,
    0x44, 0x2E, 0x00, 0x00, 0xFF, 0xE7, 0x76, 0x48, 0x17, 0x00, 0x00, 0x00,
    0x55, 0x72, 0x67, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x0D, 0x1A, 0x00, 0x00, 0xD2, 0x17, 0x00, 0x00, 0x00, 0x00, 0x4D, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x64, 0x00,
    0x00
};
static_assert(sizeof(kCharBase_EllaAnn) == 97, "captured wire len = 97 B");

static const uint8_t kMoveWalk_C446[] = {
    0xC4, 0x46,
    0x8B, 0x15, 0x00, 0x00,
    0x2A, 0x1D, 0x00, 0x00,
    0xC4, 0x15, 0x00, 0x00,
    0x4C, 0x1D, 0x00, 0x00,
    0x32,
    0x00, 0x00
};
static_assert(sizeof(kMoveWalk_C446) == 21, "captured wire len = 21 B");

static int failed = 0;

#define EXPECT(cond, msg) do {                                 \
    if (!(cond)) {                                             \
        std::printf("FAIL  %s:%d  %s\n", __FILE__, __LINE__, msg); \
        ++failed;                                              \
    }                                                          \
} while (0)

static void TestChatEllaAnn() {
    Fiesta::ChatBroadcast cb{};
    bool ok = Fiesta::DecodeChatBroadcast(kChat_EllaAnn_hi,
                                          sizeof(kChat_EllaAnn_hi), cb);
    EXPECT(ok, "DecodeChatBroadcast(EllaAnn) returned false");
    EXPECT(std::strcmp(cb.sender, "ElleAnn") == 0,
           "sender mismatch — expected 'ElleAnn'");
    EXPECT(cb.itemLinkDataCount == 0, "itemLinkDataCount must be 0");
    EXPECT(cb.content_len       == 2, "content_len must be 2");
    EXPECT(cb.content == "hi",        "content must be 'hi'");
    std::printf("PASS  Chat[EllaAnn]: sender=\"%s\" content=\"%s\" (%u B)\n",
                cb.sender, cb.content.c_str(), cb.content_len);
}

static void TestChatCrystal() {
    Fiesta::ChatBroadcast cb{};
    bool ok = Fiesta::DecodeChatBroadcast(kChat_Crystal_hi,
                                          sizeof(kChat_Crystal_hi), cb);
    EXPECT(ok, "DecodeChatBroadcast(Crystal) returned false");
    EXPECT(std::strcmp(cb.sender, "Crystal") == 0,
           "sender mismatch — expected 'Crystal'");
    EXPECT(cb.content == "hi", "content must be 'hi'");
    std::printf("PASS  Chat[Crystal]: sender=\"%s\" content=\"%s\"\n",
                cb.sender, cb.content.c_str());
}

static void TestChatTruncated() {

    Fiesta::ChatBroadcast cb{};
    bool ok = Fiesta::DecodeChatBroadcast(kChat_EllaAnn_hi, 10, cb);
    EXPECT(!ok, "Truncated buffer must return false");
    std::printf("PASS  Chat[truncated]: refused (returned false)\n");
}

static void TestEncodeChatRequest() {
    auto payload = Fiesta::EncodeChatRequest("hello world");
    EXPECT(payload.size() == 13, "encoded size must be 2+11 = 13");
    EXPECT(payload[0] == 0,      "itemLinkDataCount must be 0");
    EXPECT(payload[1] == 11,     "len must be 11");
    EXPECT(std::memcmp(payload.data() + 2, "hello world", 11) == 0,
           "content must be 'hello world'");
    std::printf("PASS  EncodeChatRequest('hello world'): %zu bytes [%02X %02X ...]\n",
                payload.size(), payload[0], payload[1]);
}

static void TestEncodeChatLong() {

    std::string big(0x100, 'x');
    auto payload = Fiesta::EncodeChatRequest(big);
    EXPECT(payload.size() == 2 + 0x7F, "long content must clamp to 0x7F");
    EXPECT(payload[1] == 0x7F,         "len byte must be 0x7F");
    std::printf("PASS  EncodeChatRequest(0x100 chars): clamped to %zu (0x%02X)\n",
                payload.size(), payload[1]);
}

static void TestCharBaseEllaAnn() {
    Fiesta::CharBase cb{};
    bool ok = Fiesta::DecodeCharBase(kCharBase_EllaAnn,
                                     sizeof(kCharBase_EllaAnn), cb);
    EXPECT(ok, "DecodeCharBase returned false");
    EXPECT(cb.chrregnum == 5, "chrregnum must be 5 (EllaAnn's userNo)");
    EXPECT(std::strcmp(cb.charid, "ElleAnn") == 0,
           "charid mismatch — expected 'ElleAnn'");
    EXPECT(cb.pad1   == 0x00, "pad1 must be 0x00");
    EXPECT(cb.marker == 0x96, "marker must be 0x96 (build constant)");
    EXPECT(cb.raw_state_len == 75,
           "raw_state should hold 75 B of post-marker body");
    std::printf("PASS  CharBase: chrregnum=%u charid=\"%s\" "
                "marker=0x%02X raw_state=%zu B\n",
                cb.chrregnum, cb.charid, cb.marker, cb.raw_state_len);
}

static void TestMoveWalkC446() {
    Fiesta::MoveWalk mw{};
    bool ok = Fiesta::DecodeMoveWalk(kMoveWalk_C446,
                                     sizeof(kMoveWalk_C446), mw);
    EXPECT(ok, "DecodeMoveWalk returned false");
    EXPECT(mw.handle == 0x46C4, "handle must be 0x46C4");
    EXPECT(mw.fromX  == 5515,   "fromX must be 5515 (0x158B)");
    EXPECT(mw.fromY  == 7466,   "fromY must be 7466 (0x1D2A)");
    EXPECT(mw.toX    == 5572,   "toX must be 5572 (0x15C4)");
    EXPECT(mw.toY    == 7500,   "toY must be 7500 (0x1D4C)");
    EXPECT(mw.movetype == 0x32, "movetype must be 0x32 (walk)");
    EXPECT(mw.flags  == 0,      "flags must be 0");
    std::printf("PASS  MoveWalk: handle=0x%04X (%u,%u)→(%u,%u) type=0x%02X\n",
                mw.handle, mw.fromX, mw.fromY, mw.toX, mw.toY, mw.movetype);
}

static void TestEncodeChatRequestEncrypted() {

    Fiesta::Cipher tx;
    tx.SetKind(Fiesta::CipherKind::XOR499);
    tx.Reset(0);

    auto frame = Fiesta::EncodeChatRequestEncrypted(
        tx, 0x0801, "hi", 0);

    EXPECT(frame.size() == 7, "frame must be 1 (len-prefix) + 6 (cipher region)");
    EXPECT(frame[0] == 6,     "length prefix byte must equal cipher region size");

    Fiesta::Cipher rx;
    rx.SetKind(Fiesta::CipherKind::XOR499);
    rx.Reset(0);

    std::vector<uint8_t> body(frame.begin() + 1, frame.end());
    rx.EncryptOut(body.data(), body.size());

    EXPECT(body[0] == 0x08,   "decrypted opcode high must be 0x08");
    EXPECT(body[1] == 0x01,   "decrypted opcode low  must be 0x01");
    EXPECT(body[2] == 0x00,   "itemLinkDataCount must be 0");
    EXPECT(body[3] == 0x02,   "len byte must be 2");
    EXPECT(body[4] == 'h' && body[5] == 'i',
           "decrypted content must be 'hi'");
    std::printf("PASS  EncodeChatRequestEncrypted round-trip: "
                "frame[%zu B] opcode=0x%02X%02X content='%c%c'\n",
                frame.size(), body[0], body[1], body[4], body[5]);
}

int main() {
    std::printf("──── Phase 6a Step 3 decoder tests ────\n");
    TestChatEllaAnn();
    TestChatCrystal();
    TestChatTruncated();
    TestEncodeChatRequest();
    TestEncodeChatLong();
    TestCharBaseEllaAnn();
    TestMoveWalkC446();
    TestEncodeChatRequestEncrypted();
    std::printf("──── Result: %s (%d failure%s) ────\n",
                failed == 0 ? "ALL PASS" : "FAILURE",
                failed, failed == 1 ? "" : "s");
    return failed == 0 ? 0 : 1;
}
