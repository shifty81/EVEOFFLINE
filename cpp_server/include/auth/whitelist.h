#ifndef EVE_WHITELIST_H
#define EVE_WHITELIST_H

#include <string>
#include <vector>
#include <set>
#include <cstdint>
#include <mutex>

namespace atlas {
namespace auth {

/**
 * @brief Whitelist management for server access control
 * 
 * Supports both Steam names and Steam IDs
 */
class Whitelist {
public:
    Whitelist();
    
    // Load/Save
    bool loadFromFile(const std::string& filepath);
    bool saveToFile(const std::string& filepath) const;
    
    // Steam name management
    void addSteamName(const std::string& steam_name);
    void removeSteamName(const std::string& steam_name);
    bool isSteamNameWhitelisted(const std::string& steam_name) const;
    
    // Steam ID management
    void addSteamId(uint64_t steam_id);
    void removeSteamId(uint64_t steam_id);
    bool isSteamIdWhitelisted(uint64_t steam_id) const;
    
    // Check access (checks both name and ID)
    bool hasAccess(const std::string& steam_name, uint64_t steam_id) const;
    
    // List management
    std::vector<std::string> getSteamNames() const;
    std::vector<uint64_t> getSteamIds() const;
    void clear();
    bool isEmpty() const;
    
private:
    std::set<std::string> whitelisted_names_;
    std::set<uint64_t> whitelisted_ids_;
    mutable std::mutex mutex_;
};

} // namespace auth
} // namespace atlas

#endif // EVE_WHITELIST_H
