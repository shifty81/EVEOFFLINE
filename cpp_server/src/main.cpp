#include "server.h"
#include "utils/logger.h"
#include <iostream>
#include <csignal>
#include <memory>
#include <exception>

static std::unique_ptr<atlas::Server> g_server;

void signalHandler(int signal) {
    const char* name = (signal == SIGINT)  ? "SIGINT"  :
                       (signal == SIGTERM) ? "SIGTERM" : "UNKNOWN";
    atlas::utils::Logger::instance().info(
        std::string("Received signal ") + name + ", shutting down...");
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
    
    try {
        // Create and initialize server
        g_server = std::make_unique<atlas::Server>(config_path);
        
        if (!g_server->initialize()) {
            atlas::utils::Logger::instance().fatal("Failed to initialize server");
            return 1;
        }
        
        // Run server (blocks until stopped)
        g_server->run();
    } catch (const std::exception& e) {
        atlas::utils::Logger::instance().fatal(
            std::string("Unhandled exception: ") + e.what());
        return 1;
    } catch (...) {
        atlas::utils::Logger::instance().fatal("Unhandled unknown exception");
        return 1;
    }
    
    return 0;
}
