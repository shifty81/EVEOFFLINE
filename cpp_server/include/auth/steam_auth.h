#ifndef EVE_STEAM_AUTH_H
#define EVE_STEAM_AUTH_H

#include <string>
#include <cstdint>

#ifdef USE_STEAM
#include "steam/steam_api.h"
#endif

namespace eve {
namespace auth {

/**
 * @brief Steam authentication and integration
 * 
 * Handles Steam API initialization, authentication,
 * and server browser integration
 */
class SteamAuth {
public:
    SteamAuth();
    ~SteamAuth();

    // Initialization
    bool initialize(uint32_t app_id);
    void shutdown();
    
    // Update (call every frame)
    void update();
    
    // Authentication
    bool authenticateUser(const std::string& steam_ticket);
    bool isUserAuthenticated(uint64_t steam_id) const;
    std::string getUserName(uint64_t steam_id) const;
    
    // Server browser
    bool registerServer(const std::string& server_name, const std::string& map_name);
    void updateServerInfo(int current_players, int max_players);
    void setServerTags(const std::string& tags);
    
    // Status
    bool isInitialized() const { return initialized_; }
    bool isSteamRunning() const;
    
private:
    bool initialized_;
    uint32_t app_id_;
    
#ifdef USE_STEAM
    // Steam callbacks would go here
#endif
};

} // namespace auth
} // namespace eve

#endif // EVE_STEAM_AUTH_H
