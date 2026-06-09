#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace ElleUpload {

enum class DetectedContent : int {
    UNKNOWN = 0,
    EMPTY,
    JPEG,
    PNG,
    GIF,
    BMP,
    WEBP,
    PDF,
    ZIP,
    RAR,
    SEVENZIP,
    GZIP,
    TAR,
    MP3,
    OGG,
    WAV,
    FLAC,
    MP4,
    MOV,
    MKV,
    WEBM,
    AVI,
    JSON_TEXT,
    PLAIN_TEXT,
    PE_EXE,
    ELF,
    MACHO,
    SCRIPT_SHEBANG,
};

const char* DetectedContentName(DetectedContent t) noexcept;

DetectedContent DetectContent(const void* data, std::size_t size) noexcept;

bool IsExecutableContent(DetectedContent t) noexcept;

bool IsAllowedForUpload(DetectedContent t) noexcept;

struct ValidationResult {
    DetectedContent detected   = DetectedContent::UNKNOWN;
    bool            allowed    = false;
    bool            isExec     = false;
    std::string     reason;
};

ValidationResult ValidateUploadContent(const std::string& body,
                                       std::size_t        maxBytes,
                                       bool               allowText = true);

}
