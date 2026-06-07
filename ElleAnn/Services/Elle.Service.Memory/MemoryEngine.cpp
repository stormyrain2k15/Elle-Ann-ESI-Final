#include "MemoryEngine.h"
#include "../_Shared/ElleLogger.h"
#include "../_Shared/ElleConfig.h"
#include "../_Shared/ElleSQLConn.h"
#include "../_Shared/ElleLLM.h"
#include "../_Shared/ElleWait.h"
#include <algorithm>
#include <cmath>
#include <sstream>
#include <set>
#include <array>

namespace {
    constexpr float kDecayRatePerSec[5] = {
        0.0000f,
        0.0005f,
        0.0020f,
        0.0080f,
        0.0500f,
    };
    constexpr float kAccessBoost[5] = {
        0.00f,
        0.30f,
        0.40f,
        0.50f,
        0.60f,
    };

    constexpr float kPromotionRelevance  = 0.15f;

    constexpr float kMinRelevanceFloor   = 0.05f;

    inline int PriorityTier(float importance) {
        if (importance >= 0.90f) return 0;
        if (importance >= 0.70f) return 1;
        if (importance >= 0.40f) return 2;
        if (importance >= 0.20f) return 3;
        return 4;
    }

    inline const char* PriorityLabel(int t) {
        static const char* L[] = { "critical", "high", "normal", "low", "fleeting" };
        return (t >= 0 && t <= 4) ? L[t] : "unknown";
    }
}

MemoryEngine::MemoryEngine() {}
MemoryEngine::~MemoryEngine() { Shutdown(); }

bool MemoryEngine::Initialize() {
    ELLE_INFO("Memory Engine initializing...");
    ELLE_INFO("  STM capacity: %d", ElleConfig::Instance().GetMemory().stm_capacity);
    ELLE_INFO("  STM decay: %.1fs", ElleConfig::Instance().GetMemory().stm_decay_seconds);
    ELLE_INFO("  Consolidation interval: %dm", ElleConfig::Instance().GetMemory().consolidation_interval_min);
    return true;
}

void MemoryEngine::Shutdown() {

    std::deque<ELLE_MEMORY_RECORD> snapshot;
    {
        std::lock_guard<std::mutex> lock(m_stmMutex);
        if (m_stm.empty()) return;
        snapshot.swap(m_stm);
    }

    uint32_t flushed = 0;
    for (auto& e : snapshot) {
        e.tier = MEM_LTM;
        if (ElleDB::StoreMemory(e)) flushed++;
    }
    ELLE_INFO("Memory shutdown: flushed %u STM entries to LTM", flushed);
}

