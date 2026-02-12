#pragma once

/**
 * @file server_console.h
 * @brief Phase 1 server admin console — command-line interface
 *
 * Provides:
 *   - Non-blocking stdin command reading
 *   - Command dispatching (status, help, kick, stop, players, uptime)
 *   - Log message buffering for display
 *
 * See docs/server_gui_design.md for full design specification.
 */

#include <string>
#include <vector>
#include <functional>
#include <sstream>
#include <map>

namespace atlas {

// Forward declarations
class Server;
struct ServerConfig;

namespace utils {
    enum class LogLevel;
}

/**
 * ServerConsole — administrative console for the dedicated server.
 *
 * Phase 1 implementation: text-based command interface with log viewer.
 */
class ServerConsole {
public:
    /// Command handler callback: takes arguments, returns output string
    using CommandHandler = std::function<std::string(const std::vector<std::string>&)>;

    ServerConsole() = default;
    ~ServerConsole() = default;

    // Non-copyable
    ServerConsole(const ServerConsole&) = delete;
    ServerConsole& operator=(const ServerConsole&) = delete;

    /**
     * Initialize the console and register built-in commands.
     * @return true on success.
     *
     * @note server and config parameters are reserved for Phase 2/3
     *       when the console needs server stats and config access.
     */
    bool init(Server& server, const ServerConfig& config);

    /**
     * Lightweight init for testing without a live Server.
     * Registers basic built-in commands.
     * @return true on success.
     */
    bool init() {
        m_initialized = true;
        registerCommand("help", "List available commands",
            [this](const std::vector<std::string>&) -> std::string {
                std::ostringstream out;
                out << "Available commands:\n";
                for (const auto& kv : m_commands) {
                    out << "  " << kv.first << " - " << kv.second.description << "\n";
                }
                return out.str();
            });
        registerCommand("status", "Show server status summary",
            [this](const std::vector<std::string>&) -> std::string {
                std::ostringstream out;
                out << "Server Status\n";
                out << "  Commands registered: " << m_commands.size() << "\n";
                out << "  Log buffer entries:  " << m_log_buffer.size() << "\n";
                out << "  Interactive mode:    " << (m_interactive ? "yes" : "no") << "\n";
                return out.str();
            });
        return true;
    }

    /**
     * Process one frame of console I/O.
     * Call from the server's main loop each tick.
     */
    void update();

    /**
     * Shutdown the console and restore terminal state.
     */
    void shutdown();

    /**
     * Add a log message to the console output buffer.
     * @param level    Log severity level.
     * @param message  Log message text.
     */
    void addLogMessage(utils::LogLevel level, const std::string& message);

    /**
     * Execute a command string and return the result.
     * @param command  Command text (e.g. "status", "help").
     * @return Command output string.
     */
    std::string executeCommand(const std::string& command);

    /**
     * Register a custom command handler.
     * @param name         Command name (lowercase).
     * @param description  Short description for help text.
     * @param handler      Callback invoked when the command is executed.
     */
    void registerCommand(const std::string& name, const std::string& description,
                         CommandHandler handler) {
        m_commands[name] = {description, std::move(handler)};
    }

    /**
     * Set whether the console operates in interactive mode.
     * @param interactive  true for interactive (captures stdin), false for headless.
     */
    void setInteractive(bool interactive) {
        m_interactive = interactive;
    }

    bool isInteractive() const { return m_interactive; }

    /**
     * Get the number of registered commands.
     */
    int getCommandCount() const { return static_cast<int>(m_commands.size()); }

    /**
     * Get the log buffer contents.
     */
    const std::vector<std::string>& getLogBuffer() const { return m_log_buffer; }

private:
    static constexpr size_t MAX_LOG_LINES = 200;

    struct CommandEntry {
        std::string description;
        CommandHandler handler;
    };

    bool m_interactive = false;
    bool m_initialized = false;
    std::map<std::string, CommandEntry> m_commands;
    std::vector<std::string> m_log_buffer;

    // Server references (set during init)
    Server* server_ = nullptr;
    const ServerConfig* config_ = nullptr;

    // Input buffer for interactive console
    std::string command_buffer_;

    // Command handlers
    std::string handleHelpCommand();
    std::string handleStatusCommand();
    std::string handlePlayersCommand();
    std::string handleKickCommand(const std::string& player_name);
    std::string handleStopCommand();
    std::string handleMetricsCommand();
    std::string handleSaveCommand();
    std::string handleLoadCommand();

    /** Tokenize a command string on whitespace. */
    static std::vector<std::string> tokenize(const std::string& input) {
        std::vector<std::string> tokens;
        std::istringstream iss(input);
        std::string token;
        while (iss >> token) {
            tokens.push_back(token);
        }
        return tokens;
    }
};

} // namespace atlas
