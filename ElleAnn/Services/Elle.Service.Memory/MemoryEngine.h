#pragma once
#include "../_Shared/ElleTypes.h"
#include "../_Shared/ElleServiceBase.h"
#include <vector>
#include <string>
#include <deque>
#include <mutex>
#include <unordered_map>

class MemoryEngine {
public:
    MemoryEngine();
    ~MemoryEngine();

    bool Initialize();
    void Shutdown();

    uint64_t StoreSTM(const std::string& content, float importance,
                       const float emotions[ELLE_MAX_EMOTIONS],
                       const std::vector<std::string>& tags = {});
    uint64_t StoreLTM(const ELLE_MEMORY_RECORD& record);

    std::vector<ELLE_MEMORY_RECORD> Recall(const std::string& query,
                                            uint32_t maxResults = 10,
                                            float minRelevance = 0.3f);
    std::vector<ELLE_MEMORY_RECORD> RecallByTags(const std::vector<std::string>& tags,
                                                  uint32_t maxResults = 10);
    std::vector<ELLE_MEMORY_RECORD> RecallByEmotion(ELLE_EMOTION_ID emotion,
                                                     float minIntensity = 0.3f,
                                                     uint32_t maxResults = 10);
    std::vector<ELLE_MEMORY_RECORD> RecallRecent(uint32_t count = 10);

    void ConsolidateMemories();

    void DecaySTM();

    void LinkMemories(uint64_t id1, uint64_t id2);

    void UpdateClusters();

    void ComputePosition(ELLE_MEMORY_RECORD& mem);

    void DreamConsolidate(const std::string& narrativePrompt = "");

    uint32_t STMCount() const;
    uint64_t LTMCount() const;
    uint32_t ClusterCount() const;

    std::string GenerateNarrativeSummary(uint32_t memoryCount = 10);

private:

    std::deque<ELLE_MEMORY_RECORD> m_stm;
    std::mutex m_stmMutex;

    uint64_t m_nextId = 1;

    struct MemoryCluster {
        uint32_t id;
        std::string theme;
        float centroid[ELLE_MAX_EMOTIONS];
        std::vector<uint64_t> member_ids;
    };
    std::vector<MemoryCluster> m_clusters;

    float ComputeRelevance(const ELLE_MEMORY_RECORD& mem, const std::string& query);
    float ComputeEmotionSimilarity(const float a[ELLE_MAX_EMOTIONS],
                                    const float b[ELLE_MAX_EMOTIONS]);

    float TextSimilarity(const std::string& a, const std::string& b);
};

class RecallLoop {
public:
    RecallLoop(MemoryEngine& engine);
    ~RecallLoop();

    void Start();
    void Stop();

private:
    MemoryEngine& m_engine;
    std::thread m_thread;
    std::atomic<bool> m_running{false};

    void Run();
};

class ElleMemoryService : public ElleServiceBase {
public:
    ElleMemoryService();

protected:
    bool OnStart() override;
    void OnStop() override;
    void OnMessage(const ElleIPCMessage& msg, ELLE_SERVICE_ID sender) override;
    std::vector<ELLE_SERVICE_ID> GetDependencies() override;

private:
    MemoryEngine m_engine;
    RecallLoop   m_recallLoop;
};
