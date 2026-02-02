# 3D Client Quick Start Guide

**Version**: 1.0  
**Date**: February 2, 2026

---

## Overview

This guide helps you get started with the new 3D client for EVE OFFLINE. The 3D client uses **Panda3D** to provide a modern 3D graphical interface while connecting to the existing Python server.

---

## Prerequisites

### Required
- Python 3.11 or higher
- pip (Python package manager)
- Git

### Recommended
- 4GB RAM minimum
- Graphics card with OpenGL 3.3+ support
- Mid-range laptop or better

---

## Installation

### 1. Install Dependencies

The 3D client requires Panda3D in addition to the base game requirements.

```bash
# Navigate to project directory
cd EVEOFFLINE

# Install all dependencies (includes Panda3D)
pip install -r requirements.txt

# Or install Panda3D separately
pip install panda3d
```

### 2. Verify Installation

```bash
# Check Panda3D installation
python -c "import direct.showbase.ShowBase; print('Panda3D installed successfully!')"
```

---

## Running the 3D Client

### Start the Server

First, start the Python game server in one terminal:

```bash
python server/server.py
```

You should see:
```
[Server] Initializing EVE OFFLINE Server
[Server] Host: localhost:8765
[Server] Server initialized successfully
```

### Start the 3D Client

In a **new terminal**, start the 3D client:

```bash
python client/client_3d.py "YourCharacterName"
```

Replace `"YourCharacterName"` with your desired character name.

---

## Controls

### Camera Controls
- **Mouse Drag (Left)**: Rotate camera around target
- **Mouse Wheel**: Zoom in/out
- **Middle Mouse Drag**: Pan camera
- **Double-click Entity**: Focus camera on entity

### Movement
- **W**: Move forward
- **S**: Move backward  
- **A**: Turn left
- **D**: Turn right
- **Q**: Strafe left
- **E**: Strafe right

### Combat
- **Left Click**: Select target
- **Space**: Fire weapons
- **Tab**: Cycle targets
- **F**: Toggle follow camera

### Utility
- **H**: Toggle help overlay
- **ESC**: Exit game
- **F1**: Toggle FPS display

---

## Configuration

### Graphics Settings

Edit `client/config_3d.ini`:

```ini
[Display]
width = 1280
height = 720
fullscreen = false
vsync = true
aa_samples = 4  # Anti-aliasing (0, 2, 4, 8, 16)

[Graphics]
texture_quality = high  # low, medium, high
model_detail = high     # low, medium, high
particle_effects = true
post_processing = true

[Network]
server_host = localhost
server_port = 8765
update_rate = 10  # Hz (server updates per second)

[Camera]
fov = 60
near_clip = 0.1
far_clip = 10000
zoom_min = 10
zoom_max = 1000
```

### Performance Tuning

If you experience low FPS:

1. **Lower anti-aliasing**: Set `aa_samples = 0` or `2`
2. **Reduce texture quality**: Set `texture_quality = medium` or `low`
3. **Disable post-processing**: Set `post_processing = false`
4. **Lower resolution**: Reduce `width` and `height`

---

## Troubleshooting

### Problem: Black screen on startup

**Solution**:
1. Check graphics drivers are up to date
2. Verify OpenGL version: `glxinfo | grep "OpenGL version"` (Linux) or use GPU-Z (Windows)
3. Try disabling post-processing in config

### Problem: "Cannot connect to server"

**Solution**:
1. Ensure server is running: `python server/server.py`
2. Check firewall isn't blocking port 8765
3. Try connecting to `127.0.0.1` instead of `localhost`

### Problem: Low FPS (< 30)

**Solution**:
1. Follow performance tuning steps above
2. Close other applications
3. Check CPU/GPU usage in task manager
4. Ensure graphics drivers are updated

### Problem: Ships appear as cubes/spheres

**Solution**:
This is normal if ship models aren't loaded yet. The project will use placeholder primitives until 3D models are created.

To check if models are available:
```bash
ls client_3d/models/ships/
```

### Problem: Panda3D import error

**Solution**:
```bash
# Reinstall Panda3D
pip uninstall panda3d
pip install panda3d --no-cache-dir

# If on Linux, you may need additional libraries:
sudo apt-get install python3-dev build-essential
```

---

## Comparing 2D vs 3D Clients

| Feature | 2D Client (pygame) | 3D Client (Panda3D) |
|---------|-------------------|---------------------|
| Graphics | 2D sprites | Full 3D models |
| Performance | Very fast | 60 FPS on mid-range hardware |
| Ease of Use | Simple | More immersive |
| System Requirements | Low | Medium |
| Camera | Top-down | Free rotation |
| Visual Effects | Basic | Advanced (PBR, particles) |

Both clients connect to the same server and offer the same gameplay features. Choose based on your preference and hardware!

---

## Development

### Project Structure

```
client_3d/
├── __init__.py
├── client_3d.py          # Main client entry point
├── config_3d.ini         # Configuration file
├── core/
│   ├── __init__.py
│   ├── network_client.py  # Network connection handler
│   ├── game_client.py     # Main game client class
│   └── entity_manager.py  # Entity state management
├── rendering/
│   ├── __init__.py
│   ├── camera.py          # Camera system
│   ├── renderer.py        # Entity renderer
│   ├── effects.py         # Visual effects (particles, etc.)
│   └── shaders/           # Custom shaders
├── ui/
│   ├── __init__.py
│   ├── hud.py             # Head-up display
│   ├── panels.py          # UI panels (ship status, target info)
│   └── styles.py          # UI styling constants
├── models/
│   └── ships/             # 3D ship models (.egg, .bam)
├── textures/
│   └── ships/             # Ship textures
└── effects/
    ├── particles/         # Particle effect definitions
    └── sounds/            # Audio files
```

### Adding New Ships

To add a new ship model:

1. **Create/obtain 3D model** (`.blend`, `.obj`, `.fbx`, `.gltf`)
2. **Convert to Panda3D format**:
   ```bash
   # Using obj2egg (comes with Panda3D)
   obj2egg -o models/ships/rifter.egg models/source/rifter.obj
   
   # Or using egg2bam for better performance
   egg2bam models/ships/rifter.egg models/ships/rifter.bam
   ```
3. **Add textures** to `textures/ships/`
4. **Register ship** in `rendering/renderer.py`

### Debugging

Enable debug mode:

```bash
# Set environment variable
export PANDA3D_DEBUG=1  # Linux/Mac
set PANDA3D_DEBUG=1     # Windows

# Run client with verbose logging
python client/client_3d.py "TestPilot" --debug
```

---

## Next Steps

1. **Play the game** - Try both 2D and 3D clients
2. **Provide feedback** - Report bugs or suggestions
3. **Create content** - Help create ship models or textures
4. **Contribute** - See [CONTRIBUTING.md](../../CONTRIBUTING.md)

---

## Resources

- [Panda3D Manual](https://docs.panda3d.org/1.10/python/index)
- [Panda3D Forums](https://discourse.panda3d.org/)
- [Phase 5 Technical Specification](../design/PHASE5_3D_SPECIFICATION.md)
- [EVE OFFLINE Documentation](../README.md)

---

## Support

- **Issues**: [GitHub Issues](https://github.com/shifty81/EVEOFFLINE/issues)
- **Discussions**: [GitHub Discussions](https://github.com/shifty81/EVEOFFLINE/discussions)

---

**Last Updated**: February 2, 2026  
**Status**: Phase 5 in development
