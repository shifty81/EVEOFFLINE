# Phase 5 UI/UX Enhancements Documentation

**Date**: February 3, 2026  
**Version**: Phase 5 UI/UX Complete  
**Status**: ✅ All major UI components implemented

---

## Overview

This document describes the new UI/UX enhancements added to the EVE OFFLINE 3D client in Phase 5. These enhancements provide EVE-styled interface panels for inventory management, ship fitting, market trading, station services, tactical overlay, and enhanced targeting.

---

## New UI Components

### 1. Base Panel System

**Files**:
- `client_3d/ui/base_panel.py`

**Classes**:
- `EVEPanel` - Base class for all UI panels
- `EVEListPanel` - Panel with scrollable list functionality

**Features**:
- **EVE Online Photon UI Styling**: Dark blue-black backgrounds, teal accents
- **Draggable Windows**: Click and drag title bar to reposition
- **Closeable**: Optional close button with callback support
- **Transparent Frames**: Semi-transparent backgrounds for immersion
- **Scrollable Lists**: Built-in scrolling for long item lists

**Usage**:
```python
from client_3d.ui.base_panel import EVEPanel, EVEListPanel

# Create basic panel
panel = EVEPanel(
    parent=aspect2d,
    title="My Panel",
    pos=(0.2, 0.3),
    size=(0.5, 0.6),
    closeable=True,
    draggable=True
)

# Create list panel
list_panel = EVEListPanel(
    parent=aspect2d,
    title="My List",
    pos=(0.1, 0.2),
    size=(0.4, 0.7)
)

# Add items to list
list_panel.add_list_item("Item 1", data={"id": 1})
list_panel.add_list_item("Item 2", data={"id": 2})
```

---

### 2. Inventory Panel

**File**: `client_3d/ui/inventory_panel.py`

**Class**: `InventoryPanel`

**Features**:
- **Two View Modes**: Cargo hold and station hangar
- **Capacity Display**: Shows used/max capacity with color coding
- **Item List**: Scrollable list of items with quantities
- **Transfer Function**: Move items between cargo and hangar
- **Jettison Function**: Drop items into space
- **Callback Support**: Connect to game logic

**Usage**:
```python
from client_3d.ui.inventory_panel import InventoryPanel

# Create inventory panel
inventory = InventoryPanel(
    parent=aspect2d,
    pos=(0.1, 0.3),
    size=(0.6, 0.7)
)

# Update inventory display
inventory.update_inventory(
    items={"Tritanium": 1000, "Pyerite": 500},
    capacity_used=25.5,
    capacity_max=100.0
)

# Set callbacks
inventory.set_transfer_callback(on_transfer_item)
inventory.set_jettison_callback(on_jettison_item)
```

---

### 3. Fitting Window

**File**: `client_3d/ui/fitting_window.py`

**Class**: `FittingWindow`

**Features**:
- **Ship Information**: Display ship name and type
- **Resource Bars**: CPU and Powergrid usage with color coding
- **Slot Displays**: High/Mid/Low/Rig slots with module names
- **Module Management**: Fit and unfit modules
- **Overload Warning**: Red color when CPU/PG exceed limits

**Usage**:
```python
from client_3d.ui.fitting_window import FittingWindow

# Create fitting window
fitting = FittingWindow(
    parent=aspect2d,
    pos=(0.2, 0.2),
    size=(0.8, 0.8)
)

# Update ship info
fitting.update_ship_info("Rifter", "Frigate")

# Update resources
fitting.update_resources(
    cpu_used=45.5,
    cpu_max=100.0,
    pg_used=32.8,
    pg_max=50.0
)

# Update fitted modules
fitting.update_slots(
    high=["200mm AutoCannon I", "200mm AutoCannon I", None],
    mid=["1MN Afterburner I", "Stasis Webifier I", None],
    low=["Damage Control I", None, None],
    rig=[None, None, None]
)
```

---

### 4. Market Window

**File**: `client_3d/ui/market_window.py`

**Class**: `MarketWindow`

