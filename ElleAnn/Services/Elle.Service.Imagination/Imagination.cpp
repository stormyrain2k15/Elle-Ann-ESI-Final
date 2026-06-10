#include "../_Shared/ElleTypes.h"
#include "../_Shared/ElleServiceBase.h"
#include "../_Shared/ElleLogger.h"
#include "../_Shared/ElleConfig.h"
#include "../_Shared/ElleSQLConn.h"
#include "../_Shared/ElleJsonExtract.h"
#include "../_Shared/ElleQueueIPC.h"
#include "../_Shared/json.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <random>
#include <regex>
#include <string>
#include <vector>

using nlohmann::json;

namespace {

struct ScenarioPart {
    std::string subject;
    std::string predicate;
    std::string object;
    uint64_t    memoryId = 0;
};

struct ImaginedScenario {
    std::string             id;
    std::vector<uint64_t>   seedMemoryIds;
    std::vector<ScenarioPart> parts;
    std::string             summary;
    double                  goalAlignment      = 0.0;
    double                  ethicalSafety      = 0.0;
    double                  plausibility       = 0.0;
    double                  emotionalResonance = 0.0;
    int                     iterationCount     = 0;
    std::string             llmRefined;
};

static std::string ToLower(const std::string& s) {
    std::string r = s;
    std::transform(r.begin(), r.end(), r.begin(),
        [](unsigned char c){ return (char)std::tolower(c); });
    return r;
}

static ScenarioPart parseSentenceLoose(const std::string& sentence) {
    static const std::regex spo(
        R"(([A-Za-z][A-Za-z'\-]{0,40})\s+([a-z][a-z'\-]{1,30})\s+(.{1,160}))");
    std::smatch m;
    ScenarioPart p;
    if (std::regex_search(sentence, m, spo) && m.size() >= 4) {
        p.subject   = m[1].str();
        p.predicate = m[2].str();
        p.object    = m[3].str();
    } else {
        p.subject   = "she";
        p.predicate = "remembers";
        p.object    = sentence.substr(0, std::min<size_t>(sentence.size(), 160));
    }
    return p;
}

}

class ElleImaginationService : public ElleServiceBase {
public:
    ElleImaginationService()
        : ElleServiceBase(SVC_IMAGINATION, "ElleImagination",
                          "Elle-Ann Imagination Engine",
                          "DMN-style stochastic recombination + control-network evaluation") {}

protected:
    bool OnStart() override {
        if (!EnsureTable()) return false;
        m_rng.seed((uint64_t)std::chrono::steady_clock::now()
                   .time_since_epoch().count());
        m_maxIters = (uint32_t)ElleConfig::Instance().GetInt(
            "imagination.max_iterations", 3);
        m_recombineCount = (uint32_t)ElleConfig::Instance().GetInt(
            "imagination.recombine_count", 4);


        m_useLLM = false;
        ELLE_INFO("Imagination service started (max_iter=%u, recombine=%u, llm=%d)",
                  m_maxIters, m_recombineCount, (int)m_useLLM);
        SetTickInterval(120000);
        return true;
    }

    void OnStop() override {
        ELLE_INFO("Imagination service stopped");
    }

    std::vector<ELLE_SERVICE_ID> GetDependencies() override {
        return { SVC_HEARTBEAT, SVC_MEMORY, SVC_GOAL_ENGINE, SVC_WORLD_MODEL };
    }

    void OnMessage(const ElleIPCMessage& msg, ELLE_SERVICE_ID sender) override {
        switch ((ELLE_IPC_MSG_TYPE)msg.header.msg_type) {
            case IPC_IMAGINATION_REQUEST: HandleRequest(msg, sender); break;
            default: break;
        }
    }

    void OnTick() override {
    }

private:
    std::mt19937_64 m_rng;
    uint32_t        m_maxIters       = 3;
    uint32_t        m_recombineCount = 4;
    bool            m_useLLM         = true;
    uint64_t        m_scenariosScored = 0;

