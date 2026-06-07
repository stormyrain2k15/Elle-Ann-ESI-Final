#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace {

std::string Slurp(const char* path) {
    std::ifstream in(path, std::ios::binary);
    std::ostringstream os; os << in.rdbuf();
    return os.str();
}

int g_failures = 0;
void RequireNotContains(const char* path, const char* needle, const char* explanation) {
    auto src = Slurp(path);
    if (src.find(needle) != std::string::npos) {
        std::fprintf(stderr, "FAIL %s contains forbidden pattern: %s\n  %s\n",
                     path, needle, explanation);
        g_failures++;
    } else {
        std::fprintf(stdout, "ok   %s — %s\n", path, explanation);
    }
}
void RequireContains(const char* path, const char* needle, const char* explanation) {
    auto src = Slurp(path);
    if (src.find(needle) == std::string::npos) {
        std::fprintf(stderr, "FAIL %s missing required pattern: %s\n  %s\n",
                     path, needle, explanation);
        g_failures++;
    } else {
        std::fprintf(stdout, "ok   %s — %s\n", path, explanation);
    }
}

}

int main() {

    RequireNotContains(
        "/app/ElleAnn/Services/Elle.Service.HTTP/HTTPServer.cpp",
        " * greppingfor Register() calls.",
        "orphaned block comment fragment must not return");

    RequireNotContains(
        "/app/ElleAnn/Shared/ElleConfig.cpp",
        "PopulateLLMConfig();",
        "non-existent populate methods must not return");
    RequireContains(
        "/app/ElleAnn/Shared/ElleConfig.cpp",
        "PopulateFromJSON(m_root);",
        "real PopulateFromJSON is the correct call");

    RequireNotContains(
        "/app/ElleAnn/Shared/ElleConfig.cpp",
        "JsonType::Number",
        "JsonType::Number is a typo (real is Int/Float)");
    RequireNotContains(
        "/app/ElleAnn/Shared/ElleConfig.cpp",
        "v.num_val",
        "num_val is a typo (real is int_val/float_val)");

    RequireContains(
        "/app/ElleAnn/Shared/ElleServiceBase.h",
        "OnMessage(const ElleIPCMessage& /*msg*/, ELLE_SERVICE_ID /*sender*/)",
        "base virtual OnMessage must mark its params unused-OK");

    RequireNotContains(
        "/app/ElleAnn/Services/Elle.Service.GoalEngine/GoalEngine.cpp",
        "g.source_drive >= 0 &&",
        "uint32 >= 0 redundant comparison");

    const char* commentNests[][2] = {
        { "/app/ElleAnn/Services/Elle.Service.HTTP/HTTPServer.cpp",       "/api/diag/* is dev-only"        },
        { "/app/ElleAnn/Services/Elle.Service.HTTP/HTTPServer.cpp",       "/api/identity/*"                },
        { "/app/ElleAnn/Shared/ElleDB_Content.cpp",                        "/api/memory/* endpoints"        },
        { "/app/ElleAnn/Services/Elle.Service.Family/Family.cpp",          "bin/*.exe"                      },
        { "/app/ElleAnn/Services/Elle.Service.XChromosome/XEngine.h",      "(HTTP /api/x/*"                 },
        { "/app/ElleAnn/Services/Elle.Service.Cognitive/CognitiveEngine.cpp", "/api/diag/* (and operator log scans)" },
        { "/app/ElleAnn/Services/Elle.Service.Fiesta/FiestaClient.cpp",    "_re_artifacts/pdb/extracted/*.md" },
    };
    for (const auto& nc : commentNests) {
        RequireNotContains(nc[0], nc[1],
            "comment-within-comment must stay rewritten (-Wcomment under MSVC /WX)");
    }

    RequireNotContains(
        "/app/ElleAnn/Shared/ElleDB_Content.cpp",
        "\"InternalNarrative\"",
        "non-existent table InternalNarrative must not return to the whitelist");
    RequireNotContains(
        "/app/ElleAnn/Shared/ElleDB_Content.cpp",
        "\"DreamIntegration\"",
        "non-existent table DreamIntegration must not return to the whitelist");

    RequireContains(
        "/app/ElleAnn/Directory.Build.props",
        "4244;4267;4996",
        "narrow-conv suppressions must stay until source-level cleanup is done");
    RequireContains(
        "/app/ElleAnn/Shared/ElleCore.Shared.vcxproj",
        "<TargetName>ElleCore.Shared</TargetName>",
        "ElleCore.Shared must pin TargetName explicitly so consumers find the lib");
    RequireContains(
        "/app/ElleAnn/Shared/ElleCore.Shared.vcxproj",
        "<TargetExt>.lib</TargetExt>",
        "ElleCore.Shared must pin TargetExt to avoid VS2026 Insiders import-order race");

    {
        const char* projects[] = {
            "/app/ElleAnn/Shared/ElleCore.Shared.vcxproj",
            "/app/ElleAnn/Services/Elle.Service.HTTP/Elle.Service.HTTP.vcxproj",
            "/app/ElleAnn/Services/Elle.Service.Fiesta/Elle.Service.Fiesta.vcxproj",
            "/app/ElleAnn/ASM/Elle.ASM.Hardware/Elle.ASM.Hardware.vcxproj",
            "/app/ElleAnn/Lua/Elle.Lua.Behavioral/Elle.Lua.Behavioral.vcxproj",
        };
        for (const char* p : projects) {
            RequireNotContains(p, "<PlatformToolset>v145</PlatformToolset>",
                "v145 toolset override must NOT pin to VS 2026 only — CI uses v143");
        }
    }

    if (g_failures) {
        std::fprintf(stderr, "\nFAIL — %d audit pin(s) regressed.\n", g_failures);
        return 1;
    }
    std::fprintf(stdout, "\nALL AUDIT PINS HOLD\n");
    return 0;
}
