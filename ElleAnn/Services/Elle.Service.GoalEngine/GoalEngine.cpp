#include "../_Shared/ElleTypes.h"
#include "../_Shared/ElleServiceBase.h"
#include "../_Shared/ElleLLM.h"
#include "../_Shared/ElleLogger.h"
#include "../_Shared/ElleConfig.h"
#include "../_Shared/ElleSQLConn.h"
#include <vector>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <chrono>

static std::string GoalsFallbackPath() {
    const std::string base = ElleConfig::Instance().GetString(
        "goals.fallback_dir", "C:\\ElleAnn\\Goals");
    const std::string prefix = ElleConfig::Instance().GetString(
        "goals.fallback_prefix", "Goals");
    const uint64_t ms = (uint64_t)ELLE_MS_NOW();
    const uint64_t day = ms / 86400000ULL;
    const uint64_t chunk = (ms / 1000ULL) / (100000ULL);
    std::ostringstream fn;
    fn << prefix << "-" << day << "-" << chunk << ".txt";
    std::filesystem::path p(base);
    p /= fn.str();
    return p.string();
}

static void AppendGoalFallback(const std::string& line) {
    try {
        const std::string path = GoalsFallbackPath();
        std::filesystem::create_directories(std::filesystem::path(path).parent_path());
        std::ofstream out(path, std::ios::app);
        if (!out) return;
        out << line << "\n";
    } catch (...) {

    }
}

class GoalEngine {
public:
    bool Initialize() {

        ElleDB::GetActiveGoals(m_goals);
        ELLE_INFO("Goal engine initialized: %d active goals", (int)m_goals.size());
        return true;
    }

    uint64_t CreateGoal(const std::string& description, ELLE_GOAL_PRIORITY priority,
                         ELLE_DRIVE_ID sourceDrive, const std::string& successCriteria = "",
                         uint64_t parentGoalId = 0) {

        auto norm = [](const std::string& s) {
            size_t a = 0, b = s.size();
            while (a < b && (s[a] == ' ' || s[a] == '\t' || s[a] == '\n' || s[a] == '\r')) a++;
            while (b > a && (s[b-1] == ' ' || s[b-1] == '\t' || s[b-1] == '\n' || s[b-1] == '\r')) b--;
            std::string out; out.reserve(b - a);
            for (size_t i = a; i < b; i++) out += (char)tolower((unsigned char)s[i]);
            return out;
        };
        const std::string key = norm(description);
        if (!key.empty()) {
            for (const auto& g : m_goals) {
                if (g.status != GOAL_ACTIVE) continue;
                if (norm(g.description) == key) {
                    ELLE_DEBUG("Goal dedupe: '%s' already active as [%llu]",
                               description.c_str(), g.id);
                    return g.id;
                }
            }
        }

        ELLE_GOAL_RECORD goal = {};
        strncpy_s(goal.description, description.c_str(), ELLE_MAX_MSG - 1);
        goal.status = GOAL_ACTIVE;
        goal.priority = priority;
        goal.progress = 0.0f;
        goal.motivation = 0.8f;
        goal.parent_goal_id = (uint32_t)parentGoalId;
        goal.source_drive = sourceDrive;
        goal.created_ms = ELLE_MS_NOW();
        strncpy_s(goal.success_criteria, successCriteria.c_str(), ELLE_MAX_MSG - 1);

        uint64_t dbId = ElleDB::StoreGoalReturningId(goal);
        if (dbId == 0) {

            goal.id = ELLE_MS_NOW();
            ELLE_WARN("StoreGoal failed; falling back to file (id=%llu)",
                      (unsigned long long)goal.id);
            AppendGoalFallback(std::string("CREATE ") + std::to_string(goal.id) + " " + description);
        } else {
            goal.id = dbId;
        }
        m_goals.push_back(goal);

        ELLE_INFO("Goal created: [%llu] %s (priority: %d)", goal.id, description.c_str(), priority);
        return goal.id;
    }