uint64_t MemoryEngine::StoreSTM(const std::string& content, float importance,
                                  const float emotions[ELLE_MAX_EMOTIONS],
                                  const std::vector<std::string>& tags) {
    ELLE_MEMORY_RECORD mem = {};

    mem.type = MEM_EPISODIC;
    mem.tier = MEM_STM;
    strncpy_s(mem.content, content.c_str(), ELLE_MAX_MSG - 1);
    mem.importance = importance;
    mem.relevance = 1.0f;
    mem.decay = 0.0f;
    mem.created_ms = ELLE_MS_NOW();
    mem.last_access_ms = mem.created_ms;
    mem.access_count = 0;

    if (emotions) {
        memcpy(mem.emotion_snapshot, emotions, sizeof(float) * ELLE_MAX_EMOTIONS);

        float v = 0.0f;
        for (int i = 0; i < ELLE_EMOTION_COUNT; i++) v += emotions[i];
        mem.emotional_valence = v / (float)ELLE_EMOTION_COUNT;
    }

    mem.tag_count = std::min((uint32_t)tags.size(), (uint32_t)ELLE_MAX_TAGS);
    for (uint32_t i = 0; i < mem.tag_count; i++) {
        strncpy_s(mem.tags[i], tags[i].c_str(), ELLE_MAX_TAG - 1);
    }

    ComputePosition(mem);

    std::vector<ELLE_MEMORY_RECORD> toPromote;
    uint64_t assignedId = 0;
    {
        std::lock_guard<std::mutex> lock(m_stmMutex);
        mem.id = m_nextId++;
        assignedId = mem.id;
        m_stm.push_back(mem);

        auto cap = ElleConfig::Instance().GetMemory().stm_capacity;
        while (m_stm.size() > cap) {
            auto victim = m_stm.end();
            float lowestRel = 2.0f;
            for (auto it = m_stm.begin(); it != m_stm.end(); ++it) {
                if (PriorityTier(it->importance) == 0) continue;
                if (it->relevance < lowestRel) {
                    lowestRel = it->relevance;
                    victim = it;
                }
            }
            if (victim == m_stm.end()) {

                victim = m_stm.begin();
            }
            ELLE_MEMORY_RECORD copy = *victim;
            copy.tier = MEM_LTM;
            toPromote.push_back(copy);
            m_stm.erase(victim);
        }
    }

    for (auto& rec : toPromote) {
        if (!ElleDB::StoreMemory(rec)) {
            ELLE_WARN("STM capacity eviction: LTM write failed for [%llu] — record lost", rec.id);
        } else {
            ELLE_DEBUG("STM capacity pressure: promoted [%llu] %.50s... (rel=%.2f)",
                       rec.id, rec.content, rec.relevance);
        }
    }

    ELLE_DEBUG("STM stored: [%llu] %.50s... (importance=%.2f)", assignedId, content.c_str(), importance);
    return assignedId;
}

uint64_t MemoryEngine::StoreLTM(const ELLE_MEMORY_RECORD& record) {
    ELLE_MEMORY_RECORD mem = record;
    if (mem.id == 0) mem.id = m_nextId++;
    mem.tier = MEM_LTM;
    if (mem.created_ms == 0) mem.created_ms = ELLE_MS_NOW();

    ComputePosition(mem);

    if (!ElleDB::StoreMemory(mem)) {
        ELLE_ERROR("LTM store FAILED (id=%llu, content=%.50s...) -- "
                   "returning 0 so caller can react",
                   (unsigned long long)mem.id, mem.content);
        return 0;
    }

    ELLE_DEBUG("LTM stored: [%llu] %.50s...", mem.id, mem.content);
    return mem.id;
}

std::vector<ELLE_MEMORY_RECORD> MemoryEngine::Recall(const std::string& query,
                                                       uint32_t maxResults,
                                                       float minRelevance) {
    std::vector<ELLE_MEMORY_RECORD> results;

    {
        std::lock_guard<std::mutex> lock(m_stmMutex);
        for (auto& mem : m_stm) {
            float rel = ComputeRelevance(mem, query);
            if (rel >= minRelevance) {
                ELLE_MEMORY_RECORD copy = mem;
                copy.relevance = rel;
                results.push_back(copy);
            }
        }
    }

    std::vector<ELLE_MEMORY_RECORD> ltmResults;
    ElleDB::RecallMemories(query, ltmResults, maxResults, minRelevance);
    results.insert(results.end(), ltmResults.begin(), ltmResults.end());

    std::sort(results.begin(), results.end(),
              [](const ELLE_MEMORY_RECORD& a, const ELLE_MEMORY_RECORD& b) {
                  return a.relevance > b.relevance;
              });

    if (results.size() > maxResults) results.resize(maxResults);

    uint64_t now = ELLE_MS_NOW();
    for (auto& mem : results) {
        mem.access_count++;
        mem.last_access_ms = now;
        if (mem.tier == MEM_STM) {
            float boost = kAccessBoost[PriorityTier(mem.importance)];
            mem.relevance = std::min(1.0f, mem.relevance + boost);

            std::lock_guard<std::mutex> lock(m_stmMutex);
            for (auto& live : m_stm) {
                if (live.id == mem.id) {
                    live.access_count = mem.access_count;
                    live.last_access_ms = mem.last_access_ms;
                    live.relevance = mem.relevance;
                    break;
                }
            }
        } else if (mem.tier == MEM_LTM) {
            ElleDB::UpdateMemoryAccess(mem.id);
        }
    }

    return results;
}

