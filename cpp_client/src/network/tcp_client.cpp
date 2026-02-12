#include "network/tcp_client.h"
#include <iostream>
#include <cstring>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef int socklen_t;
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <unistd.h>
    #include <fcntl.h>
    #include <errno.h>
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
#endif

namespace atlas {

#ifdef _WIN32
struct WSAInitializer {
    WSAInitializer() {
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
    }
    ~WSAInitializer() {
        WSACleanup();
    }
};
static WSAInitializer g_wsaInit;
#endif

TCPClient::TCPClient()
#ifdef _WIN32
    : m_socket(nullptr)
#else
    : m_socket(INVALID_SOCKET)
#endif
    , m_connected(false)
{
}

TCPClient::~TCPClient() {
    if (m_connected.load()) {
        disconnect();
    }
}

bool TCPClient::connect(const std::string& host, int port) {
    if (m_connected) {
        disconnect();
    }

    // Resolve hostname
    struct addrinfo hints, *result = nullptr;
    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    std::string portStr = std::to_string(port);
    if (getaddrinfo(host.c_str(), portStr.c_str(), &hints, &result) != 0) {
        std::cerr << "Failed to resolve host: " << host << std::endl;
        return false;
    }

    // Try to connect
    bool connected = false;
    for (struct addrinfo* rp = result; rp != nullptr; rp = rp->ai_next) {
#ifdef _WIN32
        SOCKET sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sock == INVALID_SOCKET) continue;
        m_socket = reinterpret_cast<void*>(static_cast<uintptr_t>(sock));
#else
        m_socket = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (m_socket == INVALID_SOCKET) continue;
#endif

#ifdef _WIN32
        if (::connect(static_cast<SOCKET>(reinterpret_cast<uintptr_t>(m_socket)), rp->ai_addr, static_cast<int>(rp->ai_addrlen)) == 0) {
#else
        if (::connect(m_socket, rp->ai_addr, rp->ai_addrlen) == 0) {
#endif
            connected = true;
            break;
        }

#ifdef _WIN32
        closesocket(static_cast<SOCKET>(reinterpret_cast<uintptr_t>(m_socket)));
        m_socket = nullptr;
#else
        ::close(m_socket);
        m_socket = INVALID_SOCKET;
#endif
    }

    freeaddrinfo(result);

    if (!connected) {
        std::cerr << "Failed to connect to " << host << ":" << port << std::endl;
        return false;
    }

    // Set non-blocking mode for receive
#ifdef _WIN32
    u_long mode = 1;
    ioctlsocket(static_cast<SOCKET>(reinterpret_cast<uintptr_t>(m_socket)), FIONBIO, &mode);
#else
    int flags = fcntl(m_socket, F_GETFL, 0);
    fcntl(m_socket, F_SETFL, flags | O_NONBLOCK);
#endif

    m_connected = true;
    m_receiveThread = std::make_unique<std::thread>(&TCPClient::receiveThread, this);

    std::cout << "Connected to " << host << ":" << port << std::endl;
    return true;
}

void TCPClient::disconnect() {
    if (m_connected) {
        m_connected = false;

        // Close socket first to break receive loop
#ifdef _WIN32
        if (m_socket != nullptr) {
            closesocket(static_cast<SOCKET>(reinterpret_cast<uintptr_t>(m_socket)));
        }
        m_socket = nullptr;
#else
        if (m_socket != INVALID_SOCKET) {
            ::close(m_socket);
        }
        m_socket = INVALID_SOCKET;
#endif

        // Wait for receive thread to finish
        if (m_receiveThread && m_receiveThread->joinable()) {
            m_receiveThread->join();
        }
        m_receiveThread.reset();

        std::cout << "Disconnected from server" << std::endl;
    }
}

bool TCPClient::send(const std::string& message) {
    if (!m_connected) return false;

    // Add newline delimiter (Python server expects line-delimited JSON)
    std::string msg = message + "\n";

    std::cout << "[DEBUG] Sending: " << message << std::endl;

#ifdef _WIN32
    int result = ::send(static_cast<SOCKET>(reinterpret_cast<uintptr_t>(m_socket)), msg.c_str(), static_cast<int>(msg.length()), 0);
#else
    ssize_t result = ::send(m_socket, msg.c_str(), msg.length(), 0);
#endif

    if (result == SOCKET_ERROR) {
        std::cerr << "Failed to send message" << std::endl;
        return false;
    }

    return true;
}

void TCPClient::processMessages() {
    std::lock_guard<std::mutex> lock(m_queueMutex);
    while (!m_messageQueue.empty()) {
        if (m_messageCallback) {
            m_messageCallback(m_messageQueue.front());
        }
        m_messageQueue.pop();
    }
}

void TCPClient::receiveThread() {
    char buffer[8192];
    std::string incompleteMessage;

    while (m_connected) {
#ifdef _WIN32
        int bytesReceived = recv(static_cast<SOCKET>(reinterpret_cast<uintptr_t>(m_socket)), buffer, sizeof(buffer) - 1, 0);
#else
        ssize_t bytesReceived = recv(m_socket, buffer, sizeof(buffer) - 1, 0);
#endif

        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            incompleteMessage += buffer;

            // Process complete messages (delimited by newline)
            size_t pos;
            while ((pos = incompleteMessage.find('\n')) != std::string::npos) {
                std::string message = incompleteMessage.substr(0, pos);
                incompleteMessage = incompleteMessage.substr(pos + 1);

                if (!message.empty()) {
                    std::cout << "[DEBUG] Received: " << message << std::endl;
                    std::lock_guard<std::mutex> lock(m_queueMutex);
                    m_messageQueue.push(message);
                }
            }
        } else if (bytesReceived == 0) {
            // Connection closed
            std::cout << "Server closed connection" << std::endl;
            m_connected = false;
            break;
        } else {
#ifdef _WIN32
            int err = WSAGetLastError();
            if (err != WSAEWOULDBLOCK && err != WSAECONNRESET) {
#else
            if (errno != EAGAIN && errno != EWOULDBLOCK && errno != ECONNRESET) {
#endif
                std::cerr << "Receive error" << std::endl;
                m_connected = false;
                break;
            }
        }

        // Small sleep to avoid busy waiting
        if (m_connected) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
}

} // namespace atlas
