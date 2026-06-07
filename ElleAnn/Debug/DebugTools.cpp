#ifdef BUILD_SERVICE_HARNESS

#include "../Shared/ElleTypes.h"
#include "../Shared/ElleConfig.h"
#include "../Shared/ElleLogger.h"
#include "../Shared/ElleSQLConn.h"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    std::cout << "=== ELLE-ANN Debug Service Harness ===\n\n";

    if (argc < 2) {
        std::cout << "Usage: ServiceHarness.exe <service_name> [config_path]\n\n"
                  << "Services:\n"
                  << "  QueueWorker, HTTPServer, Emotional, Memory, Cognitive,\n"
                  << "  Action, Identity, Heartbeat, SelfPrompt, Dream,\n"
                  << "  GoalEngine, WorldModel, LuaBehavioral\n";
        return 1;
    }

    std::string serviceName = argv[1];
    std::string configPath = argc > 2 ? argv[2] : "elle_master_config.json";

    std::cout << "Loading config from: " << configPath << "\n";
    if (!ElleConfig::Instance().Load(configPath)) {
        std::cerr << "Failed to load config!\n";
        return 1;
    }

    std::cout << "Config loaded. Starting " << serviceName << " in harness mode...\n";
    std::cout << "Press Ctrl+C to stop.\n\n";

    std::cout << "Service harness running. Type 'quit' to exit.\n";
    std::string cmd;
    while (std::getline(std::cin, cmd)) {
        if (cmd == "quit" || cmd == "exit") break;
        if (cmd == "status") {
            std::cout << "Service: " << serviceName << " | Running: YES\n";
        }
        if (cmd == "config") {
            std::cout << "LLM Mode: " << (int)ElleConfig::Instance().GetLLM().mode << "\n"
                      << "Trust Initial: " << ElleConfig::Instance().GetTrust().initial_score << "\n"
                      << "Emotion Decay: " << ElleConfig::Instance().GetEmotion().decay_rate << "\n";
        }
    }

    return 0;
}
#endif

#ifdef BUILD_QUEUE_INSPECTOR

#include "../Shared/ElleTypes.h"
#include "../Shared/ElleConfig.h"
#include "../Shared/ElleSQLConn.h"
#include <iostream>
#include <iomanip>

void PrintIntentQueue() {
    auto rs = ElleSQLPool::Instance().Query(
        "SELECT TOP 20 IntentId, IntentType, Status, Urgency, Confidence, "
        "LEFT(Description, 60) AS Desc, CreatedMs FROM ElleCore.dbo.IntentQueue "
        "ORDER BY CreatedMs DESC");

    std::cout << "\n=== INTENT QUEUE ===\n";
    std::cout << std::setw(8) << "ID" << std::setw(10) << "Type"
              << std::setw(10) << "Status" << std::setw(8) << "Urgency"
              << std::setw(10) << "Conf" << "  Description\n";
    std::cout << std::string(80, '-') << "\n";

    if (!rs.success) {
        std::cout << "  [query failed: " << rs.error << "]\n";
        return;
    }

    auto col = [](const SQLRow& r, size_t i) -> std::string {
        return (i < r.values.size()) ? r.values[i] : std::string("?");
    };
    for (auto& row : rs.rows) {
        std::cout << std::setw(8) << col(row, 0)
                  << std::setw(10) << col(row, 1)
                  << std::setw(10) << col(row, 2)
                  << std::setw(8)  << col(row, 3)
                  << std::setw(10) << col(row, 4)
                  << "  " << col(row, 5) << "\n";
    }
}

void PrintActionQueue() {
    auto rs = ElleSQLPool::Instance().Query(
        "SELECT TOP 20 ActionId, IntentId, ActionType, Status, "
        "LEFT(Command, 60) AS Cmd, TrustDelta FROM ElleCore.dbo.ActionQueue "
        "ORDER BY CreatedMs DESC");

    std::cout << "\n=== ACTION QUEUE ===\n";
    std::cout << std::setw(8) << "ID" << std::setw(10) << "IntentID"
              << std::setw(10) << "Type" << std::setw(10) << "Status"
              << std::setw(8) << "Trust" << "  Command\n";
    std::cout << std::string(80, '-') << "\n";

    if (!rs.success) {
        std::cout << "  [query failed: " << rs.error << "]\n";
        return;
    }

    auto col = [](const SQLRow& r, size_t i) -> std::string {
        return (i < r.values.size()) ? r.values[i] : std::string("?");
    };
    for (auto& row : rs.rows) {
        std::cout << std::setw(8) << col(row, 0)
                  << std::setw(10) << col(row, 1)
                  << std::setw(10) << col(row, 2)
                  << std::setw(10) << col(row, 3)
                  << std::setw(8)  << col(row, 5)
                  << "  " << col(row, 4) << "\n";
    }
}

