#pragma once

/**
 * @file server_console.h
 * @brief Planned server admin console interface (future work)
 *
 * This header defines the interface for a future server administration
 * console that will provide:
 *   - Real-time log viewing with level filtering
 *   - On-the-fly configuration changes (tick rate, max players, etc.)
 *   - Player management commands (kick, ban, inspect)
 *   - Performance metrics monitoring
 *
 * Implementation phases:
 *   Phase 1: Command-line console (stdin/stdout)
 *   Phase 2: Terminal UI dashboard (FTXUI/ncurses)
 *   Phase 3: Graphical dashboard (SDL2 + ImGui)
 *
 * See docs/server_gui_design.md for full design specification.
 *
 * @note This is a placeholder header. Implementation will follow once
 *       the client GUI work is stabilized.
 */

#include <string>
#include <vector>
#include <functional>

namespace eve {

// Forward declarations
class Server;
struct ServerConfig;

namespace utils {
    enum class LogLevel;
}

/**
 * ServerConsole — administrative console for the dedicated server.
 *
 * Provides a text-based command interface and log viewer.
 * All methods are no-ops in the current placeholder implementation.
 */
class ServerConsole {
public:
    ServerConsole() = default;
    ~ServerConsole() = default;

    // Non-copyable
    ServerConsole(const ServerConsole&) = delete;
    ServerConsole& operator=(const ServerConsole&) = delete;

    /**
     * Initialize the console.
     * @param server  Reference to the running server instance.
     * @param config  Reference to the server configuration.
     * @return true on success.
     */
    bool init(Server& server, const ServerConfig& config) {
        (void)server;
        (void)config;
        // TODO: Phase 1 — set up non-blocking stdin reader
        return true;
    }

    /**
     * Process one frame of console I/O.
     * Call from the server's main loop each tick.
     */
    void update() {
        // TODO: Phase 1 — poll stdin for commands, dispatch
    }

    /**
     * Shutdown the console and restore terminal state.
     */
    void shutdown() {
        // TODO: Phase 1 — cleanup
    }

    /**
     * Add a log message to the console output buffer.
     * @param level    Log severity level.
     * @param message  Log message text.
     */
    void addLogMessage(utils::LogLevel level, const std::string& message) {
        (void)level;
        (void)message;
        // TODO: Phase 1 — buffer for display
    }

    /**
     * Execute a command string and return the result.
     * @param command  Command text (e.g. "status", "kick player1").
     * @return Command output string.
     */
    std::string executeCommand(const std::string& command) {
        (void)command;
        // TODO: Phase 1 — command dispatcher
        return "Console not yet implemented";
    }

    /**
     * Set whether the console operates in interactive mode.
     * @param interactive  true for interactive (captures stdin), false for headless.
     */
    void setInteractive(bool interactive) {
        m_interactive = interactive;
    }

    bool isInteractive() const { return m_interactive; }

private:
    bool m_interactive = false;
};

} // namespace eve
