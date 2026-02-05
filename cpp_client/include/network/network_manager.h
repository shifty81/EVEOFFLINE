#pragma once

#include "network/tcp_client.h"
#include "network/protocol_handler.h"
#include <functional>
#include <memory>
#include <map>
#include <string>

namespace eve {

/**
 * High-level network manager
 * Combines TCP client and protocol handler for easy game integration
 */
class NetworkManager {
public:
    // Message handler for specific message types
    using TypedMessageHandler = std::function<void(const std::string& dataJson)>;

    NetworkManager();
    ~NetworkManager();

    /**
     * Connect to game server
     * @param host Server hostname or IP
     * @param port Server port
     * @param playerId Player ID (can be generated)
     * @param characterName Character name
     * @return true if connection successful
     */
    bool connect(const std::string& host, int port, 
                 const std::string& playerId, const std::string& characterName);

    /**
     * Disconnect from server
     */
    void disconnect();

    /**
     * Check if connected
     */
    bool isConnected() const;

    /**
     * Update network (process messages)
     * Should be called every frame
     */
    void update();

    /**
     * Register handler for specific message type
     * @param type Message type (e.g., "STATE_UPDATE", "SPAWN_ENTITY")
     * @param handler Handler function
     */
    void registerHandler(const std::string& type, TypedMessageHandler handler);

    /**
     * Send movement input
     */
    void sendMove(float vx, float vy, float vz);

    /**
     * Send chat message
     */
    void sendChat(const std::string& message);
    
    /**
     * Inventory management
     */
    void sendInventoryTransfer(const std::string& itemId, int quantity, bool fromCargo, bool toCargo);
    void sendInventoryJettison(const std::string& itemId, int quantity);
    
    /**
     * Module fitting
     */
    void sendModuleFit(const std::string& moduleId, const std::string& slotType, int slotIndex);
    void sendModuleUnfit(const std::string& slotType, int slotIndex);
    void sendModuleActivate(int slotIndex);
    
    /**
     * Market operations
     */
    void sendMarketBuy(const std::string& itemId, int quantity, double price);
    void sendMarketSell(const std::string& itemId, int quantity, double price);
    void sendMarketQuery(const std::string& itemId);

    /**
     * Get connection state string
     */
    std::string getConnectionState() const;

private:
    void onRawMessage(const std::string& message);
    void onProtocolMessage(const std::string& type, const std::string& dataJson);

    std::unique_ptr<TCPClient> m_tcpClient;
    std::unique_ptr<ProtocolHandler> m_protocolHandler;
    
    // Message handlers by type
    std::map<std::string, TypedMessageHandler> m_handlers;
    
    // Connection info
    std::string m_playerId;
    std::string m_characterName;
    bool m_authenticated;
    
    enum class State {
        DISCONNECTED,
        CONNECTING,
        CONNECTED,
        AUTHENTICATED
    };
    State m_state;
};

} // namespace eve
