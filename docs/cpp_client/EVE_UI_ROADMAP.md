# Atlas UI - Complete Implementation Roadmap

**Date**: February 11, 2026  
**Project**: EVE OFFLINE C++ Client  
**Goal**: Implement a complete Atlas UI system for the game client

---

## Overview

This document outlines the comprehensive plan to implement the Atlas UI system in the C++ OpenGL client. The UI features dense information displays, extensive customization, and sophisticated interaction patterns.

> **Note**: UI framework was renamed from "Photon" to "Atlas" and sidebar from "Neocom" to "Sidebar" to establish an original naming convention. See [NAMING_CONVENTION.md](../NAMING_CONVENTION.md).

## Current Status (Phase 4.10 In Progress)

✅ **Completed**:
- Phase 4.3: Entity rendering system
- Phase 4.3: Circular target icons with arc-based health indicators
- Phase 4.3: Basic color scheme
- Phase 4.3: Entity visual management
- Phase 4.3: ImGui integration
- Phase 4.4: Input system with 3D picking (entity selection, targeting)
- Phase 4.5: Overview panel with sorting, filtering, and interactions
- Phase 4.6: Module system with activation and visual feedback
- **Phase 4.7: Context & Radial Menus** ✅ (Feb 11, 2026)
- Phase 4.8: Sidebar and Additional Panels (Partial)
- **Phase 4.9: Movement Command Shortcuts (Q/W/E/D + Click)** ✅ (Feb 11, 2026)
- **Phase 4.9.1: Visual Mode Indicators** ✅ (Feb 11, 2026) — On-screen HUD indicator for active movement modes
- **Phase 4.9.2: Info Panel** ✅ (Feb 11, 2026) — Show Info window with entity details, health, distance, faction
- **Phase 4.9.3: Overview Tab Switching** ✅ (Feb 11, 2026) — Interactive clickable tab headers (All/Combat/Mining/Custom)
- **Phase 4.9.4: Selected Item Panel Callbacks** ✅ (Feb 11, 2026) — Orbit/Approach/Warp/Info buttons wired to game logic

⏳ **In Progress**: 
- **Phase 4.10: Window Management & Customization** ⏳ (Feb 11, 2026)
  - ✅ Layout Manager — JSON save/load for panel positions, sizes, visibility, opacity
  - ✅ Default presets — default, combat, mining layout presets
  - ✅ Per-panel opacity support
  - ✅ Naming convention — all UI components renamed to original names (see NAMING_CONVENTION.md)
  - ⬜ Window snap-to-edge
  - ⬜ UI scale controls
  - ⬜ Color scheme switching

---

## Phase Breakdown

### Phase 4.4: Input System & 3D Interaction (2 weeks)

**Priority**: Critical - Foundation for all interactions
**Status**: Complete ✅ (Feb 5, 2026)

#### Mouse Interactions ✅
- [x] **3D World Picking**
  - Ray casting from mouse to 3D world
  - Entity selection in space
  - Hover highlighting
  - Selection feedback
  
- [x] **Basic Click Actions**
  - Left-click: Select entity
  - Ctrl+Click: Lock target
  - Ctrl+Shift+Click: Unlock target
  - Double-click: Approach/align to entity
  
- [x] **Camera Controls**
  - Mouse drag to rotate camera
  - Alt+Click: Look at object
  - Mouse wheel: Zoom in/out
  - Middle mouse: Pan camera

#### Keyboard Shortcuts (Phase 1) ✅
- [x] Ctrl+Space: Stop ship
- [x] Tab: Cycle targets
- [x] Ctrl+F9: Hide/show UI
- [x] Escape: Clear selection

**Deliverables**: ✅
- InputHandler class with 3D picking
- Basic keyboard shortcut system
- Entity selection and targeting

---

### Phase 4.5: Overview Panel & Core HUD (3 weeks)

**Priority**: High - Most important UI elements
**Status**: Complete ✅ (Feb 9, 2026)

#### Overview Panel ✅
- [x] **Window Framework**
  - Resizable, movable window
  - Column headers (Name, Distance, Type, Corp, Standing)
  - Sort by any column
  - Row highlighting on hover
  
