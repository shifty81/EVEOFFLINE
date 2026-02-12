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

    /** Build the help text from registered commands. */
    std::string helpText() const {
        std::ostringstream out;
        out << "Available commands:\n";
        for (const auto& kv : m_commands) {
            out << "  " << kv.first << " - " << kv.second.description << "\n";
        }
        return out.str();
    }

    /** Shared initialization logic for both init overloads. */
    bool initInternal() {
        m_initialized = true;

        registerCommand("help", "List available commands",
            [this](const std::vector<std::string>& /*args*/) -> std::string {
                return helpText();
            });

        registerCommand("status", "Show server status summary",
            [this](const std::vector<std::string>& /*args*/) -> std::string {
                return statusText();
            });

        return true;
    }

    /** Build a short status summary. */
    std::string statusText() const {
        std::ostringstream out;
        out << "Server Status\n";
        out << "  Commands registered: " << m_commands.size() << "\n";
        out << "  Log buffer entries:  " << m_log_buffer.size() << "\n";
        out << "  Interactive mode:    " << (m_interactive ? "yes" : "no") << "\n";
        return out.str();
    }

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
