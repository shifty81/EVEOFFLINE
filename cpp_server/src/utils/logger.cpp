#include "utils/logger.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <chrono>
#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h>
#endif

namespace eve {
namespace utils {

Logger& Logger::instance() {
    static Logger inst;
    return inst;
}

Logger::Logger() = default;

Logger::~Logger() {
    shutdown();
}

bool Logger::init(const std::string& log_dir, const std::string& filename) {
    std::lock_guard<std::mutex> lock(mutex_);

    // Create directory if it doesn't exist
    struct stat st;
    if (stat(log_dir.c_str(), &st) != 0) {
#ifdef _WIN32
        _mkdir(log_dir.c_str());
#else
        mkdir(log_dir.c_str(), 0755);
#endif
    }

    std::string path = log_dir + "/" + filename;
    log_file_.open(path, std::ios::out | std::ios::app);
    if (!log_file_.is_open()) {
        std::cerr << "[Logger] Failed to open log file: " << path << std::endl;
        return false;
    }
    return true;
}

void Logger::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (log_file_.is_open()) {
        log_file_.close();
    }
}

void Logger::setLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(mutex_);
    min_level_ = level;
}

LogLevel Logger::getLevel() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return min_level_;
}

void Logger::setConsoleOutput(bool enabled) {
    std::lock_guard<std::mutex> lock(mutex_);
    console_output_ = enabled;
}

void Logger::setFileOutput(bool enabled) {
    std::lock_guard<std::mutex> lock(mutex_);
    file_output_ = enabled;
}

bool Logger::isFileOpen() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return log_file_.is_open();
}

// Convenience methods
void Logger::debug(const std::string& message) { log(LogLevel::DEBUG, message); }
void Logger::info(const std::string& message)  { log(LogLevel::INFO, message);  }
void Logger::warn(const std::string& message)  { log(LogLevel::WARN, message);  }
void Logger::error(const std::string& message) { log(LogLevel::ERROR, message); }
void Logger::fatal(const std::string& message) { log(LogLevel::FATAL, message); }

void Logger::log(LogLevel level, const std::string& message) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (level < min_level_) {
        return;
    }

    std::string ts = timestamp();
    std::string lvl = levelToString(level);
    std::string line = ts + " [" + lvl + "] " + message;

    if (console_output_) {
        auto& out = (level >= LogLevel::ERROR) ? std::cerr : std::cout;
        out << line << std::endl;
    }

    if (file_output_ && log_file_.is_open()) {
        log_file_ << line << std::endl;
        log_file_.flush();
    }
}

std::string Logger::levelToString(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO:  return "INFO";
        case LogLevel::WARN:  return "WARN";
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::FATAL: return "FATAL";
    }
    return "UNKNOWN";
}

std::string Logger::timestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;

    std::tm tm_buf;
#ifdef _WIN32
    localtime_s(&tm_buf, &time_t_now);
#else
    localtime_r(&time_t_now, &tm_buf);
#endif

    std::ostringstream oss;
    oss << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S");
    oss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return oss.str();
}

} // namespace utils
} // namespace eve
