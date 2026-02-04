# Integrated Multiplayer Hosting

## Overview

EVE OFFLINE's C++ client includes integrated multiplayer hosting capabilities, allowing players to host game sessions directly from the client without needing to set up a dedicated server. This feature makes multiplayer gaming more accessible and enables easy co-op play with friends.

## Features

### Embedded Server
- **In-Process Server**: Server runs in the same process as the client
- **Automatic Configuration**: Pre-configured for optimal local hosting
- **Auto-Connect**: Host automatically connects to their own server
- **Thread-Safe**: Server runs on a separate thread to avoid blocking the game

### Session Management
- **Host or Join**: Players can either host a new game or join an existing one
- **LAN Discovery**: Scan local network for available game sessions
- **Player Invites**: Host can invite specific players to join
- **Player Management**: Host can view connected players and kick if needed

### Networking
- **Local and Remote**: Supports both LAN and internet play
- **Password Protection**: Optional password for private games
- **Player Limit**: Configurable maximum players (default: 20)
- **Low Latency**: Optimized for local network play

## Architecture

### Components

#### 1. EmbeddedServer (`core/embedded_server.h/cpp`)
Manages the embedded game server instance:

```cpp
class EmbeddedServer {
public:
    struct Config {
        std::string server_name;
        int port = 8765;
        int max_players = 20;
        bool use_password = false;
        std::string password;
        bool lan_only = true;
        // ... more options
    };
    
    bool start(const Config& config);
    void stop();
    bool isRunning() const;
    Status getStatus() const;
};
```

**Features**:
- Starts server in separate thread
- 30Hz tick rate
- Thread-safe status queries
- Graceful shutdown

#### 2. SessionManager (`core/session_manager.h/cpp`)
Manages multiplayer session lifecycle:

```cpp
class SessionManager {
public:
    enum class SessionType {
        SinglePlayer,
        HostedMultiplayer,
        JoinedMultiplayer,
        DedicatedServer
    };
    
    bool hostSession(const SessionConfig& config, EmbeddedServer* server);
    bool joinSession(const std::string& host, int port);
    void leaveSession();
    std::vector<SessionInfo> scanLAN();
};
```

**Features**:
- Session creation and joining
- LAN discovery (UDP broadcast)
- Player list management
- Event callbacks for player join/leave

#### 3. Application Integration
The main Application class integrates hosting:

```cpp
class Application {
public:
    bool hostMultiplayerGame(const std::string& sessionName, int maxPlayers);
    bool joinMultiplayerGame(const std::string& host, int port);
    bool isHosting() const;
    
    EmbeddedServer* getEmbeddedServer();
    SessionManager* getSessionManager();
};
```

## Usage

### Hosting a Game

```cpp
// From within the application
Application* app = Application::getInstance();

// Host a multiplayer game
bool success = app->hostMultiplayerGame("My EVE Game", 20);

if (success) {
    std::cout << "Game hosted successfully!" << std::endl;
    std::cout << "Other players can connect to: localhost:8765" << std::endl;
}
```

**What Happens**:
1. Embedded server is configured and started
2. Server runs on port 8765 (default)
3. Session is created in SessionManager
4. Host automatically connects as first player
5. Server address is displayed for other players

### Joining a Game

```cpp
Application* app = Application::getInstance();

// Join a multiplayer game
bool success = app->joinMultiplayerGame("192.168.1.100", 8765);

if (success) {
    std::cout << "Connected to game!" << std::endl;
}
```

**What Happens**:
1. Client connects to specified server
2. Session information is retrieved
3. Player joins the game world
4. Game state synchronizes

### Scanning for LAN Games

```cpp
SessionManager* sessionMgr = app->getSessionManager();

// Scan for games on local network
std::vector<SessionManager::SessionInfo> sessions = sessionMgr->scanLAN();

for (const auto& session : sessions) {
    std::cout << "Found game: " << session.name << std::endl;
    std::cout << "  Address: " << session.host_address << ":" << session.port << std::endl;
    std::cout << "  Players: " << session.current_players << "/" << session.max_players << std::endl;
}
```