void MemoryEngine::ConsolidateMemories() {
    auto  cfg       = ElleConfig::Instance().GetMemory();
    float impFast   = cfg.promote_threshold;

    std::vector<ELLE_MEMORY_RECORD> toWrite;
    uint32_t dropped = 0;

    {
        std::lock_guard<std::mutex> lock(m_stmMutex);

        {
            uint64_t now = ELLE_MS_NOW();
            float    scale = (cfg.stm_decay_seconds > 0.0f) ? (15.0f / cfg.stm_decay_seconds) : 1.0f;
            for (auto& e : m_stm) {
                int tier = PriorityTier(e.importance);
                float rate = kDecayRatePerSec[tier] * scale;
                if (rate <= 0.0f) continue;

                uint64_t anchor = e.last_access_ms > 0 ? e.last_access_ms : e.created_ms;
                float age_sec = (float)(now - anchor) / 1000.0f;
                float rel = 1.0f - (rate * age_sec);
                if (e.access_count > 0) {
                    float bump = std::min(0.5f, e.access_count * 0.05f);
                    rel += bump;
                }
                e.decay     = 1.0f - rel;
                e.relevance = ELLE_CLAMP(rel, 0.0f, 1.0f);
            }
        }

        auto it = m_stm.begin();
        while (it != m_stm.end()) {
            int  tier          = PriorityTier(it->importance);
            bool criticalTier  = (tier == 0);
            bool impFastTrack  = (it->importance >= impFast);
            bool accessFreq    = (it->access_count >= 3);
            bool strongEmotion = (it->emotional_valence > 0.5f || it->emotional_valence < -0.5f);
            bool decayedOut    = (it->relevance <= kPromotionRelevance);
            bool trivialGone   = (it->relevance <= 0.0f && it->importance < 0.5f);

            if (impFastTrack || accessFreq || strongEmotion ||
                (!criticalTier && decayedOut)) {
                ELLE_MEMORY_RECORD copy = *it;
                copy.tier = MEM_LTM;
                toWrite.push_back(copy);
                it = m_stm.erase(it);
            } else if (!criticalTier && trivialGone) {
                ELLE_DEBUG("STM dropped trivial [%llu] imp=%.2f rel=%.2f | %.40s",
                           it->id, it->importance, it->relevance, it->content);
                it = m_stm.erase(it);
                dropped++;
            } else {
                ++it;
            }
        }
    }

    uint32_t promoted = 0;
    std::vector<ELLE_MEMORY_RECORD> reinsert;
    for (auto& rec : toWrite) {
        if (ElleDB::StoreMemory(rec)) {
            promoted++;
            ELLE_INFO("STM→LTM [%llu] pri=%s imp=%.2f rel=%.2f acc=%u | %.60s",
                      rec.id, PriorityLabel(PriorityTier(rec.importance)),
                      rec.importance, rec.relevance, rec.access_count,
                      rec.content);
        } else {
            ELLE_WARN("STM→LTM write failed for [%llu] — returning to STM", rec.id);
            rec.tier = MEM_STM;
            reinsert.push_back(rec);
        }
    }

    if (!reinsert.empty()) {
        std::lock_guard<std::mutex> lock(m_stmMutex);
        for (auto& r : reinsert) m_stm.push_back(r);
    }

    if (promoted > 0 || dropped > 0) {
        size_t remaining;
        {
            std::lock_guard<std::mutex> lock(m_stmMutex);
            remaining = m_stm.size();
        }
        ELLE_INFO("Consolidation pass: promoted=%u dropped=%u remaining_stm=%zu",
                  promoted, dropped, remaining);
    }
}

