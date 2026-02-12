#include "network/network_manager.h"
#include <nlohmann/json.hpp>
#include <iostream>

namespace atlas {

NetworkManager::NetworkManager()
    : m_tcpClient(std::make_unique<TCPClient>())
    , m_protocolHandler(std::make_unique<ProtocolHandler>())
    , m_authenticated(false)
    , m_state(State::DISCONNECTED)
{
    // Set up callbacks
    m_tcpClient->setMessageCallback([this](const std::string& msg) {
        onRawMessage(msg);
    });

    m_protocolHandler->setMessageHandler([this](const std::string& type, const std::string& data) {
        onProtocolMessage(type, data);
    });
}

NetworkManager::~NetworkManager() {
    disconnect();
}

bool NetworkManager::connect(const std::string& host, int port,
                             const std::string& playerId, const std::string& characterName) {
    if (m_state != State::DISCONNECTED) {
        std::cerr << "Already connected or connecting" << std::endl;
        return false;
    }

    m_playerId = playerId;
    m_characterName = characterName;
    m_state = State::CONNECTING;

    std::cout << "Connecting to " << host << ":" << port << " as " << characterName << std::endl;

    // Connect TCP
    if (!m_tcpClient->connect(host, port)) {
        m_state = State::DISCONNECTED;
        return false;
    }

    m_state = State::CONNECTED;

    // Send CONNECT message
    std::string connectMsg = m_protocolHandler->createConnectMessage(playerId, characterName);
    if (!m_tcpClient->send(connectMsg)) {
        std::cerr << "Failed to send CONNECT message" << std::endl;
        disconnect();
        return false;
    }

    std::cout << "Sent CONNECT message" << std::endl;
    return true;
}

void NetworkManager::disconnect() {
    if (m_state != State::DISCONNECTED) {
        m_tcpClient->disconnect();
        m_state = State::DISCONNECTED;
        m_authenticated = false;
        std::cout << "Disconnected" << std::endl;
    }
}

bool NetworkManager::isConnected() const {
    return m_state == State::CONNECTED || m_state == State::AUTHENTICATED;
}

void NetworkManager::update() {
    if (!isConnected()) return;

    // Process incoming messages
    m_tcpClient->processMessages();
}

void NetworkManager::registerHandler(const std::string& type, TypedMessageHandler handler) {
    m_handlers[type] = handler;
}

void NetworkManager::sendMove(float vx, float vy, float vz) {
    if (!isConnected()) return;
    
    std::string msg = m_protocolHandler->createMoveMessage(vx, vy, vz);
    m_tcpClient->send(msg);
}

void NetworkManager::sendChat(const std::string& message) {
    if (!isConnected()) return;
    
    std::string msg = m_protocolHandler->createChatMessage(message);
    m_tcpClient->send(msg);
}

// Inventory management
void NetworkManager::sendInventoryTransfer(const std::string& itemId, int quantity, 
                                          bool fromCargo, bool toCargo) {
    if (!isConnected()) return;
    
    std::string msg = m_protocolHandler->createInventoryTransferMessage(itemId, quantity, fromCargo, toCargo);
    m_tcpClient->send(msg);
    std::cout << "Sent inventory transfer: " << itemId << " x" << quantity 
              << " from " << (fromCargo ? "cargo" : "hangar") 
              << " to " << (toCargo ? "cargo" : "hangar") << std::endl;
}

void NetworkManager::sendInventoryJettison(const std::string& itemId, int quantity) {
    if (!isConnected()) return;
    
    std::string msg = m_protocolHandler->createInventoryJettisonMessage(itemId, quantity);
    m_tcpClient->send(msg);
    std::cout << "Sent jettison request: " << itemId << " x" << quantity << std::endl;
}

// Module fitting
void NetworkManager::sendModuleFit(const std::string& moduleId, const std::string& slotType, int slotIndex) {
    if (!isConnected()) return;
    
    std::string msg = m_protocolHandler->createModuleFitMessage(moduleId, slotType, slotIndex);
    m_tcpClient->send(msg);
    std::cout << "Sent module fit request: " << moduleId 
              << " to " << slotType << "[" << slotIndex << "]" << std::endl;
}

void NetworkManager::sendModuleUnfit(const std::string& slotType, int slotIndex) {
    if (!isConnected()) return;
    
    std::string msg = m_protocolHandler->createModuleUnfitMessage(slotType, slotIndex);
    m_tcpClient->send(msg);
    std::cout << "Sent module unfit request: " << slotType << "[" << slotIndex << "]" << std::endl;
}

void NetworkManager::sendModuleActivate(int slotIndex) {
    if (!isConnected()) return;
    
    std::string msg = m_protocolHandler->createModuleActivateMessage(slotIndex);
    m_tcpClient->send(msg);
}

// Market operations
void NetworkManager::sendMarketBuy(const std::string& itemId, int quantity, double price) {
    if (!isConnected()) return;
    
    std::string msg = m_protocolHandler->createMarketBuyMessage(itemId, quantity, price);
    m_tcpClient->send(msg);
    std::cout << "Sent market buy: " << itemId << " x" << quantity 
              << " @ " << price << " ISK" << std::endl;
}

void NetworkManager::sendMarketSell(const std::string& itemId, int quantity, double price) {
    if (!isConnected()) return;
    
    std::string msg = m_protocolHandler->createMarketSellMessage(itemId, quantity, price);
    m_tcpClient->send(msg);
    std::cout << "Sent market sell: " << itemId << " x" << quantity 
              << " @ " << price << " ISK" << std::endl;
}

void NetworkManager::sendMarketQuery(const std::string& itemId) {
    if (!isConnected()) return;
    
    std::string msg = m_protocolHandler->createMarketQueryMessage(itemId);
    m_tcpClient->send(msg);
}

std::string NetworkManager::getConnectionState() const {
    switch (m_state) {
        case State::DISCONNECTED: return "Disconnected";
        case State::CONNECTING: return "Connecting...";
        case State::CONNECTED: return "Connected";
        case State::AUTHENTICATED: return "Authenticated";
        default: return "Unknown";
    }
}

void NetworkManager::onRawMessage(const std::string& message) {
    // Parse and dispatch through protocol handler
    m_protocolHandler->handleMessage(message);
}

void NetworkManager::onProtocolMessage(const std::string& type, const std::string& dataJson) {
    // Handle connection acknowledgment
    if (type == "connect_ack") {
        m_state = State::AUTHENTICATED;
        m_authenticated = true;
        std::cout << "Connection acknowledged by server" << std::endl;
    } else if (type == "error") {
        handleErrorResponse(dataJson);
    }
    // Handle response messages
    else if (ProtocolHandler::isInventoryResponse(type)) {
        handleInventoryResponse(type, dataJson);
    } else if (ProtocolHandler::isFittingResponse(type)) {
        handleFittingResponse(type, dataJson);
    } else if (ProtocolHandler::isMarketResponse(type)) {
        handleMarketResponse(type, dataJson);
    }

    // Dispatch to registered handlers
    auto it = m_handlers.find(type);
    if (it != m_handlers.end()) {
        it->second(dataJson);
    }
}

void NetworkManager::handleInventoryResponse(const std::string& type, const std::string& dataJson) {
    if (!m_inventoryCallback) return;
    
    try {
        auto j = nlohmann::json::parse(dataJson);
        
        InventoryResponse response;
        response.success = ProtocolHandler::isSuccessResponse(type);
        response.message = j.value("message", response.success ? "Operation completed" : "Operation failed");
        response.itemId = j.value("item_id", "");
        response.quantity = j.value("quantity", 0);
        
        m_inventoryCallback(response);
        
    } catch (const nlohmann::json::exception& e) {
        std::cerr << "Failed to parse inventory response: " << e.what() << std::endl;
    }
}

void NetworkManager::handleFittingResponse(const std::string& type, const std::string& dataJson) {
    if (!m_fittingCallback) return;
    
    try {
        auto j = nlohmann::json::parse(dataJson);
        
        FittingResponse response;
        response.success = ProtocolHandler::isSuccessResponse(type);
        response.message = j.value("message", response.success ? "Operation completed" : "Operation failed");
        response.moduleId = j.value("module_id", "");
        response.slotType = j.value("slot_type", "");
        response.slotIndex = j.value("slot_index", -1);
        
        m_fittingCallback(response);
        
    } catch (const nlohmann::json::exception& e) {
        std::cerr << "Failed to parse fitting response: " << e.what() << std::endl;
    }
}

void NetworkManager::handleMarketResponse(const std::string& type, const std::string& dataJson) {
    if (!m_marketCallback) return;
    
    try {
        auto j = nlohmann::json::parse(dataJson);
        
        MarketResponse response;
        response.success = ProtocolHandler::isSuccessResponse(type);
        response.message = j.value("message", response.success ? "Transaction completed" : "Transaction failed");
        response.itemId = j.value("item_id", "");
        response.quantity = j.value("quantity", 0);
        response.price = j.value("price", 0.0);
        
        // Calculate total cost: prefer server value, calculate if both price and quantity are present
        if (j.contains("total_cost")) {
            response.totalCost = j["total_cost"];
        } else if (response.price > 0.0 && response.quantity > 0) {
            response.totalCost = response.price * response.quantity;
        } else {
            response.totalCost = 0.0;
        }
        
        m_marketCallback(response);
        
    } catch (const nlohmann::json::exception& e) {
        std::cerr << "Failed to parse market response: " << e.what() << std::endl;
    }
}

void NetworkManager::handleErrorResponse(const std::string& dataJson) {
    if (!m_errorCallback) {
        std::cerr << "Server error: " << dataJson << std::endl;
        return;
    }
    
    try {
        auto j = nlohmann::json::parse(dataJson);
        std::string message = j.value("message", "Unknown error");
        m_errorCallback(message);
        
    } catch (const nlohmann::json::exception& e) {
        std::cerr << "Failed to parse error response: " << e.what() << std::endl;
        m_errorCallback("Unknown error");
    }
}

} // namespace atlas