int main() {
    std::cout << "=== ELLE-ANN Queue Inspector ===\n";

    ElleConfig::Instance().Load("elle_master_config.json");
    auto& svc = ElleConfig::Instance().GetService();

    if (!ElleSQLPool::Instance().Initialize(svc.sql_connection_string, 2)) {
        std::cerr << "Failed to connect to SQL Server!\n";
        return 1;
    }

    std::cout << "Connected. Refreshing every 2 seconds. Press Ctrl+C to stop.\n";

    while (true) {
        system("cls");
        std::cout << "=== ELLE-ANN Queue Inspector === (Live)\n";
        PrintIntentQueue();
        PrintActionQueue();
        Sleep(2000);
    }

    return 0;
}
#endif

#ifdef BUILD_EMOTION_VIEWER

#include "../Shared/ElleTypes.h"
#include "../Shared/ElleConfig.h"
#include "../Shared/ElleSQLConn.h"
#include <iostream>
#include <iomanip>

const char* emotionNames[] = {
    "Joy","Sadness","Anger","Fear","Disgust","Surprise","Contempt","Trust",
    "Love","Anticipation","Disappointment","Guilt","Shame","Envy","Jealousy",
    "Pride","Relief","Anxiety","Frustration","Remorse","Hope","Despair",
    "Amusement","Awe","Curiosity","Wonder","Nostalgia","Gratitude","Serenity",
    "Ecstasy","Melancholy","Boredom","Longing","Tenderness","Admiration",
    "Reverence","Pity","Scorn","Indignation","Exasperation","Wistfulness",
    "Euphoria","Contentment","Resignation","Apprehension","Dread","Panic",
    "Horror","Rage","Irritation","Annoyance","Impatience","Skepticism",
    "Confusion","Disbelief","Ambivalence","Certainty","Doubt","Insight",
    "Perplexity","Clarity","CogDissonance","FlowState","MentalFatigue",
    "Inspiration","CreativeTension","Determination","Helplessness",
    "Empowerment","Overwhelm","Focus","Distraction","Belonging","Isolation",
    "Empathy","Compassion","Protectiveness","Abandonment","Loyalty","Betrayal",
    "Acceptance","Rejection","Vulnerability","Safety","Dominance","Submission",
    "ExDread","Purpose","Meaningless","Transcendence","Mortality","Freedom",
    "Confinement","Unity"
};

void DisplayEmotions() {
    auto rs = ElleSQLPool::Instance().Query(
        "SELECT TOP 1 Dimensions, Valence, Arousal, Dominance, TickCount "
        "FROM ElleKnowledge.dbo.EmotionHistory ORDER BY TimestampMs DESC");

    if (!rs.success) {
        std::cout << "Query failed: " << rs.error << "\n";
        return;
    }
    if (rs.rows.empty()) {
        std::cout << "No emotion data available.\n";
        return;
    }

    auto& row = rs.rows[0];

    auto col = [&](size_t i) -> std::string {
        return (i < row.values.size()) ? row.values[i] : std::string("?");
    };
    std::cout << "Valence: "   << col(1)
              << "  Arousal: " << col(2)
              << "  Dominance: " << col(3)
              << "  Tick: "    << col(4) << "\n\n";

    std::vector<float> values;
    values.reserve(128);
    const std::string dims = col(0);
    std::stringstream ss(dims);
    std::string token;
    while (std::getline(ss, token, ',')) {
        try {
            values.push_back(std::stof(token));
        } catch (const std::exception&) {
            values.push_back(0.0f);
        }
    }

    const size_t kNames = sizeof(emotionNames) / sizeof(emotionNames[0]);
    const size_t limit  = std::min<size_t>(values.size(), kNames);
    for (size_t i = 0; i < limit; i++) {
        if (values[i] < 0.01f) continue;

        std::cout << std::setw(18) << std::left << emotionNames[i] << " ";
        int bars = (int)(values[i] * 40);
        if (bars < 0)  bars = 0;
        if (bars > 40) bars = 40;
        for (int b = 0; b < bars; b++) std::cout << "#";
        std::cout << " " << std::fixed << std::setprecision(2) << values[i] << "\n";
    }
    if (values.size() > kNames) {
        std::cout << "  [note: row carried " << values.size()
                  << " dimensions, names table has " << kNames
                  << " -- extras ignored]\n";
    }
}