### Leaving a Session

```cpp
SessionManager* sessionMgr = app->getSessionManager();
sessionMgr->leaveSession();

// If hosting, this will also stop the embedded server
```

## Configuration Options

### Server Configuration

```cpp
EmbeddedServer::Config config;
config.server_name = "My Game";           // Display name
config.description = "Fun PVE co-op";     // Description
config.port = 8765;                       // Network port
config.max_players = 20;                  // Max players
config.use_password = true;               // Password protect
config.password = "secret123";            // Password
config.lan_only = true;                   // LAN games only
config.persistent_world = false;          // Save world state
config.auto_save_interval = 300;          // Auto-save (seconds)
config.data_path = "../data";             // Game data path
config.save_path = "./saves";             // Save files path
```

### Session Configuration

```cpp
SessionManager::SessionConfig config;
config.session_name = "Mining Party";
config.description = "Asteroid mining session";
config.max_players = 8;
config.use_password = false;
config.lan_only = true;
config.persistent = false;
config.auto_save_interval = 300;
```

## Player Management

### Getting Player List

```cpp
SessionManager* sessionMgr = app->getSessionManager();
std::vector<SessionManager::PlayerInfo> players = sessionMgr->getPlayers();

for (const auto& player : players) {
    std::cout << player.name << std::endl;
    std::cout << "  Ship: " << player.ship << std::endl;
    std::cout << "  System: " << player.system << std::endl;
    std::cout << "  Ping: " << player.ping_ms << " ms" << std::endl;
    std::cout << "  Host: " << (player.is_host ? "Yes" : "No") << std::endl;
}
```

### Inviting Players

```cpp
SessionManager* sessionMgr = app->getSessionManager();

// Only host can invite
if (sessionMgr->getCurrentSessionType() == SessionType::HostedMultiplayer) {
    bool success = sessionMgr->invitePlayer("FriendName");
}
```

### Kicking Players

```cpp
SessionManager* sessionMgr = app->getSessionManager();

// Only host can kick
if (sessionMgr->getCurrentSessionType() == SessionType::HostedMultiplayer) {
    bool success = sessionMgr->kickPlayer("BadPlayer");
}
```

## Event Callbacks

### Setting Up Callbacks

```cpp
SessionManager* sessionMgr = app->getSessionManager();

// Player joined event
sessionMgr->setOnPlayerJoined([](const std::string& playerName) {
    std::cout << playerName << " joined the game!" << std::endl;
    // Show notification in UI
});

// Player left event
sessionMgr->setOnPlayerLeft([](const std::string& playerName) {
    std::cout << playerName << " left the game" << std::endl;
    // Update player list in UI
});

// Session ended event
sessionMgr->setOnSessionEnded([](const std::string& reason) {
    std::cout << "Session ended: " << reason << std::endl;
    // Return to main menu
});
```

## Server Status

### Getting Server Status

```cpp
EmbeddedServer* server = app->getEmbeddedServer();

if (server && server->isRunning()) {
    auto status = server->getStatus();
    
    std::cout << "Server: " << status.server_name << std::endl;
    std::cout << "Players: " << status.connected_players << "/" << status.max_players << std::endl;
    std::cout << "Port: " << status.port << std::endl;
    std::cout << "Uptime: " << status.uptime_seconds << " seconds" << std::endl;
    std::cout << "System: " << status.current_system << std::endl;
}
```

## Future UI Integration

### Planned UI Features

1. **Main Menu Options**:
   - "Host Game" button
   - "Join Game" button
   - "Find LAN Games" button

2. **Host Game Dialog**:
   - Session name input
   - Max players slider
   - Password toggle and input
   - LAN-only checkbox
   - Persistent world checkbox
   - "Start Hosting" button

3. **Join Game Dialog**:
   - Server address input
   - Port input
   - Password input (if required)
   - LAN game browser list
   - "Connect" button

4. **In-Game Server UI**:
   - Player list panel
   - Server status (players, uptime)
   - Host controls (invite, kick)
   - "Stop Hosting" button

