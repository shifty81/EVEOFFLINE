#ifndef EVE_TCP_SERVER_H
#define EVE_TCP_SERVER_H

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <mutex>
#include <thread>
#include <atomic>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
typedef SOCKET socket_t;
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
typedef int socket_t;
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#endif

namespace eve {
namespace network {

struct ClientConnection {
    socket_t socket;
    std::string address;
    uint16_t port;
    std::string player_id;
    bool authenticated;
    uint64_t connect_time;
};

/**
 * @brief TCP server for handling client connections
 * 
 * Manages network communication with game clients
 */
class TCPServer {
public:
    explicit TCPServer(const std::string& host, uint16_t port, int max_connections);
    ~TCPServer();

    // Server control
    bool initialize();
    void start();
    void stop();
    bool isRunning() const { return running_; }
    
    // Client management
    int getClientCount() const;
    std::vector<ClientConnection> getClients() const;
    
    // Message handling
    using MessageHandler = std::function<void(const ClientConnection&, const std::string&)>;
    void setMessageHandler(MessageHandler handler);
    
    // Send data
    bool sendToClient(const ClientConnection& client, const std::string& data);
    void broadcastToAll(const std::string& data);
    
private:
    std::string host_;
    uint16_t port_;
    int max_connections_;
    socket_t server_socket_;
    std::atomic<bool> running_;
    
    std::vector<ClientConnection> clients_;
    mutable std::mutex clients_mutex_;
    
    MessageHandler message_handler_;
    
    std::thread accept_thread_;
    std::vector<std::thread> client_threads_;
    
    // Internal methods
    void acceptLoop();
    void handleClient(ClientConnection client);
    void closeSocket(socket_t socket);
    bool initializeSockets();
    void cleanupSockets();
};

} // namespace network
} // namespace eve

#endif // EVE_TCP_SERVER_H
