# EVE-Style UI Implementation Plan

## Overview
Implement EVE Online-inspired user interface for the 3D client, focusing on strategic gameplay with context menus, entity selection, and tactical commands.

## Phase 1: Entity Selection System ✅ COMPLETE

### 1.1 Mouse Picking / Ray Casting ✅
- Implement 3D ray casting from mouse click to 3D world
- Detect which entity (if any) was clicked
- Handle click on empty space vs. entity

**Technical Details:**
- Use Panda3D's `CollisionTraverser` and `CollisionHandlerQueue`
- Add collision spheres to entity nodes
- Convert 2D mouse coords to 3D ray

### 1.2 Selection State Management ✅
- Track currently selected entity
- Visual highlight for selected entity (glow, outline, or bracket highlight)
- Selected entity info display
- Deselect when clicking empty space

### 1.3 Visual Feedback ✅
- Highlight selected entity with different color/glow
- Show selection bracket with enhanced visibility
- Distance indicator to selected object

## Phase 2: Context Menu System ✅ COMPLETE

### 2.1 Right-Click Context Menu
- Detect right-click events (mouse2)
- Position menu at mouse cursor
- Menu items based on what was clicked:
  - **Empty space**: Navigate to, Bookmark location
  - **Entity**: Approach, Orbit, Keep at Range, Look at, Show info, Lock Target, Warp to
  - **Stargate**: Jump, Warp to
  - **Station**: Dock, Warp to

**Menu Structure:**
```
Right-click on Ship:
├── Approach
├── Orbit ▶
│   ├── 500m
│   ├── 1km
│   ├── 5km
│   └── 10km
├── Keep at Range ▶
│   ├── 1km
│   ├── 5km
│   ├── 10km
│   └── 20km
├── Warp to ▶
│   ├── At 0km
│   ├── At 10km
│   ├── At 50km
│   └── At 100km
├── Lock Target
├── Look at
├── Show Info
└── Cancel
```

### 2.2 Menu Rendering ✅
- Use ImGui for menu (clean integration with existing UI)
- Custom 2D overlay rendering with full control
- Semi-transparent dark background
- White/gold text (EVE color scheme)
- Mouse hover highlights

**Implementation:**
- EVE-style colors: Dark background (0.1, 0.1, 0.1, 0.95) with gold hover (0.8, 0.6, 0.2)
- Implemented in `cpp_client/src/ui/context_menu.cpp`

### 2.3 Menu Actions ✅
- Send appropriate network commands to server (callback system ready)
- Close menu after action selected
- Visual feedback (ship starts moving, etc.)

## Phase 3: HUD/UI Overlay 🔲 Important

### 3.1 Selected Object Panel
**Location:** Top-center or side panel
**Content:**
- Entity name
- Entity type (Rifter, Cruor, etc.)
- Distance (updates in real-time)
- Velocity
- Quick action buttons: [Approach] [Orbit] [Keep at Range] [Lock]

### 3.2 Ship Status Panel
**Location:** Bottom-left corner (EVE standard)
**Content:**
- Ship name and type
- Shield bar (blue)
- Armor bar (orange)
- Hull bar (red)
- Capacitor (shows current cap)
- Speed indicator

### 3.3 Target List
**Location:** Top-right (EVE standard)
**Content:**
- List of locked targets
- Each shows: Name, distance, shield/armor/hull bars
- Max 5-7 targets visible

### 3.4 Overview Panel (Future)
**Location:** Right side or separate window
**Content:**
- Filterable list of entities
- Columns: Name, Type, Distance, Corporation
- Tabs for different presets (PvP, Travel, Mining)

## Phase 4: Entity Brackets 🔲 Visual Polish

### 4.1 Bracket System
- 2D billboard icons attached to 3D entities
- Different bracket types:
  - Ships: Box bracket [ ]
  - Stations: Plus bracket +
  - Stargates: Circle bracket ○
  - Wrecks: Triangle bracket △
  
### 4.2 Bracket Information
- Entity name text above bracket
- Distance below bracket
- Color-coded by faction/standing:
  - Player/Friendly: Blue
  - Neutral: White/Gray
  - Hostile: Red/Orange
  - Corporation: Light blue

### 4.3 Bracket Behavior
- Always faces camera (billboard)
- Scale based on distance (closer = larger)
- Fade when too far or camera too close
- Selected bracket highlighted

