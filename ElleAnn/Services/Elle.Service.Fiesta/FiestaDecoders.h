#pragma once
#ifndef ELLE_FIESTA_DECODERS_H
#define ELLE_FIESTA_DECODERS_H

#include "FiestaProtoBase.h"
#include "FiestaPacket.h"

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <string>
#include <string_view>

namespace Fiesta {

struct ChatBroadcast {
    char         sender[17] = {};
    uint8_t      itemLinkDataCount = 0;
    uint8_t      content_len = 0;
    std::string  content;
};

inline bool DecodeChatBroadcast(const uint8_t* payload,
                                std::size_t len,
                                ChatBroadcast& out) {

    if (len < 18) return false;
    std::memcpy(out.sender, payload, 16);
    out.sender[16] = '\0';
    out.itemLinkDataCount = payload[16];
    out.content_len       = payload[17];
    if (len < static_cast<std::size_t>(18) + out.content_len) return false;
    out.content.assign(reinterpret_cast<const char*>(payload + 18),
                       out.content_len);
    return true;
}

inline std::vector<uint8_t>
EncodeChatRequest(std::string_view text, uint8_t itemLinkDataCount = 0) {
    std::vector<uint8_t> buf;

    uint8_t n = static_cast<uint8_t>(text.size() > 0x7F ? 0x7F : text.size());
    buf.reserve(2 + n);
    buf.push_back(itemLinkDataCount);
    buf.push_back(n);
    buf.insert(buf.end(), text.begin(), text.begin() + n);
    return buf;
}

inline std::vector<uint8_t>
EncodeChatRequestEncrypted(class Cipher& cipher,
                           uint16_t opcode_chat_req,
                           std::string_view text,
                           uint8_t itemLinkDataCount = 0);

struct CharBase {
    uint32_t chrregnum     = 0;
    char     charid[17]    = {};
    uint8_t  pad1          = 0;
    uint8_t  marker        = 0;

    const uint8_t* raw_state = nullptr;
    std::size_t    raw_state_len = 0;
};

inline bool DecodeCharBase(const uint8_t* payload,
                           std::size_t len,
                           CharBase& out) {

    if (len < 22) return false;
    std::memcpy(&out.chrregnum, payload, 4);
    std::memcpy(out.charid, payload + 4, 16);
    out.charid[16] = '\0';
    out.pad1   = payload[20];
    out.marker = payload[21];
    if (len > 22) {
        out.raw_state     = payload + 22;
        out.raw_state_len = len - 22;
    }
    return true;
}

struct MoveWalk {
    uint16_t handle    = 0;
    uint32_t fromX     = 0;
    uint32_t fromY     = 0;
    uint32_t toX       = 0;
    uint32_t toY       = 0;
    uint8_t  movetype  = 0;
    uint16_t flags     = 0;
};

inline bool DecodeMoveWalk(const uint8_t* payload,
                           std::size_t len,
                           MoveWalk& out) {
    if (len < 21) return false;
    std::memcpy(&out.handle,   payload + 0,  2);
    std::memcpy(&out.fromX,    payload + 2,  4);
    std::memcpy(&out.fromY,    payload + 6,  4);
    std::memcpy(&out.toX,      payload + 10, 4);
    std::memcpy(&out.toY,      payload + 14, 4);
    out.movetype = payload[18];
    std::memcpy(&out.flags,    payload + 19, 2);
    return true;
}

}

#include "FiestaCipher.h"

namespace Fiesta {

inline std::vector<uint8_t>
EncodeChatRequestEncrypted(Cipher& cipher,
                           uint16_t opcode_chat_req,
                           std::string_view text,
                           uint8_t itemLinkDataCount) {

    auto payload = EncodeChatRequest(text, itemLinkDataCount);

    std::vector<uint8_t> ciphered;
    ciphered.reserve(2 + payload.size());
    ciphered.push_back(static_cast<uint8_t>(opcode_chat_req >> 8));
    ciphered.push_back(static_cast<uint8_t>(opcode_chat_req & 0xFF));
    ciphered.insert(ciphered.end(), payload.begin(), payload.end());

    cipher.EncryptOut(ciphered.data(), ciphered.size());

    std::vector<uint8_t> frame;
    if (ciphered.size() < 0xFF) {
        frame.reserve(1 + ciphered.size());
        frame.push_back(static_cast<uint8_t>(ciphered.size()));
    } else {

        frame.reserve(3 + ciphered.size());
        frame.push_back(0xFF);
        frame.push_back(static_cast<uint8_t>(ciphered.size() & 0xFF));
        frame.push_back(static_cast<uint8_t>(ciphered.size() >> 8));
    }
    frame.insert(frame.end(), ciphered.begin(), ciphered.end());
    return frame;
}

}
#endif