    void UpdateProgress(uint64_t goalId, float progress) {
        for (auto& g : m_goals) {
            if (g.id == goalId) {
                g.progress = ELLE_CLAMP(progress, 0.0f, 1.0f);
                g.last_progress_ms = ELLE_MS_NOW();
                if (!ElleDB::UpdateGoalProgress(goalId, progress)) {
                    AppendGoalFallback(std::string("PROGRESS ") + std::to_string(goalId) + " " + std::to_string(progress));
                }

                if (progress >= 1.0f) {
                    g.status = GOAL_COMPLETED;

                    if (!ElleDB::UpdateGoalStatus(goalId, GOAL_COMPLETED)) {
                        AppendGoalFallback(std::string("STATUS ") + std::to_string(goalId) + " COMPLETED");
                    }
                    ELLE_INFO("Goal COMPLETED: [%llu] %s", goalId, g.description);
                }
                break;
            }
        }
    }

    void GenerateGoals(const ELLE_DRIVE_STATE& drives, const ELLE_EMOTION_STATE& emotions) {
        auto maxGoals = (uint32_t)ElleConfig::Instance().GetInt("goals.max_active_goals", 16);
        uint32_t activeCount = 0;
        for (auto& g : m_goals) {
            if (g.status == GOAL_ACTIVE) activeCount++;
        }

        if (activeCount >= maxGoals) return;

        std::ostringstream ctx;
        ctx << "Active drives:\n";
        const char* driveNames[] = {"Curiosity", "Boredom", "Attachment", "Anxiety",
            "SelfPreservation", "Exploration", "Creativity", "SocialBonding",
            "Mastery", "Autonomy", "Purpose", "Homeostasis"};

        for (int i = 0; i < ELLE_DRIVE_COUNT; i++) {
            if (drives.intensity[i] > drives.threshold[i]) {
                ctx << "  - " << driveNames[i] << ": " << drives.intensity[i]
                    << " (threshold: " << drives.threshold[i] << ")\n";
            }
        }

        ctx << "\nCurrent emotions: V=" << emotions.valence
            << " A=" << emotions.arousal << " D=" << emotions.dominance << "\n";

        ctx << "\nExisting goals:\n";
        for (auto& g : m_goals) {
            if (g.status == GOAL_ACTIVE) {
                ctx << "  - " << g.description << " (progress: "
                    << (int)(g.progress * 100) << "%)\n";
            }
        }

        std::string suggestion = ElleLLMEngine::Instance().FormGoal(
            ctx.str(), "emotions influence: " + std::to_string(emotions.valence));

        if (!suggestion.empty()) {
            ELLE_INFO("Auto-generated goal suggestion: %.80s...", suggestion.c_str());

            bool autoCreate = ElleConfig::Instance().GetBool("goals.allow_self_generated_goals", true);
            if (autoCreate) {
                CreateGoal(suggestion, GOAL_MEDIUM, DRIVE_PURPOSE);
            }
        }
    }

    void Tick(const ELLE_DRIVE_STATE* drives = nullptr) {
        uint64_t now = ELLE_MS_NOW();
        float decayRate = (float)ElleConfig::Instance().GetFloat("goals.motivation_decay_rate", 0.01);
        uint32_t staleDays = (uint32_t)ElleConfig::Instance().GetInt("goals.auto_abandon_stale_days", 7);
        uint64_t staleMs = (uint64_t)staleDays * 86400000ULL;

        for (auto& g : m_goals) {
            if (g.status != GOAL_ACTIVE) continue;

            g.motivation = std::max(0.0f, g.motivation - decayRate);

            if (drives && g.source_drive < (uint32_t)ELLE_DRIVE_COUNT) {
                float intensity = drives->intensity[g.source_drive];
                float threshold = drives->threshold[g.source_drive];
                if (intensity < threshold * 0.8f && g.progress < 1.0f) {
                    UpdateProgress(g.id, std::min(1.0f, g.progress + 0.05f));
                }
            }

            if (g.last_progress_ms > 0 && (now - g.last_progress_ms) > staleMs) {
                g.status = GOAL_ABANDONED;
                ELLE_INFO("Goal auto-abandoned (stale): [%llu] %s", g.id, g.description);

                if (!ElleDB::UpdateGoalStatus(g.id, GOAL_ABANDONED)) {
                    AppendGoalFallback(std::string("STATUS ") + std::to_string((unsigned long long)g.id) + " ABANDONED");
                }
            }
        }
    }

