# GUI Integration Architecture

```
┌─────────────────────────────────────────────────────────────────────────┐
│                        EVE OFFLINE C++ Client                            │
│                         Application Class                                 │
└─────────────────────────────────────────────────────────────────────────┘
                                    │
                    ┌───────────────┼───────────────┐
                    │               │               │
                    ▼               ▼               ▼
        ┌──────────────────┐ ┌──────────────┐ ┌─────────────┐
        │  Input Handler   │ │  Game Client │ │   Camera    │
        │   (GLFW Input)   │ │   (Entities) │ │  (Look At)  │
        └──────────────────┘ └──────────────┘ └─────────────┘
                    │               │
                    │               │
                    ▼               ▼
        ┌──────────────────────────────────────────────┐
        │         Context Menu & Radial Menu           │
        │              Integration Layer               │
        └──────────────────────────────────────────────┘
                    │
        ┌───────────┴───────────┐
        │                       │
        ▼                       ▼
┌───────────────────┐   ┌──────────────────┐
│  UI::ContextMenu  │   │  UI::RadialMenu  │
│   (ImGui-based)   │   │  (ImGui-based)   │
└───────────────────┘   └──────────────────┘
        │                       │
        └───────────┬───────────┘
                    │
                    ▼
        ┌──────────────────────┐
        │  Movement Commands   │
        │  • Approach          │
        │  • Orbit             │
        │  • Keep at Range     │
        │  • Warp To           │
        │  • Dock/Jump         │
        │  • Align To          │
        └──────────────────────┘
```

## Input Flow Diagrams

### Context Menu Flow (Right-Click)

```
User Input
    │
    ├─ Right Mouse Button Press
    │   └─ Record click position
    │
    ├─ Right Mouse Button Release
    │   ├─ Check drag distance (< 5px = click, >= 5px = camera drag)
    │   │
    │   └─ Quick Click Detected
    │       ├─ Pick entity at mouse position (3D ray casting)
    │       │
    │       ├─ Entity Found?
    │       │   ├─ YES → ShowEntityMenu(entityId, isLocked)
    │       │   │   ├─ Show Approach
    │       │   │   ├─ Show Orbit (submenu: 500m, 1km, 5km, 10km, 20km, 50km)
    │       │   │   ├─ Show Keep at Range (submenu: 1km, 5km, 10km, 20km, 50km)
    │       │   │   ├─ Show Warp To (submenu: 0km, 10km, 50km, 100km)
    │       │   │   ├─ Show Lock/Unlock (dynamic based on isLocked)
    │       │   │   ├─ Show Look At
    │       │   │   └─ Show Info
    │       │   │
    │       │   └─ NO → Close()
    │       │
    │       └─ User Clicks Menu Item
    │           └─ Execute Callback
    │               └─ Command Executes (e.g., commandApproach(entityId))
```

### Radial Menu Flow (Hold Left-Click)

```
User Input
    │
    ├─ Left Mouse Button Press
    │   ├─ Record hold position (m_radialMenuStartX/Y)
    │   └─ Record hold time (m_radialMenuHoldStartTime)
    │
    ├─ Mouse Move (while holding)
    │   ├─ Calculate hold duration (currentTime - startTime)
    │   ├─ Check movement distance from start position
    │   │
    │   └─ Hold Duration >= 300ms AND Distance < 10px?
    │       ├─ YES → Open Radial Menu
    │       │   ├─ Pick entity at hold position
    │       │   ├─ Entity Found?
    │       │   │   ├─ YES → radialMenu->Open(x, y, entityId)
    │       │   │   │   ├─ Display 8-segment circular menu
    │       │   │   │   └─ Set m_radialMenuOpen = true
    │       │   │   │
    │       │   │   └─ NO → Do nothing
    │       │   │
    │       │   └─ Update mouse position
    │       │       └─ Highlight segment based on angle
    │       │
    │       └─ NO → Continue normal click behavior
    │
    └─ Left Mouse Button Release
        ├─ Radial Menu Open?
        │   ├─ YES → Confirm selection
        │   │   ├─ Get highlighted action
        │   │   ├─ Execute callback (e.g., commandOrbit(entityId, 500.0f))
        │   │   ├─ Close menu
        │   │   └─ Set m_radialMenuOpen = false
        │   │
        │   └─ NO → Process normal click
        │       ├─ Pick entity
        │       └─ Execute movement mode command (if Q/W/E/D active)
```

### Movement Shortcut Flow (Q/W/E/D Keys)

```
User Input
    │
    ├─ Press Q/W/E/D Key
    │   ├─ Q → m_approachActive = true, others = false
    │   ├─ W → m_orbitActive = true, others = false
    │   ├─ E → m_keepRangeActive = true, others = false
    │   └─ D → m_dockingModeActive = true, others = false
    │
    │   └─ Console feedback: "[Controls] {Mode} active — click a target"
    │
    └─ Left-Click on Entity
        ├─ Check active mode
        │   ├─ m_approachActive? → commandApproach(entityId)
        │   ├─ m_orbitActive? → commandOrbit(entityId, 500.0f)
        │   ├─ m_keepRangeActive? → commandKeepAtRange(entityId, 2500.0f)
        │   └─ m_dockingModeActive? → Console: "Dock/Jump through {entityId}"
        │
        └─ Deactivate mode (set flag to false)
```

## Callback Architecture

