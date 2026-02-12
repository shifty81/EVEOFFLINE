#include "core/session_manager.h"
#include "core/embedded_server.h"
#include "network/network_manager.h"
#include <iostream>
#include <chrono>
#include <cstring>
#include <random>

// Platform-specific socket headers
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
#endif

namespace atlas {

SessionManager::SessionManager()
    : m_currentType(SessionType::SinglePlayer)
    , m_isHost(false)
    , m_hostedServer(nullptr)
    , m_networkManager(nullptr)
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

    // Create network manager if not exists
    if (!m_networkManager) {
        m_networkManager = std::make_unique<NetworkManager>();
    }

    // Connect to server with a unique player ID
    // Generate a UUID-like player ID using random device
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 999999);
    std::string playerId = "player_" + std::to_string(dis(gen));
    std::string characterName = "Commander"; // Default character name
    
    if (!m_networkManager->connect(host, port, playerId, characterName)) {
        std::cerr << "Failed to connect to server!" << std::endl;
        m_currentType = SessionType::SinglePlayer;
        return false;
    }

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

    // Disconnect network if we're connected
    if (m_networkManager && m_networkManager->isConnected()) {
        m_networkManager->disconnect();
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

    std::vector<SessionInfo> sessions;

    // Simple UDP broadcast discovery implementation
    // This is a basic implementation that sends a broadcast packet and listens for responses
    
#ifdef _WIN32
    // Windows socket implementation
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed" << std::endl;
        return sessions;
    }
    
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Failed to create UDP socket" << std::endl;
        WSACleanup();
        return sessions;
    }
    
    // Enable broadcast
    BOOL broadcast = TRUE;
    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char*)&broadcast, sizeof(broadcast)) < 0) {
        std::cerr << "Failed to enable broadcast" << std::endl;
        closesocket(sock);
        WSACleanup();
        return sessions;
    }
    
    // Set socket to non-blocking for timeout
    u_long mode = 1;
    ioctlsocket(sock, FIONBIO, &mode);
    
    // Prepare broadcast message
    sockaddr_in broadcastAddr;
    memset(&broadcastAddr, 0, sizeof(broadcastAddr));
    broadcastAddr.sin_family = AF_INET;
    broadcastAddr.sin_port = htons(8766); // Discovery port
    broadcastAddr.sin_addr.s_addr = INADDR_BROADCAST;
    
    // Send discovery packet
    const char* discoverMsg = "EVE_OFFLINE_DISCOVER";
    sendto(sock, discoverMsg, strlen(discoverMsg), 0, (sockaddr*)&broadcastAddr, sizeof(broadcastAddr));
    
    // Wait for responses (with 1 second timeout)
    auto startTime = std::chrono::steady_clock::now();
    while (std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - startTime).count() < 1000) {
        
        char buffer[1024];
        sockaddr_in senderAddr;
        int senderLen = sizeof(senderAddr);
        
        int received = recvfrom(sock, buffer, sizeof(buffer) - 1, 0, 
                               (sockaddr*)&senderAddr, &senderLen);
        
        if (received > 0) {
            buffer[received] = '\0';
            
            // Parse response (format: "EVE_OFFLINE_SESSION:name:port:players:maxplayers")
            std::string response(buffer);
            if (response.find("EVE_OFFLINE_SESSION:") == 0) {
                // Parse session info
                SessionInfo info;
                info.host_address = inet_ntoa(senderAddr.sin_addr);
                // Additional parsing would go here
                sessions.push_back(info);
            }
        }
    }
    
    closesocket(sock);
    WSACleanup();
    
