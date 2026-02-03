#pragma once

#include <string>
#include <functional>

namespace eve {

/**
 * Protocol handler for game messages (JSON-based)
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
     */
    std::string createMessage(const std::string& type, const std::string& data);

    /**
     * Set message handler
     */
    void setMessageHandler(MessageHandler handler) { m_messageHandler = handler; }

private:
    MessageHandler m_messageHandler;
};

} // namespace eve
