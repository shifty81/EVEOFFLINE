#ifndef EVE_LOGGER_H
#define EVE_LOGGER_H

#include <string>
#include <fstream>
#include <mutex>
#include <memory>

namespace eve {
namespace utils {

/**
 * @brief Log severity levels
 */
enum class LogLevel {
    DEBUG = 0,
    INFO = 1,
    WARN = 2,
    ERROR = 3,
    FATAL = 4
};

/**
 * @brief Thread-safe structured logging system
 *
 * Outputs timestamped, leveled log messages to both the console
 * and an optional log file.  Respects the `log_path` field that
 * already exists in ServerConfig but was previously unused.
 *
 * Usage:
 *   auto& log = Logger::instance();
 *   log.init("./logs");            // opens ./logs/server.log
 *   log.setLevel(LogLevel::INFO);
 *   log.info("Server started on port " + std::to_string(8765));
 */
class Logger {
public:
    /// Singleton accessor
    static Logger& instance();

    /**
     * @brief Initialise file logging
     * @param log_dir  Directory for log files (created if absent)
     * @param filename Name of the log file inside log_dir
     * @return true if the log file was opened successfully
     */
    bool init(const std::string& log_dir,
              const std::string& filename = "server.log");

    /// Close the log file (called automatically on destruction)
    void shutdown();

    /// Set minimum severity that will be recorded
    void setLevel(LogLevel level);

    /// Get current minimum severity
    LogLevel getLevel() const;

    // Convenience logging methods
    void debug(const std::string& message);
    void info(const std::string& message);
    void warn(const std::string& message);
    void error(const std::string& message);
    void fatal(const std::string& message);

    /// General-purpose log call
    void log(LogLevel level, const std::string& message);

    /// Enable or disable console output (default: enabled)
    void setConsoleOutput(bool enabled);

    /// Enable or disable file output (default: enabled when init'd)
    void setFileOutput(bool enabled);

    /// Check whether a log file is currently open
    bool isFileOpen() const;

private:
    Logger();
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    static std::string levelToString(LogLevel level);
    static std::string timestamp();

    std::ofstream log_file_;
    LogLevel min_level_ = LogLevel::INFO;
    bool console_output_ = true;
    bool file_output_ = true;
    mutable std::mutex mutex_;
};

} // namespace utils
} // namespace eve

#endif // EVE_LOGGER_H
