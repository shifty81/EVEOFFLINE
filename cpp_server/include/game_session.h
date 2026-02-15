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

namespace atlas {

// Forward declarations
namespace systems { 
    class TargetingSystem;
    class StationSystem;
    class MovementSystem;
    class CombatSystem;
    class ScannerSystem;
    class AnomalySystem;
    class MissionSystem;
    class MissionGeneratorSystem;
}

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

    /// Set pointer to the StationSystem for docking/repair handling
    void setStationSystem(systems::StationSystem* ss) { station_system_ = ss; }

    /// Set pointer to the MovementSystem for warp/approach/orbit/stop handling
    void setMovementSystem(systems::MovementSystem* ms) { movement_system_ = ms; }

    /// Set pointer to the CombatSystem for weapon firing
    void setCombatSystem(systems::CombatSystem* cs) { combat_system_ = cs; }

    /// Set pointer to the ScannerSystem for probe scanning
    void setScannerSystem(systems::ScannerSystem* ss) { scanner_system_ = ss; }

    /// Set pointer to the AnomalySystem for anomaly queries
    void setAnomalySystem(systems::AnomalySystem* as) { anomaly_system_ = as; }

    /// Set pointer to the MissionSystem for mission tracking
    void setMissionSystem(systems::MissionSystem* ms) { mission_system_ = ms; }

    /// Set pointer to the MissionGeneratorSystem for mission offers
    void setMissionGeneratorSystem(systems::MissionGeneratorSystem* mg) { mission_generator_ = mg; }

    /// Get the ship database (read-only)
    const data::ShipDatabase& getShipDatabase() const { return ship_db_; }

private:
    // --- Message handlers ---
    /**
     * Routes incoming client messages to appropriate handlers
     * @param client Client connection info
     * @param raw Raw message string from network
     */
    void onClientMessage(const network::ClientConnection& client, const std::string& raw);
    
    /**
     * Handle client connection
     * 
     * Creates a player entity and spawns them in the game world.
     * Expected message format: {"type":"connect","character_name":"PlayerName"}
     * 
     * @param client Client connection info
     * @param data JSON message data
     */
    void handleConnect(const network::ClientConnection& client, const std::string& data);
    
    /**
     * Handle client disconnection
     * 
     * Removes player entity from world and cleans up player info.
     * 
     * @param client Client connection info
     */
    void handleDisconnect(const network::ClientConnection& client);
    
    /**
     * Handle player movement input
     * 
     * Expected message format: {"type":"input_move","forward":1.0,"strafe":0.5}
     * Values range from -1.0 to 1.0 for each axis.
     * 
     * @param client Client connection info
     * @param data JSON message data with forward/strafe values
     */
    void handleInputMove(const network::ClientConnection& client, const std::string& data);
    
    /**
     * Handle chat message
     * 
     * Broadcasts chat message to all connected clients.
     * Expected format: {"type":"chat","message":"Hello world"}
     * 
     * @param client Client connection info
     * @param data JSON message data with message content
     */
    void handleChat(const network::ClientConnection& client, const std::string& data);
    
    /**
     * Handle target lock request
     * 
     * Attempts to lock the specified target entity.
     * Expected format: {"type":"target_lock","target_id":"entity_123"}
     * 
     * @param client Client connection info
     * @param data JSON message data with target_id
     */
    void handleTargetLock(const network::ClientConnection& client, const std::string& data);
    
    /**
     * Handle target unlock request
     * 
     * Clears the current target lock.
     * Expected format: {"type":"target_unlock"}
     * 
     * @param client Client connection info
     * @param data JSON message data
     */
    void handleTargetUnlock(const network::ClientConnection& client, const std::string& data);
    
    /**
     * Handle module activation
     * 
     * Activates a ship module (weapon, shield booster, etc.).
     * Expected format: {"type":"module_activate","slot":0}
     * 
     * @param client Client connection info
     * @param data JSON message data with slot number
     */
    void handleModuleActivate(const network::ClientConnection& client, const std::string& data);
    
    /**
     * Handle module deactivation
     * 
     * Deactivates a ship module.
     * Expected format: {"type":"module_deactivate","slot":0}
     * 
     * @param client Client connection info
     * @param data JSON message data with slot number
     */
    void handleModuleDeactivate(const network::ClientConnection& client, const std::string& data);
    
    /**
     * Handle dock request
     * 
     * Attempts to dock player at specified station.
     * Expected format: {"type":"dock_request","station_id":"station_jita4"}
     * 
     * @param client Client connection info
     * @param data JSON message data with station_id
     */
    void handleDockRequest(const network::ClientConnection& client, const std::string& data);
    
    /**
     * Handle undock request
     * 
     * Undocks player from current station.
     * Expected format: {"type":"undock_request"}
     * 
     * @param client Client connection info
     * @param data JSON message data
     */
    void handleUndockRequest(const network::ClientConnection& client, const std::string& data);
    
    /**
     * Handle repair request
     * 
     * Repairs player ship at current station.
     * Expected format: {"type":"repair_request"}
     * 
     * @param client Client connection info
     * @param data JSON message data
     */
    void handleRepairRequest(const network::ClientConnection& client, const std::string& data);

    /**
     * Handle warp request
     *
     * Initiates warp to a destination position.
     * Expected format: {"type":"warp_request","dest_x":1000,"dest_y":0,"dest_z":5000}
     */
    void handleWarpRequest(const network::ClientConnection& client, const std::string& data);

