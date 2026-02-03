#pragma once

#include <string>
#include <memory>
#include <functional>

namespace eve {

// Forward declarations
class TCPClient;
class ProtocolHandler;

/**
 * Game client - manages connection to server and game state
 */
class GameClient {
public:
    GameClient();
    ~GameClient();

    /**
     * Connect to game server
     */
    bool connect(const std::string& host, int port);

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
     */
    void update(float deltaTime);

    /**
     * Send player input to server
     */
    void sendInput(const std::string& command);

    /**
     * Get player entity ID
     */
    uint32_t getPlayerEntityId() const { return m_playerEntityId; }

private:
    void handleServerMessage(const std::string& message);

    std::unique_ptr<TCPClient> m_client;
    std::unique_ptr<ProtocolHandler> m_protocolHandler;

    uint32_t m_playerEntityId;
    std::string m_characterName;
    bool m_connected;
};

} // namespace eve
