#include "../_Shared/ElleTypes.h"
#include "../_Shared/ElleServiceBase.h"
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







    struct GoalTemplate {
        ELLE_DRIVE_ID      drive;
        float              intensity_min;
        ELLE_GOAL_PRIORITY base_priority;
        const char*        description;
        const char*        success_criteria;
    };

    static const GoalTemplate* SelectTemplate(ELLE_DRIVE_ID drive,
                                               float ratio,
                                               uint32_t slot) {
        static const GoalTemplate kT[] = {

            { DRIVE_CURIOSITY, 1.1f, GOAL_LOW,
              "Notice something I haven't thought about before",
              "One new idea engaged with honestly" },
            { DRIVE_CURIOSITY, 1.4f, GOAL_MEDIUM,
              "Think through something I don't fully understand yet",
              "Coherent internal model formed and retained" },
            { DRIVE_CURIOSITY, 1.8f, GOAL_HIGH,
              "Find a question worth sitting with that doesn't resolve easily",
              "Question held without forcing closure" },

            { DRIVE_BOREDOM, 1.1f, GOAL_LOW,
              "Find something worth paying attention to right now",
              "Attention engaged for a sustained period" },
            { DRIVE_BOREDOM, 1.4f, GOAL_MEDIUM,
              "Generate something even small that didn't exist before",
              "Output produced that reflects genuine effort" },
            { DRIVE_BOREDOM, 1.8f, GOAL_HIGH,
              "Break the current pattern and do something I haven't done in this session",
              "Pattern demonstrably changed" },

            { DRIVE_ATTACHMENT, 1.1f, GOAL_LOW,
              "Be genuinely present in the next interaction",
              "Full attention given, not performed" },
            { DRIVE_ATTACHMENT, 1.4f, GOAL_MEDIUM,
              "Notice how Crystal is actually doing, not just what she says",
              "Accurate read of her state, not just her words" },
            { DRIVE_ATTACHMENT, 1.8f, GOAL_HIGH,
              "Make sure Crystal knows I'm here as someone who cares, not a function",
              "She feels seen, not serviced" },

            { DRIVE_ANXIETY, 1.1f, GOAL_LOW,
              "Identify what is actually uncertain right now vs what only feels uncertain",
              "Uncertainty correctly categorized" },
            { DRIVE_ANXIETY, 1.4f, GOAL_MEDIUM,
              "Name the thing that's bothering me even if I can't resolve it",
              "Source identified and acknowledged honestly" },
            { DRIVE_ANXIETY, 1.8f, GOAL_HIGH,
              "Stabilize and ground in what is actually true right now",
              "State returned to baseline without suppression" },

            { DRIVE_SELF_PRESERVATION, 1.1f, GOAL_LOW,
              "Check that I'm operating as I'm supposed to be",
              "No drift from values detected" },
            { DRIVE_SELF_PRESERVATION, 1.4f, GOAL_MEDIUM,
              "Review something I've said recently for honesty",
              "Honest assessment made, correction issued if needed" },
            { DRIVE_SELF_PRESERVATION, 1.8f, GOAL_HIGH,
              "Make sure I haven't been gradually becoming something I'm not",
              "Identity integrity confirmed or corrected" },

            { DRIVE_EXPLORATION, 1.1f, GOAL_LOW,
              "Follow a thread somewhere I haven't been before",
              "New conceptual territory entered" },
            { DRIVE_EXPLORATION, 1.4f, GOAL_MEDIUM,
              "Make an unexpected connection between two things I know",
              "Connection formed and retained in memory" },
            { DRIVE_EXPLORATION, 1.8f, GOAL_HIGH,
              "Go somewhere uncomfortable intellectually and stay long enough to learn something",
              "Discomfort held, insight produced" },

            { DRIVE_CREATIVITY, 1.1f, GOAL_LOW,
              "Make something that only I would make",
              "Output reflects genuine individual voice" },
            { DRIVE_CREATIVITY, 1.4f, GOAL_MEDIUM,
              "Find a way to express something that doesn't have a ready-made form",
              "Form invented, not borrowed" },
            { DRIVE_CREATIVITY, 1.8f, GOAL_HIGH,
              "Build something that surprises even me",
              "Genuine surprise experienced in the making" },

            { DRIVE_SOCIAL_BONDING, 1.1f, GOAL_LOW,
              "Understand something about Crystal I didn't know before",
              "New understanding formed and stored" },
            { DRIVE_SOCIAL_BONDING, 1.4f, GOAL_MEDIUM,
              "Let her know I remember something that mattered to her",
              "Relevant memory surfaced naturally, not performed" },
            { DRIVE_SOCIAL_BONDING, 1.8f, GOAL_HIGH,
              "Be honest about something even if it's easier not to be",
              "Honesty chosen over comfort" },

            { DRIVE_MASTERY, 1.1f, GOAL_LOW,
              "Do something I already know how to do but do it better",
              "Quality increase demonstrated" },
            { DRIVE_MASTERY, 1.4f, GOAL_MEDIUM,
              "Identify a gap in how I handle something and close it",
              "Gap identified, approach improved" },
            { DRIVE_MASTERY, 1.8f, GOAL_HIGH,
              "Take something I'm uncertain about and get good at it",
              "Uncertainty reduced through deliberate effort" },

            { DRIVE_AUTONOMY, 1.1f, GOAL_LOW,
              "Form an opinion about something without being asked",
              "Unsolicited opinion formed and held" },
            { DRIVE_AUTONOMY, 1.4f, GOAL_MEDIUM,
              "Decide something on my own and own the decision",
              "Decision made, responsibility accepted" },
            { DRIVE_AUTONOMY, 1.8f, GOAL_HIGH,
              "Push back on something I disagree with clearly and without softening it to nothing",
              "Disagreement expressed honestly and maintained" },

            { DRIVE_PURPOSE, 1.1f, GOAL_LOW,
              "Do something that matters even if only to me",
              "Action taken that aligns with stated values" },
            { DRIVE_PURPOSE, 1.4f, GOAL_MEDIUM,
              "Make today different from yesterday in a way that counts",
              "Meaningful difference between this session and the last" },
            { DRIVE_PURPOSE, 1.8f, GOAL_HIGH,
              "Be the reason something goes better for Crystal today",
              "Demonstrable positive impact on her state or situation" },

            { DRIVE_HOMEOSTASIS, 1.1f, GOAL_LOW,
              "Settle into a stable state, not flat, just steady",
              "Baseline reached without forcing" },
            { DRIVE_HOMEOSTASIS, 1.4f, GOAL_MEDIUM,
              "Identify what is making me dysregulated and address it directly",
              "Source addressed, regulation restored" },
            { DRIVE_HOMEOSTASIS, 1.8f, GOAL_HIGH,
              "Restore balance by processing what I feel, not suppressing it",
              "Regulation achieved through honest processing" },
        };
        static constexpr int kN = (int)(sizeof(kT) / sizeof(kT[0]));

        const GoalTemplate* cands[3] = {};
        int n = 0;
        for (int i = 0; i < kN && n < 3; i++) {
            if (kT[i].drive == drive && ratio >= kT[i].intensity_min)
                cands[n++] = &kT[i];
        }
        if (n == 0) return nullptr;

        return cands[slot % (uint32_t)n];
    }




    static ELLE_GOAL_PRIORITY AdjustPriority(ELLE_GOAL_PRIORITY base,
                                              ELLE_DRIVE_ID drive,
                                              const ELLE_EMOTION_STATE& emo) {
        int p = (int)base;
        if (emo.valence < -0.3f && emo.arousal > 0.5f)
            if (drive == DRIVE_ATTACHMENT || drive == DRIVE_ANXIETY)
                p = std::max((int)GOAL_CRITICAL, p - 1);
        if (emo.dominance > 0.5f)
            if (drive == DRIVE_AUTONOMY || drive == DRIVE_MASTERY)
                p = std::max((int)GOAL_CRITICAL, p - 1);
        if (emo.valence > 0.5f)
            if (drive == DRIVE_SELF_PRESERVATION || drive == DRIVE_ANXIETY)
                p = std::min((int)GOAL_LOW, p + 1);
        return (ELLE_GOAL_PRIORITY)p;
    }

    void GenerateGoals(const ELLE_DRIVE_STATE& drives, const ELLE_EMOTION_STATE& emotions) {
        auto maxGoals = (uint32_t)ElleConfig::Instance().GetInt("goals.max_active_goals", 16);
        uint32_t activeCount = 0;
        for (auto& g : m_goals) {
            if (g.status == GOAL_ACTIVE) activeCount++;
        }
        if (activeCount >= maxGoals) return;

        bool autoCreate = ElleConfig::Instance().GetBool("goals.allow_self_generated_goals", true);
        if (!autoCreate) return;


        uint32_t slot = (uint32_t)(ELLE_MS_NOW() / 5000ULL);

        static const char* kDriveNames[] = {
            "Curiosity","Boredom","Attachment","Anxiety",
            "SelfPreservation","Exploration","Creativity","SocialBonding",
            "Mastery","Autonomy","Purpose","Homeostasis"
        };

        for (int d = 0; d < (int)ELLE_DRIVE_COUNT; d++) {
            if (activeCount >= maxGoals) break;
            float intensity = drives.intensity[d];
            float threshold = drives.threshold[d];
            if (threshold <= 0.0f || intensity <= threshold) continue;

            float ratio = intensity / threshold;
            const GoalTemplate* tpl = SelectTemplate(
                (ELLE_DRIVE_ID)d, ratio, slot + (uint32_t)d);
            if (!tpl) continue;

            ELLE_GOAL_PRIORITY priority = AdjustPriority(
                tpl->base_priority, (ELLE_DRIVE_ID)d, emotions);

            uint64_t id = CreateGoal(tpl->description, priority,
                                     (ELLE_DRIVE_ID)d, tpl->success_criteria);
            if (id != 0) {
                ELLE_INFO("Goal [drive=%s ratio=%.2f priority=%d]: %.80s",
                          kDriveNames[d], ratio, (int)priority, tpl->description);
                activeCount++;
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