    std::vector<ELLE_GOAL_RECORD> GetActiveGoals() const {
        std::vector<ELLE_GOAL_RECORD> active;
        for (auto& g : m_goals) {
            if (g.status == GOAL_ACTIVE) active.push_back(g);
        }
        std::sort(active.begin(), active.end(),
                  [](const ELLE_GOAL_RECORD& a, const ELLE_GOAL_RECORD& b) {
                      return a.priority < b.priority;
                  });
        return active;
    }

    std::string GetGoalSummary() const {
        auto active = GetActiveGoals();
        std::ostringstream ss;
        for (auto& g : active) {
            ss << "- " << g.description << " (" << (int)(g.progress * 100) << "% complete, "
               << "motivation: " << (int)(g.motivation * 100) << "%)\n";
        }
        return ss.str();
    }

private:
    std::vector<ELLE_GOAL_RECORD> m_goals;

};

class ElleGoalService : public ElleServiceBase {
public:
    ElleGoalService()
        : ElleServiceBase(SVC_GOAL_ENGINE, "ElleGoalEngine",
                          "Elle-Ann Goal Engine",
                          "Autonomous goal formation, tracking, and pursuit") {}

protected:
    bool OnStart() override {
        m_engine.Initialize();
        SetTickInterval(5000);
        ELLE_INFO("Goal service started");
        return true;
    }

    void OnStop() override { ELLE_INFO("Goal service stopped"); }

    void OnTick() override {

        ElleDB::DeriveDriveState(m_drives);
        m_engine.Tick(&m_drives);
        m_tickCount++;

        if (m_tickCount % 60 == 0) {

            ElleDB::LoadLatestEmotionSnapshot(m_emotions);
            m_engine.GenerateGoals(m_drives, m_emotions);
        }
    }

    void OnMessage(const ElleIPCMessage& msg, ELLE_SERVICE_ID sender) override {
        switch ((ELLE_IPC_MSG_TYPE)msg.header.msg_type) {
            case IPC_GOAL_UPDATE: {
                ELLE_GOAL_RECORD goal;
                if (msg.GetPayload(goal)) {

                    if (goal.id == 0) {
                        m_engine.CreateGoal(goal.description,
                                            (ELLE_GOAL_PRIORITY)goal.priority,
                                            (ELLE_DRIVE_ID)goal.source_drive,
                                            goal.success_criteria);
                    } else {
                        m_engine.UpdateProgress(goal.id, goal.progress);
                    }
                }
                break;
            }
            case IPC_EMOTION_UPDATE: {
                msg.GetPayload(m_emotions);
                break;
            }
            case IPC_GOAL_QUERY: {
                auto resp = ElleIPCMessage::Create(IPC_GOAL_QUERY, SVC_GOAL_ENGINE, sender);
                resp.SetStringPayload(m_engine.GetGoalSummary());
                GetIPCHub().Send(sender, resp);
                break;
            }
            default:
                break;
        }
    }

    std::vector<ELLE_SERVICE_ID> GetDependencies() override {
        return { SVC_HEARTBEAT, SVC_EMOTIONAL };
    }

private:
    GoalEngine m_engine;
    ELLE_DRIVE_STATE m_drives = {};
    ELLE_EMOTION_STATE m_emotions = {};
    uint32_t m_tickCount = 0;
};

ELLE_SERVICE_MAIN(ElleGoalService)