    /**
     * Handle approach command
     *
     * Commands the player's ship to approach a target entity.
     * Expected format: {"type":"approach","target_id":"entity_123"}
     */
    void handleApproach(const network::ClientConnection& client, const std::string& data);

    /**
     * Handle orbit command
     *
     * Commands the player's ship to orbit a target entity at a given distance.
     * Expected format: {"type":"orbit","target_id":"entity_123","distance":5000}
     */
    void handleOrbit(const network::ClientConnection& client, const std::string& data);

    /**
     * Handle stop command
     *
     * Commands the player's ship to stop all movement.
     * Expected format: {"type":"stop"}
     */
    void handleStop(const network::ClientConnection& client, const std::string& data);

    /**
     * Handle scan start request
     *
     * Begins scanning the current solar system for anomalies.
     * Expected format: {"type":"scan_start","system_id":"system_01"}
     */
    void handleScanStart(const network::ClientConnection& client, const std::string& data);

    /**
     * Handle scan stop request
     *
     * Stops an active scan.
     * Expected format: {"type":"scan_stop"}
     */
    void handleScanStop(const network::ClientConnection& client, const std::string& data);

    /**
     * Handle anomaly list request
     *
     * Returns a list of anomalies in the current system.
     * Expected format: {"type":"anomaly_list","system_id":"system_01"}
     */
    void handleAnomalyList(const network::ClientConnection& client, const std::string& data);

    /**
     * Handle mission list request
     *
     * Returns available missions for the current system.
     * Expected format: {"type":"mission_list","system_id":"system_01"}
     */
    void handleMissionList(const network::ClientConnection& client, const std::string& data);

    /**
     * Handle accept mission request
     *
     * Accepts an offered mission by index.
     * Expected format: {"type":"accept_mission","system_id":"system_01","mission_index":0}
     */
    void handleAcceptMission(const network::ClientConnection& client, const std::string& data);

    /**
     * Handle abandon mission request
     *
     * Abandons an active mission.
     * Expected format: {"type":"abandon_mission","mission_id":"mission_001"}
     */
    void handleAbandonMission(const network::ClientConnection& client, const std::string& data);

    /**
     * Handle mission progress report
     *
     * Records objective progress for an active mission.
     * Expected format: {"type":"mission_progress","mission_id":"mission_001","objective_type":"destroy","target":"pirate","count":1}
     */
    void handleMissionProgress(const network::ClientConnection& client, const std::string& data);

    // --- State broadcast ---
    /**
     * Build full state update message
     * 
     * Creates JSON message with all entity states including:
     * - Position, velocity, rotation
     * - Health (shield, armor, hull)
     * - Target locks
     * - Active modules
     * 
     * @return JSON string with format: {"type":"state_update","entities":[...]}
     */
    std::string buildStateUpdate() const;
    
    /**
     * Build entity spawn notification
     * 
     * Creates JSON message to notify clients when a new entity appears.
     * Includes full entity data (type, ship, faction, initial stats).
     * 
     * @param entity_id ID of entity to spawn
     * @return JSON string with format: {"type":"spawn_entity","entity":{...}}
     */
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
    /**
     * Extract a string value from a simple JSON object (lightweight parser)
     * 
     * Simple JSON parser for extracting string fields. Looks for "key":"value"
     * pattern in the JSON string. Not a full JSON parser - only handles simple
     * key-value pairs.
     * 
     * @param json JSON string to parse
     * @param key Key name to extract
     * @return Extracted string value, or empty string if not found
     */
    static std::string extractJsonString(const std::string& json, const std::string& key);
    
    /**
     * Extract a float value from a simple JSON object
     * 
     * Simple JSON parser for extracting numeric fields. Looks for "key":value
     * pattern (no quotes around value).
     * 
     * @param json JSON string to parse
     * @param key Key name to extract
     * @param fallback Default value if key not found or parsing fails
     * @return Extracted float value, or fallback if not found
     */
    static float extractJsonFloat(const std::string& json, const std::string& key, float fallback = 0.0f);

    ecs::World* world_;
    network::TCPServer* tcp_server_;
    network::ProtocolHandler protocol_;
    data::ShipDatabase ship_db_;
    systems::TargetingSystem* targeting_system_ = nullptr;
    systems::StationSystem* station_system_ = nullptr;
    systems::MovementSystem* movement_system_ = nullptr;
    systems::CombatSystem* combat_system_ = nullptr;
    systems::ScannerSystem* scanner_system_ = nullptr;
    systems::AnomalySystem* anomaly_system_ = nullptr;
    systems::MissionSystem* mission_system_ = nullptr;
    systems::MissionGeneratorSystem* mission_generator_ = nullptr;

    // Map socket → entity_id for connected players
    struct PlayerInfo {
        std::string entity_id;
        std::string character_name;
        network::ClientConnection connection;
    };

    std::unordered_map<int, PlayerInfo> players_;  // keyed by socket fd
    mutable std::mutex players_mutex_;

    std::atomic<uint32_t> next_entity_id_{1};
    mutable std::atomic<uint64_t> snapshot_sequence_{0};  // Sequence number for snapshots
};

} // namespace atlas

#endif // EVE_GAME_SESSION_H
