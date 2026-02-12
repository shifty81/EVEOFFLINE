#ifndef EVE_SERVER_CONFIG_H
#define EVE_SERVER_CONFIG_H

#include <string>
#include <vector>
#include <cstdint>

namespace atlas {

/**
 * @brief Server configuration structure
 * 
 * Loaded from JSON configuration file
 */
struct ServerConfig {
    // Network settings
    std::string host = "0.0.0.0";
    uint16_t port = 8765;
    int max_connections = 100;
    
    // Server settings
    std::string server_name = "EVE OFFLINE Dedicated Server";
    std::string server_description = "A PVE-focused space MMO server";
    bool persistent_world = true;
    bool auto_save = true;
    int save_interval_seconds = 300; // 5 minutes
    
    // Access control
    bool use_whitelist = false;
    bool public_server = true;
    std::string password = "";
    
    // Steam integration
    bool use_steam = true;
    uint32_t steam_app_id = 0;
    bool steam_authentication = false;
    bool steam_server_browser = true;
    
    // Game settings
    float tick_rate = 30.0f;
    int max_entities = 10000;
    
    // Paths
    std::string data_path = "../data";
    std::string save_path = "./saves";
    std::string log_path = "./logs";
    
    // Load from JSON file
    bool loadFromFile(const std::string& filepath);
    
    // Save to JSON file
    bool saveToFile(const std::string& filepath) const;
};

} // namespace atlas

#endif // EVE_SERVER_CONFIG_H
