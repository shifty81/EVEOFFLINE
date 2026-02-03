# Python Client Updates - C++ Server Features Integration

## Overview

The Python clients (text and GUI) have been updated to incorporate all the features from the new C++ dedicated server implementation, providing a seamless and feature-rich gaming experience.

## New Features

### 1. Configuration System

**File**: `client/client_config.json`

A comprehensive JSON-based configuration system that allows users to customize all client settings.

#### Configuration Categories:

- **Connection Settings**
  - Server host and port
  - Auto-reconnect on disconnect
  - Reconnect delay (seconds)
  - Connection timeout (seconds)

- **Client Settings**
  - Auto-generate character names
  - Name generation style (random/male/female)
  - Log level
  - Credential saving

- **Display Settings** (GUI Client)
  - Window resolution (width x height)
  - Fullscreen mode
  - VSync
  - FPS limit
  - Show FPS counter

- **Steam Integration**
  - Enable/disable Steam
  - Auto-login
  - Overlay support

- **Audio Settings**
  - Master, music, and SFX volumes

- **Network Settings**
  - Buffer size
  - Compression (future)
  - Encryption (future)

#### Example Configuration:

```json
{
  "connection": {
    "host": "localhost",
    "port": 8765,
    "auto_reconnect": true,
    "reconnect_delay": 5,
    "connection_timeout": 10
  },
  "display": {
    "width": 1280,
    "height": 720,
    "fullscreen": false,
    "fps_limit": 60,
    "show_fps": true
  }
}
```

### 2. Name Generator Integration

Both clients now support the random name generator for character creation.

#### Features:
- Generate names via command-line option
- Support for male, female, and random styles
- Configuration-based auto-generation
- Over 1,000 unique name combinations

#### Usage:

```bash
# Generate random name
python client/client.py --generate-name

# Generate male name
python client/client.py --generate-name --name-style male

# Generate female name
python client/gui_client.py --generate-name --name-style female
```

#### Example Generated Names:
- Male: "Drake Voidwalker", "Marcus Ironheart", "Kane Starfire"
- Female: "Nova Stormbreaker", "Aria Shadowborn", "Luna Nightfall"

### 3. Enhanced Connection Management

#### Auto-Reconnect:
- Automatically reconnect on disconnect
- Configurable retry attempts (default: 5)
- Configurable delay between attempts (default: 5s)
- Status updates during reconnection

#### Timeout Handling:
- Connection timeout detection
- Configurable timeout duration (default: 10s)
- Graceful failure with error messages

#### Error Reporting:
- Detailed error messages from server
- Protocol error handling
- Network error diagnostics

### 4. Improved GUI Client

#### Visual Enhancements:
- **Connection Status Indicator**
  - Green: Connected
  - Red: Disconnected/Error
  - Yellow: Connecting/Reconnecting
  - Real-time status updates

- **FPS Counter**
  - Configurable display
  - Real-time frame rate monitoring
  - Performance optimization

- **Error Display**
  - In-game error messages
  - Connection diagnostics
  - Retry information

#### Configuration Integration:
- Uses config file for all display settings
- FPS limit from configuration
- Fullscreen mode support
- VSync control

### 5. Better Command-Line Interface

Both clients now support comprehensive command-line arguments:

```bash
# Text Client
python client/client.py [character_name] [options]

Options:
  --host HOST           Server host (overrides config)
  --port PORT           Server port (overrides config)
  --config PATH         Path to config file
  --generate-name       Generate random character name
  --name-style STYLE    Name style (random/male/female)

# GUI Client
python client/gui_client.py [character_name] [options]

Options:
  --host HOST           Server host (overrides config)
  --port PORT           Server port (overrides config)
  --config PATH         Path to config file
  --generate-name       Generate random character name
  --name-style STYLE    Name style (random/male/female)
  --fullscreen          Run in fullscreen mode
```

## Usage Examples

### Basic Usage

