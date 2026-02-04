#pragma once

#include <memory>
#include <string>
#include <thread>
#include <atomic>

// Forward declare server components
namespace eve {
namespace server {
    class Server;
}
}

namespace eve {

/**
 * Embedded server manager for hosting games from the client
 * Allows players to host multiplayer sessions without a dedicated server
 */
class EmbeddedServer {
public:
    struct Config {
        std::string server_name = "My Game";
        std::string description = "EVE OFFLINE Game";
        int port = 8765;
        int max_players = 20;
        bool use_password = false;
        std::string password = "";
        bool lan_only = false;
        bool persistent_world = false;
        int auto_save_interval = 300; // seconds
        std::string data_path = "../data";
        std::string save_path = "./saves";
    };

    EmbeddedServer();
    ~EmbeddedServer();

    // Delete copy constructor and assignment
    EmbeddedServer(const EmbeddedServer&) = delete;
    EmbeddedServer& operator=(const EmbeddedServer&) = delete;

    /**
     * Start the embedded server with given configuration
     */
    bool start(const Config& config);

    /**
     * Stop the embedded server
     */
    void stop();

    /**
     * Check if server is currently running
     */
    bool isRunning() const { return m_running; }

    /**
     * Get server status information
     */
    struct Status {
        bool running;
        int connected_players;
        int max_players;
        std::string server_name;
        int port;
        double uptime_seconds;
        std::string current_system;
    };
    Status getStatus() const;

    /**
     * Get local server address for auto-connect
     */
    std::string getLocalAddress() const;

    /**
     * Get server port
     */
    int getPort() const { return m_config.port; }

    /**
     * Update server (call from main thread)
     */
    void update(float deltaTime);

private:
    void serverThread();

    std::unique_ptr<server::Server> m_server;
    std::unique_ptr<std::thread> m_serverThread;
    std::atomic<bool> m_running;
    std::atomic<bool> m_shouldStop;
    Config m_config;
    double m_uptime;
};

} // namespace eve
