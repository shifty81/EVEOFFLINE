# EVE OFFLINE 3D Client

## Overview

The 3D client is a modern Panda3D-based graphical interface for EVE OFFLINE. It provides immersive 3D graphics while connecting to the existing Python game server.

## Features

- ✅ **Full 3D rendering** with Panda3D
- ✅ **EVE Online-inspired camera** (orbit, zoom, pan)
- ✅ **Entity interpolation** for smooth movement
- ✅ **Dark space theme** matching EVE's aesthetic
- ✅ **Real-time networking** via TCP/JSON protocol
- 🚧 **Ship models** (using placeholders for now)
- 🚧 **Visual effects** (in development)
- 🚧 **HUD/UI** (in development)

## Quick Start

### 1. Install Dependencies

```bash
pip install panda3d
```

Or install all requirements:

```bash
pip install -r requirements.txt
```

### 2. Test the 3D Client (Standalone)

Before connecting to a server, test the 3D client standalone:

```bash
python test_3d_client.py
```

This creates a simple scene with mock entities to verify the 3D client works. Use mouse to rotate camera and mouse wheel to zoom.

### 3. Start the Server

In one terminal:

```bash
python server/server.py
```

### 4. Start the 3D Client

In another terminal:

```bash
python client_3d.py "YourCharacterName"
```

## Controls

### Camera
- **Left Mouse Drag**: Rotate camera around target
- **Mouse Wheel**: Zoom in/out
- **Middle Mouse Drag**: Pan camera
- **F**: Toggle camera follow mode
- **R**: Reset camera to default position

### Movement (Coming Soon)
- **W/A/S/D**: Move ship
- **Space**: Fire weapons

### Utility
- **H**: Toggle help
- **ESC**: Exit game

## Architecture

```
client_3d/
├── __init__.py
├── core/
│   ├── network_client.py    # TCP/JSON network client
│   ├── entity_manager.py    # Entity state management
│   └── game_client.py       # Main game client class
└── rendering/
    ├── camera.py            # EVE-style camera system
    └── renderer.py          # Entity rendering

client_3d.py                 # Entry point script
```

## Network Protocol

The 3D client uses the same network protocol as the 2D client:

- **Protocol**: TCP with JSON messages
- **Port**: 8765 (default)
- **Update Rate**: 10 Hz from server, 60 FPS rendering
- **Messages**: Connection, state updates, player input

See `engine/network/protocol.py` for full protocol details.

## Development Status

### ✅ Complete (Phase 5.1)
- Network client implementation
- Entity manager with interpolation
- Camera system (orbit, zoom, pan)
- Basic entity rendering (placeholders)
- Panda3D integration
- Main game loop

### 🚧 In Progress (Phase 5.2-5.3)
- Ship 3D models
- Visual effects (weapons, explosions)
- Lighting and materials (PBR)
- Star field background

### 📋 Planned (Phase 5.4-5.5)
- HUD/UI overlay
- Ship status panels
- Target information
- Combat log
- Audio system
- Performance optimization

## Configuration

Create `client_3d/config.ini` to customize settings:

```ini
[Display]
width = 1280
height = 720
fullscreen = false
vsync = true

[Network]
host = localhost
port = 8765

[Camera]
fov = 60
near_clip = 0.1
far_clip = 10000
zoom_min = 10
zoom_max = 1000
```

## Troubleshooting

### "Panda3D not installed"
```bash
pip install panda3d
```

### "Cannot connect to server"
1. Ensure server is running: `python server/server.py`
2. Check firewall settings
3. Try `127.0.0.1` instead of `localhost`

### Black screen / No entities visible
1. Check if server has spawned entities
2. Try zooming out (mouse wheel)
3. Press 'R' to reset camera

### Low FPS
1. Ensure graphics drivers are updated
2. Close other applications
3. Disable v-sync in config (if needed)

## Contributing

See [CONTRIBUTING.md](../CONTRIBUTING.md) for guidelines.

### Adding Ship Models

1. Create/obtain 3D model (Blender, etc.)
2. Export to `.egg`, `.bam`, `.gltf`, or `.obj`
3. Place in `client_3d/models/ships/`
4. Name format: `shipname.ext` (e.g., `rifter.egg`)

The renderer will automatically load models if available, otherwise uses placeholders.

## Documentation

- [Phase 5 Technical Specification](../docs/design/PHASE5_3D_SPECIFICATION.md)
- [3D Client Quick Start](../docs/getting-started/3D_CLIENT_QUICKSTART.md)
- [Network Protocol](../engine/network/protocol.py)

## Support

- **Issues**: [GitHub Issues](https://github.com/shifty81/EVEOFFLINE/issues)
- **Discussions**: [GitHub Discussions](https://github.com/shifty81/EVEOFFLINE/discussions)

---

**Version**: 0.1.0 (Phase 5 Development)  
**Status**: Early development - basic functionality working  
**Last Updated**: February 2, 2026