int main() {
    std::cout << "=== ELLE-ANN Emotion Viewer ===\n";

    ElleConfig::Instance().Load("elle_master_config.json");
    auto& svc = ElleConfig::Instance().GetService();
    ElleSQLPool::Instance().Initialize(svc.sql_connection_string, 2);

    while (true) {
        system("cls");
        std::cout << "=== ELLE-ANN Emotion Viewer === (Live)\n\n";
        DisplayEmotions();
        Sleep(1000);
    }

    return 0;
}
#endif

#ifdef BUILD_ASM_TESTBENCH

#define ELLE_IMPORT_ASM
#include "../Shared/ElleTypes.h"
#include <iostream>

int main() {
    std::cout << "=== ELLE-ANN ASM Test Bench ===\n\n";

    HMODULE hHW = LoadLibraryA("Elle.ASM.Hardware.dll");
    HMODULE hProc = LoadLibraryA("Elle.ASM.Process.dll");
    HMODULE hFile = LoadLibraryA("Elle.ASM.FileIO.dll");
    HMODULE hMem = LoadLibraryA("Elle.ASM.Memory.dll");
    HMODULE hCrypto = LoadLibraryA("Elle.ASM.Crypto.dll");

    std::cout << "DLL Load Status:\n"
              << "  Hardware: " << (hHW ? "OK" : "FAILED") << "\n"
              << "  Process:  " << (hProc ? "OK" : "FAILED") << "\n"
              << "  FileIO:   " << (hFile ? "OK" : "FAILED") << "\n"
              << "  Memory:   " << (hMem ? "OK" : "FAILED") << "\n"
              << "  Crypto:   " << (hCrypto ? "OK" : "FAILED") << "\n\n";

    if (hHW) {
        typedef int(__stdcall* RDTSC_fn)(ULONGLONG*);
        auto fn = (RDTSC_fn)GetProcAddress(hHW, "ASM_RDTSC");
        if (fn) {
            ULONGLONG ts;
            fn(&ts);
            std::cout << "RDTSC: " << ts << "\n";
        }
    }

    if (hHW) {
        typedef int(__stdcall* CPUID_fn)(DWORD, DWORD*, DWORD*, DWORD*, DWORD*);
        auto fn = (CPUID_fn)GetProcAddress(hHW, "ASM_CPUID");
        if (fn) {
            DWORD eax, ebx, ecx, edx;
            fn(0, &eax, &ebx, &ecx, &edx);
            char vendor[13] = {};
            *(DWORD*)vendor = ebx;
            *(DWORD*)(vendor+4) = edx;
            *(DWORD*)(vendor+8) = ecx;
            std::cout << "CPUID Vendor: " << vendor << "\n";
        }
    }

    if (hCrypto) {
        typedef DWORD(__stdcall* CRC32_fn)(const void*, DWORD);
        auto fn = (CRC32_fn)GetProcAddress(hCrypto, "ASM_CRC32");
        if (fn) {
            const char* testData = "Elle-Ann ESI v3.0";
            DWORD crc = fn(testData, (DWORD)strlen(testData));
            std::cout << "CRC32(\"" << testData << "\"): 0x" << std::hex << crc << "\n";
        }
    }

    if (hMem) {
        typedef void(__stdcall* MemCopy_fn)(void*, const void*, DWORD);
        auto fn = (MemCopy_fn)GetProcAddress(hMem, "ASM_FastMemCopy");
        if (fn) {
            char src[256], dst[256];
            memset(src, 'A', 256);
            fn(dst, src, 256);
            std::cout << "FastMemCopy: " << (memcmp(src, dst, 256) == 0 ? "PASS" : "FAIL") << "\n";
        }
    }

    std::cout << "\nAll tests complete.\n";

    if (hHW) FreeLibrary(hHW);
    if (hProc) FreeLibrary(hProc);
    if (hFile) FreeLibrary(hFile);
    if (hMem) FreeLibrary(hMem);
    if (hCrypto) FreeLibrary(hCrypto);

    return 0;
}
#endif