```
┌─────────────────────────────────────────────────────────────────────┐
│                  Application::setupUICallbacks()                     │
│                   (Called during initialization)                      │
└─────────────────────────────────────────────────────────────────────┘
                                │
                ┌───────────────┴───────────────┐
                │                               │
                ▼                               ▼
┌───────────────────────────────┐   ┌──────────────────────────────┐
│  Context Menu Callbacks       │   │  Radial Menu Callbacks       │
│                               │   │                              │
│  SetApproachCallback()        │   │  SetActionCallback()         │
│  SetOrbitCallback()           │   │   (Switch on action enum)    │
│  SetKeepAtRangeCallback()     │   │                              │
│  SetWarpToCallback()          │   │   APPROACH → commandApproach │
│  SetLockTargetCallback()      │   │   ORBIT → commandOrbit       │
│  SetUnlockTargetCallback()    │   │   KEEP_AT_RANGE → ...        │
│  SetLookAtCallback()          │   │   WARP_TO → ...              │
│  SetShowInfoCallback()        │   │   LOCK_TARGET → ...          │
│                               │   │   ALIGN_TO → ...             │
└───────────────────────────────┘   │   LOOK_AT → ...              │
                │                   │   SHOW_INFO → ...            │
                │                   │                              │
                │                   └──────────────────────────────┘
                │                               │
                └───────────────┬───────────────┘
                                │
                                ▼
                ┌──────────────────────────────────┐
                │   Movement Command Functions     │
                │                                  │
                │   commandApproach(entityId)      │
                │   commandOrbit(entityId, dist)   │
                │   commandKeepAtRange(id, dist)   │
                │   commandWarpTo(entityId)        │
                │   commandAlignTo(entityId)       │
                │   commandStopShip()              │
                └──────────────────────────────────┘
                                │
                                ▼
                ┌──────────────────────────────────┐
                │   Game State Updates             │
                │                                  │
                │   m_currentMoveCommand = ...     │
                │   m_moveTargetId = entityId      │
                │   m_orbitDistance = ...          │
                │   m_keepAtRangeDistance = ...    │
                └──────────────────────────────────┘
                                │
                                ▼
                ┌──────────────────────────────────┐
                │   Update Loop                    │
                │   (applies movement commands)    │
                └──────────────────────────────────┘
```

## Render Pipeline

```
Application::render()
    │
    ├─ Renderer::beginFrame()
    │
    ├─ Renderer::renderScene(*camera)
    │   └─ Renders 3D entities, starfield, effects
    │
    ├─ RmlUiManager::Update() & Render()
    │   └─ Renders game UI panels (Inventory, Fitting, etc.)
    │
    ├─ PhotonHUD::update(*context, shipData, targets, overview, selectedItem)
    │   └─ Renders ship HUD, target cards, overview panel
    │
    ├─ ContextMenu::Render()  ← NEW
    │   └─ If menu is open, renders ImGui popup with hierarchical menu
    │
    ├─ RadialMenu::Render()  ← NEW
    │   └─ If m_radialMenuOpen, renders circular menu with segments
    │
    └─ Renderer::endFrame()
```

## State Management

```
┌─────────────────────────────────────────────────────────────────┐
│                     Application State                            │
└─────────────────────────────────────────────────────────────────┘
│
├─ Context Menu State
│   ├─ m_showContextMenu (bool) - unused, context menu manages itself
│   ├─ m_contextMenuEntityId (string) - unused, kept for compatibility
│   └─ m_contextMenuX/Y (double) - unused, kept for compatibility
│
├─ Radial Menu State
│   ├─ m_radialMenuOpen (bool) - true when menu is visible
│   ├─ m_radialMenuStartX/Y (double) - hold position
│   └─ m_radialMenuHoldStartTime (double) - for 300ms threshold
│
├─ Movement Mode State
│   ├─ m_approachActive (bool) - Q-key mode
│   ├─ m_orbitActive (bool) - W-key mode
│   ├─ m_keepRangeActive (bool) - E-key mode
│   └─ m_dockingModeActive (bool) - D-key mode (NEW)
│
└─ Movement Command State
    ├─ m_currentMoveCommand (enum) - active command
    ├─ m_moveTargetId (string) - target entity
    ├─ m_orbitDistance (float) - orbit radius
    └─ m_keepAtRangeDistance (float) - keep at range distance
```

## Memory Safety

All menus are managed via `std::unique_ptr`:
- ✅ Automatic cleanup on Application destruction
- ✅ No manual delete required
- ✅ No dangling pointers
- ✅ Exception-safe

```cpp
std::unique_ptr<UI::ContextMenu> m_contextMenu;
std::unique_ptr<UI::RadialMenu> m_radialMenu;
```

## Performance Characteristics

### Context Menu
- **Activation**: O(1) - immediate on right-click
- **Entity Picking**: O(n) - iterates all entities for ray casting
- **Rendering**: O(1) - fixed number of menu items
- **Memory**: < 1 KB per menu instance

### Radial Menu
- **Hold Detection**: O(1) - simple time/distance check per frame
- **Entity Picking**: O(n) - only on menu open
- **Rendering**: O(1) - 8 fixed segments
- **Mouse Update**: O(1) - angle calculation
- **Memory**: < 1 KB per menu instance

### Movement Shortcuts
- **Key Press**: O(1) - flag toggle
- **Click Handling**: O(n) - entity picking
- **Execution**: O(1) - direct function call
- **Memory**: 4 bytes per boolean flag

**Total overhead**: Negligible (<0.1ms per frame)