void MemoryEngine::DecaySTM() {
    auto  cfg   = ElleConfig::Instance().GetMemory();
    uint64_t now = ELLE_MS_NOW();
    float scale = (cfg.stm_decay_seconds > 0.0f) ? (15.0f / cfg.stm_decay_seconds) : 1.0f;

    std::vector<ELLE_MEMORY_RECORD> floorPromoted;
    {
        std::lock_guard<std::mutex> lock(m_stmMutex);

        for (auto& e : m_stm) {
            int   tier = PriorityTier(e.importance);
            float rate = kDecayRatePerSec[tier] * scale;
            if (rate <= 0.0f) continue;

            uint64_t anchor = e.last_access_ms > 0 ? e.last_access_ms : e.created_ms;
            float age_sec = (float)(now - anchor) / 1000.0f;
            float rel     = 1.0f - (rate * age_sec);
            if (e.access_count > 0) {
                float bump = std::min(0.5f, e.access_count * 0.05f);
                rel += bump;
            }
            e.decay     = 1.0f - rel;
            e.relevance = ELLE_CLAMP(rel, 0.0f, 1.0f);
        }

        auto it = m_stm.begin();
        while (it != m_stm.end()) {
            if (PriorityTier(it->importance) != 0 &&
                it->relevance <= kMinRelevanceFloor) {
                ELLE_MEMORY_RECORD copy = *it;
                copy.tier = MEM_LTM;
                floorPromoted.push_back(copy);
                it = m_stm.erase(it);
            } else {
                ++it;
            }
        }
    }

    for (auto& rec : floorPromoted) {
        if (ElleDB::StoreMemory(rec)) {
            ELLE_DEBUG("DecaySTM floor-promoted [%llu] rel=%.3f", rec.id, rec.relevance);
        } else {
            ELLE_WARN("DecaySTM floor-promote write failed for [%llu] — record lost", rec.id);
        }
    }
}

void MemoryEngine::ComputePosition(ELLE_MEMORY_RECORD& mem) {

    mem.position_x = (float)(mem.created_ms % 86400000) / 86400000.0f * 100.0f;
    mem.position_y = mem.emotional_valence * 50.0f + 50.0f;
    mem.position_z = mem.importance * 100.0f;
}

float MemoryEngine::ComputeRelevance(const ELLE_MEMORY_RECORD& mem, const std::string& query) {
    float textSim = TextSimilarity(std::string(mem.content), query);
    float recency = 1.0f - (float)(ELLE_MS_NOW() - mem.last_access_ms) / (86400000.0f * 7);
    recency = ELLE_CLAMP(recency, 0.0f, 1.0f);
    float importance = mem.importance;
    return textSim * 0.5f + recency * 0.3f + importance * 0.2f;
}

float MemoryEngine::TextSimilarity(const std::string& a, const std::string& b) {

    auto norm = [](const std::string& s) {
        std::string out; out.reserve(s.size());
        for (char c : s) {
            if ((unsigned char)c < 128 && (isalnum((unsigned char)c) || c == ' '))
                out += (char)tolower((unsigned char)c);
            else if (c != '\r' && c != '\n' && c != '\t')
                out += ' ';
        }
        return out;
    };
    std::string A = norm(a), B = norm(b);
    if (A.empty() || B.empty()) return 0.0f;

    std::set<std::string> wordsA, wordsB;
    {
        std::istringstream issA(A), issB(B);
        std::string w;
        while (issA >> w) wordsA.insert(w);
        while (issB >> w) wordsB.insert(w);
    }
    float wordJaccard = 0.0f;
    if (!wordsA.empty() && !wordsB.empty()) {
        size_t inter = 0;
        for (const auto& w : wordsA) if (wordsB.count(w)) inter++;
        size_t uni = wordsA.size() + wordsB.size() - inter;
        if (uni > 0) wordJaccard = (float)inter / (float)uni;
    }

    auto shingles = [](const std::string& s) {
        std::set<std::string> out;
        if (s.size() < 3) { out.insert(s); return out; }
        for (size_t i = 0; i + 2 < s.size(); i++) out.insert(s.substr(i, 3));
        return out;
    };
    auto sa = shingles(A), sb = shingles(B);
    float triJaccard = 0.0f;
    if (!sa.empty() && !sb.empty()) {
        size_t inter = 0;
        const auto& small = (sa.size() < sb.size()) ? sa : sb;
        const auto& big   = (sa.size() < sb.size()) ? sb : sa;
        for (const auto& t : small) if (big.count(t)) inter++;
        size_t uni = sa.size() + sb.size() - inter;
        if (uni > 0) triJaccard = (float)inter / (float)uni;
    }

    return 0.65f * wordJaccard + 0.35f * triJaccard;
}

