#ifndef EVE_PROTOCOL_HANDLER_H
#define EVE_PROTOCOL_HANDLER_H

#include <string>
#include <functional>
#include <map>

namespace atlas {
namespace network {

/**
 * @brief Message types matching Python server protocol
 */
enum class MessageType {
    CONNECT,
    CONNECT_ACK,
    DISCONNECT,
    INPUT_MOVE,
    STATE_UPDATE,
    CHAT,
    COMMAND,
    SPAWN_ENTITY,
    REMOVE_ENTITY,
    TARGET_LOCK,
    TARGET_UNLOCK,
    MODULE_ACTIVATE,
    MODULE_DEACTIVATE,
    WORMHOLE_SCAN,
    WORMHOLE_JUMP,
    ERROR
};

/**
 * @brief Protocol handler for JSON-based messages
 * 
 * Compatible with existing Python client/server protocol
 */
class ProtocolHandler {
public:
    ProtocolHandler();
    
    // Message parsing
    bool parseMessage(const std::string& json, MessageType& type, std::string& data);
    
    // Message creation
    std::string createConnectAck(bool success, const std::string& message);
    std::string createStateUpdate(const std::string& game_state);
    std::string createChatMessage(const std::string& sender, const std::string& message);
    std::string createError(const std::string& error_message);
    
    // Message validation
    bool validateMessage(const std::string& json);
    
private:
    std::map<std::string, MessageType> message_type_map_;
    
    void initializeMessageTypes();
    std::string messageTypeToString(MessageType type);
};

} // namespace network
} // namespace atlas

#endif // EVE_PROTOCOL_HANDLER_H