## Phase 5: Navigation Commands 🔲 Core Gameplay

### 5.1 Approach Command
- Server-side: Move ship toward target
- Visual: Show direction indicator
- Stop when within 1km (default)

### 5.2 Orbit Command
- Server-side: Circular motion around target
- Parameters: orbit distance (500m - 50km)
- Visual: Show orbit path (optional)

### 5.3 Keep at Range
- Server-side: Maintain distance from target
- Move away if too close, approach if too far
- Parameters: range (1km - 50km)

### 5.4 Warp To
- Server-side: Warp drive mechanics
- Visual: Warp tunnel effect (future)
- Land at specified distance

### 5.5 Look At
- Client-side: Camera focuses on entity
- Smooth camera transition

## Phase 6: Hotkey System 🔲 Efficiency

### 6.1 Modifier + Click
- Q + Click: Approach
- W + Click: Orbit (default range)
- E + Click: Keep at Range (default)
- D: Dock/Jump (if applicable)
- Ctrl+Space: Stop ship

### 6.2 Hotkey Configuration
- Allow rebinding hotkeys
- Save/load hotkey profiles
- Show hotkey hints in UI

## Technical Implementation Details

### File Structure
```
client_3d/
├── ui/
│   ├── __init__.py
│   ├── context_menu.py      # Right-click menu system
│   ├── selection.py          # Entity selection and picking
│   ├── hud_overlay.py        # HUD panels and overlays
│   ├── brackets.py           # Entity bracket system
│   └── panels/
│       ├── selected_object.py
│       ├── ship_status.py
│       └── target_list.py
└── core/
    └── game_client.py        # Integrate UI systems
```

### Key Libraries/Techniques
- **Panda3D DirectGUI**: For UI elements
- **CollisionRay**: For mouse picking
- **OnscreenText/Image**: For HUD overlays
- **NodePath transformations**: For brackets
- **asyncio**: For network commands

### Color Scheme (EVE-Inspired)
- Background: Dark gray/black with transparency
- Primary text: White/light gray
- Accent: Gold/yellow
- Friendly: Blue (#4A90E2)
- Hostile: Red/Orange (#E24A4A)
- Selected: Bright gold highlight

## Implementation Priority

**Week 1:** ✅ COMPLETE
1. ✅ Entity selection with ray casting
2. ✅ Basic context menu rendering
3. ✅ Menu actions (Approach, Orbit, Keep at Range, Warp To)
4. ✅ Tactical overlay with range circles and velocity vectors

**Week 2:** (Remaining work)
4. Selected object panel (already exists in overview)
5. Ship status HUD (already exists)
6. Entity brackets (basic implementation exists)

**Week 3:** (Future)
7. Complete navigation commands (network integration)
8. Target list UI (already exists in EVE Target List)
9. Hotkey system

**Week 4:** (Future)
10. Polish and refinement
11. Overview panel (already exists - fully functional)
12. Performance optimization

## Testing Strategy

1. **Unit Tests**: Test menu logic, selection logic
2. **Integration Tests**: Test with server
3. **Visual Tests**: Ensure UI looks good at different resolutions
4. **Usability Tests**: Verify controls feel responsive
5. **Performance Tests**: Ensure 60 FPS with UI active

## Success Metrics

- ✅ Can select entities with left-click
- ✅ Right-click shows context menu with working commands
- ✅ Selected object shows in HUD panel
- ✅ Can issue Approach/Orbit/Keep at Range commands (UI implementation complete)
- ✅ Entity brackets visible and informative
- ✅ All interactions feel responsive (< 100ms)
- ✅ Maintains 60 FPS with full UI
- ✅ Tactical overlay renders range circles and velocity vectors
- ✅ Context menu supports hierarchical submenus
- ⏳ Network integration with server commands (next step)

## Notes

- Keep EVE's clean, minimalist aesthetic ✅
- Dark backgrounds with semi-transparency ✅
- Ensure readability in all situations ✅
- Performance is critical - UI updates 60 times/sec ✅
- Server is authoritative - UI just sends commands (callbacks ready)
- Test with multiple ships on screen (fleet scenarios)

---

**Document Version:** 1.1  
**Last Updated:** February 7, 2026  
**Status:** Phase 1 & 2 Complete - Context Menu and Tactical Overlay Implemented!