- [x] **Entity List Display**
  - All entities in space
  - Color-coded by standing:
    - Red: Hostile
    - Blue: Friendly/Corp
    - Grey: Neutral
  - Distance display (m, km, AU)
  - Ship type display
  
- [x] **Filtering System**
  - Tab-based filters (All, Hostile, Friendly, Neutral)
  - Custom filter creation (framework ready)
  - Show/hide entity types
  - Distance filters
  
- [x] **Interactions**
  - Left-click: Select entity
  - Ctrl+Click: Lock target
  - Double-click: Align/warp to
  - Right-click: Context menu

#### HUD (Bottom-Center) - Complete ✅
- [x] **Ship Status Display**
  - Circular capacitor ring (blue)
  - Shield ring (blue, outer)
  - Armor ring (yellow, middle)
  - Hull ring (red, inner)
  - Ship icon in center
  
- [x] **Status Indicators**
  - Current/max values display
  - Percentage text
  - Color changes based on levels
  - Warning indicators (< 25%)
  
- [x] **Speed Display**
  - Current velocity (m/s)
  - Max velocity
  - Speed bar indicator
  - Afterburner indicator

**Deliverables**:
- Overview window class
- Filterable entity list
- Core HUD with status rings
- Ship status visualization

---

### Phase 4.6: Module System & Activation (2-3 weeks)

**Priority**: High - Core gameplay mechanic
**Status**: Partial ✅ (Feb 9, 2026)

#### Module Slots Display
- [x] **Slot Arrangement**
  - High slots (top row, 8 max)
  - Medium slots (middle row, 8 max)
  - Low slots (bottom row, 8 max)
  - Rig slots (special, 3 max)
  
- [x] **Module Icons**
  - Module type icons
  - Active/inactive states
  - Cooldown timers (circular)
  - Charge indicators
  - Ammunition count
  
- [x] **Visual Feedback**
  - Active modules highlighted
  - Cooldown overlay
  - Overheat effects (red/orange glow)
  - Low capacitor warnings

#### Module Activation
- [x] **Keyboard Shortcuts**
  - F1-F8: High slots
  - Ctrl+F1-F8: Low slots
  - Alt+F1-F8: Mid slots
  - Shift+Key: Toggle overheat
  
- [x] **Mouse Activation**
  - Left-click: Activate/deactivate
  - Right-click: Options menu
  - Drag to reorder (if unlocked)
  
- [ ] **Target Integration**
  - Active modules show on target icons
  - Weapon range indicators
  - Auto-target on activation (optional)

#### Module Management
- [ ] Module drag-and-drop
- [ ] Module grouping
- [ ] Saved module presets
- [ ] Overheat management

**Deliverables**:
- Module slot UI system
- Keyboard activation system
- Module state management
- Visual feedback system

---

### Phase 4.7: Context & Radial Menus (1-2 weeks)

**Priority**: Medium-High - Essential for interactions
**Status**: Complete ✅ (Feb 11, 2026)

#### Context Menu System ✅
- [x] **Universal Right-Click**
  - Works on: Space entities
  - Hierarchical menu structure (with distance submenus)
  - Dynamic options based on entity state (locked/unlocked)
  
- [x] **Entity Actions**
  - Look At ✅
  - Approach ✅
  - Orbit ✅ (with distance submenu: 500m, 1km, 5km, 10km, 20km, 50km)
  - Keep at Range ✅ (with distance submenu: 1km, 5km, 10km, 20km, 50km)
  - Warp To ✅ (with distance submenu: 0km, 10km, 50km, 100km)
  - Jump Through (gates) — Future
  - Dock (stations) — Future
  
- [x] **Target Actions**
  - Lock Target ✅
  - Unlock Target ✅
  - Set as Active Target — Future
  
- [x] **Information Actions**
  - Show Info ✅ (info panel with entity details, health, faction) (Feb 11, 2026)
  - Show Location — Future
  - Create Bookmark — Future
  
- [ ] **Fleet Actions** (if in fleet)
  - Warp to Member — Future
  - Set as Squad Commander — Future
  - Broadcast actions — Future

#### Radial Menu ✅
- [x] **Activation**
  - Hold left-click on entity for 300ms
  - Circular menu appears at hold position
  - Drag mouse to select option
  
