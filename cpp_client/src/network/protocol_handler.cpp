#include "network/protocol_handler.h"
#include <nlohmann/json.hpp>
#include <iostream>
#include <chrono>

using json = nlohmann::json;

namespace atlas {

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
    data["version"] = "0.1.0";
    return createMessage("connect", data.dump());
}

std::string ProtocolHandler::createMoveMessage(float vx, float vy, float vz) {
    json data;
    data["velocity"] = {
        {"x", vx},
        {"y", vy},
        {"z", vz}
    };
    return createMessage("input_move", data.dump());
}

std::string ProtocolHandler::createChatMessage(const std::string& message) {
    json data;
    data["message"] = message;
    return createMessage("chat", data.dump());
}

// Inventory management messages
std::string ProtocolHandler::createInventoryTransferMessage(const std::string& itemId, int quantity,
                                                           bool fromCargo, bool toCargo) {
    json data;
    data["item_id"] = itemId;
    data["quantity"] = quantity;
    data["from_location"] = fromCargo ? "cargo" : "hangar";
    data["to_location"] = toCargo ? "cargo" : "hangar";
    return createMessage("inventory_transfer", data.dump());
}

std::string ProtocolHandler::createInventoryJettisonMessage(const std::string& itemId, int quantity) {
    json data;
    data["item_id"] = itemId;
    data["quantity"] = quantity;
    data["from_location"] = "cargo";
    data["to_location"] = "space";
    return createMessage("inventory_jettison", data.dump());
}

// Module fitting messages
std::string ProtocolHandler::createModuleFitMessage(const std::string& moduleId, 
                                                   const std::string& slotType, int slotIndex) {
    json data;
    data["module_id"] = moduleId;
    data["slot_type"] = slotType;
    data["slot_index"] = slotIndex;
    return createMessage("module_fit", data.dump());
}

std::string ProtocolHandler::createModuleUnfitMessage(const std::string& slotType, int slotIndex) {
    json data;
    data["slot_type"] = slotType;
    data["slot_index"] = slotIndex;
    return createMessage("module_unfit", data.dump());
}

std::string ProtocolHandler::createModuleActivateMessage(int slotIndex) {
    json data;
    data["slot_index"] = slotIndex;
    return createMessage("module_activate", data.dump());
}

// Market messages
std::string ProtocolHandler::createMarketBuyMessage(const std::string& itemId, int quantity, double price) {
    json data;
    data["item_id"] = itemId;
    data["quantity"] = quantity;
    data["price"] = price;
    data["action"] = "buy";
    return createMessage("market_transaction", data.dump());
}

std::string ProtocolHandler::createMarketSellMessage(const std::string& itemId, int quantity, double price) {
    json data;
    data["item_id"] = itemId;
    data["quantity"] = quantity;
    data["price"] = price;
    data["action"] = "sell";
    return createMessage("market_transaction", data.dump());
}

std::string ProtocolHandler::createMarketQueryMessage(const std::string& itemId) {
    json data;
    data["item_id"] = itemId;
    return createMessage("market_query", data.dump());
}

// Station docking and repair messages
std::string ProtocolHandler::createDockRequestMessage(const std::string& stationId) {
    json data;
    data["station_id"] = stationId;
    return createMessage("dock_request", data.dump());
}

std::string ProtocolHandler::createUndockRequestMessage() {
    json data;
    return createMessage("undock_request", data.dump());
}

std::string ProtocolHandler::createRepairRequestMessage() {
    json data;
    return createMessage("repair_request", data.dump());
}

// Response message type helpers
bool ProtocolHandler::isSuccessResponse(const std::string& type) {
    return type.find("_success") != std::string::npos ||
           type.find("_ack") != std::string::npos ||
           type.find("_result") != std::string::npos;
}

bool ProtocolHandler::isErrorResponse(const std::string& type) {
    return type.find("_error") != std::string::npos ||
           type.find("_failed") != std::string::npos ||
           type == "error";
}

bool ProtocolHandler::isInventoryResponse(const std::string& type) {
    return type.find("inventory_") == 0 &&
           (isSuccessResponse(type) || isErrorResponse(type));
}

bool ProtocolHandler::isFittingResponse(const std::string& type) {
    return type.find("module_") == 0 &&
           (isSuccessResponse(type) || isErrorResponse(type));
}

bool ProtocolHandler::isMarketResponse(const std::string& type) {
    return type.find("market_") == 0 &&
           (isSuccessResponse(type) || isErrorResponse(type));
}

bool ProtocolHandler::isStationResponse(const std::string& type) {
    return type == "dock_success" || type == "dock_failed" ||
           type == "undock_success" || type == "repair_result" ||
           (type.find("dock_") == 0 && (isSuccessResponse(type) || isErrorResponse(type))) ||
           (type.find("repair_") == 0 && (isSuccessResponse(type) || isErrorResponse(type))) ||
           (type.find("undock_") == 0 && (isSuccessResponse(type) || isErrorResponse(type)));
}

} // namespace atlas
