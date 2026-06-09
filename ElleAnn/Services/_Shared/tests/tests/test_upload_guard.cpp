#include <doctest/doctest.h>

#include "ElleUploadGuard.h"

#include <string>

using namespace ElleUpload;

namespace {

std::string bytes(std::initializer_list<unsigned char> bs) {
    std::string out;
    out.reserve(bs.size());
    for (unsigned char b : bs) out.push_back(static_cast<char>(b));
    return out;
}

}

TEST_CASE("DetectContent: empty body is EMPTY") {
    CHECK(DetectContent("", 0) == DetectedContent::EMPTY);
}

TEST_CASE("DetectContent recognises PNG signature") {
    auto s = bytes({0x89,'P','N','G',0x0D,0x0A,0x1A,0x0A, 0,0,0,0});
    CHECK(DetectContent(s.data(), s.size()) == DetectedContent::PNG);
}

TEST_CASE("DetectContent recognises JPEG signature") {
    auto s = bytes({0xFF, 0xD8, 0xFF, 0xE0, 0, 0});
    CHECK(DetectContent(s.data(), s.size()) == DetectedContent::JPEG);
}

TEST_CASE("DetectContent recognises GIF87a and GIF89a") {
    auto a = std::string("GIF87a\x00\x00", 8);
    auto b = std::string("GIF89a\x00\x00", 8);
    CHECK(DetectContent(a.data(), a.size()) == DetectedContent::GIF);
    CHECK(DetectContent(b.data(), b.size()) == DetectedContent::GIF);
}

TEST_CASE("DetectContent recognises RIFF/WEBP, RIFF/WAVE, RIFF/AVI") {
    auto w = bytes({'R','I','F','F', 0,0,0,0, 'W','E','B','P', 0});
    CHECK(DetectContent(w.data(), w.size()) == DetectedContent::WEBP);
    auto a = bytes({'R','I','F','F', 0,0,0,0, 'W','A','V','E', 0});
    CHECK(DetectContent(a.data(), a.size()) == DetectedContent::WAV);
    auto v = bytes({'R','I','F','F', 0,0,0,0, 'A','V','I',' ', 0});
    CHECK(DetectContent(v.data(), v.size()) == DetectedContent::AVI);
}

TEST_CASE("DetectContent recognises PDF, ZIP, GZIP") {
    auto pdf = std::string("%PDF-1.7\n", 9);
    CHECK(DetectContent(pdf.data(), pdf.size()) == DetectedContent::PDF);

    auto zip = bytes({'P','K',0x03,0x04, 0,0,0,0});
    CHECK(DetectContent(zip.data(), zip.size()) == DetectedContent::ZIP);

    auto gz = bytes({0x1F, 0x8B, 0x08, 0x00});
    CHECK(DetectContent(gz.data(), gz.size()) == DetectedContent::GZIP);
}

TEST_CASE("DetectContent recognises MP4 ftyp container") {
    auto mp4 = bytes({0,0,0,0x18, 'f','t','y','p', 'i','s','o','m', 0,0,0,0});
    CHECK(DetectContent(mp4.data(), mp4.size()) == DetectedContent::MP4);
}

TEST_CASE("DetectContent recognises executable formats as exec") {
    auto pe = bytes({'M','Z', 0x90, 0});
    auto elf = bytes({0x7F, 'E','L','F', 0,0});
    auto sh = std::string("#!/bin/bash\n", 12);

    CHECK(DetectContent(pe.data(),  pe.size())  == DetectedContent::PE_EXE);
    CHECK(DetectContent(elf.data(), elf.size()) == DetectedContent::ELF);
    CHECK(DetectContent(sh.data(),  sh.size())  == DetectedContent::SCRIPT_SHEBANG);

    CHECK(IsExecutableContent(DetectedContent::PE_EXE));
    CHECK(IsExecutableContent(DetectedContent::ELF));
    CHECK(IsExecutableContent(DetectedContent::SCRIPT_SHEBANG));
}

TEST_CASE("DetectContent treats clean printable bytes as PLAIN_TEXT") {
    std::string s = "hello there, this is a friendly note.\n";
    CHECK(DetectContent(s.data(), s.size()) == DetectedContent::PLAIN_TEXT);
}

TEST_CASE("DetectContent treats curly-brace start as JSON_TEXT") {
    std::string s = "  { \"key\": \"value\" }";
    CHECK(DetectContent(s.data(), s.size()) == DetectedContent::JSON_TEXT);
}

TEST_CASE("DetectContent returns UNKNOWN for random binary garbage") {
    auto s = bytes({0xCA, 0xFE, 0xBA, 0xBE, 0x01, 0x02, 0x03, 0x04});
    CHECK(DetectContent(s.data(), s.size()) == DetectedContent::UNKNOWN);
}

TEST_CASE("ValidateUploadContent rejects executable content") {
    auto pe = bytes({'M','Z', 0x90, 0});
    std::string s(pe.begin(), pe.end());
    auto vr = ValidateUploadContent(s, 1024);
    CHECK(!vr.allowed);
    CHECK(vr.isExec);
    CHECK(vr.detected == DetectedContent::PE_EXE);
    CHECK(vr.reason.find("executable") != std::string::npos);
}

TEST_CASE("ValidateUploadContent rejects empty body") {
    auto vr = ValidateUploadContent(std::string(), 1024);
    CHECK(!vr.allowed);
    CHECK(vr.detected == DetectedContent::EMPTY);
}

TEST_CASE("ValidateUploadContent rejects unknown content signature") {
    auto s = bytes({0xCA, 0xFE, 0xBA, 0xBE, 0x01, 0x02, 0x03, 0x04});
    std::string sx(s);
    auto vr = ValidateUploadContent(sx, 1024);
    CHECK(!vr.allowed);
    CHECK(vr.detected == DetectedContent::UNKNOWN);
}

TEST_CASE("ValidateUploadContent honours maxBytes cap") {
    std::string huge(2048, 'a');
    auto vr = ValidateUploadContent(huge, 1024);
    CHECK(!vr.allowed);
    CHECK(vr.reason.find("max_upload_bytes") != std::string::npos);
}

TEST_CASE("ValidateUploadContent: PNG passes, text refused on binary-only endpoint") {
    auto pngBytes = bytes({0x89,'P','N','G',0x0D,0x0A,0x1A,0x0A, 0,0,0,0});
    auto vr = ValidateUploadContent(pngBytes, 1024, true);
    CHECK(vr.allowed);
    CHECK(vr.detected == DetectedContent::PNG);

    std::string text = "just some plain text";
    auto vrText = ValidateUploadContent(text, 1024, /*allowText=*/false);
    CHECK(!vrText.allowed);
    CHECK(vrText.detected == DetectedContent::PLAIN_TEXT);
}

TEST_CASE("ValidateUploadContent: PNG accepted on text-allowing endpoint too") {
    auto pngBytes = bytes({0x89,'P','N','G',0x0D,0x0A,0x1A,0x0A});
    auto vr = ValidateUploadContent(pngBytes, 1024, true);
    CHECK(vr.allowed);
}