float MemoryEngine::ComputeEmotionSimilarity(const float a[ELLE_MAX_EMOTIONS],
                                               const float b[ELLE_MAX_EMOTIONS]) {
    float dot = 0, magA = 0, magB = 0;
    for (int i = 0; i < ELLE_EMOTION_COUNT; i++) {
        dot += a[i] * b[i];
        magA += a[i] * a[i];
        magB += b[i] * b[i];
    }
    float denom = std::sqrt(magA) * std::sqrt(magB);
    return denom > 0.0f ? dot / denom : 0.0f;
}

void MemoryEngine::DreamConsolidate(const std::string& ) {
    ELLE_INFO("Dream consolidation starting...");

    ConsolidateMemories();
    UpdateClusters();

    auto recent = RecallRecent(20);
    if (!recent.empty()) {
        std::ostringstream ss;
        ss << "Process and find connections between these memories:\n";
        for (auto& m : recent) {
            ss << "- " << m.content << " (importance: " << m.importance << ")\n";
        }

        auto narrative = ElleLLMEngine::Instance().Ask(ss.str(),
            "You are processing memories during a dream state. Find patterns, "
            "connections, and insights. Generate a brief dream narrative.");

        if (!narrative.empty()) {
            float emo[ELLE_MAX_EMOTIONS] = {};
            StoreSTM(narrative, 0.6f, emo, {"dream", "insight"});
            ELLE_INFO("Dream narrative generated: %.80s...", narrative.c_str());
        }
    }

    ELLE_INFO("Dream consolidation complete");
}

std::vector<ELLE_MEMORY_RECORD> MemoryEngine::RecallRecent(uint32_t count) {

    std::vector<ELLE_MEMORY_RECORD> merged;
    {
        std::lock_guard<std::mutex> lock(m_stmMutex);
        merged.reserve(m_stm.size() + count);
        for (auto& m : m_stm) merged.push_back(m);
    }

    std::vector<ELLE_MEMORY_RECORD> ltm;
    ElleDB::RecallRecentLTM(ltm, count);
    merged.insert(merged.end(), ltm.begin(), ltm.end());

    std::sort(merged.begin(), merged.end(),
              [](const ELLE_MEMORY_RECORD& a, const ELLE_MEMORY_RECORD& b) {
                  return a.created_ms > b.created_ms;
              });
    if (merged.size() > count) merged.resize(count);
    return merged;
}

