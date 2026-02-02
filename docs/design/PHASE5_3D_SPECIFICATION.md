# Phase 5: 3D Graphics & Polish - Technical Specification

**Version**: 1.0  
**Date**: February 2, 2026  
**Status**: Planning

---

## Table of Contents
1. [Overview](#overview)
2. [Architecture Decision](#architecture-decision)
3. [3D Engine Evaluation](#3d-engine-evaluation)
4. [Network Protocol](#network-protocol)
5. [Visual Requirements](#visual-requirements)
6. [Implementation Phases](#implementation-phases)
7. [Asset Requirements](#asset-requirements)
8. [Performance Targets](#performance-targets)
9. [Development Timeline](#development-timeline)

---

## Overview

Phase 5 focuses on implementing 3D graphics and polishing the visual experience of EVE OFFLINE. The goal is to create a modern 3D client that connects to the existing Python server while maintaining the EVE Online-inspired aesthetic.

### Core Objectives
- Implement high-quality 3D graphics
- Maintain existing Python server architecture
- Create EVE Online-inspired visual style
- Achieve 60+ FPS performance
- Support all existing game features

---

## Architecture Decision

### Recommended: Hybrid Architecture

**Python Server (Keep)** + **3D Client (New)**

```
┌──────────────────────┐           ┌──────────────────────┐
│   Python Server      │◄─Network─►│   3D Client          │
│   (Existing)         │  Protocol │   (New)              │
├──────────────────────┤           ├──────────────────────┤
│ • Game Logic         │           │ • 3D Rendering       │
│ • World Simulation   │           │ • Input Handling     │
│ • AI & Combat        │           │ • Audio System       │
│ • Systems (ECS)      │           │ • Camera Controls    │
│ • Data Management    │           │ • Visual Effects     │
│ • TCP/JSON Protocol  │           │ • UI/HUD System      │
└──────────────────────┘           └──────────────────────┘
      (Port 8765)                        (Connects)
```

### Why This Architecture?

✅ **Advantages**:
- Keep working Python server (no rewrite needed)
- Focus effort on visual improvements
- Easy modding via JSON data files
- Server-authoritative gameplay (anti-cheat)
- Can develop client independently
- Players can use text or GUI client

❌ **Minimal Downside**:
- Two separate codebases to maintain
- Build/deployment complexity for client

---

## 3D Engine Evaluation

### Option 1: Panda3D (Python) ⭐ RECOMMENDED FOR MVP

**Language**: Python  
**License**: BSD (Free)  
**Complexity**: Medium  

#### Pros
- ✅ **Stay in Python** - No new language needed
- ✅ **Fast to implement** - Python integration is seamless
- ✅ **Good 3D support** - Full 3D engine with shader support
- ✅ **Proven for MMOs** - Used by Disney for Toontown Online
- ✅ **Easy debugging** - Python stack traces, REPL
- ✅ **Cross-platform** - Windows, Linux, Mac
- ✅ **Built-in features**: Physics, shaders, particle effects, networking

#### Cons
- ⚠️ Performance ceiling (but fine for 2-20 players)
- ⚠️ Not as polished as commercial engines
- ⚠️ Smaller community than Unity/Unreal

#### Code Example
```python
from panda3d.core import *
from direct.showbase.ShowBase import ShowBase

class EVEOfflineClient(ShowBase):
    def __init__(self):
        ShowBase.__init__(self)
        
        # Load ship model
        self.ship = self.loader.loadModel("models/rifter.egg")
        self.ship.reparentTo(self.render)
        
        # Camera setup
        self.camera.setPos(0, -50, 10)
        self.camera.lookAt(self.ship)
        
        # Lighting
        dlight = DirectionalLight('dlight')
        dlnp = self.render.attachNewNode(dlight)
        self.render.setLight(dlnp)
```

**Decision**: Use Panda3D for Phase 5 MVP. Can upgrade to Unreal later if needed.

---

### Option 2: Unreal Engine 5 (Future Enhancement)

**Language**: C++ / Blueprints  
**License**: Free (<$1M revenue), 5% royalty after  
**Complexity**: High  

#### Pros
- ✅ **Best graphics quality** - Industry-leading visuals
- ✅ **Nanite** - Unlimited polygon detail
- ✅ **Lumen** - Real-time global illumination
- ✅ **Professional tools** - Best-in-class editor
- ✅ **Asset marketplace** - Thousands of ready-to-use assets
- ✅ **Blueprint scripting** - Visual scripting option

#### Cons
- ⚠️ Large download (~40GB)
- ⚠️ Steep learning curve
- ⚠️ Requires C++ knowledge
- ⚠️ Overkill for small player count

**Decision**: Consider for Phase 6+ if commercial release is planned.

---

### Option 3: Unity (Alternative)

**Language**: C# (IL2CPP to native)  
**License**: Free (Personal), $40/month (Plus)  
**Complexity**: Medium  

#### Pros
- ✅ **Good balance** - Easier than UE5, better than Panda3D
- ✅ **Proven for space games** - Used by Astrox Imperium
- ✅ **Large asset store**
- ✅ **Great tools**

#### Cons
- ⚠️ C# instead of C++ or Python
- ⚠️ Recent licensing changes
- ⚠️ Not as advanced graphics as UE5

**Decision**: Good alternative if team knows C#.

---

### Option 4: Custom OpenGL/Vulkan (Not Recommended)

**Language**: C++  
**License**: Open source (free)  
**Complexity**: Very High  

#### Why Not?
- ❌ Months of work just for basic rendering
- ❌ Need to implement everything from scratch
- ❌ High expertise required
- ❌ Not practical for indie project

**Decision**: Skip unless this is a learning exercise.

---

## Network Protocol

### Current Protocol (Python Server)

The existing server uses **asyncio TCP sockets** with **JSON messages**.

#### Connection Flow
```
Client                          Server
  |                               |
  |----CONNECT (player_id)------->|
  |<---CONNECT_ACK (success)------|
  |                               |
  |<---STATE_UPDATE (entities)----|  (10 Hz)
  |                               |
  |----INPUT_MOVE (direction)---->|
  |----INPUT_TARGET (entity_id)-->|
  |----INPUT_FIRE---------------->|
```

#### Message Types
See `engine/network/protocol.py` for full protocol:

```python
class MessageType(Enum):
    # Connection
    CONNECT = "connect"
    CONNECT_ACK = "connect_ack"
    DISCONNECT = "disconnect"
    
    # State sync
    STATE_UPDATE = "state_update"
    SPAWN_ENTITY = "spawn_entity"
    DESTROY_ENTITY = "destroy_entity"
    
    # Player input
    INPUT_MOVE = "input_move"
    INPUT_TARGET = "input_target"
    INPUT_FIRE = "input_fire"
    INPUT_DOCK = "input_dock"
    INPUT_UNDOCK = "input_undock"
    
    # Combat
    DAMAGE = "damage"
    TARGET_LOCK = "target_lock"
    TARGET_UNLOCK = "target_unlock"
    
    # Chat
    CHAT = "chat"
    
    # Missions
    MISSION_ACCEPT = "mission_accept"
    MISSION_COMPLETE = "mission_complete"
    MISSION_UPDATE = "mission_update"
```

#### State Update Format
```json
{
  "type": "state_update",
  "timestamp": 1738463924.5,
  "data": {
    "entities": [
      {
        "id": "player_1",
        "position": {"x": 100.0, "y": 200.0, "z": 0.0},
        "velocity": {"x": 5.0, "y": 0.0, "z": 0.0},
        "health": {"shield": 500, "armor": 400, "hull": 300},
        "ship": {"type": "Rifter", "faction": "Minmatar"},
        "target": "npc_123"
      },
      {
        "id": "npc_123",
        "position": {"x": 500.0, "y": 200.0, "z": 0.0},
        "health": {"shield": 200, "armor": 150, "hull": 100},
        "ship": {"type": "Serpentis Scout", "faction": "Serpentis"}
      }
    ],
    "tick": 12345
  }
}
```

### 3D Client Requirements

The 3D client must:
1. Connect to server on `localhost:8765` (configurable)
2. Send/receive JSON messages via TCP
3. Parse entity state updates at 10 Hz
4. Interpolate entity positions for smooth rendering at 60 FPS
5. Send player input commands
6. Handle connection/disconnection gracefully

---

## Visual Requirements

### EVE Online-Inspired Style

#### 1. Space Environment
- **Background**: Deep black/dark blue space
- **Stars**: Scattered white/blue point lights
- **Nebulae**: Optional colored fog/particle clouds
- **Ambient**: Very low ambient light (space is dark!)

#### 2. Ship Rendering
- **Models**: Low-poly (5k-20k triangles per ship)
- **Materials**: PBR (Metallic, Roughness, Normal, Emissive)
- **Shading**: Physically-based rendering
- **Colors**: 
  - Minmatar: Brown/rust tones
  - Caldari: Blue/gray metallic
  - Gallente: Green/teal
  - Amarr: Gold/white
- **Details**:
  - Glowing engine trails (emissive)
  - Running lights (colored points)
  - Damage decals (optional)

#### 3. UI/HUD Style
- **Theme**: Semi-transparent dark panels
- **Colors**: 
  - Primary: Gold (#C8B464)
  - Secondary: Light blue (#6496C8)
  - Success: Green (#64C864)
  - Danger: Red (#C86464)
  - Background: Dark blue with transparency (rgba(20, 25, 35, 0.8))
- **Fonts**: Monospace or sans-serif, high contrast
- **Layout**: 
  - Top-left: Ship status (shield/armor/hull bars)
  - Top-right: Target info
  - Bottom: Selected item/module bar
  - Center: Crosshair/targeting reticle

#### 4. Visual Effects
- **Weapon Fire**:
  - Projectiles: Tracer lines with glow
  - Lasers: Beam with bloom effect
  - Missiles: Particle trail
- **Impacts**:
  - Shield hit: Blue flash and ripple
  - Armor hit: Orange sparks
  - Hull hit: White explosion particles
- **Explosions**:
  - Ship destruction: Expanding fireball with debris
  - Duration: 2-3 seconds
  - Particles: Orange/red/white with bloom

#### 5. Camera System
- **Default**: Third-person orbit camera
- **Controls**:
  - Mouse drag: Rotate around target
  - Mouse wheel: Zoom in/out (10m to 1000m)
  - Middle mouse: Pan
  - Double-click entity: Focus camera
- **Behavior**:
  - Smooth interpolation (lerp)
  - No clipping through ships
  - Auto-follow player ship option

---

## Implementation Phases

### Phase 5.1: Foundation (Week 1-2)
- [x] Create technical specification (this document)
- [ ] Set up Panda3D project structure
- [ ] Implement network client (TCP/JSON)
- [ ] Connect to existing Python server
- [ ] Parse state updates
- [ ] Display entity positions (cubes/spheres)

**Deliverable**: 3D client that connects to server and shows entities as colored primitives.

---

### Phase 5.2: Basic Rendering (Week 3-4)
- [ ] Implement camera system (orbit, zoom, pan)
- [ ] Add star field background
- [ ] Load 3D ship models (start with 1-2 ships)
- [ ] Implement entity interpolation (smooth movement)
- [ ] Add basic lighting

**Deliverable**: 3D client with camera controls and simple ship models.

---

### Phase 5.3: Visual Enhancement (Week 5-6)
- [ ] Create ship models for all 14 ships (or use placeholders)
- [ ] Implement PBR materials
- [ ] Add engine trails (particle effects)
- [ ] Add weapon effects (projectiles, lasers)
- [ ] Add hit effects (shield flash, sparks)
- [ ] Add explosion effects

**Deliverable**: 3D client with combat visual effects.

---

### Phase 5.4: UI/HUD (Week 7-8)
- [ ] Design UI layout
- [ ] Implement HUD overlay (ship status)
- [ ] Add target information panel
- [ ] Add module/weapon display
- [ ] Add crosshair/targeting reticle
- [ ] Add combat log (scrolling text)

**Deliverable**: 3D client with full HUD.

---

### Phase 5.5: Polish & Optimization (Week 9-10)
- [ ] Performance profiling
- [ ] Optimize rendering (LOD, culling)
- [ ] Add audio (weapon sounds, explosions)
- [ ] Add music (ambient space music)
- [ ] Cross-platform testing
- [ ] Bug fixes

**Deliverable**: Production-ready 3D client at 60+ FPS.

---

## Asset Requirements

### 3D Models

#### Ships (14 total)
**Frigates** (4):
- Rifter (Minmatar)
- Merlin (Caldari)
- Tristan (Gallente)
- Punisher (Amarr)

**Destroyers** (4):
- Thrasher (Minmatar)
- Cormorant (Caldari)
- Catalyst (Gallente)
- Coercer (Amarr)

**Cruisers** (6):
- Stabber, Rupture (Minmatar)
- Caracal, Moa (Caldari)
- Vexor (Gallente)
- Maller (Amarr)

**Format**: Panda3D `.egg` or `.bam`, or import from `.obj`, `.gltf`, `.fbx`

**Poly Count**: 5,000-20,000 triangles per ship

**Textures**: 
- Albedo (Base Color): 1024x1024 or 2048x2048
- Normal Map: Same resolution
- Metallic/Roughness: Combined, same resolution
- Emissive: For glowing parts, same resolution

---

### Effects

#### Particle Systems
- Engine trails (glowing particles)
- Weapon fire (tracer particles)
- Explosions (expanding fireball)
- Shield hit (ripple effect)
- Warp tunnel (optional, for warp effect)

#### Post-Processing
- Bloom (glow effect)
- FXAA/TAA (anti-aliasing)
- Color grading (slight blue tint)
- Vignette (darken edges)

---

### Audio

#### Sound Effects
- Weapon fire (different per weapon type)
- Weapon impact (shield, armor, hull)
- Explosions (ship destruction)
- UI clicks
- Warp activation/deactivation
- Docking/undocking

#### Music
- Ambient space music (looping)
- Combat music (triggered on combat)
- Exploration music (calm)

**Format**: `.ogg` or `.wav`

---

### UI Assets

#### Textures
- UI panel background (semi-transparent)
- Button states (normal, hover, pressed)
- Icons (modules, weapons, drones)
- Health bar backgrounds

#### Fonts
- Primary: Monospace or EVE-style font
- Size: 12pt, 14pt, 18pt, 24pt

---

## Performance Targets

### Frame Rate
- **Target**: 60 FPS
- **Minimum**: 30 FPS
- **Test Hardware**: Mid-range laptop (GTX 1060 or equivalent)

### Network
- **Latency**: < 50ms (LAN), < 150ms (Internet)
- **Bandwidth**: < 100 KB/s per client
- **Update Rate**: 10 Hz from server, 60 Hz rendering

### Memory
- **RAM**: < 500 MB
- **VRAM**: < 200 MB (for all textures/models)

### Load Time
- **Initial**: < 5 seconds
- **Asset loading**: < 2 seconds

---

## Development Timeline

### Total Estimated Time: 10 weeks

| Phase | Duration | Start | End | Deliverable |
|-------|----------|-------|-----|-------------|
| 5.1 Foundation | 2 weeks | Week 1 | Week 2 | Basic client |
| 5.2 Rendering | 2 weeks | Week 3 | Week 4 | Camera + models |
| 5.3 Visual FX | 2 weeks | Week 5 | Week 6 | Combat effects |
| 5.4 UI/HUD | 2 weeks | Week 7 | Week 8 | Full interface |
| 5.5 Polish | 2 weeks | Week 9 | Week 10 | Production ready |

### Parallel Work
- **Server**: No changes needed (existing Python server works)
- **Assets**: Can be created in parallel (use placeholders initially)
- **Testing**: Continuous throughout all phases

---

## Risks & Mitigations

### Risk 1: Performance Issues
**Mitigation**: 
- Use Panda3D's built-in optimization (LOD, culling)
- Profile early and often
- Use low-poly models
- Limit particle count

### Risk 2: Asset Creation Time
**Mitigation**:
- Use placeholder models initially (cubes, spheres)
- Focus on 2-3 ships first
- Consider free/paid asset stores
- Simple textures (solid colors) for MVP

### Risk 3: Network Latency
**Mitigation**:
- Client-side prediction (extrapolate positions)
- Entity interpolation (lerp between updates)
- Lag compensation (buffer 100ms)
- Visual feedback for network issues

### Risk 4: Platform Compatibility
**Mitigation**:
- Test on Windows, Linux, Mac regularly
- Use cross-platform libraries (Panda3D is)
- Provide fallback options (disable effects if slow)

---

## Success Metrics

### Phase 5 is complete when:
- ✅ 3D client connects to Python server
- ✅ All 14 ships are rendered in 3D
- ✅ Camera system works smoothly
- ✅ Combat has visual effects
- ✅ HUD shows all necessary information
- ✅ Performance is 60+ FPS
- ✅ Works on Windows, Linux, Mac
- ✅ All existing game features work

---

## Next Steps

1. **Review & Approve** this specification
2. **Set up Panda3D** development environment
3. **Create basic client** (Phase 5.1)
4. **Iterate** on features
5. **Test** with real gameplay
6. **Release** MVP 3D client

---

## References

- [Panda3D Documentation](https://docs.panda3d.org/)
- [EVE Online Visual Style](https://www.eveonline.com/)
- [Astrox Imperium](https://store.steampowered.com/app/954870/Astrox_Imperium/)
- [PBR Texturing Guide](https://marmoset.co/posts/physically-based-rendering-and-you-can-too/)

---

**Last Updated**: February 2, 2026  
**Author**: EVE OFFLINE Development Team  
**Status**: Ready for implementation
