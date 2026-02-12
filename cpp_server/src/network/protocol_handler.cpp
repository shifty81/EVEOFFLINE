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

bool ProtocolHandler::validateMessage(const std::string& json) {
    // Basic validation - check for required fields
    return json.find("\"message_type\":") != std::string::npos;
}

} // namespace network
} // namespace atlas
