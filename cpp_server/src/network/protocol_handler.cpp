#include "network/protocol_handler.h"
#include <sstream>

namespace atlas {
namespace network {

ProtocolHandler::ProtocolHandler() {
    initializeMessageTypes();
}

void ProtocolHandler::initializeMessageTypes() {
    message_type_map_["connect"] = MessageType::CONNECT;
    message_type_map_["connect_ack"] = MessageType::CONNECT_ACK;
    message_type_map_["disconnect"] = MessageType::DISCONNECT;
    message_type_map_["input_move"] = MessageType::INPUT_MOVE;
    message_type_map_["state_update"] = MessageType::STATE_UPDATE;
    message_type_map_["chat"] = MessageType::CHAT;
    message_type_map_["command"] = MessageType::COMMAND;
    message_type_map_["spawn_entity"] = MessageType::SPAWN_ENTITY;
    message_type_map_["remove_entity"] = MessageType::REMOVE_ENTITY;
    message_type_map_["target_lock"] = MessageType::TARGET_LOCK;
    message_type_map_["target_unlock"] = MessageType::TARGET_UNLOCK;
    message_type_map_["module_activate"] = MessageType::MODULE_ACTIVATE;
    message_type_map_["module_deactivate"] = MessageType::MODULE_DEACTIVATE;
    message_type_map_["wormhole_scan"] = MessageType::WORMHOLE_SCAN;
    message_type_map_["wormhole_jump"] = MessageType::WORMHOLE_JUMP;
    message_type_map_["dock_request"] = MessageType::DOCK_REQUEST;
    message_type_map_["dock_success"] = MessageType::DOCK_SUCCESS;
    message_type_map_["dock_failed"] = MessageType::DOCK_FAILED;
    message_type_map_["undock_request"] = MessageType::UNDOCK_REQUEST;
    message_type_map_["undock_success"] = MessageType::UNDOCK_SUCCESS;
    message_type_map_["repair_request"] = MessageType::REPAIR_REQUEST;
    message_type_map_["repair_result"] = MessageType::REPAIR_RESULT;
    message_type_map_["damage_event"] = MessageType::DAMAGE_EVENT;
    message_type_map_["warp_request"] = MessageType::WARP_REQUEST;
    message_type_map_["warp_result"] = MessageType::WARP_RESULT;
    message_type_map_["approach"] = MessageType::APPROACH;
    message_type_map_["orbit"] = MessageType::ORBIT;
    message_type_map_["stop"] = MessageType::STOP;
    message_type_map_["salvage_request"] = MessageType::SALVAGE_REQUEST;
    message_type_map_["salvage_result"] = MessageType::SALVAGE_RESULT;
    message_type_map_["loot_all"] = MessageType::LOOT_ALL;
    message_type_map_["loot_result"] = MessageType::LOOT_RESULT;
    message_type_map_["mining_start"] = MessageType::MINING_START;
    message_type_map_["mining_stop"] = MessageType::MINING_STOP;
    message_type_map_["mining_result"] = MessageType::MINING_RESULT;
    message_type_map_["scan_start"] = MessageType::SCAN_START;
    message_type_map_["scan_stop"] = MessageType::SCAN_STOP;
    message_type_map_["scan_result"] = MessageType::SCAN_RESULT;
    message_type_map_["anomaly_list"] = MessageType::ANOMALY_LIST;
    message_type_map_["mission_list"] = MessageType::MISSION_LIST;
    message_type_map_["accept_mission"] = MessageType::ACCEPT_MISSION;
    message_type_map_["abandon_mission"] = MessageType::ABANDON_MISSION;
    message_type_map_["mission_progress"] = MessageType::MISSION_PROGRESS;
    message_type_map_["mission_result"] = MessageType::MISSION_RESULT;
    message_type_map_["error"] = MessageType::ERROR;
}

std::string ProtocolHandler::messageTypeToString(MessageType type) {
    switch (type) {
        case MessageType::CONNECT: return "connect";
        case MessageType::CONNECT_ACK: return "connect_ack";
        case MessageType::DISCONNECT: return "disconnect";
        case MessageType::INPUT_MOVE: return "input_move";
        case MessageType::STATE_UPDATE: return "state_update";
        case MessageType::CHAT: return "chat";
        case MessageType::COMMAND: return "command";
        case MessageType::SPAWN_ENTITY: return "spawn_entity";
        case MessageType::REMOVE_ENTITY: return "remove_entity";
        case MessageType::TARGET_LOCK: return "target_lock";
        case MessageType::TARGET_UNLOCK: return "target_unlock";
        case MessageType::MODULE_ACTIVATE: return "module_activate";
        case MessageType::MODULE_DEACTIVATE: return "module_deactivate";
        case MessageType::WORMHOLE_SCAN: return "wormhole_scan";
        case MessageType::WORMHOLE_JUMP: return "wormhole_jump";
        case MessageType::DOCK_REQUEST: return "dock_request";
        case MessageType::DOCK_SUCCESS: return "dock_success";
        case MessageType::DOCK_FAILED: return "dock_failed";
        case MessageType::UNDOCK_REQUEST: return "undock_request";
        case MessageType::UNDOCK_SUCCESS: return "undock_success";
        case MessageType::REPAIR_REQUEST: return "repair_request";
        case MessageType::REPAIR_RESULT: return "repair_result";
        case MessageType::DAMAGE_EVENT: return "damage_event";
        case MessageType::WARP_REQUEST: return "warp_request";
        case MessageType::WARP_RESULT: return "warp_result";
        case MessageType::APPROACH: return "approach";
        case MessageType::ORBIT: return "orbit";
        case MessageType::STOP: return "stop";
        case MessageType::SALVAGE_REQUEST: return "salvage_request";
        case MessageType::SALVAGE_RESULT: return "salvage_result";
        case MessageType::LOOT_ALL: return "loot_all";
        case MessageType::LOOT_RESULT: return "loot_result";
        case MessageType::MINING_START: return "mining_start";
        case MessageType::MINING_STOP: return "mining_stop";
        case MessageType::MINING_RESULT: return "mining_result";
        case MessageType::SCAN_START: return "scan_start";
        case MessageType::SCAN_STOP: return "scan_stop";
        case MessageType::SCAN_RESULT: return "scan_result";
        case MessageType::ANOMALY_LIST: return "anomaly_list";
        case MessageType::MISSION_LIST: return "mission_list";
        case MessageType::ACCEPT_MISSION: return "accept_mission";
        case MessageType::ABANDON_MISSION: return "abandon_mission";
        case MessageType::MISSION_PROGRESS: return "mission_progress";
        case MessageType::MISSION_RESULT: return "mission_result";
        case MessageType::ERROR: return "error";
        default: return "unknown";
    }
}

bool ProtocolHandler::parseMessage(const std::string& json, MessageType& type, std::string& data) {
    // Simple JSON parsing (in production, use a library like nlohmann/json or rapidjson)
    // This is a simplified version compatible with both the Python and C++ client protocols
    
    // Find message type field – accept both "message_type" and "type"
    size_t type_pos = json.find("\"message_type\":");
    size_t key_len = 15;  // length of "message_type":
    if (type_pos == std::string::npos) {
        type_pos = json.find("\"type\":");
        key_len = 7;  // length of "type":
    }
    if (type_pos == std::string::npos) {
        return false;
    }
    
    size_t type_start = json.find("\"", type_pos + key_len);
    size_t type_end = json.find("\"", type_start + 1);
    if (type_start == std::string::npos || type_end == std::string::npos) {
        return false;
    }
    
    std::string type_str = json.substr(type_start + 1, type_end - type_start - 1);
    
    auto it = message_type_map_.find(type_str);
    if (it == message_type_map_.end()) {
        return false;
    }
    
    type = it->second;
    
    // Extract data field
    size_t data_pos = json.find("\"data\":");
    if (data_pos != std::string::npos) {
        size_t data_start = json.find("{", data_pos);
        size_t data_end = json.rfind("}");
        if (data_start != std::string::npos && data_end != std::string::npos) {
            data = json.substr(data_start, data_end - data_start + 1);
        }
    }
    
    return true;
}

std::string ProtocolHandler::createConnectAck(bool success, const std::string& message) {
    std::ostringstream json;
    json << "{";
    json << "\"message_type\":\"" << messageTypeToString(MessageType::CONNECT_ACK) << "\",";
    json << "\"data\":{";
    json << "\"success\":" << (success ? "true" : "false") << ",";
    json << "\"message\":\"" << message << "\"";
    json << "}";
    json << "}";
    return json.str();
}

std::string ProtocolHandler::createStateUpdate(const std::string& game_state) {
    std::ostringstream json;
    json << "{";
    json << "\"message_type\":\"" << messageTypeToString(MessageType::STATE_UPDATE) << "\",";
    json << "\"data\":" << game_state;
    json << "}";
    return json.str();
}

std::string ProtocolHandler::createChatMessage(const std::string& sender, const std::string& message) {
    std::ostringstream json;
    json << "{";
    json << "\"message_type\":\"" << messageTypeToString(MessageType::CHAT) << "\",";
    json << "\"data\":{";
    json << "\"sender\":\"" << sender << "\",";
    json << "\"message\":\"" << message << "\"";
    json << "}";
    json << "}";
    return json.str();
}

std::string ProtocolHandler::createError(const std::string& error_message) {
    std::ostringstream json;
    json << "{";
    json << "\"message_type\":\"" << messageTypeToString(MessageType::ERROR) << "\",";
    json << "\"data\":{";
    json << "\"error\":\"" << error_message << "\"";
    json << "}";
    json << "}";
    return json.str();
}

std::string ProtocolHandler::createDockSuccess(const std::string& station_id) {
    std::ostringstream json;
    json << "{";
    json << "\"message_type\":\"" << messageTypeToString(MessageType::DOCK_SUCCESS) << "\",";
    json << "\"data\":{";
    json << "\"station_id\":\"" << station_id << "\"";
    json << "}";
    json << "}";
    return json.str();
}

std::string ProtocolHandler::createDockFailed(const std::string& reason) {
    std::ostringstream json;
    json << "{";
    json << "\"message_type\":\"" << messageTypeToString(MessageType::DOCK_FAILED) << "\",";
    json << "\"data\":{";
    json << "\"reason\":\"" << reason << "\"";
    json << "}";
    json << "}";
    return json.str();
}

std::string ProtocolHandler::createUndockSuccess() {
    std::ostringstream json;
    json << "{";
    json << "\"message_type\":\"" << messageTypeToString(MessageType::UNDOCK_SUCCESS) << "\",";
    json << "\"data\":{}";
    json << "}";
    return json.str();
}

std::string ProtocolHandler::createRepairResult(float cost, float shield_hp, float armor_hp, float hull_hp) {
    std::ostringstream json;
    json << "{";
    json << "\"message_type\":\"" << messageTypeToString(MessageType::REPAIR_RESULT) << "\",";
    json << "\"data\":{";
    json << "\"cost\":" << cost << ",";
    json << "\"shield_hp\":" << shield_hp << ",";
    json << "\"armor_hp\":" << armor_hp << ",";
    json << "\"hull_hp\":" << hull_hp;
    json << "}";
    json << "}";
    return json.str();
}

std::string ProtocolHandler::createDamageEvent(const std::string& target_id, float damage,
                                                const std::string& damage_type, const std::string& layer_hit,
                                                bool shield_depleted, bool armor_depleted, bool hull_critical) {
    std::ostringstream json;
    json << "{";
    json << "\"message_type\":\"" << messageTypeToString(MessageType::DAMAGE_EVENT) << "\",";
    json << "\"data\":{";
    json << "\"target_id\":\"" << target_id << "\",";
    json << "\"damage\":" << damage << ",";
    json << "\"damage_type\":\"" << damage_type << "\",";
    json << "\"layer_hit\":\"" << layer_hit << "\",";
    json << "\"shield_depleted\":" << (shield_depleted ? "true" : "false") << ",";
    json << "\"armor_depleted\":" << (armor_depleted ? "true" : "false") << ",";
    json << "\"hull_critical\":" << (hull_critical ? "true" : "false");
    json << "}";
    json << "}";
    return json.str();
}

bool ProtocolHandler::validateMessage(const std::string& json) {
    // Basic validation - check for required fields
    return json.find("\"message_type\":") != std::string::npos ||
           json.find("\"type\":") != std::string::npos;
}

std::string ProtocolHandler::createWarpResult(bool success, const std::string& reason) {
    std::ostringstream json;
    json << "{\"type\":\"warp_result\",\"data\":{";
    json << "\"message_type\":\"" << messageTypeToString(MessageType::WARP_RESULT) << "\",";
    json << "\"success\":" << (success ? "true" : "false");
    if (!reason.empty()) {
        json << ",\"reason\":\"" << reason << "\"";
    }
    json << "}}";
    return json.str();
}

std::string ProtocolHandler::createMovementAck(const std::string& command, bool success) {
    std::ostringstream json;
    json << "{\"type\":\"movement_ack\",\"data\":{";
    json << "\"command\":\"" << command << "\",";
    json << "\"success\":" << (success ? "true" : "false");
    json << "}}";
    return json.str();
}

std::string ProtocolHandler::createSalvageResult(bool success, const std::string& wreck_id,
                                                  int items_recovered) {
    std::ostringstream json;
    json << "{\"message_type\":\"salvage_result\",\"data\":{";
    json << "\"success\":" << (success ? "true" : "false") << ",";
    json << "\"wreck_id\":\"" << wreck_id << "\",";
    json << "\"items_recovered\":" << items_recovered;
    json << "}}";
    return json.str();
}

std::string ProtocolHandler::createLootResult(bool success, const std::string& wreck_id,
                                               int items_collected, double isk_gained) {
    std::ostringstream json;
    json << "{\"message_type\":\"loot_result\",\"data\":{";
    json << "\"success\":" << (success ? "true" : "false") << ",";
    json << "\"wreck_id\":\"" << wreck_id << "\",";
    json << "\"items_collected\":" << items_collected << ",";
    json << "\"isk_gained\":" << isk_gained;
    json << "}}";
    return json.str();
}

std::string ProtocolHandler::createMiningResult(bool success, const std::string& deposit_id,
                                                 const std::string& mineral_type, int quantity_mined) {
    std::ostringstream json;
    json << "{\"message_type\":\"mining_result\",\"data\":{";
    json << "\"success\":" << (success ? "true" : "false") << ",";
    json << "\"deposit_id\":\"" << deposit_id << "\",";
    json << "\"mineral_type\":\"" << mineral_type << "\",";
    json << "\"quantity_mined\":" << quantity_mined;
    json << "}}";
    return json.str();
}

std::string ProtocolHandler::createScanResult(const std::string& scanner_id, int anomalies_found,
                                               const std::string& results_json) {
    std::ostringstream json;
    json << "{\"message_type\":\"scan_result\",\"data\":{";
    json << "\"scanner_id\":\"" << scanner_id << "\",";
    json << "\"anomalies_found\":" << anomalies_found << ",";
    json << "\"results\":" << results_json;
    json << "}}";
    return json.str();
}

std::string ProtocolHandler::createAnomalyList(const std::string& system_id, int count,
                                                const std::string& anomalies_json) {
    std::ostringstream json;
    json << "{\"message_type\":\"anomaly_list\",\"data\":{";
    json << "\"system_id\":\"" << system_id << "\",";
    json << "\"count\":" << count << ",";
    json << "\"anomalies\":" << anomalies_json;
    json << "}}";
    return json.str();
}

std::string ProtocolHandler::createMissionList(const std::string& system_id, int count,
                                                const std::string& missions_json) {
    std::ostringstream json;
    json << "{\"message_type\":\"mission_list\",\"data\":{";
    json << "\"system_id\":\"" << system_id << "\",";
    json << "\"count\":" << count << ",";
    json << "\"missions\":" << missions_json;
    json << "}}";
    return json.str();
}

std::string ProtocolHandler::createMissionResult(bool success, const std::string& mission_id,
                                                  const std::string& action,
                                                  const std::string& message) {
    std::ostringstream json;
    json << "{\"message_type\":\"mission_result\",\"data\":{";
    json << "\"success\":" << (success ? "true" : "false") << ",";
    json << "\"mission_id\":\"" << mission_id << "\",";
    json << "\"action\":\"" << action << "\"";
    if (!message.empty()) {
        json << ",\"message\":\"" << message << "\"";
    }
    json << "}}";
    return json.str();
}

} // namespace network
} // namespace atlas