**Features**:
- **Search Bar**: Find items by name
- **Order Type Tabs**: Switch between buy and sell orders
- **Location Display**: Shows current market location
- **Order List**: Scrollable list of market orders
- **Quick Buy/Sell**: Instant market transactions
- **Place Order**: Create buy/sell orders at custom prices

**Usage**:
```python
from client_3d.ui.market_window import MarketWindow

# Create market window
market = MarketWindow(
    parent=aspect2d,
    pos=(0.15, 0.25),
    size=(0.7, 0.8)
)

# Update orders
market.update_orders([
    {"item_id": "Tritanium", "price": 5.50, "quantity": 10000, "location": "Jita"},
    {"item_id": "Pyerite", "price": 8.25, "quantity": 5000, "location": "Jita"}
])

# Set callbacks
market.set_buy_callback(on_buy)
market.set_sell_callback(on_sell)
market.set_place_order_callback(on_place_order)
```

---

### 5. Station Services Window

**File**: `client_3d/ui/station_services.py`

**Class**: `StationServicesWindow`

**Features**:
- **Station Information**: Display station name and type
- **Five Services**:
  - Repair Ship (green)
  - Fitting Service (blue)
  - Reprocessing (orange)
  - Manufacturing (purple)
  - Research (cyan)
- **Service Descriptions**: Help text for each service
- **Enable/Disable**: Control which services are available

**Usage**:
```python
from client_3d.ui.station_services import StationServicesWindow

# Create station services window
services = StationServicesWindow(
    parent=aspect2d,
    pos=(0.3, 0.3),
    size=(0.5, 0.6)
)

# Update station info
services.update_station_info("Jita IV - Moon 4", "Caldari Navy Station")

# Set callbacks
services.set_repair_callback(on_repair)
services.set_refit_callback(on_refit)
services.set_reprocess_callback(on_reprocess)
services.set_manufacturing_callback(on_manufacturing)
services.set_research_callback(on_research)

# Enable/disable specific services
services.enable_service('manufacturing', False)  # Disable manufacturing
```

---

### 6. Minimap Radar

**File**: `client_3d/ui/minimap_radar.py`

**Class**: `MinimapRadar`

**Features**:
- **Tactical Overlay**: Top-down view of nearby space
- **Range Rings**: 25%, 50%, 75%, 100% distance indicators
- **Entity Markers**: Show ships, NPCs, structures
- **Faction Colors**:
  - Green: Friendly
  - Red: Hostile
  - Gray: Neutral
  - Blue: Ally
- **Center Crosshair**: Player position marker
- **Configurable Range**: Adjust radar range

**Usage**:
```python
from client_3d.ui.minimap_radar import MinimapRadar
from panda3d.core import Vec3

# Create minimap
minimap = MinimapRadar(
    parent=aspect2d,
    pos=(0.7, 0.5),
    size=0.25,
    max_range=500.0
)

# Update player position
minimap.update_player_position(Vec3(1000, 2000, 0))

# Add entities
minimap.add_entity(
    entity_id="npc_001",
    pos=Vec3(1100, 2050, 0),
    entity_type="npc",
    faction="hostile"
)

# Update display
minimap.update_display()

# Change range
minimap.set_range(1000.0)
```

---

### 7. Enhanced Targeting Interface

**File**: `client_3d/ui/targeting_interface.py`

**Classes**:
- `TargetLockDisplay` - Individual target slot
- `EnhancedTargetingInterface` - Manages multiple targets

**Features**:
- **Multiple Target Slots**: Up to 5 simultaneous targets
- **Target Information**: Name, type, distance
- **Health Bars**: Shield (blue), Armor (yellow), Hull (red)
- **Unlock Button**: Quick target unlock
- **Color-Coded Borders**: Active target highlighting

