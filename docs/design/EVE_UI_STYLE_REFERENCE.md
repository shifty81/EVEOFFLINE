# EVE Online UI Style Reference

This document serves as the visual design reference for making EVE OFFLINE's
UI closely mimic EVE Online's **Photon UI** (the current default interface).
All UI code in `cpp_client/src/ui/` and `client_3d/ui/` should follow these
guidelines.

---

## 1. Overall Design Language

EVE Online's Photon UI follows a **dark, semi-transparent, futuristic** visual
language designed for space gameplay:

- **Dark backgrounds** with subtle blue/teal tint — never pure black
- **Semi-transparent panels** (75-90% opacity) so the space scene shows through
- **Thin glowing borders** in teal/cyan that subtly highlight panel edges
- **Flat, minimalist controls** — no heavy 3D bevels or gradients
- **Consistent header bars** across all windows with title + close/minimize buttons
- **Compact layout** — EVE panels are dense with information, not spacious

## 2. Color Palette

### 2.1 Background Colors

| Usage | Hex | RGB (0-255) | Float (0-1) |
|-------|-----|-------------|-------------|
| Window BG | `#0D1117` | 13, 17, 23 | 0.051, 0.067, 0.090 |
| Panel BG (darker) | `#080C12` | 8, 12, 18 | 0.031, 0.047, 0.071 |
| Secondary BG / Row alt | `#161B22` | 22, 27, 34 | 0.086, 0.106, 0.133 |
| Header / Title bar | `#0A0E14` | 10, 14, 20 | 0.039, 0.055, 0.078 |
| Popup / Tooltip BG | `#1C2128` | 28, 33, 40 | 0.110, 0.129, 0.157 |

### 2.2 Accent / Highlight Colors (Teal/Cyan theme)

| Usage | Hex | RGB | Float |
|-------|-----|-----|-------|
| Primary Accent (teal) | `#45D0E8` | 69, 208, 232 | 0.271, 0.816, 0.910 |
| Secondary Accent | `#78E1F0` | 120, 225, 240 | 0.471, 0.882, 0.941 |
| Dim Accent (borders) | `#2A5A6A` | 42, 90, 106 | 0.165, 0.353, 0.416 |
| Selection / Hover | `#1A3A4A` | 26, 58, 74 | 0.102, 0.227, 0.290 |
| Active / Pressed | `#45D0E8` | 69, 208, 232 | 0.271, 0.816, 0.910 |

### 2.3 Border Colors

| Usage | Hex | RGB | Float |
|-------|-----|-----|-------|
| Normal border | `#283848` | 40, 56, 72 | 0.157, 0.220, 0.282 |
| Highlighted border | `#45D0E8` | 69, 208, 232 | 0.271, 0.816, 0.910 |
| Subtle separator | `#1E2A36` | 30, 42, 54 | 0.118, 0.165, 0.212 |

### 2.4 Text Colors

| Usage | Hex | RGB | Float |
|-------|-----|-----|-------|
| Primary text | `#E6EDF3` | 230, 237, 243 | 0.902, 0.929, 0.953 |
| Secondary / dimmed | `#8B949E` | 139, 148, 158 | 0.545, 0.580, 0.620 |
| Disabled | `#484F58` | 72, 79, 88 | 0.282, 0.310, 0.345 |
| Header / Label | `#45D0E8` | 69, 208, 232 | 0.271, 0.816, 0.910 |

### 2.5 Status / Game Colors

| Usage | Hex | RGB | Float |
|-------|-----|-----|-------|
| Shield (blue) | `#3399FF` | 51, 153, 255 | 0.200, 0.600, 1.000 |
| Armor (gold) | `#FFD040` | 255, 208, 64 | 1.000, 0.816, 0.251 |
| Hull (red) | `#E64545` | 230, 69, 69 | 0.902, 0.271, 0.271 |
| Capacitor (yellow) | `#FFE066` | 255, 224, 102 | 1.000, 0.878, 0.400 |
| Hostile (red) | `#CC3333` | 204, 51, 51 | 0.800, 0.200, 0.200 |
| Friendly (blue) | `#3399FF` | 51, 153, 255 | 0.200, 0.600, 1.000 |
| Neutral (grey) | `#AAAAAA` | 170, 170, 170 | 0.667, 0.667, 0.667 |
| Corp member (purple) | `#AA66FF` | 170, 102, 255 | 0.667, 0.400, 1.000 |
| Fleet member (green) | `#66CC66` | 102, 204, 102 | 0.400, 0.800, 0.400 |
| Success | `#33CC66` | 51, 204, 102 | 0.200, 0.800, 0.400 |
| Warning | `#FFB833` | 255, 184, 51 | 1.000, 0.722, 0.200 |
| Error / Danger | `#FF3333` | 255, 51, 51 | 1.000, 0.200, 0.200 |

## 3. Typography

