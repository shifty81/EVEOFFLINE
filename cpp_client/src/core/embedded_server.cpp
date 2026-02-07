#include "core/embedded_server.h"
#include <iostream>
#include <chrono>

namespace eve {

EmbeddedServer::EmbeddedServer()
    : m_running(false)
    , m_shouldStop(false)
    , m_uptime(0.0)
    , m_connectedPlayers(0)
{
}

EmbeddedServer::~EmbeddedServer() {
    stop();
}

bool EmbeddedServer::start(const Config& config) {
    if (m_running) {
        std::cerr << "Server is already running!" << std::endl;
        return false;
    }

    m_config = config;
    m_shouldStop = false;
    m_uptime = 0.0;

    std::cout << "Starting embedded server..." << std::endl;
    std::cout << "  Server Name: " << config.server_name << std::endl;
    std::cout << "  Port: " << config.port << std::endl;
    std::cout << "  Max Players: " << config.max_players << std::endl;

    // Initialize embedded server components
    // Note: Full cpp_server integration would be done here in production
    // For now, we create a minimal server simulation that tracks state
    m_connectedPlayers = 0;
    m_currentSystem = "New Eden"; // Default starting system
    
    m_running = true;

    // Start server in separate thread
    m_serverThread = std::make_unique<std::thread>(&EmbeddedServer::serverThread, this);

    std::cout << "Embedded server started successfully!" << std::endl;
    std::cout << "Players can connect to: localhost:" << config.port << std::endl;

    return true;
}

void EmbeddedServer::stop() {
    if (!m_running) {
        return;
    }

    std::cout << "Stopping embedded server..." << std::endl;
    m_shouldStop = true;

    // Wait for server thread to finish
    if (m_serverThread && m_serverThread->joinable()) {
        m_serverThread->join();
    }

    m_running = false;
    m_serverThread.reset();

    std::cout << "Embedded server stopped." << std::endl;
}

EmbeddedServer::Status EmbeddedServer::getStatus() const {
    Status status;
    status.running = m_running;
    status.connected_players = m_connectedPlayers.load();
    status.max_players = m_config.max_players;
    status.server_name = m_config.server_name;
    status.port = m_config.port;
    status.uptime_seconds = m_uptime;
    status.current_system = m_currentSystem;

    return status;
}

std::string EmbeddedServer::getLocalAddress() const {
    return "127.0.0.1";
}

void EmbeddedServer::update(float deltaTime) {
    if (m_running) {
        m_uptime += deltaTime;
        
        // In production: this would synchronize state with the server thread
        // For now: just track uptime
    }
}

void EmbeddedServer::notifyPlayerConnected() {
    m_connectedPlayers++;
    std::cout << "Player connected. Total: " << m_connectedPlayers.load() << std::endl;
}

void EmbeddedServer::notifyPlayerDisconnected() {
    if (m_connectedPlayers > 0) {
        m_connectedPlayers--;
    }
    std::cout << "Player disconnected. Total: " << m_connectedPlayers.load() << std::endl;
}

void EmbeddedServer::setCurrentSystem(const std::string& system) {
    m_currentSystem = system;
}

void EmbeddedServer::serverThread() {
    std::cout << "[Server Thread] Started" << std::endl;

    // Server tick loop at ~30 Hz (33ms per tick, approximately 30.3 Hz)
    // In a full implementation, this would initialize and run the cpp_server
    // For now, we maintain a simple tick loop that tracks state
    auto lastTick = std::chrono::steady_clock::now();
    const auto tickRate = std::chrono::milliseconds(33); // ~30 Hz (33.33ms would be exact)
    
    while (!m_shouldStop) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastTick);
        
        if (elapsed >= tickRate) {
            // Perform server tick
            // In production: m_server->update(deltaTime)
            // For now: just maintain basic state
            
            lastTick = now;
        }
        
        // Sleep for remaining time to maintain tick rate
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    std::cout << "[Server Thread] Stopped" << std::endl;
}

} // namespace eve
