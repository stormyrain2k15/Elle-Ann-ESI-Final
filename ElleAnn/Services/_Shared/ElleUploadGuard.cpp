#include "ElleUploadGuard.h"

#include <cstring>

namespace ElleUpload {

const char* DetectedContentName(DetectedContent t) noexcept {
    switch (t) {
        case DetectedContent::EMPTY:           return "EMPTY";
        case DetectedContent::JPEG:            return "JPEG";
        case DetectedContent::PNG:             return "PNG";
        case DetectedContent::GIF:             return "GIF";
        case DetectedContent::BMP:             return "BMP";
        case DetectedContent::WEBP:            return "WEBP";
        case DetectedContent::PDF:             return "PDF";
        case DetectedContent::ZIP:             return "ZIP";
        case DetectedContent::RAR:             return "RAR";
        case DetectedContent::SEVENZIP:        return "7Z";
        case DetectedContent::GZIP:            return "GZIP";
        case DetectedContent::TAR:             return "TAR";
        case DetectedContent::MP3:             return "MP3";
        case DetectedContent::OGG:             return "OGG";
        case DetectedContent::WAV:             return "WAV";
        case DetectedContent::FLAC:            return "FLAC";
        case DetectedContent::MP4:             return "MP4";
        case DetectedContent::MOV:             return "MOV";
        case DetectedContent::MKV:             return "MKV";
        case DetectedContent::WEBM:            return "WEBM";
        case DetectedContent::AVI:             return "AVI";
        case DetectedContent::JSON_TEXT:       return "JSON_TEXT";
        case DetectedContent::PLAIN_TEXT:      return "PLAIN_TEXT";
        case DetectedContent::PE_EXE:          return "PE_EXE";
        case DetectedContent::ELF:             return "ELF";
        case DetectedContent::MACHO:           return "MACHO";
        case DetectedContent::SCRIPT_SHEBANG:  return "SCRIPT_SHEBANG";
        case DetectedContent::UNKNOWN:
        default:                               return "UNKNOWN";
    }
}

static bool startsWith(const unsigned char* p, std::size_t n,
                       const unsigned char* sig, std::size_t siglen) noexcept {
    if (n < siglen) return false;
    return std::memcmp(p, sig, siglen) == 0;
}

DetectedContent DetectContent(const void* data, std::size_t size) noexcept {
    if (size == 0) return DetectedContent::EMPTY;
    const unsigned char* p = static_cast<const unsigned char*>(data);

    static const unsigned char SIG_PNG[]    = { 0x89,'P','N','G',0x0D,0x0A,0x1A,0x0A };
    static const unsigned char SIG_JPEG[]   = { 0xFF, 0xD8, 0xFF };
    static const unsigned char SIG_GIF87[]  = { 'G','I','F','8','7','a' };
    static const unsigned char SIG_GIF89[]  = { 'G','I','F','8','9','a' };
    static const unsigned char SIG_BMP[]    = { 'B','M' };
    static const unsigned char SIG_PDF[]    = { '%','P','D','F','-' };
    static const unsigned char SIG_ZIP1[]   = { 'P','K',0x03,0x04 };
    static const unsigned char SIG_ZIP2[]   = { 'P','K',0x05,0x06 };
    static const unsigned char SIG_ZIP3[]   = { 'P','K',0x07,0x08 };
    static const unsigned char SIG_RAR4[]   = { 'R','a','r','!',0x1A,0x07,0x00 };
    static const unsigned char SIG_RAR5[]   = { 'R','a','r','!',0x1A,0x07,0x01,0x00 };
    static const unsigned char SIG_7Z[]     = { '7','z',0xBC,0xAF,0x27,0x1C };
    static const unsigned char SIG_GZIP[]   = { 0x1F, 0x8B };
    static const unsigned char SIG_OGG[]    = { 'O','g','g','S' };
    static const unsigned char SIG_FLAC[]   = { 'f','L','a','C' };
    static const unsigned char SIG_MKV[]    = { 0x1A, 0x45, 0xDF, 0xA3 };
    static const unsigned char SIG_PE_MZ[]  = { 'M','Z' };
    static const unsigned char SIG_ELF[]    = { 0x7F, 'E','L','F' };
    static const unsigned char SIG_MACHO1[] = { 0xFE, 0xED, 0xFA, 0xCE };
    static const unsigned char SIG_MACHO2[] = { 0xFE, 0xED, 0xFA, 0xCF };
    static const unsigned char SIG_MACHO3[] = { 0xCF, 0xFA, 0xED, 0xFE };
    static const unsigned char SIG_MACHO4[] = { 0xCE, 0xFA, 0xED, 0xFE };
    static const unsigned char SIG_SHEBANG[]= { '#', '!' };
    static const unsigned char SIG_ID3[]    = { 'I','D','3' };
    static const unsigned char SIG_MP3F1[]  = { 0xFF, 0xFB };
    static const unsigned char SIG_MP3F2[]  = { 0xFF, 0xF3 };
    static const unsigned char SIG_MP3F3[]  = { 0xFF, 0xF2 };

    if (startsWith(p, size, SIG_PNG,   sizeof(SIG_PNG)))   return DetectedContent::PNG;
    if (startsWith(p, size, SIG_JPEG,  sizeof(SIG_JPEG)))  return DetectedContent::JPEG;
    if (startsWith(p, size, SIG_GIF87, sizeof(SIG_GIF87))) return DetectedContent::GIF;
    if (startsWith(p, size, SIG_GIF89, sizeof(SIG_GIF89))) return DetectedContent::GIF;
    if (startsWith(p, size, SIG_BMP,   sizeof(SIG_BMP)))   return DetectedContent::BMP;
    if (startsWith(p, size, SIG_PDF,   sizeof(SIG_PDF)))   return DetectedContent::PDF;

    if (startsWith(p, size, SIG_ZIP1, sizeof(SIG_ZIP1)) ||
        startsWith(p, size, SIG_ZIP2, sizeof(SIG_ZIP2)) ||
        startsWith(p, size, SIG_ZIP3, sizeof(SIG_ZIP3))) {
        return DetectedContent::ZIP;
    }
    if (startsWith(p, size, SIG_RAR5, sizeof(SIG_RAR5))) return DetectedContent::RAR;
    if (startsWith(p, size, SIG_RAR4, sizeof(SIG_RAR4))) return DetectedContent::RAR;
    if (startsWith(p, size, SIG_7Z,   sizeof(SIG_7Z)))   return DetectedContent::SEVENZIP;
    if (startsWith(p, size, SIG_GZIP, sizeof(SIG_GZIP))) return DetectedContent::GZIP;

    if (size >= 12) {
        if (std::memcmp(p, "RIFF", 4) == 0) {
            if (std::memcmp(p + 8, "WAVE", 4) == 0) return DetectedContent::WAV;
            if (std::memcmp(p + 8, "WEBP", 4) == 0) return DetectedContent::WEBP;
            if (std::memcmp(p + 8, "AVI ", 4) == 0) return DetectedContent::AVI;
        }
    }

    if (startsWith(p, size, SIG_OGG,  sizeof(SIG_OGG)))  return DetectedContent::OGG;
    if (startsWith(p, size, SIG_FLAC, sizeof(SIG_FLAC))) return DetectedContent::FLAC;

    if (startsWith(p, size, SIG_ID3, sizeof(SIG_ID3)) ||
        startsWith(p, size, SIG_MP3F1, sizeof(SIG_MP3F1)) ||
        startsWith(p, size, SIG_MP3F2, sizeof(SIG_MP3F2)) ||
        startsWith(p, size, SIG_MP3F3, sizeof(SIG_MP3F3))) {
        return DetectedContent::MP3;
    }

    if (size >= 12 && std::memcmp(p + 4, "ftyp", 4) == 0) {
        const char* brand = reinterpret_cast<const char*>(p + 8);
        if (std::memcmp(brand, "qt  ", 4) == 0) return DetectedContent::MOV;
        if (std::memcmp(brand, "isom", 4) == 0) return DetectedContent::MP4;
        if (std::memcmp(brand, "mp4",  3) == 0) return DetectedContent::MP4;
        if (std::memcmp(brand, "M4V",  3) == 0) return DetectedContent::MP4;
        if (std::memcmp(brand, "M4A",  3) == 0) return DetectedContent::MP4;
        if (std::memcmp(brand, "avc1", 4) == 0) return DetectedContent::MP4;
        if (std::memcmp(brand, "dash", 4) == 0) return DetectedContent::MP4;
        return DetectedContent::MP4;
    }

    if (startsWith(p, size, SIG_MKV, sizeof(SIG_MKV))) return DetectedContent::MKV;

    if (startsWith(p, size, SIG_PE_MZ, sizeof(SIG_PE_MZ))) return DetectedContent::PE_EXE;
    if (startsWith(p, size, SIG_ELF,   sizeof(SIG_ELF)))   return DetectedContent::ELF;
    if (startsWith(p, size, SIG_MACHO1, sizeof(SIG_MACHO1)) ||
        startsWith(p, size, SIG_MACHO2, sizeof(SIG_MACHO2)) ||
        startsWith(p, size, SIG_MACHO3, sizeof(SIG_MACHO3)) ||
        startsWith(p, size, SIG_MACHO4, sizeof(SIG_MACHO4))) {
        return DetectedContent::MACHO;
    }
    if (startsWith(p, size, SIG_SHEBANG, sizeof(SIG_SHEBANG))) return DetectedContent::SCRIPT_SHEBANG;

    {
        std::size_t scan = size < 4096 ? size : 4096;
        std::size_t skip = 0;
        if (size >= 3 && p[0] == 0xEF && p[1] == 0xBB && p[2] == 0xBF) skip = 3;
        std::size_t firstNonWs = skip;
        while (firstNonWs < scan && (p[firstNonWs] == ' ' || p[firstNonWs] == '\t' ||
                                      p[firstNonWs] == '\r' || p[firstNonWs] == '\n')) {
            ++firstNonWs;
        }
        bool printable = true;
        for (std::size_t i = skip; i < scan; ++i) {
            unsigned char c = p[i];
            if (c == 0) { printable = false; break; }
            if (c < 0x09 || (c > 0x0D && c < 0x20)) { printable = false; break; }
        }
        if (printable) {
            if (firstNonWs < scan && (p[firstNonWs] == '{' || p[firstNonWs] == '[')) {
                return DetectedContent::JSON_TEXT;
            }
            return DetectedContent::PLAIN_TEXT;
        }
    }

    return DetectedContent::UNKNOWN;
}

bool IsExecutableContent(DetectedContent t) noexcept {
    return t == DetectedContent::PE_EXE
        || t == DetectedContent::ELF
        || t == DetectedContent::MACHO
        || t == DetectedContent::SCRIPT_SHEBANG;
}

bool IsAllowedForUpload(DetectedContent t) noexcept {
    if (IsExecutableContent(t))                     return false;
    if (t == DetectedContent::UNKNOWN)              return false;
    if (t == DetectedContent::EMPTY)                return false;
    return true;
}

ValidationResult ValidateUploadContent(const std::string& body,
                                       std::size_t        maxBytes,
                                       bool               allowText) {
    ValidationResult vr;
    if (maxBytes > 0 && body.size() > maxBytes) {
        vr.reason = "payload exceeds max_upload_bytes";
        return vr;
    }
    vr.detected = DetectContent(body.data(), body.size());
    vr.isExec   = IsExecutableContent(vr.detected);

    if (vr.isExec) {
        vr.reason  = std::string("executable content rejected (")
                   + DetectedContentName(vr.detected) + ")";
        vr.allowed = false;
        return vr;
    }
    if (vr.detected == DetectedContent::EMPTY) {
        vr.reason  = "empty body";
        vr.allowed = false;
        return vr;
    }
    if (vr.detected == DetectedContent::UNKNOWN) {
        vr.reason  = "unrecognised content signature";
        vr.allowed = false;
        return vr;
    }
    if (!allowText && (vr.detected == DetectedContent::PLAIN_TEXT ||
                       vr.detected == DetectedContent::JSON_TEXT)) {
        vr.reason  = "text content not accepted on binary-only endpoint";
        vr.allowed = false;
        return vr;
    }
    vr.allowed = true;
    return vr;
}

}
