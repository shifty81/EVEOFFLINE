#include "network/protocol_handler.h"
#include <nlohmann/json.hpp>
#include <iostream>
#include <chrono>

using json = nlohmann::json;

namespace eve {

ProtocolHandler::ProtocolHandler() {
}

void ProtocolHandler::handleMessage(const std::string& message) {
    try {
        auto j = json::parse(message);
        
        // Extract message type and data
        std::string type = j.value("type", "");
        if (type.empty()) {
            std::cerr << "Message missing 'type' field" << std::endl;
            return;
        }

        // Convert data to string (if exists)
        std::string dataStr;
        if (j.contains("data")) {
            dataStr = j["data"].dump();
        }

        // Call handler
        if (m_messageHandler) {
            m_messageHandler(type, dataStr);
        }
    } catch (const json::exception& e) {
        std::cerr << "Failed to parse JSON message: " << e.what() << std::endl;
        std::cerr << "Message: " << message << std::endl;
    }
}

std::string ProtocolHandler::createMessage(const std::string& type, const std::string& dataJson) {
    try {
        json j;
        j["type"] = type;
        
        // Add timestamp
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration<double>(now.time_since_epoch()).count();
        j["timestamp"] = timestamp;
        
        // Add data if provided
        if (!dataJson.empty()) {
            j["data"] = json::parse(dataJson);
        } else {
            j["data"] = json::object();
        }
        
        return j.dump();
    } catch (const json::exception& e) {
        std::cerr << "Failed to create JSON message: " << e.what() << std::endl;
        return "{}";
    }
}

std::string ProtocolHandler::createConnectMessage(const std::string& playerId, const std::string& characterName) {
    json data;
    data["player_id"] = playerId;
    data["character_name"] = characterName;
    data["version"] = "1.0";
    return createMessage("CONNECT", data.dump());
}

std::string ProtocolHandler::createMoveMessage(float vx, float vy, float vz) {
    json data;
    data["velocity"] = {
        {"x", vx},
        {"y", vy},
        {"z", vz}
    };
    return createMessage("INPUT_MOVE", data.dump());
}

std::string ProtocolHandler::createChatMessage(const std::string& message) {
    json data;
    data["message"] = message;
    return createMessage("CHAT", data.dump());
}

} // namespace eve
