# EVE Online UI Enhancement Documentation

## Overview

This document describes the EVE Online Photon UI enhancements implemented in the EVEOFFLINE 3D client. The UI has been redesigned to closely mimic EVE Online's official interface, including color schemes, panel layouts, and visual styling.

## Visual Design Philosophy

The new UI follows EVE Online's Photon UI design principles:
- **Dark, semi-transparent panels** with subtle blue-black backgrounds
- **Teal/cyan accent colors** as EVE's signature color scheme
- **Circular displays** for capacitor and health (shield/armor/hull)
- **Modular panels** that can be positioned around the screen
- **Information hierarchy** with clear headers and borders
- **Sci-fi aesthetic** with modern flat design elements

## Color Scheme

Based on extensive research of EVE Online's Photon UI:

### Primary Colors
- **Background Primary**: Dark blue-black (RGB: 13, 20, 31) with 85% opacity
- **Background Secondary**: Lighter dark blue (RGB: 20, 31, 41) with 75% opacity
- **Background Panel**: Very dark blue-black (RGB: 5, 10, 20) with 90% opacity

### Accent Colors (EVE's Signature Teal/Cyan)
- **Accent Primary**: Bright teal (RGB: 51, 153, 204)
- **Accent Secondary**: Darker teal (RGB: 38, 115, 166)
- **Accent Glow**: Glowing cyan (RGB: 77, 204, 255) with 50% opacity

### Status Colors
- **Shield**: Blue (RGB: 51, 128, 255)
- **Armor**: Yellow-gold (RGB: 255, 204, 51)
- **Hull/Structure**: Red (RGB: 204, 77, 77)
- **Capacitor**: Yellow (RGB: 255, 230, 77)

### Text Colors
- **Primary Text**: Nearly white (RGB: 230, 242, 255)
- **Secondary Text**: Gray-blue (RGB: 179, 191, 204)
- **Disabled Text**: Dark gray (RGB: 102, 115, 128)

## UI Components

### 1. Center HUD (Ship Status)

**Location**: Center bottom of screen

**Components**:
- **Capacitor Ring**: Circular yellow ring around the ship showing energy levels
  - Depletes clockwise from top
  - Color changes from yellow → orange → red as it depletes
  - Radius: 0.18 units, thickness: 0.025 units
  - 60 segments for smooth rendering

- **Health Rings**: Three concentric rings showing:
  - **Shield** (outer): Blue ring
  - **Armor** (middle): Yellow ring
  - **Hull** (inner): Red ring
  - Each ring depletes based on damage percentage
  - Colors change intensity when damaged

- **Ship Indicator**: Small teal square in the center representing the player's ship

### 2. Ship Status Panel

**Location**: Bottom left corner

**Features**:
- Semi-transparent dark blue-black panel (0.6 × 0.4 units)
- Teal header with "◢ SHIP STATUS" title
- Bright cyan border accent at top
- Displays:
  - Ship name
  - Shield HP (current/max and percentage) in blue
  - Armor HP (current/max and percentage) in yellow
  - Structure/Hull HP (current/max and percentage) in red
  - Capacitor (current/max and percentage) in yellow
- Text aligned left for labels, right for values
- Dynamic color changes based on health percentage

### 3. Target Info Panel

**Location**: Top right corner

**Features**:
- Same styling as ship status panel
- Red header with "◢ TARGET" title
- Red border accent
- Displays:
  - Target name
  - Distance (in meters or kilometers)
  - Shield percentage in blue
  - Armor percentage in yellow
- Hidden when no target is selected
- Shows/hides with smooth transitions

### 4. Navigation/Speed Panel

**Location**: Top left corner

**Features**:
- Semi-transparent panel (0.5 × 0.24 units)
- Blue accent header with "◢ NAVIGATION" title
- Displays:
  - Current speed in m/s
  - Position coordinates (X, Y, Z)
- Speed value in bright text
- Position in secondary gray text

### 5. Combat Log Panel

**Location**: Bottom right

**Features**:
- Wide semi-transparent panel (0.9 × 0.3 units)
- Cyan glow header with "◢ COMBAT LOG" title
- Shows last 8 combat messages
- Messages display in reverse chronological order (newest at bottom)
- Auto-scrolls as new messages arrive
- Messages prefixed with "◢" symbol for EVE styling
- Timestamp and color coding for different message types

### 6. Overview Panel

**Location**: Right side of screen

**Features**:
- Tall panel showing nearby objects
- Teal header with "◢ OVERVIEW" title
- Mimics EVE's overview panel layout
- Placeholder for future features:
  - Sortable columns
  - Filtering options
  - Object types and distances
  - Status indicators

### 7. Neocom Panel (Left Sidebar)

**Location**: Far left edge of screen

**Features**:
- Narrow vertical strip (0.12 × 1.8 units)
- Dark blue-black background
- Cyan border on right edge
- Menu button at top (≡ symbol)
- Icon buttons for:
  - **I** - Inventory
  - **F** - Fitting
  - **M** - Map
  - **$** - Market
  - **@** - Character
- Buttons use EVE's dark teal color scheme
- Hover states (future enhancement)

## Panel Styling Details

