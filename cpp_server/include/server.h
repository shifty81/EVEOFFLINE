#ifndef EVE_SERVER_H
#define EVE_SERVER_H

#include <string>
#include <memory>
#include <atomic>
#include "network/tcp_server.h"
#include "config/server_config.h"
#include "auth/steam_auth.h"
#include "auth/whitelist.h"

namespace eve {

/**
 * @brief Main dedicated server class
 * 
 * Manages the game server lifecycle, client connections,
 * and integration with Steam services.
 */
class Server {
public:
    explicit Server(const std::string& config_path = "config/server.json");
    ~Server();

    // Server lifecycle
    bool initialize();
    void start();
    void stop();
    void run();

    // Status
    bool isRunning() const { return running_; }
    int getPlayerCount() const;
    
private:
    std::unique_ptr<ServerConfig> config_;
    std::unique_ptr<network::TCPServer> tcp_server_;
    std::unique_ptr<auth::SteamAuth> steam_auth_;
    std::unique_ptr<auth::Whitelist> whitelist_;
    
    std::atomic<bool> running_;
    
    // Internal methods
    void mainLoop();
    void updateSteam();
};

} // namespace eve

#endif // EVE_SERVER_H
