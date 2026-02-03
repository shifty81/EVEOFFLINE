#include "server.h"
#include <csignal>
#include <memory>

static std::unique_ptr<eve::Server> g_server;

void signalHandler(int signal) {
    if (g_server) {
        g_server->stop();
    }
}

int main(int argc, char* argv[]) {
    // Parse command line arguments
    std::string config_path = "config/server.json";
    if (argc > 1) {
        config_path = argv[1];
    }
    
    // Setup signal handlers for graceful shutdown
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
    
    // Create and initialize server
    g_server = std::make_unique<eve::Server>(config_path);
    
    if (!g_server->initialize()) {
        std::cerr << "Failed to initialize server" << std::endl;
        return 1;
    }
    
    // Run server (blocks until stopped)
    g_server->run();
    
    return 0;
}