**Usage**:
```python
from client_3d.ui.targeting_interface import EnhancedTargetingInterface

# Create targeting interface
targeting = EnhancedTargetingInterface(
    parent=aspect2d,
    max_targets=5,
    pos=(0.68, 0.8)
)

# Update target in slot 0
targeting.update_target(
    slot_index=0,
    target_id="npc_001",
    name="Guristas Pithum Killer",
    target_type="Frigate",
    distance=12.5,
    shield_hp=500,
    shield_max=1000,
    armor_hp=800,
    armor_max=800,
    hull_hp=600,
    hull_max=600
)

# Clear target from slot
targeting.clear_target(0)

# Set unlock callback
targeting.set_unlock_callback(on_unlock_target)
```

---

## Integration Guide

### Adding UI Panels to Game Client

To integrate these panels into your 3D game client:

```python
from direct.showbase.ShowBase import ShowBase
from client_3d.ui import (
    InventoryPanel, FittingWindow, MarketWindow,
    StationServicesWindow, MinimapRadar, EnhancedTargetingInterface
)

class GameClient3D(ShowBase):
    def __init__(self):
        ShowBase.__init__(self)
        
        # Create UI panels
        self.inventory_panel = InventoryPanel(self.aspect2d)
        self.fitting_window = FittingWindow(self.aspect2d)
        self.market_window = MarketWindow(self.aspect2d)
        self.station_services = StationServicesWindow(self.aspect2d)
        self.minimap = MinimapRadar(self.aspect2d)
        self.targeting = EnhancedTargetingInterface(self.aspect2d)
        
        # Hide panels initially
        self.inventory_panel.hide()
        self.fitting_window.hide()
        self.market_window.hide()
        self.station_services.hide()
        
        # Setup keyboard shortcuts
        self.accept("i", self.inventory_panel.toggle)
        self.accept("f", self.fitting_window.toggle)
        self.accept("m", self.market_window.toggle)
        self.accept("s", self.station_services.toggle)
        self.accept("t", self.minimap.toggle)
```

### Keyboard Shortcuts (Suggested)

- `I` - Toggle Inventory Panel
- `F` - Toggle Fitting Window
- `M` - Toggle Market Window
- `S` - Toggle Station Services
- `T` - Toggle Tactical Overlay (Minimap)
- `ESC` - Close all panels

---

## Technical Details

### EVE Online Photon UI Color Scheme

All panels use the EVE Online color scheme:

```python
# Backgrounds
BACKGROUND_PRIMARY = (0.05, 0.08, 0.12, 0.85)    # Dark blue-black
BACKGROUND_SECONDARY = (0.08, 0.12, 0.16, 0.75)  # Lighter variant

# Accents
ACCENT_PRIMARY = (0.2, 0.6, 0.8, 1.0)            # Bright teal
BORDER_NORMAL = (0.2, 0.3, 0.4, 0.6)             # Subtle blue-gray

# Text
TEXT_PRIMARY = (0.9, 0.95, 1.0, 1.0)             # Nearly white
TEXT_SECONDARY = (0.7, 0.75, 0.8, 1.0)           # Gray-blue

# Health Colors
SHIELD_COLOR = (0.2, 0.5, 1.0, 1.0)              # Blue
ARMOR_COLOR = (1.0, 0.8, 0.2, 1.0)               # Yellow
HULL_COLOR = (0.8, 0.3, 0.3, 1.0)                # Red
```

### Panel Positioning

All positions are in normalized screen coordinates:
- X: -1.0 (left) to 1.0 (right)
- Y: -1.0 (bottom) to 1.0 (top)
- Origin (0, 0) is center of screen

### Performance Considerations

- **Panel Hiding**: Hidden panels don't update, saving performance
- **Lazy Updates**: Only update displays when data changes
- **List Scrolling**: Efficiently handles hundreds of items
- **Minimap Updates**: Update only when entities move significantly

---

## Testing

### Syntax Validation

All UI components have been syntax-checked:

```bash
python -m py_compile client_3d/ui/base_panel.py
python -m py_compile client_3d/ui/inventory_panel.py
python -m py_compile client_3d/ui/fitting_window.py
python -m py_compile client_3d/ui/market_window.py
python -m py_compile client_3d/ui/station_services.py
python -m py_compile client_3d/ui/minimap_radar.py
python -m py_compile client_3d/ui/targeting_interface.py
```

