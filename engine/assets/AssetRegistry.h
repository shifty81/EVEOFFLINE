#pragma once
#include <string>
#include <unordered_map>
#include <filesystem>
#include <functional>

namespace atlas::asset {

struct AssetEntry {
    std::string id;
    std::string path;
    uint64_t version = 1;
};

class AssetRegistry {
public:
    using ReloadCallback = std::function<void(const AssetEntry&)>;

    void Scan(const std::string& root);
    const AssetEntry* Get(const std::string& id) const;

    void SetReloadCallback(ReloadCallback cb);
    void PollHotReload();

    size_t Count() const;

private:
    std::unordered_map<std::string, AssetEntry> m_assets;
    std::unordered_map<std::string, std::filesystem::file_time_type> m_timestamps;
    ReloadCallback m_onReload;
};

}