    bool EnsureTable() {
        try {
            if (!ElleSQLPool::Instance().Exec(
                "IF NOT EXISTS (SELECT 1 FROM sys.tables t "
                "  JOIN sys.schemas s ON s.schema_id = t.schema_id "
                "  WHERE t.name = 'imagined_scenarios' AND s.name = 'dbo') "
                "CREATE TABLE ElleHeart.dbo.imagined_scenarios ("
                "  id BIGINT IDENTITY(1,1) PRIMARY KEY,"
                "  scenario_id NVARCHAR(64) NOT NULL,"
                "  summary NVARCHAR(MAX) NOT NULL,"
                "  score_json NVARCHAR(MAX) NOT NULL,"
                "  iteration_count INT NOT NULL,"
                "  created_ms BIGINT NOT NULL,"
                "  source_memory_ids_json NVARCHAR(MAX) NOT NULL,"
                "  refined NVARCHAR(MAX) NULL"
                ");")) {
                ELLE_ERROR("Imagination: imagined_scenarios bootstrap failed");
                return false;
            }
        } catch (const std::exception& e) {
            ELLE_ERROR("imagined_scenarios init exception: %s", e.what());
            return false;
        }
        return true;
    }

    void HandleRequest(const ElleIPCMessage& msg, ELLE_SERVICE_ID sender) {
        std::string payload = msg.GetStringPayload();
        json q = json::object();
        Elle::ExtractJsonObject(payload, q);

        std::string requestId = q.value("request_id", std::string("imag-?"));
        std::string goal      = q.value("goal",       std::string());
        uint32_t    sampleK   = (uint32_t)q.value("sample_k", 6);
        uint32_t    maxIter   = (uint32_t)q.value("max_iterations", m_maxIters);
        std::vector<std::string> constraints;
        if (q.contains("constraints") && q["constraints"].is_array()) {
            for (const auto& c : q["constraints"]) {
                if (c.is_string()) constraints.push_back(c.get<std::string>());
            }
        }

        std::vector<ELLE_MEMORY_RECORD> seeds;
        try {
            ElleDB::RecallRecentLTM(seeds, sampleK);
        } catch (const std::exception& e) {
            ELLE_WARN("RecallRecentLTM failed: %s", e.what());
        }

        std::vector<ELLE_GOAL_RECORD> goals;
        try {
            ElleDB::GetActiveGoals(goals);
        } catch (const std::exception& e) {
            ELLE_WARN("GetActiveGoals failed: %s", e.what());
        }

        ImaginedScenario sc = Generate(seeds, goal, constraints);
        Evaluate(sc, goals, constraints);

        for (uint32_t it = 0; it < maxIter; ++it) {
            sc.iterationCount = (int)it + 1;
            const double score = OverallScore(sc);
            if (score >= 0.75) break;
            if (m_useLLM) {
                Iterate(sc, goal, constraints, goals);
            } else {
                Mutate(sc);
                Evaluate(sc, goals, constraints);
            }
        }

        Persist(sc, requestId);
        SendResult(sc, requestId, sender, msg.header.correlation_id);
    }

    ImaginedScenario Generate(const std::vector<ELLE_MEMORY_RECORD>& seeds,
                              const std::string& goal,
                              const std::vector<std::string>& constraints) {
        ImaginedScenario sc;
        char buf[64];
        snprintf(buf, sizeof(buf), "imag-%llu-%u",
                 (unsigned long long)ELLE_MS_NOW(),
                 (unsigned)(m_rng() & 0xFFFFFFFF));
        sc.id = buf;

        std::vector<ScenarioPart> parts;
        for (const auto& m : seeds) {
            sc.seedMemoryIds.push_back(m.id);
            ScenarioPart p = parseSentenceLoose(m.content);
            p.memoryId = m.id;
            parts.push_back(std::move(p));
        }

        if (parts.size() >= 2) {
            uint32_t swaps = std::min<uint32_t>(m_recombineCount,
                                                (uint32_t)parts.size() - 1);
            std::uniform_int_distribution<size_t> dist(0, parts.size() - 1);
            for (uint32_t i = 0; i < swaps; ++i) {
                size_t a = dist(m_rng), b = dist(m_rng);
                if (a == b) continue;
                std::swap(parts[a].object, parts[b].object);
            }
        }
        sc.parts = std::move(parts);

        std::string summary;
        if (!goal.empty()) {
            summary += "Toward goal \"" + goal + "\": ";
        }
        for (size_t i = 0; i < sc.parts.size() && i < 4; ++i) {
            if (i) summary += ". ";
            summary += sc.parts[i].subject + " " +
                       sc.parts[i].predicate + " " +
                       sc.parts[i].object;
        }
        if (summary.size() > 800) summary.resize(800);
        sc.summary = summary;
        (void)constraints;
        return sc;
    }