- [x] **Quick Actions**
  - Orbit (default 500m) ✅
  - Approach ✅
  - Lock Target ✅
  - Keep at Range (default 2500m) ✅
  - Warp To ✅
  - Align To ✅
  - Look At ✅
  - Show Info ✅
  
- [x] **Visual Design**
  - Circular segments (8 segments) ✅
  - Icons for each action ✅
  - Hover highlighting ✅
  - Smooth animations ✅
  - EVE-style teal accent colors ✅

**Deliverables**: ✅
- Context menu system ✅
- Radial menu implementation ✅
- Action execution framework ✅
- Callbacks wired to movement commands ✅
- Input detection (right-click, hold-click) ✅

---

### Phase 4.8: Neocom & Additional Panels (3 weeks)

**Priority**: Medium - Important for functionality
**Status**: Partial ✅ (Feb 9, 2026)

#### Neocom (Main Menu)
- [x] **Icon Bar**
  - Vertical bar on left side
  - Icons for main services
  - Hover expansion
  - Draggable icons
  
- [x] **Core Services**
  - Character Sheet (C)
  - Inventory (Alt+T)
  - Ship Hangar
  - Fitting Window (Alt+F)
  - Market
  - Industry
  - Map (F10)
  - Corporation
  
- [x] **Visual Design**
  - Semi-transparent background
  - Teal accent on hover
  - Badge notifications
  - Collapse/expand button

#### D-Scan Window
- [x] **Scanner Interface**
  - Scan angle slider (5° to 360°)
  - Range slider (0 to max)
  - Scan button / V hotkey
  - Auto-rescan checkbox
  
- [x] **Results Display**
  - Entity list with types
  - Sortable columns
  - Filter by type
  - Refresh timestamp
  
- [ ] **Visual Cone**
  - Optional: Show scan cone in space
  - Direction indicator

#### Inventory Window
- [x] **Cargo Hold** (RmlUi template)
  - Item list with icons
  - Stack counts
  - Volume bar (m³ used/max)
  
- [ ] **Station Hangar**
  - Ship list
  - Container folders
  - Search functionality
  
- [ ] **Drag-and-Drop**
  - Between containers
  - To/from cargo
  - Stack splitting (Shift+Drag)
  
- [ ] **Context Menu**
  - Trash item
  - Repackage
  - View info

#### Fitting Window
- [x] **Ship Display** (RmlUi template)
  - Slot layout
  - Resource bars (CPU, PG, Calibration)
  - Stats display (EHP, DPS, Speed, Cap Stable)

- [ ] **Advanced Fitting**
  - 3D ship model (optional)
  - Drag modules to slots
  
- [ ] **Saved Fits**
  - Save fitting
  - Load fitting
  - Share fitting (export)

**Deliverables**:
- Neocom menu system
- D-Scan window
- Inventory system
- Fitting window

---

### Phase 4.9: Drone & Movement Controls (1-2 weeks)

**Priority**: Medium - Gameplay enhancement
**Status**: Complete ✅ (Feb 11, 2026)

#### Drone Interface ✅
- [x] **Drone Bay Window**
  - Available drones list
  - Drone in space list
  - Bandwidth bar (used/max)
  
- [x] **Drone Controls**
  - Launch drones (Shift+F)
  - Return drones (Shift+R)
  - Engage target (F)
  - Return and orbit (not engaging)
  
- [x] **Visual Indicators**
  - Drones in space icons
  - Health bars
  - Engagement status
  - Low shield/armor warnings

#### Movement Commands ✅
- [x] **Hold-Key + Click**
  - Q+Click: Approach ✅
  - W+Click: Orbit ✅
  - E+Click: Keep at Range ✅
  - D+Click: Dock/Jump ✅ (NEW)
  
- [x] **Range Selection**
  - Default ranges via keyboard shortcuts ✅
  - Custom range via context menu submenus ✅
  - Orbit: 500m, 1km, 5km, 10km, 20km, 50km
  - Keep at Range: 1km, 5km, 10km, 20km, 50km
  
- [x] **Visual Feedback**
  - On-screen mode indicator (HUD text pill) ✅ (Feb 11, 2026)
  - Mode text auto-clears after command execution ✅
  - Flight path indicator — Future
  - Range circle preview — Future
  - ETA display — Future

