#pragma once

#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <ctime>
#include <mutex>

#ifdef _WIN32
#include <direct.h>
#define MKDIR(dir) _mkdir(dir)
#else
#include <sys/stat.h>
#define MKDIR(dir) mkdir(dir, 0755)
#endif

namespace atlas {

/**
 * Simple file logger that duplicates stdout/stderr to a log file.
 * 
 * When the application starts, call FileLogger::init() to begin
 * capturing all std::cout and std::cerr output to "logs/eve_client.log".
 * This ensures error messages are preserved even if the console window
 * closes immediately on crash or exit.
 */
class FileLogger {
public:
    /**
     * Initialize file logging.
     * Creates the logs directory if needed and opens the log file.
     * Redirects std::cout and std::cerr to also write to the file.
     */
    static bool init(const std::string& logDir = "logs",
                     const std::string& logFile = "eve_client.log");

    /**
     * Shut down file logging and restore original stream buffers.
     */
    static void shutdown();

    /**
     * Write a message directly to the log file (thread-safe).
     */
    static void log(const std::string& message);

    /**
     * Check if the logger is currently active.
     */
    static bool isActive();

private:
    /**
     * Custom streambuf that writes to both the original stream and the log file.
     */
    class TeeStreamBuf : public std::streambuf {
    public:
        TeeStreamBuf(std::streambuf* original, std::ofstream& file, std::mutex& mtx)
            : m_original(original), m_file(file), m_mutex(mtx) {}

    protected:
        int overflow(int c) override {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (c != EOF) {
                if (m_original) m_original->sputc(static_cast<char>(c));
                if (m_file.is_open()) m_file.put(static_cast<char>(c));
            }
            return c;
        }

        std::streamsize xsputn(const char* s, std::streamsize n) override {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (m_original) m_original->sputn(s, n);
            if (m_file.is_open()) m_file.write(s, n);
            return n;
        }

        int sync() override {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (m_original) m_original->pubsync();
            if (m_file.is_open()) m_file.flush();
            return 0;
        }

    private:
        std::streambuf* m_original;
        std::ofstream& m_file;
        std::mutex& m_mutex;
    };

    static std::ofstream s_logFile;
    static std::mutex s_mutex;
    static std::streambuf* s_origCout;
    static std::streambuf* s_origCerr;
    static TeeStreamBuf* s_teeCout;
    static TeeStreamBuf* s_teeCerr;
    static bool s_active;
};

} // namespace atlas
