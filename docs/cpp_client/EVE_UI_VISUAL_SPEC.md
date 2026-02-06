# EVE Online Photon UI - Visual Specifications

**Date**: February 5, 2026  
**Based On**: EVE Online Photon UI (2024)  
**Purpose**: Visual reference for C++ client implementation

---

## Core Visual Design Principles

### Color Palette

**Primary Background**:
- `RGB(12, 18, 25)` - Very dark blue-black
- `RGB(15, 21, 28)` - Slightly lighter panels
- `RGB(18, 24, 31)` - Hover/active states

**Accent Colors**:
- `RGB(0, 204, 230)` - Teal/cyan (primary accent)
- `RGB(89, 195, 255)` - Light blue (call-to-action)
- `RGB(255, 165, 0)` - Orange (warnings)
- `RGB(255, 77, 77)` - Red (danger/hostile)

**Status Colors**:
- **Shield**: `RGB(51, 153, 255)` - Bright blue
- **Armor**: `RGB(255, 204, 51)` - Yellow/gold
- **Hull**: `RGB(230, 77, 77)` - Red
- **Capacitor**: `RGB(51, 204, 230)` - Cyan/blue

**Text Colors**:
- **Primary**: `RGB(230, 235, 240)` - Near white
- **Secondary**: `RGB(178, 185, 192)` - Grey
- **Disabled**: `RGB(102, 110, 117)` - Dark grey
- **Hostile**: `RGB(255, 77, 77)` - Red
- **Friendly**: `RGB(51, 204, 51)` - Green
- **Neutral**: `RGB(178, 185, 192)` - Grey

### Typography

**Fonts**:
- Primary: "EVE Sans Neue" (or fallback to "Arial", "Helvetica")
- Monospace: "Consolas", "Courier New" (for numbers)

**Sizes**:
- Header: 14-16pt
- Body: 11-12pt
- Small: 9-10pt
- Tiny: 8pt (for labels)

---

## Component Specifications

### 1. Neocom (Main Menu Bar)

**Position**: Top-left corner, vertical orientation  
**Width**: 56px (collapsed), 200px (expanded)  
**Height**: Full screen height

**Visual Design**:
- Semi-transparent background: `RGBA(12, 18, 25, 0.9)`
- Icons: 40x40px, white with 80% opacity
- Hover: Teal accent glow `RGB(0, 204, 230)`
- Active: Solid teal background
- Badge notifications: Orange `RGB(255, 165, 0)` with count

**Icon List** (top to bottom):
1. Character Sheet (person icon)
2. Inventory (box icon)
3. Ship Hangar (ship icon)
4. Fitting (wrench icon)
5. Market (graph icon)
6. Industry (factory icon)
7. Map (globe icon)
8. Corporation (group icon)
9. Settings (gear icon)

**Spacing**: 4px between icons

---

### 2. HUD (Ship Status Display)

**Position**: Bottom-center  
**Size**: 240x240px (circular area)

**Visual Design**:
- Background: Semi-transparent dark `RGBA(12, 18, 25, 0.85)`
- Ship icon: 60x60px in center
- Status rings (from center outward):
  1. **Capacitor**: Teal ring, 6px thick, 90px radius
  2. **Hull**: Red ring, 8px thick, 105px radius
  3. **Armor**: Yellow ring, 8px thick, 115px radius
  4. **Shield**: Blue ring, 8px thick, 125px radius

