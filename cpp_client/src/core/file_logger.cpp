#include "core/file_logger.h"

#include <iomanip>

namespace eve {

std::ofstream FileLogger::s_logFile;
std::mutex FileLogger::s_mutex;
std::streambuf* FileLogger::s_origCout = nullptr;
std::streambuf* FileLogger::s_origCerr = nullptr;
FileLogger::TeeStreamBuf* FileLogger::s_teeCout = nullptr;
FileLogger::TeeStreamBuf* FileLogger::s_teeCerr = nullptr;
bool FileLogger::s_active = false;

bool FileLogger::init(const std::string& logDir, const std::string& logFile) {
    if (s_active) return true;

    // Create log directory (ignore error if it already exists)
    MKDIR(logDir.c_str());

    std::string logPath = logDir + "/" + logFile;

    // Open log file (overwrite each run so old logs don't accumulate)
    s_logFile.open(logPath, std::ios::out | std::ios::trunc);
    if (!s_logFile.is_open()) {
        std::cerr << "Warning: Could not open log file: " << logPath << std::endl;
        return false;
    }

    // Write header with timestamp
    std::time_t now = std::time(nullptr);
    std::tm* tm = std::localtime(&now);
    s_logFile << "=== EVE OFFLINE Client Log ===" << std::endl;
    s_logFile << "Started: "
              << std::put_time(tm, "%Y-%m-%d %H:%M:%S")
              << std::endl;
    s_logFile << "==============================" << std::endl;
    s_logFile.flush();

    // Save original stream buffers
    s_origCout = std::cout.rdbuf();
    s_origCerr = std::cerr.rdbuf();

    // Create tee stream buffers that write to both console and file
    s_teeCout = new TeeStreamBuf(s_origCout, s_logFile, s_mutex);
    s_teeCerr = new TeeStreamBuf(s_origCerr, s_logFile, s_mutex);

    // Redirect cout and cerr through the tee buffers
    std::cout.rdbuf(s_teeCout);
    std::cerr.rdbuf(s_teeCerr);

    s_active = true;
    std::cout << "Logging to file: " << logPath << std::endl;
    return true;
}

void FileLogger::shutdown() {
    if (!s_active) return;

    std::cout << "Closing log file." << std::endl;

    // Restore original stream buffers
    std::cout.rdbuf(s_origCout);
    std::cerr.rdbuf(s_origCerr);

    s_origCout = nullptr;
    s_origCerr = nullptr;

    delete s_teeCout;
    delete s_teeCerr;
    s_teeCout = nullptr;
    s_teeCerr = nullptr;

    if (s_logFile.is_open()) {
        std::time_t now = std::time(nullptr);
        std::tm* tm = std::localtime(&now);
        s_logFile << "==============================" << std::endl;
        s_logFile << "Ended: "
                  << std::put_time(tm, "%Y-%m-%d %H:%M:%S")
                  << std::endl;
        s_logFile.close();
    }

    s_active = false;
}

void FileLogger::log(const std::string& message) {
    std::lock_guard<std::mutex> lock(s_mutex);
    if (s_logFile.is_open()) {
        std::time_t now = std::time(nullptr);
        std::tm* tm = std::localtime(&now);
        s_logFile << "[" << std::put_time(tm, "%H:%M:%S") << "] "
                  << message << std::endl;
        s_logFile.flush();
    }
}

bool FileLogger::isActive() {
    return s_active;
}

} // namespace eve