```bash
# Text client with character name
python client/client.py "TestPilot"

# GUI client with character name
python client/gui_client.py "TestPilot"
```

### With Name Generation

```bash
# Auto-generate random name
python client/client.py --generate-name

# Generate male name
python client/client.py --generate-name --name-style male

# Generate female name
python client/gui_client.py --generate-name --name-style female
```

### Connect to Remote Server

```bash
# Text client to remote server
python client/client.py "MyCharacter" --host game.example.com --port 8765

# GUI client to remote server
python client/gui_client.py "MyCharacter" --host 192.168.1.100 --port 8765
```

### With Custom Configuration

```bash
# Use custom config file
python client/client.py "MyCharacter" --config my_config.json

# GUI client with fullscreen
python client/gui_client.py "MyCharacter" --fullscreen
```

## Configuration Management

### Loading Configuration

The configuration is automatically loaded from `client/client_config.json`. If the file doesn't exist, default values are used and a new config file is created.

### Modifying Configuration

Edit `client/client_config.json` to change settings:

```json
{
  "connection": {
    "host": "my.server.com",
    "port": 8765,
    "auto_reconnect": true
  },
  "client": {
    "auto_generate_name": true,
    "name_style": "random"
  }
}
```

### Using Python API

```python
from client.client_config import ClientConfig

# Load configuration
config = ClientConfig()

# Get values
host = config.get_host()
port = config.get_port()

# Set values
config.set('connection', 'host', value='localhost')
config.save()
```

## Features Comparison

| Feature | Before | After |
|---------|--------|-------|
| Configuration | Hardcoded | JSON file |
| Name Generation | Manual only | Auto + Manual |
| Reconnect | Manual only | Auto-reconnect |
| Timeouts | None | Configurable |
| Error Display | Basic | Detailed |
| Status Indicator | None | Real-time |
| FPS Counter | None | Optional |
| CLI Options | Limited | Comprehensive |

## Compatibility

- **Python Version**: 3.11+ (same as before)
- **Server Compatibility**: Works with both Python and C++ servers
- **Protocol**: JSON-based network protocol (unchanged)
- **Backward Compatible**: Old usage still works

## Benefits

### For Players:
- Easy server connection configuration
- Quick character name generation
- Automatic reconnection on network issues
- Better visual feedback
- Customizable display settings

### For Server Administrators:
- Easy to distribute configurations
- Remote server connection support
- Standardized settings format
- Better diagnostics

### For Developers:
- Clean configuration management
- Extensible system
- Easy to add new features
- Better error handling

## Future Enhancements

Planned features for future updates:

- **Steam Integration**: Full Steam API support
- **Server Browser**: Discover and join servers
- **Configuration UI**: In-game settings menu
- **Profile Management**: Multiple character profiles
- **Network Statistics**: Ping, bandwidth monitoring
- **Advanced Audio**: 3D spatial audio (from C++ server)
- **PBR Rendering**: Physically-based rendering (from C++ server)

## Testing

To test the new features:

```bash
# Test configuration system
python client/client_config.py

# Demo all client features
python demo_client_features.py

# Test name generation
python client/client.py --generate-name

# Test with various options
python client/gui_client.py --generate-name --name-style female --fullscreen
```

## Troubleshooting

### Configuration Not Loading

- Check that `client/client_config.json` exists
- Verify JSON syntax is valid
- Check file permissions

### Connection Timeout

- Increase `connection_timeout` in config
- Check server is running
- Verify network connectivity

### Name Generation Not Working

- Ensure `engine/utils/name_generator.py` is present
- Check Python path includes parent directory

### GUI Client Issues

- Ensure Pygame is installed: `pip install pygame`
- Check display settings in config file
- Try windowed mode if fullscreen fails

## See Also

- [Name Generator Documentation](NAME_GENERATOR.md)
- [C++ Server README](../cpp_server/README.md)
- [EVE Feature Gap Analysis](../design/EVE_FEATURE_GAP.md)
- [Main README](../../README.md)