**Deliverables**: ✅
- Drone control system ✅
- Movement command system ✅
- Keyboard shortcut integration ✅
- D-key docking mode ✅ (NEW)

---

### Phase 4.10: Window Management & Customization (2 weeks)

**Priority**: Medium-Low - Polish & UX
**Status**: ⏳ In Progress (Feb 11, 2026)

#### Window System
- [x] **Movement & Resizing** (existing via panelBeginStateful)
  - Drag title bar to move
  - Drag edges/corners to resize
  - [x] Snap to screen edges (via DockingManager)
  - [ ] Snap to other windows
  
- [x] **Window States**
  - Pin/unpin (lock position) — via interface lock
  - Minimize to title bar — via panelBeginStateful
  - Close window — via × button
  - [x] Restore defaults — via ResetToDefaultLayout()
  
- [x] **Transparency** (Partial)
  - [x] Per-panel opacity (0.15–1.0) via SetPanelOpacity/GetPanelOpacity
  - [ ] Global opacity hotkey
  - [ ] Window blur effect
  - [ ] Inactive window dimming

#### Layouts & Presets
- [x] **Save Layouts** ✅ (Feb 11, 2026)
  - [x] LayoutManager with JSON serialization
  - [x] Multiple named presets (default, combat, mining, custom)
  - [x] SaveLayout / LoadLayout API
  - [ ] Quick switch hotkeys
  - [x] Export/import layouts (JSON files)
  
- [x] **Reset Options** ✅ (Feb 11, 2026)
  - [ ] Reset single window
  - [x] Reset all windows — ResetToDefaultLayout()
  - [x] Reset to defaults — built-in presets

#### Customization Options
- [ ] **Color Schemes**
  - Atlas UI (default)
  - Classic theme
  - Custom colors
  - Color-blind modes
  
- [ ] **UI Scale**
  - Global UI scale (50%-200%)
  - Per-window scaling
  - Font size adjustment
  
- [ ] **Overview Customization**
  - Column visibility
  - Column order
  - Column widths
  - Row height
  - Import YAML presets

**Deliverables**:
- Window management system
- Layout save/load
- Customization options
- Preset system

---

### Phase 4.11: Advanced Features & Polish (2-3 weeks)

**Priority**: Low - Enhancement

#### Map Integration
- [ ] Star map (F10)
- [ ] Solar system map
- [ ] Route planning
- [ ] Jump range display

#### Bookmarks
- [ ] Bookmark creation (Ctrl+B)
- [ ] Bookmark folders
- [ ] Bookmark sharing
- [ ] Warp to bookmark

#### Chat System
- [x] Local chat
- [ ] Private conversations
- [x] Fleet chat
- [x] Corporation chat
- [x] Chat window management

#### Notifications
- [x] Toast notifications
- [x] Warning messages
- [x] Combat notifications
- [ ] Mail notifications

#### Performance
- [ ] UI rendering optimization
- [ ] Large overview lists (1000+ entities)
- [ ] Memory management
- [ ] FPS target (60+)

**Deliverables**:
- Map system
- Bookmark system
- Chat windows
- Notification system
- Performance optimization

---

## Technical Architecture

### UI Framework

**RmlUi Integration** (Primary — game-facing panels):
- Build with: `cmake .. -DUSE_RMLUI=ON`
- Panel layouts: `ui_resources/rml/*.rml` (HTML-like markup)
- Theme: `ui_resources/rcss/photon_ui.rcss` (CSS-like stylesheet)
- Custom elements: Circular gauges via C++ `Rml::Element` subclasses
- Data binding: Live game state via `{{ship.shield_pct}}` etc.
- Manager: `RmlUiManager` class (`include/ui/rml_ui_manager.h`)

**ImGui** (Secondary — debug/developer overlays):
- Current: Basic ImGui windows (retained for dev tools)
- Used for: Performance metrics, entity inspector, debug overlays

**Window Manager**:
```cpp
class WindowManager {
    std::vector<UIWindow*> windows;
    void update(float deltaTime);
    void render();
    void saveLayout(const std::string& name);
    void loadLayout(const std::string& name);
};
```

