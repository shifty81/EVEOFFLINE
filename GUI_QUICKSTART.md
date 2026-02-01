# EVE OFFLINE - GUI Quick Start Guide

## 🎨 Visual Gameplay is Now Available!

EVE OFFLINE now includes a **graphical user interface** using Pygame for visual gameplay!

## Installation

### Install Pygame

```bash
pip install pygame
```

Or install all requirements:

```bash
pip install -r requirements.txt
```

## Running the GUI

### Option 1: Standalone GUI Demo (Recommended)

The easiest way to see the visual interface:

```bash
python gui_demo.py
```

**Features:**
- ✅ No server required - runs standalone
- ✅ Interactive controls (WASD to move, SPACE to fire)
- ✅ Visual space representation with star field
- ✅ Real-time combat with weapon effects
- ✅ Health bars showing Shield/Armor/Hull
- ✅ Camera controls (pan and zoom)
- ✅ Combat log and status displays

### Option 2: GUI Multiplayer Client

Connect to a server with graphics:

```bash
# Terminal 1 - Start server
python server/server.py

# Terminal 2 - Start GUI client
python client/gui_client.py "YourPilotName"
```

**Features:**
- ✅ All features from standalone demo
- ✅ Connects to multiplayer server
- ✅ See other players in real-time
- ✅ Visual representation of networked entities

## Controls

### Standalone GUI Demo

**Ship Movement:**
- `W` - Move forward (up)
- `A` - Move left
- `S` - Move backward (down)
- `D` - Move right
- `SPACE` - Fire weapons at nearest enemy

**Camera:**
- `Arrow Keys` - Pan camera view
- `+` / `-` - Zoom in/out
- `Home` - Reset camera

**Other:**
- `H` - Toggle help overlay
- `P` - Pause/Resume
- `ESC` - Quit

### GUI Multiplayer Client

**Camera:**
- `Arrow Keys` - Pan camera view
- `+` / `-` - Zoom in/out
- `Home` - Reset camera

**Other:**
- `H` - Toggle help overlay
- `ESC` - Disconnect and quit

## Visual Features

### What You'll See

1. **Space Environment**
   - Beautiful star field background (300 stars)
   - Dark blue space theme
   - Smooth 60 FPS rendering

2. **Ships**
   - Player ship (green triangle)
   - Enemy ships (red triangles)
   - Ship names displayed above each entity
   - Distance indicators

3. **Health Bars**
   - **Blue bar** - Shield HP
   - **Yellow bar** - Armor HP
   - **Gray bar** - Hull HP
   - Real-time updates during combat

4. **Combat Effects**
   - Weapon fire visualized as laser beams
   - Combat log showing damage dealt
   - Visual feedback for hits

5. **HUD Overlay**
   - Ship status (name, class, position)
   - Current HP values for all layers
   - Camera position and zoom level
   - Combat log with recent messages
   - Controls reminder

6. **Help System**
   - Press `H` to toggle comprehensive help
   - Lists all controls and features
   - Explains visual elements

## Screenshots

### Initial Game State
![Screenshot 1](https://github.com/user-attachments/assets/dca2f7db-39ed-41f7-9772-f60838eb0df1)
*Player ship (green) with multiple enemy ships visible, health bars shown, help overlay displayed*

### Combat Action
![Screenshot 2](https://github.com/user-attachments/assets/9eb865e7-6d94-46d6-9448-a4440b0fc5e8)
*Weapon fire effect (red laser) connecting player to enemy, combat log showing damage*

### After Movement
![Screenshot 3](https://github.com/user-attachments/assets/a5005e40-9a24-456a-a5a5-25a0fbdf55c0)
*Ship position updated after player movement*

## Troubleshooting

### "pygame not installed" Error

If you see this error:
```
Warning: pygame not installed. Run: pip install pygame
```

**Solution:**
```bash
pip install pygame
```

### Black Screen or No Window

If the window appears black:
1. Make sure pygame installed correctly: `python -c "import pygame; print(pygame.version.ver)"`
2. Update your graphics drivers
3. Try running in a different terminal

### Poor Performance

If the game runs slowly:
1. Close other applications
2. Update pygame: `pip install --upgrade pygame`
3. Reduce the number of stars (edit `gui_demo.py`, line ~98)

### "No module named 'engine'" Error

Make sure you're running from the repository root:
```bash
cd /path/to/EVEOFFLINE
python gui_demo.py
```

## Comparison: Text vs GUI

| Feature | Text Client | GUI Demo |
|---------|-------------|----------|
| Ship visualization | Text description | Visual triangle with color |
| Health display | Text "HP: 450/600" | Color-coded bars |
| Combat feedback | Text log only | Visual effects + log |
| Space environment | Not shown | Star field rendered |
| Controls | Type commands | Keyboard controls |
| Frame rate | Updates per tick | Smooth 60 FPS |
| Multiplayer | ✅ Yes | ✅ Yes (gui_client.py) |
| Feature complete | ✅ Full features | 🔨 Basic visuals |

## What's Next?

The current GUI implementation provides:
- ✅ Basic 2D visualization
- ✅ Interactive controls
- ✅ Visual feedback

**Future enhancements planned:**
- [ ] Clickable targeting
- [ ] Module activation buttons
- [ ] Inventory/cargo UI
- [ ] System map
- [ ] More visual effects
- [ ] Custom ship sprites
- [ ] Better UI/UX design

## Technical Details

**Rendering:**
- Pygame 2.5+ with SDL2
- 60 FPS target frame rate
- Resolution: 1280x720 (configurable)

**Architecture:**
- Uses the same ECS engine as text client
- Real-time state synchronization
- Client-side rendering only
- Server-authoritative gameplay

**Performance:**
- Minimal overhead (~1-2ms per frame)
- Efficient star field rendering
- Optimized entity drawing
- Smooth camera interpolation

## Feedback

The GUI is a new feature! Please report:
- Visual bugs or glitches
- Performance issues
- Suggestions for improvements
- Feature requests

Open an issue on GitHub or contribute directly!

---

**Enjoy the visual experience!** 🚀🎮