5. **Session Browser**:
   - List of available LAN sessions
   - Session details (name, players, ping)
   - Quick join button
   - Refresh button

## Technical Details

### Threading Model

```
Main Thread:
  ├─ Window/Rendering (60 FPS)
  ├─ Input Handling
  ├─ Game Client
  └─ Server Status Updates

Server Thread:
  ├─ Network I/O
  ├─ Game Logic (30 Hz)
  ├─ State Synchronization
  └─ Player Management
```

### Network Protocol

- **Transport**: TCP for reliability
- **Format**: JSON messages (compatible with Python clients)
- **Ports**: Default 8765, configurable
- **Discovery**: UDP broadcast on port 8766 (planned)

### Performance Considerations

- **CPU Usage**: Server runs at 30 Hz, minimal overhead
- **Memory**: ~50-100 MB for server instance
- **Network**: ~10-50 KB/s per player
- **Latency**: <10ms on LAN, variable on internet

## Compatibility

### Cross-Platform
- **Windows**: Full support
- **Linux**: Full support
- **macOS**: Full support

### Client Compatibility
- **C++ Client**: Full support
- **Python Client**: Full support (same protocol)
- **3D Client**: Full support (same protocol)

### Mixed Clients
Yes! A C++ client can host a game that Python clients join, and vice versa. All clients use the same JSON protocol.

## Troubleshooting

### Server Won't Start
- Check if port 8765 is already in use
- Verify firewall allows the port
- Try a different port number

### Can't Connect to Hosted Game
- Verify server is running (`isRunning()`)
- Check firewall settings
- Ensure correct IP address and port
- Try connecting via 127.0.0.1 (localhost) first

### LAN Discovery Not Finding Games
- Ensure all computers are on same network
- Check firewall allows UDP broadcast
- Verify LAN-only mode is enabled

### High Latency/Lag
- Check network connection quality
- Reduce number of players
- Consider dedicated server for better performance

## Best Practices

1. **For Hosts**:
   - Use wired ethernet connection
   - Close bandwidth-heavy applications
   - Set appropriate player limit
   - Monitor server status

2. **For Players**:
   - Use stable internet connection
   - Choose nearby servers (low ping)
   - Respect host's rules
   - Report issues promptly

3. **For Development**:
   - Always stop server on shutdown
   - Handle thread cleanup properly
   - Test with multiple clients
   - Log all server events

## Example: Complete Host Flow

```cpp
// 1. Create application
Application app("EVE OFFLINE", 1920, 1080);

// 2. Host multiplayer game
bool hosting = app.hostMultiplayerGame("Weekend Mining Party", 10);

if (hosting) {
    std::cout << "Hosting started!" << std::endl;
    
    // 3. Set up callbacks
    SessionManager* sm = app.getSessionManager();
    sm->setOnPlayerJoined([](const std::string& name) {
        std::cout << name << " joined!" << std::endl;
    });
    
    // 4. Run game
    app.run();
    
    // 5. Cleanup (automatic)
    // Server stops when app destructs
}
```

## Example: Complete Join Flow

```cpp
// 1. Create application
Application app("EVE OFFLINE", 1920, 1080);

// 2. Scan for games
SessionManager* sm = app.getSessionManager();
auto sessions = sm->scanLAN();

// 3. Join first available game
if (!sessions.empty()) {
    const auto& session = sessions[0];
    bool joined = app.joinMultiplayerGame(
        session.host_address, 
        session.port
    );
    
    if (joined) {
        std::cout << "Joined " << session.name << "!" << std::endl;
        
        // 4. Run game
        app.run();
    }
}
```

## Summary

The integrated multiplayer hosting system in EVE OFFLINE makes it easy to:
- Host games without dedicated server setup
- Join friend's games quickly
- Discover LAN games automatically
- Manage players and sessions

All with a simple, intuitive API that integrates seamlessly with the game client.

---

**Version**: 1.0  
**Last Updated**: February 2026  
**Status**: Ready for UI integration
