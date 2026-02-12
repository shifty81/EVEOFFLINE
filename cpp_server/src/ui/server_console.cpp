#include "ui/server_console.h"
#include "server.h"
#include "config/server_config.h"
#include "utils/logger.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cctype>

#ifdef _WIN32
    #include <windows.h>
    #include <conio.h>
#else
    #include <termios.h>
    #include <unistd.h>
    #include <fcntl.h>
    #include <sys/select.h>
#endif

namespace atlas {

// Platform-specific stdin helpers
namespace {

#ifdef _WIN32
    bool stdinHasInput() {
        return _kbhit() != 0;
    }

    char getStdinChar() {
        return static_cast<char>(_getch());
    }

    void setNonBlockingStdin(bool /*enable*/) {
        // On Windows, _kbhit() already provides non-blocking check
    }
#else
    // Unix/Linux/macOS
    static struct termios g_old_termios;
    static bool g_termios_changed = false;

    void setNonBlockingStdin(bool enable) {
        if (enable && !g_termios_changed) {
            // Save old terminal settings
            tcgetattr(STDIN_FILENO, &g_old_termios);
            struct termios new_termios = g_old_termios;
            
            // Disable canonical mode and echo
            new_termios.c_lflag &= ~(ICANON | ECHO);
            new_termios.c_cc[VMIN] = 0;
            new_termios.c_cc[VTIME] = 0;
            
            tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);
            
            // Set stdin to non-blocking
            int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
            fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
            
            g_termios_changed = true;
        } else if (!enable && g_termios_changed) {
            // Restore old terminal settings
            tcsetattr(STDIN_FILENO, TCSANOW, &g_old_termios);
            
            // Restore blocking stdin
            int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
            fcntl(STDIN_FILENO, F_SETFL, flags & ~O_NONBLOCK);
            
            g_termios_changed = false;
        }
    }

    bool stdinHasInput() {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        
        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 0;
        
        return select(STDIN_FILENO + 1, &readfds, nullptr, nullptr, &timeout) > 0;
    }

    char getStdinChar() {
        char c = 0;
        if (read(STDIN_FILENO, &c, 1) == 1) {
            return c;
        }
        return 0;
    }
#endif

} // anonymous namespace

bool ServerConsole::init(Server& server, const ServerConfig& config) {
    server_ = &server;
    config_ = &config;
    
    if (m_interactive) {
        setNonBlockingStdin(true);
        std::cout << "\n=== EVE OFFLINE Server Console ===\n";
        std::cout << "Type 'help' for available commands\n";
        std::cout << "> " << std::flush;
    }
    
    return true;
}

void ServerConsole::update() {
    if (!m_interactive) {
        return;
    }
    
    // Read characters from stdin
    while (stdinHasInput()) {
        char c = getStdinChar();
        
        if (c == '\n' || c == '\r') {
            // Execute command
            if (!command_buffer_.empty()) {
                std::cout << "\n";
                std::string result = executeCommand(command_buffer_);
                if (!result.empty()) {
                    std::cout << result << "\n";
                }
                command_buffer_.clear();
                std::cout << "> " << std::flush;
            }
        } else if (c == 127 || c == 8) {
            // Backspace
            if (!command_buffer_.empty()) {
                command_buffer_.pop_back();
                std::cout << "\b \b" << std::flush;
            }
        } else if (c >= 32 && c < 127) {
            // Printable character
            command_buffer_ += c;
            std::cout << c << std::flush;
        }
    }
}

void ServerConsole::shutdown() {
    if (m_interactive) {
        setNonBlockingStdin(false);
        std::cout << "\nServer console shutdown.\n";
    }
    
    server_ = nullptr;
    config_ = nullptr;
}

void ServerConsole::addLogMessage(utils::LogLevel level, const std::string& message) {
    if (!m_interactive) {
        return;
    }
    
    // Store log messages for potential future display
    // For Phase 1, we just let them go to stdout via Logger
    (void)level;
    (void)message;
}