### Unit Tests

Run the UI component tests:

```bash
python test_ui_panels.py
```

**Note**: Full runtime testing requires Panda3D to be installed.

---

## Future Enhancements

Possible future improvements:

1. **Interactive UI Elements**:
   - Drag-and-drop module fitting
   - Right-click context menus
   - Multi-select in lists

2. **Additional Panels**:
   - Character Sheet (skills, attributes)
   - Fleet Window (fleet management)
   - Probe Scanner (exploration)
   - Ship Tree (show/hide modules)
   - Drone Bay (drone control)

3. **Visual Polish**:
   - Smooth panel animations
   - Glowing borders on active panels
   - Progress bars for actions
   - Notification toasts

4. **Minimap Enhancements**:
   - Rotate with player ship
   - Show warp destinations
   - Distance circles
   - Entity filtering

5. **Targeting Enhancements**:
   - Drag-to-reorder targets
   - Target priority indicators
   - Range circles
   - Weapon range indicators

---

## Troubleshooting

### Panel Not Showing
- Ensure parent node is valid
- Check if panel is hidden (`panel.show()`)
- Verify position is on-screen

### Callbacks Not Working
- Make sure callbacks are set before triggering actions
- Check callback function signature matches expected parameters
- Verify game logic is connected

### Performance Issues
- Hide unused panels
- Limit update frequency for minimap
- Reduce max_targets if needed

---

## API Reference

### EVEPanel

```python
EVEPanel(parent, title, pos, size, closeable, draggable)
```

**Methods**:
- `show()` - Show the panel
- `hide()` - Hide the panel
- `toggle()` - Toggle visibility
- `close()` - Close the panel (calls on_close_callback)
- `destroy()` - Destroy and clean up
- `set_on_close_callback(callback)` - Set close callback

### EVEListPanel

Inherits from EVEPanel.

**Methods**:
- `add_list_item(text, data)` - Add item to list
- `clear_list()` - Clear all items

### InventoryPanel

**Methods**:
- `update_inventory(items, capacity_used, capacity_max)`
- `set_transfer_callback(callback)`
- `set_jettison_callback(callback)`

### FittingWindow

**Methods**:
- `update_ship_info(ship_name, ship_type)`
- `update_resources(cpu_used, cpu_max, pg_used, pg_max)`
- `update_slots(high, mid, low, rig)`

### MarketWindow

**Methods**:
- `update_orders(orders)`
- `set_buy_callback(callback)`
- `set_sell_callback(callback)`
- `set_place_order_callback(callback)`

### StationServicesWindow

**Methods**:
- `update_station_info(station_name, station_type)`
- `enable_service(service_name, enabled)`
- `set_repair_callback(callback)`
- `set_refit_callback(callback)`
- `set_reprocess_callback(callback)`
- `set_manufacturing_callback(callback)`
- `set_research_callback(callback)`

### MinimapRadar

**Methods**:
- `update_player_position(pos, rotation)`
- `add_entity(entity_id, pos, entity_type, faction)`
- `remove_entity(entity_id)`
- `clear_entities()`
- `update_display()`
- `set_range(new_range)`

### EnhancedTargetingInterface

**Methods**:
- `update_target(slot_index, target_id, name, target_type, distance, shield_hp, shield_max, armor_hp, armor_max, hull_hp, hull_max)`
- `clear_target(slot_index)`
- `clear_all_targets()`
- `set_unlock_callback(callback)`

---

## Credits

**Implementation**: Phase 5 UI/UX Enhancement Session  
**Date**: February 3, 2026  
**Inspired by**: EVE Online Photon UI

---

## Changelog

### Version 1.0 (February 3, 2026)
- Initial implementation of all UI components
- Base panel system with EVE styling
- Inventory, Fitting, Market, Station Services panels
- Minimap/Radar tactical overlay
- Enhanced targeting interface
- Complete API documentation

---

**For additional help, see [docs/README.md](../README.md) for complete documentation index.**