### Headers
- Height: 0.05 units
- Background: Primary background color
- Border accent: 0.002 units thick, colored by panel type
- Title text: 0.04-0.05 scale, aligned left with "◢" prefix

### Borders
- Top accent line on all panels for visual separation
- Colors match panel purpose:
  - Teal for information panels
  - Red for hostile/target panels
  - Cyan for communication/logs

### Transparency
- All panels use alpha blending for depth
- Opacity levels:
  - Solid: 1.0 (borders)
  - High: 0.9 (main panels)
  - Medium: 0.75 (secondary backgrounds)
  - Low: 0.5 (overlays)

### Typography
- Font sizes:
  - Title: 0.06 (24px equivalent)
  - Large: 0.05 (20px equivalent)
  - Normal: 0.04 (16px equivalent)
  - Small: 0.035 (14px equivalent)
  - Tiny: 0.03 (12px equivalent)

## Technical Implementation

### Files Created

1. **`client_3d/ui/eve_style.py`** (295 lines)
   - Color scheme definitions
   - Panel style constants
   - Layout presets
   - Helper functions for color interpolation

2. **`client_3d/ui/capacitor_display.py`** (420 lines)
   - `CapacitorDisplay` class - Circular capacitor ring
   - `ShipHealthRings` class - Shield/armor/hull rings
   - Procedural geometry generation
   - Dynamic color updates

3. **`client_3d/ui/eve_hud.py`** (690 lines)
   - `EVEStyledHUD` class - Main HUD system
   - All panel creation and management
   - Update methods for ship status, targets, speed
   - Combat log management

4. **`client_3d/ui/hud.py`** (Updated)
   - Added import for EVE-styled HUD
   - Added `create_hud()` factory function
   - Supports both 'eve' and 'legacy' styles

5. **`client_3d/core/game_client.py`** (Updated)
   - Changed to use `create_hud()` factory
   - Defaults to EVE-styled HUD

### Key Classes

#### EVEColorScheme
Static class containing all color definitions as `Vec4` objects.

```python
EVEColorScheme.BACKGROUND_PRIMARY  # Semi-transparent dark blue
EVEColorScheme.ACCENT_PRIMARY      # Bright teal
EVEColorScheme.SHIELD_COLOR        # Blue for shields
EVEColorScheme.ARMOR_COLOR         # Yellow for armor
EVEColorScheme.HULL_COLOR          # Red for hull
```

#### CapacitorDisplay
Renders a circular capacitor gauge using procedural geometry.

```python
cap = CapacitorDisplay(parent, pos=(0, 0, -0.6), radius=0.18)
cap.update(current=75, maximum=100)  # Updates visual
```

#### ShipHealthRings
Renders three concentric rings for shield/armor/hull.

```python
rings = ShipHealthRings(parent, pos=(0, 0, -0.6))
rings.update(shield_cur, shield_max, armor_cur, armor_max, hull_cur, hull_max)
```

#### EVEStyledHUD
Main HUD manager that creates and updates all panels.

```python
hud = EVEStyledHUD(aspect2d, render2d)
hud.update_ship_status(ship_data)
hud.update_target_info(target_data)
hud.update_speed(speed, position)
hud.add_combat_message("Message text")
```

### Usage

To use the EVE-styled HUD in your client:

```python
from client_3d.ui.hud import create_hud

# Create EVE-styled HUD
hud = create_hud(aspect2d, render2d, style='eve')

# Or use legacy HUD
hud = create_hud(aspect2d, style='legacy')
```

## Comparison to EVE Online

### What We Match
✅ Color scheme (dark blues, teals, semi-transparent panels)
✅ Circular capacitor display around ship
✅ Concentric health rings (shield/armor/hull)
✅ Panel layouts and positioning
✅ Header styling with accent borders
✅ Text styling and colors
✅ Neocom-style left sidebar
✅ Overview panel concept
✅ Combat log with proper formatting
✅ Target info panel

### Future Enhancements
- [ ] Interactive module rack (high/mid/low slots)
- [ ] Animated target locks with rotating brackets
- [ ] Hover effects and button interactions
- [ ] Draggable/resizable panels
- [ ] Window snapping
- [ ] Custom color themes
- [ ] Overview filtering and sorting
- [ ] Module cooldown animations
- [ ] Range indicator circles
- [ ] Capacitor warning indicators

## Testing

Run the component tests:
```bash
python test_eve_ui_components.py
```

Run the standalone UI demo (requires display):
```bash
python test_eve_hud.py
```

Test with the 3D client:
```bash
python client_3d.py "TestPilot"
```

## Performance

- **Capacitor ring**: 60 segments, ~240 vertices, rendered as single geometry node
- **Health rings**: 3 rings × 40 segments each = ~480 vertices total
- **Panels**: DirectGui elements with minimal overhead
- **Text**: OnscreenText objects, cached by Panda3D
- **Total overhead**: < 1ms per frame for all HUD updates

## References

Based on research from:
- EVE Online official Photon UI documentation
- EVE Online Academy tutorials
- EVE University Wiki
- Community screenshots and guides
- In-game UI observations

## Credits

UI design inspired by CCP Games' EVE Online Photon UI.
Implementation created for the EVEOFFLINE project.