- **Primary font**: Sans-serif (the project uses ImGui's default which is close)
- **Headers**: Uppercase, accent-colored, with a separator line below
- **Labels**: Secondary (dimmed) text color, smaller scale
- **Values**: Primary text color, normal or slightly larger scale
- Panel titles should be **UPPERCASE** (e.g., "SHIP STATUS", "OVERVIEW")

## 4. Window / Panel Anatomy

```
┌─────────────────────────────────────────┐  ← Thin border (BORDER_NORMAL)
│ ■ PANEL TITLE                     _ □ × │  ← Header bar (HEADER_BG)
├─────────────────────────────────────────┤  ← Separator line
│                                         │
│   Panel content area                    │  ← Window BG (semi-transparent)
│                                         │
│   ┌───────────────────────────────┐     │
│   │ Sub-section or table          │     │  ← Secondary BG for nested areas
│   └───────────────────────────────┘     │
│                                         │
└─────────────────────────────────────────┘
```

### Key style values (ImGui):
- **Window rounding**: 2.0 (EVE uses very subtle rounding, nearly square)
- **Frame rounding**: 1.0
- **Window border**: 1.0px
- **Frame border**: 0.0px (frames use color distinction, not borders)
- **Window padding**: 8×8 (compact)
- **Frame padding**: 6×3 (tight)
- **Item spacing**: 6×4
- **Scrollbar size**: 10px (slim)

## 5. Key UI Elements

### 5.1 Health Bars (Shield / Armor / Hull)
- Thin horizontal bars (4-6px tall) stacked vertically
- Color-coded: blue (shield), gold (armor), red (hull)
- Show percentage fill; dim portion shows empty space in dark grey
- Numeric value displayed to the right of the bar

### 5.2 Overview Table
- Sortable columns: Name, Type, Distance, Corp/Alliance, Standing
- Standing indicator uses small colored icon/dot (red=hostile, blue=friendly, grey=neutral)
- Row background alternates between two dark shades
- Selected row highlighted with `SELECTION` color
- Compact row height — no wasted vertical space

### 5.3 Target Icons (Locked Targets)
- Circular icons arranged horizontally at the top of the screen
- Three concentric health arcs: shield (outer, blue), armor (middle, gold), hull (inner, red)
- Active target has a brighter/yellow border; others have teal border
- Ship type text in the center of the circle
- Distance shown below

### 5.4 Capacitor Display
- Circular gauge (ring) rather than a bar in EVE Online
- Color shifts from yellow → orange → red as capacitor depletes
- Percentage displayed in center

### 5.5 Speed Display
- Current speed in large text (accent color)
- Small "max speed" label below
- Optional speed gauge bar

### 5.6 Module Rack
- Row of module icons at bottom-center of screen
- Green border = online/ready, Red pulse = overheated
- Cycling animation when active
- Grouped by rack: High slots, Mid slots, Low slots

## 6. Neocom Bar (Left Sidebar)

EVE's Neocom is a vertical toolbar on the left edge:
- Dark, narrow vertical strip
- Small square icons for each system (Inventory, Fitting, Market, etc.)
- Tooltip on hover showing the system name
- Highlight glow on hover/active icon

## 7. Implementation Notes

### RmlUi (Primary — game-facing panels)
- `cpp_client/ui_resources/rcss/photon_ui.rcss` — Complete Photon UI theme
- `cpp_client/ui_resources/rml/ship_hud.rml` — HUD with circular gauges
- `cpp_client/ui_resources/rml/overview.rml` — Overview table panel
- `cpp_client/ui_resources/rml/fitting.rml` — Fitting window
- `cpp_client/include/ui/rml_ui_manager.h` — RmlUi manager class
- `cpp_client/src/ui/rml_ui_manager.cpp` — OpenGL render backend

### ImGui (Secondary — debug overlays)
- `cpp_client/include/ui/ui_manager.h` — `EVEColors` struct holds all color constants
- `cpp_client/src/ui/ui_manager.cpp` — `SetupEVEStyle()` applies the ImGui theme
- `cpp_client/src/ui/eve_panels.cpp` — Panel rendering with styled health bars, headers
- `cpp_client/src/ui/eve_target_list.cpp` — Target icon rendering
- `cpp_client/src/ui/overview_panel.cpp` — Overview table

### Where to Apply (Python / Panda3D)
- `client_3d/ui/` — UI panel implementations
- Use the same color values from this reference

### Key Principles
1. **Consistency**: Every panel should use the same color palette and spacing
2. **Semi-transparency**: Window backgrounds should be ~85-95% opaque
3. **Accent glow**: Interactive elements glow teal on hover
4. **Information density**: EVE's UI is designed for at-a-glance information
5. **Headers are uppercase**: Panel titles always use `UPPER CASE`

---

*References:*
- EVE Online Photon UI — https://www.eveonline.com/news/view/improving-photon-ui
- EVE University Wiki — https://wiki.eveuniversity.org/Category:User_Interface
- Community color themes — https://forums.eveonline.com/t/share-your-custom-colour-themes/435156
