# EVE OFFLINE C++ Dedicated Server

## Overview

The EVE OFFLINE C++ Dedicated Server is a high-performance, cross-platform server implementation designed for 24/7 uptime with Steam integration support.

## Features

- **Cross-Platform**: Builds on Windows, Linux, and macOS
- **High Performance**: C++ implementation with optimized networking
- **Steam Integration**: Optional Steamworks SDK support for authentication and server browser
- **Whitelist Support**: Control access via Steam names or Steam IDs
- **Persistent World**: Auto-save functionality for continuous gameplay
- **Protocol Compatible**: Works with existing Python clients
- **Configurable**: JSON-based configuration system

## Building the Server

### Prerequisites

- **C++ Compiler**: 
  - Windows: Visual Studio 2017+ or MinGW
  - Linux: GCC 7+ or Clang 5+
  - macOS: Xcode Command Line Tools
- **CMake**: Version 3.15 or higher
- **Steamworks SDK** (Optional): Download from [Steamworks Partner Site](https://partner.steamgames.com/)

### Build Steps

#### Linux/macOS

```bash
cd cpp_server
mkdir build
cd build
cmake ..
make
```

#### Windows

```bash
cd cpp_server
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

### With Steam Support

1. Download the Steamworks SDK from https://partner.steamgames.com/
2. Extract to `cpp_server/external/steamworks_sdk/`
3. Build with Steam enabled:

```bash
cmake .. -DUSE_STEAM_SDK=ON
make
```

### Without Steam Support

```bash
cmake .. -DUSE_STEAM_SDK=OFF
make
```

## Configuration

### Server Configuration (`config/server.json`)

```json
{
  "host": "0.0.0.0",
  "port": 8765,
  "max_connections": 100,
  "server_name": "My EVE OFFLINE Server",
  "server_description": "A PVE-focused space MMO server",
  "persistent_world": true,
  "auto_save": true,
  "save_interval_seconds": 300,
  "use_whitelist": false,
  "public_server": true,
  "password": "",
  "use_steam": true,
  "steam_app_id": 0,
  "steam_authentication": false,
  "steam_server_browser": true,
  "tick_rate": 30.0,
  "max_entities": 10000,
  "data_path": "../data",
  "save_path": "./saves",
  "log_path": "./logs"
}
```

### Whitelist Configuration (`config/whitelist.json`)

Enable whitelist in `server.json` with `"use_whitelist": true`, then configure:

```json
{
  "steam_names": [
    "PlayerSteamName1",
    "PlayerSteamName2"
  ],
  "steam_ids": [
    76561198000000001,
    76561198000000002
  ]
}
```

## Running the Server

### Basic Usage

```bash
cd build/bin
./eve_dedicated_server
```

### With Custom Config

```bash
./eve_dedicated_server /path/to/server.json
```

### Background (Linux)

```bash
nohup ./eve_dedicated_server > server.log 2>&1 &
```

### As a Service (systemd)

Create `/etc/systemd/system/eve-server.service`:

```ini
[Unit]
Description=EVE OFFLINE Dedicated Server
After=network.target

[Service]
Type=simple
User=gameserver
WorkingDirectory=/opt/eve-offline/cpp_server/build/bin
ExecStart=/opt/eve-offline/cpp_server/build/bin/eve_dedicated_server
Restart=on-failure
RestartSec=10

[Install]
WantedBy=multi-user.target
```

Enable and start:

```bash
sudo systemctl enable eve-server
sudo systemctl start eve-server
sudo systemctl status eve-server
```

## Steam Integration

### Setup Steam Server

1. **Get Steam App ID**: Register your game with Steamworks
2. **Configure**: Set `steam_app_id` in `server.json`
3. **Enable Features**:
   - `steam_authentication`: Require Steam login
   - `steam_server_browser`: Appear in Steam server browser

### Steam Server Browser

When enabled, your server will appear in the Steam server browser with:
- Server name
- Current player count
- Max players
- Map name
- Custom tags

### Steam Authentication

When enabled, players must authenticate via Steam to join. The server validates Steam tickets and can query player information.

## Firewall Configuration

### Required Ports

- **Game Port**: 8765 (TCP) - configurable in `server.json`
- **Steam Query**: 27015 (UDP) - if using Steam integration

### Linux (ufw)

```bash
sudo ufw allow 8765/tcp
sudo ufw allow 27015/udp
```

### Linux (iptables)

```bash
sudo iptables -A INPUT -p tcp --dport 8765 -j ACCEPT
sudo iptables -A INPUT -p udp --dport 27015 -j ACCEPT
```

### Windows

```powershell
netsh advfirewall firewall add rule name="EVE Server" dir=in action=allow protocol=TCP localport=8765
```

## Client Connection

### Python Client

The C++ server is protocol-compatible with the existing Python clients:

```bash
# Text client
python client/client.py "CharacterName"

# 2D GUI client
python client/gui_client.py "CharacterName"

# 3D client
python client_3d.py "CharacterName"
```

### Connection String

By default, clients connect to `localhost:8765`. To connect to a dedicated server:

1. Edit client connection settings
2. Or use environment variables:
   ```bash
   export EVE_SERVER_HOST=your.server.ip
   export EVE_SERVER_PORT=8765
   ```

## Administration

### Server Commands

Currently managed via configuration files. Future versions will support:
- In-game admin commands
- Web-based admin panel
- REST API for management

### Monitoring

Check server logs in the `logs/` directory:

```bash
tail -f logs/server.log
```

### Backups

World saves are stored in `saves/` directory. Backup regularly:

```bash
#!/bin/bash
DATE=$(date +%Y%m%d_%H%M%S)
tar -czf "backup_${DATE}.tar.gz" saves/
```

## Performance Tuning

### Tick Rate

- Default: 30 Hz (suitable for most cases)
- Low-end: 20 Hz (reduces CPU usage)
- High-end: 60 Hz (smoother for fast-paced action)

Set in `server.json`: `"tick_rate": 30.0`

### Max Entities

Limit concurrent entities to control memory usage:

```json
"max_entities": 10000
```

### Connection Limits

Adjust based on server capacity:

```json
"max_connections": 100
```

## Troubleshooting

### "Failed to bind socket"

- Port already in use
- Check: `netstat -an | grep 8765`
- Solution: Change port or kill conflicting process

### "Failed to initialize Steam API"

- Steam not running
- Invalid App ID
- Missing Steamworks SDK files
- Solution: Disable Steam or fix configuration

### "Could not load config"

- Check JSON syntax
- Verify file path
- Check file permissions

### High CPU Usage

- Lower tick rate
- Reduce max entities
- Check for infinite loops in game logic

### Connection Timeout

- Check firewall rules
- Verify network connectivity
- Check server logs for errors

## Development

### Building from Source

```bash
git clone https://github.com/shifty81/EVEOFFLINE.git
cd EVEOFFLINE/cpp_server
mkdir build && cd build
cmake ..
make
```

### Running Tests

The server includes a comprehensive test suite with 832 tests covering all game systems.

**Quick build and test:**
```bash
cd cpp_server
./build.sh       # Build the server and tests
./run_tests.sh   # Run all tests
```

The `run_tests.sh` script ensures tests are run from the repository root for correct data file path resolution.

**Manual test execution:**
```bash
# From repository root
cd /path/to/EVEOFFLINE
./cpp_server/build/bin/test_systems

# All 832 tests should pass
```

**Test Coverage:**
- Capacitor & Shield Systems (15 tests)
- Weapon & Combat Systems (32 tests)
- Targeting System (8 tests)
- AI System (4 tests)
- Movement System (8 tests)
- Ship Database (31 tests)
- Wormhole System (15 tests)
- Fleet System (49 tests)
- Mission System (7 tests)
- Skill System (9 tests)
- Module System (13 tests)
- Inventory System (15 tests)
- Loot System (7 tests)
- NPC Database (3 tests)
- Drone System (33 tests)
- Insurance System (21 tests)
- Bounty System (14 tests)
- Market System (11 tests)
- Corporation System (37 tests)
- Contract System (36 tests)
- PI System (14 tests)
- Manufacturing System (21 tests)
- Research System (18 tests)
- Chat System (28 tests)
- Character Creation (23 tests)
- Tournament System (24 tests)
- Leaderboard System (23 tests)
- World Persistence (91 tests)
- Logger (24 tests)
- Server Metrics (19 tests)

Total: **832 test assertions**

### Adding Features

1. Add header files to `include/`
2. Add source files to `src/`
3. Update `CMakeLists.txt` if needed
4. Rebuild

### Code Structure

```
cpp_server/
├── include/          # Header files
│   ├── server.h
│   ├── network/      # Networking
│   ├── config/       # Configuration
│   ├── auth/         # Authentication
│   └── utils/        # Utilities
├── src/              # Source files
│   ├── main.cpp
│   ├── server.cpp
│   ├── network/
│   ├── config/
│   ├── auth/
│   └── utils/
├── config/           # Configuration files
└── CMakeLists.txt    # Build configuration
```

## License

[To be determined]

## Support

For issues and support:
- GitHub Issues: https://github.com/shifty81/EVEOFFLINE/issues
- Discord: [TBD]

## Roadmap

- [ ] Web-based admin panel
- [ ] REST API for management
- [ ] Database integration (PostgreSQL/MySQL)
- [ ] Clustering support for horizontal scaling
- [ ] Advanced anti-cheat measures
- [ ] Metrics and analytics integration
