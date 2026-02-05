#pragma once

#include <string>
#include <functional>

namespace eve {

/**
 * Protocol handler for game messages (JSON-based)
 * Compatible with Python server protocol
 */
class ProtocolHandler {
public:
    using MessageHandler = std::function<void(const std::string& type, const std::string& data)>;

    ProtocolHandler();

    /**
     * Parse incoming message
     */
    void handleMessage(const std::string& message);

    /**
     * Create outgoing message
     * @param type Message type
     * @param dataJson JSON string for data field (empty for no data)
     */
    std::string createMessage(const std::string& type, const std::string& dataJson);

    /**
     * Helper methods for common messages
     */
    std::string createConnectMessage(const std::string& playerId, const std::string& characterName);
    std::string createMoveMessage(float vx, float vy, float vz);
    std::string createChatMessage(const std::string& message);

    /**
     * Set message handler
     */
    void setMessageHandler(MessageHandler handler) { m_messageHandler = handler; }

private:
    MessageHandler m_messageHandler;
};

} // namespace eve
