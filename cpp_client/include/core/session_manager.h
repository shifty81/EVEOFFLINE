#pragma once

#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <algorithm>

namespace atlas {

// Forward declarations
class EmbeddedServer;
class NetworkManager;

/**
 * Multiplayer session manager
 * Handles creation, joining, and management of multiplayer game sessions
 */
class SessionManager {
public:
    enum class SessionType {
        SinglePlayer,
        HostedMultiplayer,
        JoinedMultiplayer,
        DedicatedServer
    };

    struct SessionInfo {
        std::string id;
        std::string name;
        std::string host_address;
        int port;
        int current_players;
        int max_players;
        bool password_protected;
        bool lan_only;
        float ping_ms;
        std::string game_mode;
        std::string description;
    };

    struct SessionConfig {
        std::string session_name = "My Game";
        std::string description = "EVE OFFLINE Session";
        int max_players = 20;
        bool use_password = false;
        std::string password = "";
        bool lan_only = true;
        bool persistent = false;
        int auto_save_interval = 300;
    };

    SessionManager();
    ~SessionManager();

    // Delete copy constructor and assignment
    SessionManager(const SessionManager&) = delete;
    SessionManager& operator=(const SessionManager&) = delete;

    /**
     * Create and host a new multiplayer session
     */
    bool hostSession(const SessionConfig& config, EmbeddedServer* server);

    /**
     * Join an existing multiplayer session
     */
    bool joinSession(const std::string& host, int port, const std::string& password = "");

    /**
     * Leave current session
     */
    void leaveSession();

    /**
     * Get current session type
     */
    SessionType getCurrentSessionType() const { return m_currentType; }

    /**
     * Check if currently in a session
     */
    bool isInSession() const { return m_currentType != SessionType::SinglePlayer; }

    /**
     * Get current session info
     */
    const SessionInfo* getCurrentSession() const;

    /**
     * Scan for LAN sessions
     */
    std::vector<SessionInfo> scanLAN();

    /**
     * Invite player to current session
     */
    bool invitePlayer(const std::string& player_name);

    /**
     * Kick player from hosted session
     */
    bool kickPlayer(const std::string& player_name);

    /**
     * Get list of players in current session
     */
    struct PlayerInfo {
        std::string name;
        std::string ship;
        std::string system;
        float ping_ms;
        bool is_host;
    };
    std::vector<PlayerInfo> getPlayers() const;

    /**
     * Set callbacks for session events
     */
    using SessionCallback = std::function<void(const std::string&)>;
    void setOnPlayerJoined(SessionCallback callback) { m_onPlayerJoined = callback; }
    void setOnPlayerLeft(SessionCallback callback) { m_onPlayerLeft = callback; }
    void setOnSessionEnded(SessionCallback callback) { m_onSessionEnded = callback; }

    /**
     * Update session state (call every frame)
     */
    void update(float deltaTime);

private:
    SessionType m_currentType;
    SessionInfo m_currentSession;
    std::vector<PlayerInfo> m_players;
    
    SessionCallback m_onPlayerJoined;
    SessionCallback m_onPlayerLeft;
    SessionCallback m_onSessionEnded;

    bool m_isHost;
    EmbeddedServer* m_hostedServer;
    std::unique_ptr<NetworkManager> m_networkManager;
};

} // namespace atlas
