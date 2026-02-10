#ifndef EVE_SERVER_H
#define EVE_SERVER_H

#include <string>
#include <memory>
#include <atomic>
#include "network/tcp_server.h"
#include "config/server_config.h"
#include "auth/steam_auth.h"
#include "auth/whitelist.h"
#include "ecs/world.h"
#include "game_session.h"
#include "systems/targeting_system.h"
#include "data/world_persistence.h"
#include "utils/server_metrics.h"

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
    
    // Get game world
    ecs::World* getWorld() { return game_world_.get(); }

    // World persistence
    bool saveWorld();
    bool loadWorld();

    // Metrics
    const utils::ServerMetrics& getMetrics() const { return metrics_; }
    
private:
    std::unique_ptr<ServerConfig> config_;
    std::unique_ptr<network::TCPServer> tcp_server_;
    std::unique_ptr<auth::SteamAuth> steam_auth_;
    std::unique_ptr<auth::Whitelist> whitelist_;
    std::unique_ptr<ecs::World> game_world_;
    std::unique_ptr<GameSession> game_session_;
    data::WorldPersistence world_persistence_;
    utils::ServerMetrics metrics_;
    systems::TargetingSystem* targeting_system_ = nullptr;
    
    std::atomic<bool> running_;
    
    // Internal methods
    void mainLoop();
    void updateSteam();
    void initializeGameWorld();
};

} // namespace eve

#endif // EVE_SERVER_H
