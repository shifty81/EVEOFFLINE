#include "network/network_manager.h"
#include <iostream>

namespace eve {

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
        std::cerr << "Server error: " << dataJson << std::endl;
    }

    // Dispatch to registered handlers
    auto it = m_handlers.find(type);
    if (it != m_handlers.end()) {
        it->second(dataJson);
    }
}

} // namespace eve