#else
    // Unix/Linux socket implementation
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        std::cerr << "Failed to create UDP socket" << std::endl;
        return sessions;
    }
    
    // Enable broadcast
    int broadcast = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) < 0) {
        std::cerr << "Failed to enable broadcast" << std::endl;
        close(sock);
        return sessions;
    }
    
    // Set socket timeout
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    
    // Prepare broadcast message
    struct sockaddr_in broadcastAddr;
    memset(&broadcastAddr, 0, sizeof(broadcastAddr));
    broadcastAddr.sin_family = AF_INET;
    broadcastAddr.sin_port = htons(8766); // Discovery port
    broadcastAddr.sin_addr.s_addr = INADDR_BROADCAST;
    
    // Send discovery packet
    const char* discoverMsg = "EVE_OFFLINE_DISCOVER";
    sendto(sock, discoverMsg, strlen(discoverMsg), 0, 
           (struct sockaddr*)&broadcastAddr, sizeof(broadcastAddr));
    
    // Wait for responses
    auto startTime = std::chrono::steady_clock::now();
    while (std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - startTime).count() < 1000) {
        
        char buffer[1024];
        struct sockaddr_in senderAddr;
        socklen_t senderLen = sizeof(senderAddr);
        
        ssize_t received = recvfrom(sock, buffer, sizeof(buffer) - 1, 0, 
                                   (struct sockaddr*)&senderAddr, &senderLen);
        
        if (received > 0) {
            buffer[received] = '\0';
            
            // Parse response (format: "EVE_OFFLINE_SESSION:name:port:players:maxplayers")
            std::string response(buffer);
            if (response.find("EVE_OFFLINE_SESSION:") == 0) {
                // Parse session info
                SessionInfo info;
                char addrStr[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &senderAddr.sin_addr, addrStr, INET_ADDRSTRLEN);
                info.host_address = addrStr;
                // Additional parsing would go here
                sessions.push_back(info);
            }
        }
    }
    
    close(sock);
#endif

    std::cout << "Found " << sessions.size() << " LAN session(s)" << std::endl;
    return sessions;
}

bool SessionManager::invitePlayer(const std::string& player_name) {
    if (!m_isHost) {
        std::cerr << "Only host can invite players!" << std::endl;
        return false;
    }

    std::cout << "Inviting player: " << player_name << std::endl;
    
    // Send invite through network if available
    // Note: In production, this should use a dedicated INVITE message type
    // instead of a chat-based protocol to avoid conflicts with actual chat
    if (m_networkManager && m_networkManager->isConnected()) {
        // Send a chat message as a simple invite mechanism (temporary)
        m_networkManager->sendChat("INVITE:" + player_name);
        std::cout << "Invite sent to " << player_name << std::endl;
        return true;
    }
    
    std::cerr << "Network manager not available" << std::endl;
    return false;
}

bool SessionManager::kickPlayer(const std::string& player_name) {
    if (!m_isHost) {
        std::cerr << "Only host can kick players!" << std::endl;
        return false;
    }

    std::cout << "Kicking player: " << player_name << std::endl;
    
    // Send kick command through network if available
    // Note: In production, this should use a dedicated KICK message type
    // instead of a chat-based protocol to avoid conflicts with actual chat
    if (m_networkManager && m_networkManager->isConnected()) {
        // Send a chat message as a simple kick mechanism (temporary)
        m_networkManager->sendChat("KICK:" + player_name);
        
        // Remove from local player list
        m_players.erase(
            std::remove_if(m_players.begin(), m_players.end(),
                [&player_name](const PlayerInfo& p) { return p.name == player_name; }),
            m_players.end()
        );
        
        if (m_onPlayerLeft) {
            m_onPlayerLeft(player_name);
        }
        
        std::cout << "Player " << player_name << " kicked" << std::endl;
        return true;
    }
    
    std::cerr << "Network manager not available" << std::endl;
    return false;
}

std::vector<SessionManager::PlayerInfo> SessionManager::getPlayers() const {
    // Return the current player list
    // The list is updated in the update() method from server state
    return m_players;
}

void SessionManager::update(float deltaTime) {
    if (!isInSession()) {
        return;
    }

    // Update network manager
    if (m_networkManager && m_networkManager->isConnected()) {
        m_networkManager->update();
    }
    
    // Update player list if hosting
    if (m_isHost && m_hostedServer) {
        auto status = m_hostedServer->getStatus();
        m_currentSession.current_players = status.connected_players + 1; // +1 for host
        
        // In a full implementation, we would query the server for the actual player list
        // and update m_players accordingly. For now, we just track the count.
    }
}

} // namespace atlas
