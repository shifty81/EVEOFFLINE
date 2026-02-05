#pragma once

#include "core/entity_manager.h"
#include "network/network_manager.h"
#include <string>
#include <memory>
#include <functional>

namespace eve {

/**
 * Game client - manages connection to server and game state
 * Integrates networking and entity management
 */
class GameClient {
public:
    GameClient();
    ~GameClient();

    /**
     * Connect to game server
     * @param host Server hostname or IP
     * @param port Server port
     * @param characterName Character name for login
     */
    bool connect(const std::string& host, int port, const std::string& characterName);

    /**
     * Disconnect from server
     */
    void disconnect();

    /**
     * Check if connected
     */
    bool isConnected() const;

    /**
     * Update game state (call each frame)
     * @param deltaTime Time since last frame in seconds
     */
    void update(float deltaTime);

    /**
     * Send movement command to server
     */
    void sendMove(float vx, float vy, float vz);

    /**
     * Send chat message to server
     */
    void sendChat(const std::string& message);

    /**
     * Get entity manager (for rendering integration)
     */
    EntityManager& getEntityManager() { return m_entityManager; }
    const EntityManager& getEntityManager() const { return m_entityManager; }
    
    /**
     * Get network manager (for UI integration)
     */
    NetworkManager* getNetworkManager() { return &m_networkManager; }

    /**
     * Get player entity ID
     */
    const std::string& getPlayerEntityId() const { return m_playerEntityId; }

    /**
     * Set callbacks for entity events
     */
    void setOnEntitySpawned(EntityManager::EntityCallback callback) {
        m_entityManager.setOnEntitySpawned(callback);
    }
    
    void setOnEntityDestroyed(EntityManager::EntityCallback callback) {
        m_entityManager.setOnEntityDestroyed(callback);
    }

private:
    void setupMessageHandlers();
    void handleSpawnEntity(const std::string& dataJson);
    void handleDestroyEntity(const std::string& dataJson);
    void handleStateUpdate(const std::string& dataJson);
    void handleConnectAck(const std::string& dataJson);

    NetworkManager m_networkManager;
    EntityManager m_entityManager;

    std::string m_playerEntityId;
    std::string m_characterName;
};

} // namespace eve
