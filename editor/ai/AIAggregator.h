#pragma once
#include <string>
#include <vector>

namespace atlas::ai {

enum class AIRequestType {
    GraphGeneration,
    WorldGeneration,
    CodeAssist,
    Analysis,
};

struct AIContext {
    std::string projectName;
    std::vector<std::string> loadedAssets;
    std::string selectedAsset;
    std::string networkMode;
};

struct AIResponse {
    std::string content;
    float confidence = 0.0f;
};

class AIBackend {
public:
    virtual ~AIBackend() = default;
    virtual AIResponse Query(
        const std::string& prompt,
        const AIContext& context) = 0;
};

class AIAggregator {
public:
    void RegisterBackend(AIBackend* backend);

    AIResponse Execute(
        AIRequestType type,
        const std::string& prompt,
        const AIContext& context);

private:
    std::vector<AIBackend*> m_backends;

    AIResponse SelectBest(const std::vector<AIResponse>& responses) const;
};

}