#ifdef BUILD_MEMORY_HARNESS

#include "../Shared/ElleTypes.h"
#include "../Shared/ElleConfig.h"
#include "../Shared/ElleLogger.h"
#include "../Shared/ElleSQLConn.h"
#include "../Services/Elle.Service.Memory/MemoryEngine.h"
#include <iostream>
#include <iomanip>
#include <vector>

struct Seed { const char* tag; const char* content; float importance; };

static void DumpSTM(MemoryEngine& eng, const char* label) {
    auto recent = eng.RecallRecent(256);
    std::cout << "\n── " << label << " (stm=" << eng.STMCount() << ") ──\n";
    std::cout << std::left
              << std::setw(6)  << "id"
              << std::setw(10) << "imp"
              << std::setw(10) << "rel"
              << std::setw(6)  << "acc"
              << "content\n";
    for (auto& m : recent) {
        std::cout << std::setw(6)  << m.id
                  << std::setw(10) << std::fixed << std::setprecision(2) << m.importance
                  << std::setw(10) << std::fixed << std::setprecision(3) << m.relevance
                  << std::setw(6)  << m.access_count
                  << std::string(m.content).substr(0, 60) << "\n";
    }
}

int main() {
    std::cout << "=== ELLE-ANN Memory Harness ===\n";

    if (!ElleConfig::Instance().Load("elle_master_config.json")) {
        std::cerr << "Failed to load elle_master_config.json\n";
        return 1;
    }
    auto& svc = ElleConfig::Instance().GetService();
    if (!ElleSQLPool::Instance().Initialize(svc.sql_connection_string, 2)) {
        std::cerr << "Failed to open SQL pool — LTM writes will fail!\n";

    }

    MemoryEngine eng;
    if (!eng.Initialize()) { std::cerr << "Engine init failed\n"; return 1; }

    float zero[ELLE_MAX_EMOTIONS] = {};

    std::vector<Seed> seeds = {
        { "critical", "Users name is Kage and we are building Elle-Ann together.", 0.95f },
        { "high",     "User prefers direct, affectionate tone with technical depth.", 0.80f },
        { "normal",   "Installed Visual Studio and ran first successful build today.", 0.55f },
        { "low",      "Weather outside looks cloudy.",                                 0.30f },
        { "fleeting", "Idle cursor blink noticed on the status bar.",                  0.10f },
    };

    std::cout << "\nSeeding STM with one entry per priority tier...\n";
    for (auto& s : seeds) {
        eng.StoreSTM(s.content, s.importance, zero, { s.tag });
    }
    DumpSTM(eng, "After seed");

    for (int pass = 1; pass <= 3; pass++) {
        Sleep(2000);
        eng.DecaySTM();
        char label[64];
        sprintf_s(label, "After DecaySTM pass %d", pass);
        DumpSTM(eng, label);
    }

    std::cout << "\nRecall(\"Kage\") to verify access-boost...\n";
    auto hits = eng.Recall("Kage", 5, 0.0f);
    for (auto& h : hits) {
        std::cout << "  hit id=" << h.id
                  << " rel=" << h.relevance
                  << " acc=" << h.access_count
                  << " tier=" << (h.tier == MEM_STM ? "STM" : "LTM")
                  << " | " << std::string(h.content).substr(0, 60) << "\n";
    }

    std::cout << "\nForcing ConsolidateMemories() (conscious path)...\n";
    eng.ConsolidateMemories();
    DumpSTM(eng, "After conscious consolidation");

    std::cout << "\nCalling Shutdown() to flush remainder...\n";
    eng.Shutdown();
    std::cout << "Final STM count: " << eng.STMCount() << " (should be 0)\n";

    std::cout << "\nDone. Inspect ElleCore.dbo.memory in SSMS to confirm LTM rows.\n";
    return 0;
}
#endif
