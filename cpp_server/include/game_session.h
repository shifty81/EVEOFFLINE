#ifndef EVE_GAME_SESSION_H
#define EVE_GAME_SESSION_H

#include "ecs/world.h"
#include "network/tcp_server.h"
#include "network/protocol_handler.h"
#include "data/ship_database.h"
#include <string>
#include <unordered_map>
#include <mutex>
#include <atomic>

namespace eve {

// Forward declaration
namespace systems { class TargetingSystem; }

/**
 * @brief Manages game sessions: connects networking to the ECS world
 *
 * Bridges TCP client connections with the game world by:
 * - Handling connect/disconnect messages
 * - Spawning player entities on connect
 * - Processing player input (movement, commands)
 * - Broadcasting entity state updates each tick
 * - Spawning NPC entities on startup
 */
class GameSession {
public:
    explicit GameSession(ecs::World* world, network::TCPServer* tcp_server,
                         const std::string& data_path = "../data");
    ~GameSession() = default;

    /// Initialize message handlers and spawn initial NPCs
    void initialize();

    /// Called each server tick to broadcast state to all clients
    void update(float delta_time);

    /// Get the number of connected players
    int getPlayerCount() const;

    /// Set pointer to the TargetingSystem for lock/unlock handling
    void setTargetingSystem(systems::TargetingSystem* ts) { targeting_system_ = ts; }

    /// Get the ship database (read-only)
    const data::ShipDatabase& getShipDatabase() const { return ship_db_; }

private:
    // --- Message handlers ---
    void onClientMessage(const network::ClientConnection& client, const std::string& raw);
    void handleConnect(const network::ClientConnection& client, const std::string& data);
    void handleDisconnect(const network::ClientConnection& client);
    void handleInputMove(const network::ClientConnection& client, const std::string& data);
    void handleChat(const network::ClientConnection& client, const std::string& data);
    void handleTargetLock(const network::ClientConnection& client, const std::string& data);
    void handleTargetUnlock(const network::ClientConnection& client, const std::string& data);
    void handleModuleActivate(const network::ClientConnection& client, const std::string& data);
    void handleModuleDeactivate(const network::ClientConnection& client, const std::string& data);

    // --- State broadcast ---
    std::string buildStateUpdate() const;
    std::string buildSpawnEntity(const std::string& entity_id) const;

    // --- NPC management ---
    void spawnInitialNPCs();
    void spawnNPC(const std::string& id, const std::string& name, const std::string& ship,
                  const std::string& faction, float x, float y, float z);

    // --- Player entity helpers ---
    std::string createPlayerEntity(const std::string& player_id,
                                   const std::string& character_name,
                                   const std::string& ship_type = "rifter");

    // --- Helpers ---
    /// Extract a string value from a simple JSON object (lightweight parser)
    static std::string extractJsonString(const std::string& json, const std::string& key);
    /// Extract a float value from a simple JSON object
    static float extractJsonFloat(const std::string& json, const std::string& key, float fallback = 0.0f);

    ecs::World* world_;
    network::TCPServer* tcp_server_;
    network::ProtocolHandler protocol_;
    data::ShipDatabase ship_db_;
    systems::TargetingSystem* targeting_system_ = nullptr;

    // Map socket → entity_id for connected players
    struct PlayerInfo {
        std::string entity_id;
        std::string character_name;
        network::ClientConnection connection;
    };

    std::unordered_map<int, PlayerInfo> players_;  // keyed by socket fd
    mutable std::mutex players_mutex_;

    std::atomic<uint32_t> next_entity_id_{1};
};

} // namespace eve

#endif // EVE_GAME_SESSION_H
