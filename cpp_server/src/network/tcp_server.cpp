#include "network/tcp_server.h"
#include <iostream>
#include <cstring>

#ifdef _WIN32
#pragma comment(lib, "ws2_32.lib")
#endif

namespace eve {
namespace network {

TCPServer::TCPServer(const std::string& host, uint16_t port, int max_connections)
    : host_(host)
    , port_(port)
    , max_connections_(max_connections)
    , server_socket_(INVALID_SOCKET)
    , running_(false) {
}

TCPServer::~TCPServer() {
    stop();
}

bool TCPServer::initialize() {
    if (!initializeSockets()) {
        return false;
    }
    
    // Create socket
    server_socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket_ == INVALID_SOCKET) {
        std::cerr << "Failed to create socket" << std::endl;
        cleanupSockets();
        return false;
    }
    
    // Set socket options
    int opt = 1;
#ifdef _WIN32
    setsockopt(server_socket_, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
#else
    setsockopt(server_socket_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif
    
    // Bind socket
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_);
    
    if (host_ == "0.0.0.0" || host_ == "") {
        server_addr.sin_addr.s_addr = INADDR_ANY;
    } else {
        inet_pton(AF_INET, host_.c_str(), &server_addr.sin_addr);
    }
    
    if (bind(server_socket_, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        std::cerr << "Failed to bind socket to " << host_ << ":" << port_ << std::endl;
        closeSocket(server_socket_);
        cleanupSockets();
        return false;
    }
    
    // Listen
    if (listen(server_socket_, max_connections_) == SOCKET_ERROR) {
        std::cerr << "Failed to listen on socket" << std::endl;
        closeSocket(server_socket_);
        cleanupSockets();
        return false;
    }
    
    return true;
}

void TCPServer::start() {
    if (running_) {
        return;
    }
    
    running_ = true;
    accept_thread_ = std::thread(&TCPServer::acceptLoop, this);
}

void TCPServer::stop() {
    if (!running_) {
        return;
    }
    
    running_ = false;
    
    // Close server socket to unblock accept
    if (server_socket_ != INVALID_SOCKET) {
        closeSocket(server_socket_);
        server_socket_ = INVALID_SOCKET;
    }
    
    // Wait for accept thread
    if (accept_thread_.joinable()) {
        accept_thread_.join();
    }
    
    // Close all client connections
    {
        std::lock_guard<std::mutex> lock(clients_mutex_);
        for (auto& client : clients_) {
            closeSocket(client.socket);
        }
        clients_.clear();
    }
    
    // Wait for client threads
    for (auto& thread : client_threads_) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    client_threads_.clear();
    
    cleanupSockets();
}

void TCPServer::acceptLoop() {
    while (running_) {
        sockaddr_in client_addr{};
        socklen_t client_addr_len = sizeof(client_addr);
        
        socket_t client_socket = accept(server_socket_, (sockaddr*)&client_addr, &client_addr_len);
        
        if (client_socket == INVALID_SOCKET) {
            if (running_) {
                std::cerr << "Accept failed" << std::endl;
            }
            break;
        }
        
        // Get client address
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        uint16_t client_port = ntohs(client_addr.sin_port);
        
        ClientConnection client;
        client.socket = client_socket;
        client.address = client_ip;
        client.port = client_port;
        client.authenticated = false;
        client.connect_time = std::time(nullptr);
        
        std::cout << "[TCPServer] New connection from " << client_ip << ":" << client_port << std::endl;
        
        // Add to clients list
        {
            std::lock_guard<std::mutex> lock(clients_mutex_);
            clients_.push_back(client);
        }
        
        // Start client handler thread
        client_threads_.push_back(std::thread(&TCPServer::handleClient, this, client));
    }
}

void TCPServer::handleClient(ClientConnection client) {
    char buffer[4096];
    
    while (running_) {
        int bytes_received = recv(client.socket, buffer, sizeof(buffer) - 1, 0);
        
        if (bytes_received <= 0) {
            // Connection closed or error
            break;
        }
        
        buffer[bytes_received] = '\0';
        std::string message(buffer, bytes_received);
        
        // Call message handler if set
        if (message_handler_) {
            message_handler_(client, message);
        }
    }
    
    // Remove client from list
    {
        std::lock_guard<std::mutex> lock(clients_mutex_);
        clients_.erase(
            std::remove_if(clients_.begin(), clients_.end(),
                [&](const ClientConnection& c) { return c.socket == client.socket; }),
            clients_.end()
        );
    }
    
    std::cout << "[TCPServer] Client disconnected: " << client.address << ":" << client.port << std::endl;
    closeSocket(client.socket);
}

int TCPServer::getClientCount() const {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    return static_cast<int>(clients_.size());
}

std::vector<ClientConnection> TCPServer::getClients() const {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    return clients_;
}

void TCPServer::setMessageHandler(MessageHandler handler) {
    message_handler_ = handler;
}

bool TCPServer::sendToClient(const ClientConnection& client, const std::string& data) {
    int bytes_sent = send(client.socket, data.c_str(), static_cast<int>(data.size()), 0);
    return bytes_sent > 0;
}

void TCPServer::broadcastToAll(const std::string& data) {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    for (const auto& client : clients_) {
        send(client.socket, data.c_str(), static_cast<int>(data.size()), 0);
    }
}

void TCPServer::closeSocket(socket_t socket) {
    if (socket != INVALID_SOCKET) {
#ifdef _WIN32
        closesocket(socket);
#else
        close(socket);
#endif
    }
}

bool TCPServer::initializeSockets() {
#ifdef _WIN32
    WSADATA wsa_data;
    int result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
    if (result != 0) {
        std::cerr << "WSAStartup failed: " << result << std::endl;
        return false;
    }
#endif
    return true;
}

void TCPServer::cleanupSockets() {
#ifdef _WIN32
    WSACleanup();
#endif
}

} // namespace network
} // namespace eve