**Ring Rendering**:
- Arc length represents current/max percentage
- Smooth gradient for low values (<25%)
- Pulsing animation for critical (<10%)
- Gap at top (12 o'clock position, 10° gap)

**Text Display**:
- Current/Max values below rings
- Percentage on hover
- Monospace font for numbers

---

### 3. Module Slots

**Position**: Above HUD, three horizontal rows  
**Slot Size**: 48x48px per module

**Row Layout**:
- **High Slots**: Top row, up to 8 slots, red accent
- **Medium Slots**: Middle row, up to 8 slots, yellow accent
- **Low Slots**: Bottom row, up to 8 slots, blue accent

**Visual States**:
- **Empty**: Dark grey outline `RGB(51, 51, 51)`, dashed border
- **Inactive**: Module icon, 60% opacity, solid border
- **Active**: Module icon, 100% opacity, glowing border
- **Cooldown**: Circular overlay, teal `RGB(0, 204, 230)`
- **Overheated**: Orange/red glow `RGB(255, 165, 0)`

**Module Icon**:
- 40x40px icon centered in slot
- Charge indicator: Small circle bottom-right
- Ammunition count: White text, 9pt

**Cooldown Timer**:
- Circular arc overlay
- Starts at 12 o'clock, clockwise
- Teal color with 70% opacity
- Fades out on completion

---

### 4. Target List

**Position**: Top-center, horizontal layout  
**Icon Size**: 80x80px per target (default)

**Circular Target Icon**:
- **Background**: `RGBA(12, 18, 25, 0.9)`, radius 38px
- **Border**: Teal `RGB(0, 204, 230)`, 2px thick
- **Active Target**: Yellow border `RGB(255, 204, 51)`, 3px thick

**Health Arcs** (3 arcs, 120° each):
1. **Shield** (top, 12-4 o'clock): Blue `RGB(51, 153, 255)`
2. **Armor** (right, 4-8 o'clock): Yellow `RGB(255, 204, 51)`
3. **Hull** (left, 8-12 o'clock): Red `RGB(230, 77, 77)`

**Arc Rendering**:
- Thickness: 6px
- Radius: 36px from center
- Arc length = percentage of max
- Smooth rounded caps

**Center Content**:
- Ship type icon or text, 11pt
- Distance below, 9pt grey

**Locking Animation**:
- Circular progress ring
- Starts empty, fills clockwise
- Blue-white color
- 1-3 seconds duration
- Beep sound on lock complete

---

### 5. Overview Panel

**Position**: Top-right (default), resizable  
**Default Size**: 400x600px  
**Min Size**: 300x400px

**Header**:
- Height: 32px
- Background: `RGB(15, 21, 28)`
- Title: "Overview" (14pt, white)
- Tabs: Horizontal tabs below header (All, Hostile, Friendly, etc.)
- Close/minimize buttons: Top-right

**Tab Design**:
- Height: 28px
- Inactive: `RGB(18, 24, 31)`
- Active: `RGB(0, 204, 230)` underline (3px)
- Hover: Slight teal glow

**Column Headers**:
- Height: 24px
- Background: `RGB(18, 24, 31)`
- Sort indicator: Small arrow (▲/▼)
- Columns: Name (200px), Distance (80px), Type (100px)

**Row Design**:
- Height: 22px
- Even rows: `RGBA(12, 18, 25, 0.5)`
- Odd rows: `RGBA(15, 21, 28, 0.5)`
- Hover: `RGBA(0, 204, 230, 0.2)`
- Selected: `RGBA(0, 204, 230, 0.3)` with teal left border (3px)

**Color Coding**:
- Hostile NPC: `RGB(255, 77, 77)` - Red text
- Hostile Player: `RGB(255, 51, 51)` - Bright red text
- Friendly: `RGB(51, 153, 255)` - Blue text
- Corp/Alliance: `RGB(51, 204, 51)` - Green text
- Neutral: `RGB(178, 185, 192)` - Grey text

**Icons**:
- 16x16px ship type icons
- Position: Left of name
- State icons: Right side (locked, engaged, etc.)

---

### 6. Context Menu

**Visual Design**:
- Background: `RGBA(18, 24, 31, 0.95)`
- Border: 1px `RGB(0, 204, 230)`
- Drop shadow: 4px blur, `RGBA(0, 0, 0, 0.5)`
- Rounded corners: 4px

**Menu Items**:
- Height: 28px
- Padding: 8px horizontal
- Text: 11pt, white
- Hover: `RGBA(0, 204, 230, 0.2)` background
- Separator: 1px line `RGB(51, 51, 51)`

**Submenu Indicator**:
- Right arrow: `►`
- Opens on hover (250ms delay)

**Icons**:
- 16x16px on left side
- 4px margin from text

---

### 7. Radial Menu

**Visual Design**:
- Appears on hold left-click (500ms)
- Circular layout, 8 segments
- Radius: 100px
- Background: `RGBA(12, 18, 25, 0.8)`
- Borders: Teal `RGB(0, 204, 230)`, 2px

**Segments**:
- 45° each (8 total)
- Icon in center of segment (32x32px)
- Label below icon (9pt)
- Hover: Brighter teal fill `RGBA(0, 204, 230, 0.4)`

**Actions** (clockwise from top):
1. Orbit
2. Approach
3. Keep at Range
4. Lock Target
5. Look At
6. Warp To
7. Dock
8. Jump

---

### 8. D-Scan Window

**Position**: Movable, default right side  
**Size**: 350x500px

**Scan Controls**:
- Angle slider: 5° to 360°, teal track
- Range slider: 0.1 AU to 14.3 AU
- Scan button: Large, teal `RGB(0, 204, 230)`
- Hotkey label: "V" in grey (9pt)

**Results Table**:
- Similar to Overview panel
- Columns: Type, Name, Distance
- Auto-updates on scan

**Visual Cone** (optional):
- Semi-transparent cone in 3D space
- Blue-white color `RGBA(51, 153, 255, 0.2)`
- Points in camera direction

---

### 9. Window Management

**Title Bar**:
- Height: 28px
- Background: `RGB(15, 21, 28)`
- Drag handle: Entire bar
- Text: 12pt, centered or left-aligned

**Window Buttons** (right side):
- Size: 20x20px each
- Spacing: 2px between
- Hover: Teal highlight
- **Pin** (lock): 📌 icon
- **Minimize**: – icon
- **Close**: × icon

**Resizing**:
- Grab handles: 8px on all edges
- Corner resize: 12x12px
- Cursor changes on hover
- Min/max size constraints

**Window States**:
- **Active**: Full opacity
- **Inactive**: 80% opacity
- **Minimized**: Title bar only
- **Locked**: No drag/resize, pin icon highlighted

---

## Animation Guidelines

**Timing**:
- Fast: 150ms (hover, clicks)
- Medium: 300ms (window open/close)
- Slow: 500ms (complex transitions)
- Locking: 1-3 seconds (based on scan resolution)

**Easing**:
- UI elements: Ease-out cubic
- Health changes: Linear
- Alerts: Ease-in-out

**Frame Rate**:
- Target: 60 FPS
- Acceptable: 30 FPS (low-end)
- UI updates: 30 Hz minimum

---

## Responsive Scaling

**UI Scale Breakpoints**:
- 1920x1080: 100% (baseline)
- 1600x900: 90%
- 1366x768: 85%
- 2560x1440: 110%
- 3840x2160: 150%

**Anchoring**:
- **Neocom**: Top-left, full height
- **HUD**: Bottom-center, fixed position
- **Overview**: Top-right, resizable
- **Target List**: Top-center, horizontal
- **Module Slots**: Above HUD, centered

**Aspect Ratio Handling**:
- **16:9 (1920x1080)**: Baseline design
- **16:10 (1920x1200)**: Letterbox or expand vertical space
- **21:9 (2560x1080)**: Expand horizontal, keep vertical
- **4:3 (1024x768)**: Pillarbox sides

---

## Accessibility

**Colorblind Modes**:
- Protanopia: Red/green adjusted
- Deuteranopia: Green/red adjusted  
- Tritanopia: Blue/yellow adjusted

**High Contrast**:
- Increase border thickness to 3px
- Higher opacity for backgrounds (95%)
- Brighter text colors

**Font Scaling**:
- Small: 90%
- Medium: 100% (default)
- Large: 125%
- Extra Large: 150%

---

## Implementation Notes

### OpenGL/ImGui

**Custom Widgets Needed**:
1. Circular progress rings (capacitor, health)
2. Arc segments (health arcs on targets)
3. Radial menu
4. Rounded rectangles with borders

**Texture Atlas**:
- Module icons: 512x512px atlas
- Ship type icons: 256x256px atlas
- UI elements: 256x256px atlas

**Shaders**:
- UI shader with alpha blending
- Circular mask shader for rings
- Glow effect shader for highlights

### Performance

**Draw Calls**:
- Batch similar UI elements
- Use texture atlases
- Minimize state changes

**Memory**:
- Cache rendered text
- Reuse UI buffers
- Lazy load module icons

---

## References

- **Official**: EVE Online Support - Photon UI Tips
- **Wiki**: EVE University Wiki - Interface Guide
- **Videos**: YouTube - Photon UI Setup Guides
- **Forums**: EVE Online Forums - Photon UI Feedback

---

**Last Updated**: February 5, 2026  
**Accuracy**: Based on 2024 Photon UI  
**Status**: Reference for Implementation
