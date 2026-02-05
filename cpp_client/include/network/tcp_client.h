#pragma once

#include <string>
#include <functional>
#include <memory>
#include <thread>
#include <atomic>
#include <queue>
#include <mutex>

namespace eve {

/**
 * TCP Client for connecting to game server
 */
class TCPClient {
public:
    using MessageCallback = std::function<void(const std::string&)>;

    TCPClient();
    ~TCPClient();

    /**
     * Connect to server
     */
    bool connect(const std::string& host, int port);

    /**
     * Disconnect from server
     */
    void disconnect();

    /**
     * Check if connected
     */
    bool isConnected() const { return m_connected; }

    /**
     * Send message to server
     */
    bool send(const std::string& message);

    /**
     * Set callback for received messages
     */
    void setMessageCallback(MessageCallback callback) { m_messageCallback = callback; }

    /**
     * Process received messages (call from main thread)
     */
    void processMessages();

private:
    void receiveThread();

#ifdef _WIN32
    void* m_socket; // SOCKET on Windows (stored as void* to avoid including winsock2.h in header)
#else
    int m_socket;
#endif

    std::atomic<bool> m_connected;
    std::unique_ptr<std::thread> m_receiveThread;
    MessageCallback m_messageCallback;

    // Thread-safe message queue
    std::queue<std::string> m_messageQueue;
    std::mutex m_queueMutex;
};

} // namespace eve