void MemoryEngine::UpdateClusters() {
    std::lock_guard<std::mutex> lock(m_stmMutex);
    if (m_stm.size() < 2) { m_clusters.clear(); return; }

    auto& cfg = ElleConfig::Instance().GetMemory();
    const uint32_t maxK = std::max(2u, std::min(cfg.max_clusters,
                                                (uint32_t)(m_stm.size() / 3 + 1)));
    const float    simThresh = cfg.cluster_threshold;
    const int      maxIter   = 8;

    std::vector<std::array<float, ELLE_MAX_EMOTIONS>> centroids;
    centroids.reserve(maxK);

    {
        std::array<float, ELLE_MAX_EMOTIONS> c{};
        memcpy(c.data(), m_stm.front().emotion_snapshot,
               sizeof(float) * ELLE_MAX_EMOTIONS);
        centroids.push_back(c);
    }

    while (centroids.size() < maxK) {
        float bestDist = -1.0f;
        size_t bestIdx = 0;
        for (size_t i = 0; i < m_stm.size(); i++) {
            float minSim = 1.0f;
            for (auto& cen : centroids) {
                float s = ComputeEmotionSimilarity(m_stm[i].emotion_snapshot,
                                                    cen.data());
                if (s < minSim) minSim = s;
            }
            float dist = 1.0f - minSim;
            if (dist > bestDist) { bestDist = dist; bestIdx = i; }
        }
        if (bestDist <= (1.0f - simThresh)) break;
        std::array<float, ELLE_MAX_EMOTIONS> c{};
        memcpy(c.data(), m_stm[bestIdx].emotion_snapshot,
               sizeof(float) * ELLE_MAX_EMOTIONS);
        centroids.push_back(c);
    }

    std::vector<uint32_t> assign(m_stm.size(), 0);
    for (int iter = 0; iter < maxIter; iter++) {
        bool changed = false;

        for (size_t i = 0; i < m_stm.size(); i++) {
            float bestSim = -2.0f;
            uint32_t best = 0;
            for (size_t k = 0; k < centroids.size(); k++) {
                float s = ComputeEmotionSimilarity(m_stm[i].emotion_snapshot,
                                                    centroids[k].data());
                if (s > bestSim) { bestSim = s; best = (uint32_t)k; }
            }
            if (assign[i] != best) { assign[i] = best; changed = true; }
        }
        if (!changed) break;

        std::vector<std::array<float, ELLE_MAX_EMOTIONS>> sum(centroids.size());
        std::vector<uint32_t> count(centroids.size(), 0);
        for (size_t k = 0; k < sum.size(); k++) sum[k].fill(0.0f);
        for (size_t i = 0; i < m_stm.size(); i++) {
            uint32_t k = assign[i];
            for (int d = 0; d < ELLE_EMOTION_COUNT; d++)
                sum[k][d] += m_stm[i].emotion_snapshot[d];
            count[k]++;
        }
        for (size_t k = 0; k < centroids.size(); k++) {
            if (count[k] == 0) continue;
            for (int d = 0; d < ELLE_EMOTION_COUNT; d++)
                centroids[k][d] = sum[k][d] / (float)count[k];
        }
    }

    m_clusters.clear();
    m_clusters.resize(centroids.size());
    for (size_t k = 0; k < centroids.size(); k++) {
        m_clusters[k].id = (uint32_t)k;
        memcpy(m_clusters[k].centroid, centroids[k].data(),
               sizeof(float) * ELLE_MAX_EMOTIONS);
    }
    for (size_t i = 0; i < m_stm.size(); i++) {
        uint32_t k = assign[i];
        m_stm[i].cluster_id = k;
        if (k < m_clusters.size())
            m_clusters[k].member_ids.push_back(m_stm[i].id);
    }

    for (auto& cl : m_clusters) {
        int domIdx = 0; float domVal = cl.centroid[0];
        for (int d = 1; d < ELLE_EMOTION_COUNT; d++) {
            if (cl.centroid[d] > domVal) { domVal = cl.centroid[d]; domIdx = d; }
        }
        cl.theme = "emotion_dim_" + std::to_string(domIdx);
    }

    ELLE_DEBUG("UpdateClusters: %zu clusters over %zu entries",
               m_clusters.size(), m_stm.size());
}

void MemoryEngine::LinkMemories(uint64_t id1, uint64_t id2) {
    std::lock_guard<std::mutex> lock(m_stmMutex);
    for (auto& mem : m_stm) {
        if (mem.id == id1 && mem.link_count < 16) {
            mem.linked_ids[mem.link_count++] = (uint32_t)id2;
        }
        if (mem.id == id2 && mem.link_count < 16) {
            mem.linked_ids[mem.link_count++] = (uint32_t)id1;
        }
    }
}

