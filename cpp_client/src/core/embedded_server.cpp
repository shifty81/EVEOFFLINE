#include "core/embedded_server.h"
#include <iostream>
#include <chrono>

// Define the placeholder Server class (forward-declared in embedded_server.h)
namespace eve { namespace server {
class Server {};
} }

namespace eve {

EmbeddedServer::EmbeddedServer()
    : m_running(false)
    , m_shouldStop(false)
    , m_uptime(0.0)
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

    // TODO: Initialize actual server from cpp_server
    // For now, simulate server startup
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
    status.connected_players = 0; // TODO: Get from actual server
    status.max_players = m_config.max_players;
    status.server_name = m_config.server_name;
    status.port = m_config.port;
    status.uptime_seconds = m_uptime;
    status.current_system = "Unknown"; // TODO: Get from actual server

    return status;
}

std::string EmbeddedServer::getLocalAddress() const {
    return "127.0.0.1";
}

void EmbeddedServer::update(float deltaTime) {
    if (m_running) {
        m_uptime += deltaTime;
    }
}

void EmbeddedServer::serverThread() {
    std::cout << "[Server Thread] Started" << std::endl;

    // TODO: Run actual server tick loop from cpp_server
    // For now, just simulate server running
    while (!m_shouldStop) {
        // Simulate server tick (30 Hz)
        std::this_thread::sleep_for(std::chrono::milliseconds(33));
        
        // TODO: Call server->update()
    }

    std::cout << "[Server Thread] Stopped" << std::endl;
}

} // namespace eve
