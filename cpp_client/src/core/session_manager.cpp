#include "core/session_manager.h"
#include "core/embedded_server.h"
#include <iostream>

namespace eve {

SessionManager::SessionManager()
    : m_currentType(SessionType::SinglePlayer)
    , m_isHost(false)
    , m_hostedServer(nullptr)
{
}

SessionManager::~SessionManager() {
    leaveSession();
}

bool SessionManager::hostSession(const SessionConfig& config, EmbeddedServer* server) {
    if (isInSession()) {
        std::cerr << "Already in a session!" << std::endl;
        return false;
    }

    if (!server) {
        std::cerr << "No embedded server provided!" << std::endl;
        return false;
    }

    std::cout << "Hosting multiplayer session: " << config.session_name << std::endl;

    // Set up current session info
    m_currentSession.id = "local_host";
    m_currentSession.name = config.session_name;
    m_currentSession.host_address = "localhost";
    m_currentSession.port = 8765; // Default port
    m_currentSession.current_players = 1; // Host
    m_currentSession.max_players = config.max_players;
    m_currentSession.password_protected = config.use_password;
    m_currentSession.lan_only = config.lan_only;
    m_currentSession.ping_ms = 0.0f;
    m_currentSession.game_mode = "PVE Co-op";
    m_currentSession.description = config.description;

    m_currentType = SessionType::HostedMultiplayer;
    m_isHost = true;
    m_hostedServer = server;

    std::cout << "Session hosted successfully!" << std::endl;
    std::cout << "  Address: " << m_currentSession.host_address << ":" << m_currentSession.port << std::endl;
    std::cout << "  Max Players: " << m_currentSession.max_players << std::endl;

    return true;
}

bool SessionManager::joinSession(const std::string& host, int port, const std::string& password) {
    if (isInSession()) {
        std::cerr << "Already in a session!" << std::endl;
        return false;
    }

    std::cout << "Joining session at " << host << ":" << port << std::endl;

    // Set up current session info
    m_currentSession.id = host + ":" + std::to_string(port);
    m_currentSession.name = "Remote Game";
    m_currentSession.host_address = host;
    m_currentSession.port = port;
    m_currentSession.current_players = 0; // Will be updated
    m_currentSession.max_players = 0; // Will be updated
    m_currentSession.password_protected = !password.empty();
    m_currentSession.lan_only = false;
    m_currentSession.ping_ms = 0.0f;
    m_currentSession.game_mode = "PVE Co-op";
    m_currentSession.description = "";

    m_currentType = SessionType::JoinedMultiplayer;
    m_isHost = false;

    // TODO: Actually connect to server
    std::cout << "Connected to session!" << std::endl;

    return true;
}

void SessionManager::leaveSession() {
    if (!isInSession()) {
        return;
    }

    std::cout << "Leaving session..." << std::endl;

    if (m_isHost && m_hostedServer) {
        std::cout << "Stopping hosted server..." << std::endl;
        // Server will be stopped by Application
    }

    m_currentType = SessionType::SinglePlayer;
    m_isHost = false;
    m_hostedServer = nullptr;
    m_players.clear();

    if (m_onSessionEnded) {
        m_onSessionEnded("Session ended");
    }

    std::cout << "Left session." << std::endl;
}

const SessionManager::SessionInfo* SessionManager::getCurrentSession() const {
    if (!isInSession()) {
        return nullptr;
    }
    return &m_currentSession;
}

std::vector<SessionManager::SessionInfo> SessionManager::scanLAN() {
    std::cout << "Scanning for LAN sessions..." << std::endl;

    // TODO: Implement UDP broadcast discovery
    std::vector<SessionInfo> sessions;

    // Mock data for testing
    SessionInfo mockSession;
    mockSession.id = "lan_1";
    mockSession.name = "Friend's Game";
    mockSession.host_address = "192.168.1.100";
    mockSession.port = 8765;
    mockSession.current_players = 2;
    mockSession.max_players = 20;
    mockSession.password_protected = false;
    mockSession.lan_only = true;
    mockSession.ping_ms = 15.0f;
    mockSession.game_mode = "PVE Co-op";
    mockSession.description = "Mining and missions";

    // sessions.push_back(mockSession);

    std::cout << "Found " << sessions.size() << " LAN session(s)" << std::endl;
    return sessions;
}

bool SessionManager::invitePlayer(const std::string& player_name) {
    if (!m_isHost) {
        std::cerr << "Only host can invite players!" << std::endl;
        return false;
    }

    std::cout << "Inviting player: " << player_name << std::endl;
    // TODO: Send invite through network
    return true;
}

bool SessionManager::kickPlayer(const std::string& player_name) {
    if (!m_isHost) {
        std::cerr << "Only host can kick players!" << std::endl;
        return false;
    }

    std::cout << "Kicking player: " << player_name << std::endl;
    // TODO: Kick player from server
    return true;
}

std::vector<SessionManager::PlayerInfo> SessionManager::getPlayers() const {
    // TODO: Get actual player list from server
    return m_players;
}

void SessionManager::update(float deltaTime) {
    if (!isInSession()) {
        return;
    }

    // TODO: Update session state, handle events, etc.
    
    // Update player list if hosting
    if (m_isHost && m_hostedServer) {
        auto status = m_hostedServer->getStatus();
        m_currentSession.current_players = status.connected_players + 1; // +1 for host
    }
}

} // namespace eve