uint32_t MemoryEngine::STMCount() const { return (uint32_t)m_stm.size(); }
uint32_t MemoryEngine::ClusterCount() const { return (uint32_t)m_clusters.size(); }

RecallLoop::RecallLoop(MemoryEngine& engine) : m_engine(engine) {}
RecallLoop::~RecallLoop() { Stop(); }

void RecallLoop::Start() {
    m_running = true;
    m_thread = std::thread(&RecallLoop::Run, this);
}

void RecallLoop::Stop() {
    m_running = false;
    if (m_thread.joinable()) m_thread.join();
}

void RecallLoop::Run() {
    auto& cfg = ElleConfig::Instance().GetMemory();
    uint32_t recallMs = cfg.recall_interval_sec * 1000;
    uint32_t consolidateMs = cfg.consolidation_interval_min * 60 * 1000;
    uint64_t lastConsolidate = ELLE_MS_NOW();

    while (m_running) {

        m_engine.DecaySTM();

        if (ELLE_MS_NOW() - lastConsolidate > consolidateMs) {
            m_engine.ConsolidateMemories();
            lastConsolidate = ELLE_MS_NOW();
        }

        ElleWait::PollingSleep(recallMs, m_running);
    }
}

ElleMemoryService::ElleMemoryService()
    : ElleServiceBase(SVC_MEMORY, "ElleMemory",
                      "Elle-Ann Memory Engine",
                      "STM/LTM memory with 3D positioning and consolidation")
    , m_recallLoop(m_engine)
{}

bool ElleMemoryService::OnStart() {
    if (!m_engine.Initialize()) return false;
    m_recallLoop.Start();
    ELLE_INFO("Memory service started (STM: %d slots)",
              ElleConfig::Instance().GetMemory().stm_capacity);
    return true;
}

void ElleMemoryService::OnStop() {
    m_recallLoop.Stop();
    m_engine.Shutdown();
    ELLE_INFO("Memory service stopped");
}

void ElleMemoryService::OnMessage(const ElleIPCMessage& msg, ELLE_SERVICE_ID sender) {
    switch ((ELLE_IPC_MSG_TYPE)msg.header.msg_type) {
        case IPC_MEMORY_STORE: {
            ELLE_MEMORY_RECORD rec;
            if (msg.GetPayload(rec)) {
                m_engine.StoreLTM(rec);
            } else {

                float emo[ELLE_MAX_EMOTIONS] = {};
                m_engine.StoreSTM(msg.GetStringPayload(), 0.5f, emo);
            }
            break;
        }
        case IPC_MEMORY_RECALL: {
            std::string query = msg.GetStringPayload();
            auto results = m_engine.Recall(query);

            for (auto& mem : results) {
                auto resp = ElleIPCMessage::Create(IPC_MEMORY_RESULT, SVC_MEMORY, sender);
                resp.SetPayload(mem);
                GetIPCHub().Send(sender, resp);
            }
            break;
        }
        case IPC_DREAM_TRIGGER:
            m_engine.DreamConsolidate();

            {
                auto ack = ElleIPCMessage::Create(
                    IPC_MEMORY_CONSOLIDATE, SVC_MEMORY, sender);
                GetIPCHub().Send(sender, ack);
            }
            break;
        case IPC_MEMORY_CONSOLIDATE:
            ELLE_INFO("Conscious consolidation requested by service %u", (unsigned)sender);
            m_engine.ConsolidateMemories();
            {
                auto ack = ElleIPCMessage::Create(
                    IPC_MEMORY_CONSOLIDATE, SVC_MEMORY, sender);
                GetIPCHub().Send(sender, ack);
            }
            break;
        default:
            break;
    }
}

std::vector<ELLE_SERVICE_ID> ElleMemoryService::GetDependencies() {
    return { SVC_HEARTBEAT, SVC_EMOTIONAL };
}

ELLE_SERVICE_MAIN(ElleMemoryService)
