#ifndef EVE_PROTOCOL_HANDLER_H
#define EVE_PROTOCOL_HANDLER_H

#include <string>
#include <functional>
#include <map>

namespace atlas {
namespace network {

/**
 * @brief Message types matching Python server protocol
 */
// The Windows SDK defines ERROR as a macro; temporarily remove it so the
// enum value compiles on MSVC.
#ifdef ERROR
#pragma push_macro("ERROR")
#undef ERROR
#define EVE_PROTOCOL_RESTORE_ERROR_MACRO
#endif

enum class MessageType {
    CONNECT,
    CONNECT_ACK,
    DISCONNECT,
    INPUT_MOVE,
    STATE_UPDATE,
    CHAT,
    COMMAND,
    SPAWN_ENTITY,
    REMOVE_ENTITY,
    TARGET_LOCK,
    TARGET_UNLOCK,
    MODULE_ACTIVATE,
    MODULE_DEACTIVATE,
    WORMHOLE_SCAN,
    WORMHOLE_JUMP,
    DOCK_REQUEST,
    DOCK_SUCCESS,
    DOCK_FAILED,
    UNDOCK_REQUEST,
    UNDOCK_SUCCESS,
    REPAIR_REQUEST,
    REPAIR_RESULT,
    DAMAGE_EVENT,
    WARP_REQUEST,
    WARP_RESULT,
    APPROACH,
    ORBIT,
    STOP,
    SALVAGE_REQUEST,
    SALVAGE_RESULT,
    LOOT_ALL,
    LOOT_RESULT,
    MINING_START,
    MINING_STOP,
    MINING_RESULT,
    SCAN_START,
    SCAN_STOP,
    SCAN_RESULT,
    ANOMALY_LIST,
    ERROR
};

#ifdef EVE_PROTOCOL_RESTORE_ERROR_MACRO
#pragma pop_macro("ERROR")
#undef EVE_PROTOCOL_RESTORE_ERROR_MACRO
#endif

/**
 * @brief Protocol handler for JSON-based messages
 * 
 * Compatible with existing Python client/server protocol
 */
class ProtocolHandler {
public:
    ProtocolHandler();
    
    // Message parsing
    bool parseMessage(const std::string& json, MessageType& type, std::string& data);
    
    // Message creation
    std::string createConnectAck(bool success, const std::string& message);
    std::string createStateUpdate(const std::string& game_state);
    std::string createChatMessage(const std::string& sender, const std::string& message);
    std::string createError(const std::string& error_message);
    
    // Station docking messages
    std::string createDockSuccess(const std::string& station_id);
    std::string createDockFailed(const std::string& reason);
    std::string createUndockSuccess();
    std::string createRepairResult(float cost, float shield_hp, float armor_hp, float hull_hp);
    
    // Damage event messages
    std::string createDamageEvent(const std::string& target_id, float damage,
                                  const std::string& damage_type, const std::string& layer_hit,
                                  bool shield_depleted, bool armor_depleted, bool hull_critical);
    
    // Movement command responses
    std::string createWarpResult(bool success, const std::string& reason = "");
    std::string createMovementAck(const std::string& command, bool success);
    
    // Salvage / Loot messages
    std::string createSalvageResult(bool success, const std::string& wreck_id,
                                    int items_recovered);
    std::string createLootResult(bool success, const std::string& wreck_id,
                                 int items_collected, double isk_gained);

    // Mining messages
    std::string createMiningResult(bool success, const std::string& deposit_id,
                                   const std::string& mineral_type, int quantity_mined);

    // Scanner / Anomaly messages
    std::string createScanResult(const std::string& scanner_id, int anomalies_found,
                                 const std::string& results_json);
    std::string createAnomalyList(const std::string& system_id, int count,
                                  const std::string& anomalies_json);
    
    // Message validation
    bool validateMessage(const std::string& json);
    
private:
    std::map<std::string, MessageType> message_type_map_;
    
    void initializeMessageTypes();
    std::string messageTypeToString(MessageType type);
};

} // namespace network
} // namespace atlas

#endif // EVE_PROTOCOL_HANDLER_H