std::string ServerConsole::executeCommand(const std::string& command) {
    if (!server_ || command.empty()) {
        return "";
    }
    
    // Trim whitespace
    std::string cmd = command;
    cmd.erase(0, cmd.find_first_not_of(" \t\n\r"));
    cmd.erase(cmd.find_last_not_of(" \t\n\r") + 1);
    
    // Split into command and arguments
    std::istringstream iss(cmd);
    std::string base_cmd;
    iss >> base_cmd;
    
    // Convert to lowercase for case-insensitive comparison
    std::transform(base_cmd.begin(), base_cmd.end(), base_cmd.begin(),
                   [](unsigned char c) -> char { return static_cast<char>(std::tolower(c)); });
    
    // Dispatch to command handlers
    if (base_cmd == "help") {
        return handleHelpCommand();
    } else if (base_cmd == "status") {
        return handleStatusCommand();
    } else if (base_cmd == "players") {
        return handlePlayersCommand();
    } else if (base_cmd == "kick") {
        std::string player_name;
        std::getline(iss, player_name);
        // Trim leading whitespace
        player_name.erase(0, player_name.find_first_not_of(" \t"));
        return handleKickCommand(player_name);
    } else if (base_cmd == "stop" || base_cmd == "shutdown" || base_cmd == "quit") {
        return handleStopCommand();
    } else if (base_cmd == "metrics") {
        return handleMetricsCommand();
    } else if (base_cmd == "save") {
        return handleSaveCommand();
    } else if (base_cmd == "load") {
        return handleLoadCommand();
    } else {
        return "Unknown command: '" + base_cmd + "'. Type 'help' for available commands.";
    }
}

std::string ServerConsole::handleHelpCommand() {
    std::ostringstream oss;
    oss << "Available commands:\n";
    oss << "  help            - Show this help message\n";
    oss << "  status          - Show server status\n";
    oss << "  players         - List connected players\n";
    oss << "  kick <player>   - Kick a player (not yet implemented)\n";
    oss << "  metrics         - Show detailed performance metrics\n";
    oss << "  save            - Save world state\n";
    oss << "  load            - Load world state (not yet implemented)\n";
    oss << "  stop            - Gracefully stop the server";
    return oss.str();
}

std::string ServerConsole::handleStatusCommand() {
    std::ostringstream oss;
    oss << "Server Status:\n";
    oss << "  Running: " << (server_->isRunning() ? "Yes" : "No") << "\n";
    oss << "  Players: " << server_->getPlayerCount() << "\n";
    
    const auto& metrics = server_->getMetrics();
    oss << "  Uptime: " << metrics.getUptimeString() << "\n";
    oss << "  Entities: " << metrics.getEntityCount() << "\n";
    oss << "  Avg Tick: " << metrics.getAvgTickMs() << " ms";
    
    return oss.str();
}

std::string ServerConsole::handlePlayersCommand() {
    int count = server_->getPlayerCount();
    std::ostringstream oss;
    oss << "Connected players: " << count;
    
    // TODO: Add player list when GameSession provides player enumeration
    // For now, just show count
    
    return oss.str();
}

std::string ServerConsole::handleKickCommand(const std::string& player_name) {
    if (player_name.empty()) {
        return "Usage: kick <player_name>";
    }
    
    // TODO: Implement player kick functionality
    // Requires GameSession to provide player management API
    return "Kick command not yet implemented. Player: " + player_name;
}

std::string ServerConsole::handleStopCommand() {
    utils::Logger::instance().info("Stop command received from console");
    server_->stop();
    return "Shutting down server...";
}

std::string ServerConsole::handleMetricsCommand() {
    const auto& metrics = server_->getMetrics();
    return metrics.summary();
}

std::string ServerConsole::handleSaveCommand() {
    if (server_->saveWorld()) {
        return "World saved successfully";
    } else {
        return "Failed to save world";
    }
}

std::string ServerConsole::handleLoadCommand() {
    // Loading world state at runtime could be dangerous
    // For now, return not implemented
    return "Load command not yet implemented (use at server startup only)";
}

} // namespace atlas
