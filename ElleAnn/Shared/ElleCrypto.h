#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ElleCrypto {

bool Sha256(const void* data, size_t len, uint8_t out[32]);

std::string Sha256Hex(const std::string& data);

bool HmacSha256(const void* key, size_t keyLen,
                const void* data, size_t dataLen,
                uint8_t out[32]);

bool RandomBytes(void* out, size_t len);

std::string RandomDigits(uint32_t digits);

std::string RandomUrlToken(size_t bytes);

std::string RandomHex(size_t bytes);

bool ConstantTimeEquals(const void* a, const void* b, size_t len);

std::string Base64UrlEncode(const void* data, size_t len);

std::vector<uint8_t> Base64UrlDecode(const std::string& s);

}
