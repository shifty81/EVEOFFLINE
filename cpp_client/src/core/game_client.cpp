#include "core/game_client.h"
#include "core/entity_message_parser.h"
#include <iostream>

namespace atlas {

GameClient::GameClient() {
    std::cout << "GameClient created" << std::endl;
    setupMessageHandlers();
}

GameClient::~GameClient() {
    disconnect();
}

void GameClient::setupMessageHandlers() {
    // Register handlers for entity-related messages
    m_networkManager.registerHandler("spawn_entity", [this](const std::string& data) {
        handleSpawnEntity(data);
    });
    
    m_networkManager.registerHandler("destroy_entity", [this](const std::string& data) {
        handleDestroyEntity(data);
    });
    
    m_networkManager.registerHandler("state_update", [this](const std::string& data) {
        handleStateUpdate(data);
    });
    
    m_networkManager.registerHandler("connect_ack", [this](const std::string& data) {
        handleConnectAck(data);
    });
}

bool GameClient::connect(const std::string& host, int port, const std::string& characterName) {
    std::cout << "GameClient: Connecting to " << host << ":" << port << " as " << characterName << std::endl;
    
    m_characterName = characterName;
    
    // Generate player ID (could be more sophisticated)
    std::string playerId = "player_" + characterName;
    
    // Connect via network manager
    bool success = m_networkManager.connect(host, port, playerId, characterName);
    
    if (success) {
        std::cout << "GameClient: Connected successfully" << std::endl;
    } else {
        std::cerr << "GameClient: Connection failed" << std::endl;
    }
    
    return success;
}

void GameClient::disconnect() {
    if (isConnected()) {
        std::cout << "GameClient: Disconnecting..." << std::endl;
        m_networkManager.disconnect();
        m_entityManager.clear();
        m_playerEntityId.clear();
    }
}

bool GameClient::isConnected() const {
    return m_networkManager.isConnected();
}

void GameClient::update(float deltaTime) {
    // Process network messages
    m_networkManager.update();
    
    // Update entity interpolation
    m_entityManager.update(deltaTime);
}

void GameClient::sendMove(float vx, float vy, float vz) {
    m_networkManager.sendMove(vx, vy, vz);
}

void GameClient::sendChat(const std::string& message) {
    m_networkManager.sendChat(message);
}

void GameClient::handleSpawnEntity(const std::string& dataJson) {
    if (!EntityMessageParser::parseSpawnEntity(dataJson, m_entityManager)) {
        std::cerr << "GameClient: Failed to parse SPAWN_ENTITY message" << std::endl;
    }
}

void GameClient::handleDestroyEntity(const std::string& dataJson) {
    if (!EntityMessageParser::parseDestroyEntity(dataJson, m_entityManager)) {
        std::cerr << "GameClient: Failed to parse DESTROY_ENTITY message" << std::endl;
    }
}

void GameClient::handleStateUpdate(const std::string& dataJson) {
    if (!EntityMessageParser::parseStateUpdate(dataJson, m_entityManager)) {
        std::cerr << "GameClient: Failed to parse STATE_UPDATE message" << std::endl;
    }
}

void GameClient::handleConnectAck(const std::string& dataJson) {
    // Parse player entity ID from connect acknowledgment
    try {
        auto data = nlohmann::json::parse(dataJson);
        if (data.contains("player_entity_id")) {
            m_playerEntityId = data["player_entity_id"].get<std::string>();
            std::cout << "GameClient: Assigned player entity ID: " << m_playerEntityId << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "GameClient: Failed to parse connect_ack: " << e.what() << std::endl;
    }
}

} // namespace atlas
