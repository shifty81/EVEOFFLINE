# EVE Online Photon UI - Complete Implementation Roadmap

**Date**: February 5, 2026  
**Project**: EVE OFFLINE C++ Client  
**Goal**: Fully replicate EVE Online's Photon UI system

---

## Overview

This document outlines the comprehensive plan to implement EVE Online's complete Photon UI system in the C++ OpenGL client. The UI is the most complex aspect of EVE Online, featuring dense information displays, extensive customization, and sophisticated interaction patterns.

## Current Status (Phase 4.5 In Progress)

✅ **Completed**:
- Phase 4.3: Entity rendering system
- Phase 4.3: Circular target icons with arc-based health indicators
- Phase 4.3: Basic EVE color scheme
- Phase 4.3: Entity visual management
- Phase 4.3: ImGui integration
- Phase 4.4: Input system with 3D picking (entity selection, targeting)
- Phase 4.5: Overview panel with sorting, filtering, and interactions

⏳ **In Progress**: 
- Phase 4.5: HUD enhancements (ship status rings, warning indicators)

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

#### Context Menu System
- [ ] **Universal Right-Click**
  - Works on: Space entities, overview items, inventory
  - Hierarchical menu structure
  - Dynamic options based on context
  
- [ ] **Entity Actions**
  - Look At
  - Approach
  - Orbit
  - Keep at Range
  - Warp To
  - Jump Through (gates)
  - Dock (stations)
  
- [ ] **Target Actions**
  - Lock Target
  - Unlock Target
  - Set as Active Target
  
- [ ] **Information Actions**
  - Show Info
  - Show Location
  - Create Bookmark
  
- [ ] **Fleet Actions** (if in fleet)
  - Warp to Member
  - Set as Squad Commander
  - Broadcast actions

#### Radial Menu
- [ ] **Activation**
  - Hold left-click on entity/space
  - Circular menu appears
  - Drag to select option
  
- [ ] **Quick Actions**
  - Orbit (at different ranges)
  - Approach
  - Lock Target
  - Keep at Range
  - Warp To
  
- [ ] **Visual Design**
  - Circular segments
  - Icons for each action
  - Hover highlighting
  - Smooth animations

**Deliverables**:
- Context menu system
- Radial menu implementation
- Action execution framework

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

#### Drone Interface
- [ ] **Drone Bay Window**
  - Available drones list
  - Drone in space list
  - Bandwidth bar (used/max)
  
- [ ] **Drone Controls**
  - Launch drones (Shift+F)
  - Return drones (Shift+R)
  - Engage target (F)
  - Return and orbit (not engaging)
  
- [ ] **Visual Indicators**
  - Drones in space icons
  - Health bars
  - Engagement status
  - Low shield/armor warnings

#### Movement Commands
- [ ] **Hold-Key + Click**
  - Q+Click: Approach
  - W+Click: Orbit
  - E+Click: Keep at Range
  - D+Click: Dock/Jump
  
- [ ] **Range Selection**
  - Default ranges (500m, 1km, 5km, etc.)
  - Custom range input
  - Saved preferences
  
- [ ] **Visual Feedback**
  - Flight path indicator
  - Range circle preview
  - ETA display

**Deliverables**:
- Drone control system
- Movement command system
- Visual flight indicators

---

### Phase 4.10: Window Management & Customization (2 weeks)

**Priority**: Medium-Low - Polish & UX

#### Window System
- [ ] **Movement & Resizing**
  - Drag title bar to move
  - Drag edges/corners to resize
  - Snap to screen edges
  - Snap to other windows
  
- [ ] **Window States**
  - Pin/unpin (lock position)
  - Minimize to title bar
  - Close window
  - Restore defaults
  
- [ ] **Transparency**
  - Opacity slider (per window)
  - Global opacity hotkey
  - Window blur effect
  - Inactive window dimming

#### Layouts & Presets
- [ ] **Save Layouts**
  - Multiple layout slots
  - Quick switch hotkeys
  - Export/import layouts
  
- [ ] **Reset Options**
  - Reset single window
  - Reset all windows
  - Reset to defaults

#### Customization Options
- [ ] **Color Schemes**
  - Photon UI (default)
  - Classic EVE
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
- [ ] Local chat
- [ ] Private conversations
- [ ] Fleet chat
- [ ] Corporation chat
- [ ] Chat window management

#### Notifications
- [ ] Toast notifications
- [ ] Warning messages
- [ ] Combat notifications
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