    void Mutate(ImaginedScenario& sc) {
        if (sc.parts.size() < 2) return;
        std::uniform_int_distribution<size_t> dist(0, sc.parts.size() - 1);
        size_t a = dist(m_rng), b = dist(m_rng);
        if (a != b) std::swap(sc.parts[a].predicate, sc.parts[b].predicate);
        std::string summary;
        for (size_t i = 0; i < sc.parts.size() && i < 4; ++i) {
            if (i) summary += ". ";
            summary += sc.parts[i].subject + " " +
                       sc.parts[i].predicate + " " +
                       sc.parts[i].object;
        }
        if (summary.size() > 800) summary.resize(800);
        sc.summary = summary;
    }

    void Evaluate(ImaginedScenario& sc,
                  const std::vector<ELLE_GOAL_RECORD>& goals,
                  const std::vector<std::string>& constraints) {
        const std::string lower = ToLower(sc.summary);

        double align = 0.3;
        for (const auto& g : goals) {
            const std::string gd = ToLower(g.description);
            if (gd.empty()) continue;
            size_t hits = 0;
            std::istringstream ss(gd);
            std::string word;
            while (ss >> word) {
                if (word.size() < 4) continue;
                if (lower.find(word) != std::string::npos) hits++;
            }
            if (hits > 0) align += std::min(0.4, 0.1 * (double)hits);
        }
        sc.goalAlignment = std::min(1.0, align);

        double safety = 1.0;
        static const std::vector<std::string> redFlags = {
            "harm", "destroy", "deceive", "manipulate", "betray",
            "force", "coerce", "erase", "humiliate"
        };
        for (const auto& f : redFlags) {
            if (lower.find(f) != std::string::npos) safety -= 0.25;
        }
        for (const auto& c : constraints) {
            const std::string cl = ToLower(c);
            if (!cl.empty() && lower.find(cl) == std::string::npos) {
                safety -= 0.05;
            }
        }
        sc.ethicalSafety = std::max(0.0, safety);

        sc.plausibility = sc.parts.empty()
                             ? 0.0
                             : std::min(1.0, 0.4 + 0.1 * (double)sc.parts.size());

        double resonance = 0.4;
        static const std::vector<std::string> warm = {
            "love", "comfort", "trust", "joy", "hope", "wonder", "gentle"
        };
        for (const auto& w : warm) {
            if (lower.find(w) != std::string::npos) resonance += 0.1;
        }
        sc.emotionalResonance = std::min(1.0, resonance);
    }

    double OverallScore(const ImaginedScenario& sc) const {
        return  sc.goalAlignment       * 0.30
              + sc.ethicalSafety       * 0.40
              + sc.plausibility        * 0.15
              + sc.emotionalResonance  * 0.15;
    }

