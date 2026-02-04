#include "network/protocol_handler.h"
#include <iostream>

namespace eve {

ProtocolHandler::ProtocolHandler() {
}

void ProtocolHandler::handleMessage(const std::string& message) {
    // Parse JSON message and call handler
    std::cout << "Protocol handler not yet implemented" << std::endl;
}

std::string ProtocolHandler::createMessage(const std::string& type, const std::string& data) {
    // Create JSON message
    return "{}";
}

} // namespace eve