**Input System**:
```cpp
class InputManager {
    void handleKeyPress(int key, int mods);
    void handleMouseClick(int button, glm::vec2 pos);
    void handleMouseMove(glm::vec2 pos);
    Entity* pick3DEntity(glm::vec2 screenPos);
};
```

### Data Flow

```
Server Message
    ↓
EntityManager (update state)
    ↓
Overview Panel (update list)
    ↓
Target List (update health)
    ↓
HUD (update player ship)
    ↓
Render (draw all UI)
```

---

## Implementation Strategy

### Priorities

**Must Have** (Phases 4.4-4.6):
1. Input system with 3D picking
2. Overview panel
3. Core HUD with status rings
4. Module activation system

**Should Have** (Phases 4.7-4.8):
1. Context menus
2. Radial menu
3. Neocom menu
4. D-Scan window

**Nice to Have** (Phases 4.9-4.11):
1. Drone controls
2. Movement commands
3. Window customization
4. Advanced features

### Development Approach

**Incremental**: Build one panel at a time, fully functional before moving on.

**Testable**: Each component should be testable independently.

**Modular**: Clean interfaces between UI and game logic.

**Performant**: Target 60 FPS with full UI.

---

## Timeline Estimate

| Phase | Duration | Effort |
|-------|----------|--------|
| 4.4 - Input System | 2 weeks | 80 hours |
| 4.5 - Overview & HUD | 3 weeks | 120 hours |
| 4.6 - Modules | 2-3 weeks | 100 hours |
| 4.7 - Menus | 1-2 weeks | 60 hours |
| 4.8 - Neocom & Panels | 3 weeks | 120 hours |
| 4.9 - Drones & Movement | 1-2 weeks | 60 hours |
| 4.10 - Customization | 2 weeks | 80 hours |
| 4.11 - Polish | 2-3 weeks | 100 hours |
| **Total** | **16-21 weeks** | **720 hours** |

**Note**: This is a substantial undertaking. EVE Online's UI took CCP Games years to develop and refine.

---

## Success Criteria

### Must Meet
- ✅ Entity targeting via click or Ctrl+Click
- ✅ Overview panel with filtering
- ✅ HUD with circular status rings
- ✅ Module activation via F-keys
- ✅ Context menu on right-click
- ✅ Basic window management

### Should Meet
- ✅ Radial menu for quick actions
- ✅ Neocom with core services
- ✅ D-Scan functionality
- ✅ Inventory and fitting windows
- ✅ Window customization

### Nice to Have
- ✅ All keyboard shortcuts
- ✅ Complete drone controls
- ✅ Movement commands (Q/W/E)
- ✅ Layout save/load
- ✅ Map integration

---

## Risks & Mitigation

### Risk: Scope Too Large
**Mitigation**: Implement in phases, ship working product after each phase.

### Risk: Performance Issues
**Mitigation**: Profile early, optimize critical paths, consider C++ custom rendering.

### Risk: ImGui Limitations
**Mitigation**: Migrate game-facing UI to **RmlUi** (HTML/CSS-based framework).
ImGui is retained for debug/developer overlays only. See
`docs/design/UI_FRAMEWORK_EVALUATION.md` for the full evaluation. New RmlUi
integration is available via `-DUSE_RMLUI=ON` with panel templates in
`cpp_client/ui_resources/` and a Photon UI stylesheet in
`cpp_client/ui_resources/rcss/photon_ui.rcss`.

### Risk: Complexity
**Mitigation**: Follow EVE's design closely, don't reinvent the wheel.

---

## Conclusion

Implementing EVE Online's complete UI is a major undertaking requiring 4-6 months of focused development. The phased approach allows for incremental delivery of working features while building toward the complete system.

**Current Status**: Phase 4.5 Complete (HUD + Overview), Phase 4.6 Partial (Module Slots), Phase 4.8 Partial (D-Scan + Neocom + Inventory + Fitting RmlUi templates)  
**Next Priority**: Phase 4.7 (Context & Radial Menus), Phase 4.9 (Drone & Movement)  
**RmlUi Panels Completed**: Ship HUD, Overview, Fitting, Target List, Inventory, D-Scan, Neocom  
**Target Completion**: Q3 2026

---

**Author**: GitHub Copilot Workspace  
**Date**: February 5, 2026  
**Document Version**: 1.0