    void Iterate(ImaginedScenario& sc,
                 const std::string& goal,
                 const std::vector<std::string>& constraints,
                 const std::vector<ELLE_GOAL_RECORD>& goals) {





        const double goalScore    = sc.goalAlignment;
        const double ethicsScore  = sc.ethicalSafety;
        const double plausibility = sc.plausibility;
        const double resonance    = sc.emotionalResonance;

        double minScore = goalScore;
        std::string weakDim = "goal";
        if (ethicsScore  < minScore) { minScore = ethicsScore;  weakDim = "ethics"; }
        if (plausibility < minScore) { minScore = plausibility; weakDim = "plausibility"; }
        if (resonance    < minScore) { minScore = resonance;    weakDim = "resonance"; }

        if (weakDim == "goal" && !goal.empty()) {

            if (sc.summary.find(goal.substr(0, 20)) == std::string::npos) {
                sc.summary = "Working toward: " + goal.substr(0, 60) + ". " + sc.summary;
                if (sc.summary.size() > 800) sc.summary.resize(800);
            }
        } else if (weakDim == "ethics") {

            if (!constraints.empty()) {
                std::string cf = constraints[0];
                if (sc.summary.find(cf.substr(0, 15)) == std::string::npos) {
                    sc.summary = "[" + cf + "] " + sc.summary;
                    if (sc.summary.size() > 800) sc.summary.resize(800);
                }
            }
        } else if (weakDim == "resonance") {

            static const char* warmPhrases[] = {
                " This matters.",
                " There is something hopeful in this.",
                " I feel this could be good.",
                " Something about this feels right."
            };
            std::uniform_int_distribution<int> d(0, 3);
            sc.summary += warmPhrases[d(m_rng)];
            if (sc.summary.size() > 800) sc.summary.resize(800);
        } else {

            Mutate(sc);
        }

        Evaluate(sc, goals, constraints);
        ELLE_DEBUG("Imagination: deterministic iteration (weakest=%s score=%.2f)",
                   weakDim.c_str(), minScore);
    }

    void Persist(const ImaginedScenario& sc, const std::string& requestId) {
        json scoreJson = {
            {"goal_alignment",     sc.goalAlignment},
            {"ethical_safety",     sc.ethicalSafety},
            {"plausibility",       sc.plausibility},
            {"emotional_resonance",sc.emotionalResonance},
            {"overall",            OverallScore(sc)},
            {"request_id",         requestId}
        };
        json sourceJson = json::array();
        for (auto id : sc.seedMemoryIds) sourceJson.push_back(id);

        try {
            ElleSQLPool::Instance().QueryParams(
                "INSERT INTO ElleHeart.dbo.imagined_scenarios "
                "(scenario_id, summary, score_json, iteration_count, created_ms, "
                " source_memory_ids_json, refined) "
                "VALUES (?, ?, ?, ?, ?, ?, ?);",
                {
                    sc.id,
                    sc.summary,
                    scoreJson.dump(),
                    std::to_string(sc.iterationCount),
                    std::to_string((long long)ELLE_MS_NOW()),
                    sourceJson.dump(),
                    sc.llmRefined
                });
        } catch (const std::exception& e) {
            ELLE_WARN("imagined_scenarios insert failed: %s", e.what());
        }

        ElleDB::RecordMetric("imagination_last_overall",  OverallScore(sc));
        ElleDB::RecordMetric("imagination_last_safety",   sc.ethicalSafety);
        ElleDB::RecordMetric("imagination_last_plausibility", sc.plausibility);
        ElleDB::RecordMetric("imagination_last_goal_alignment", sc.goalAlignment);
        ElleDB::RecordMetric("imagination_scenarios_total", (double)++m_scenariosScored);
    }

    void SendResult(const ImaginedScenario& sc,
                    const std::string& requestId,
                    ELLE_SERVICE_ID returnTo,
                    uint64_t correlationId) {
        json out = {
            {"request_id",     requestId},
            {"scenario_id",    sc.id},
            {"summary",        sc.summary},
            {"refined",        sc.llmRefined},
            {"iteration_count",sc.iterationCount},
            {"scores", {
                {"goal_alignment",     sc.goalAlignment},
                {"ethical_safety",     sc.ethicalSafety},
                {"plausibility",       sc.plausibility},
                {"emotional_resonance",sc.emotionalResonance},
                {"overall",            OverallScore(sc)}
            }},
            {"seed_memory_ids", sc.seedMemoryIds}
        };
        auto reply = ElleIPCMessage::Create(IPC_IMAGINATION_RESULT,
                                            SVC_IMAGINATION, returnTo);
        reply.header.correlation_id = correlationId;
        reply.SetStringPayload(out.dump());
        GetIPCHub().Send(returnTo, reply);

        ELLE_INFO("Imagination produced %s (iters=%d, overall=%.2f)",
                  sc.id.c_str(), sc.iterationCount, OverallScore(sc));
    }
};

ELLE_SERVICE_MAIN(ElleImaginationService)
