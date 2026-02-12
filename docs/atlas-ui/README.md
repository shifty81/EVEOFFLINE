# Atlas UI Framework

A custom, immediate-mode, GPU-accelerated UI toolkit designed for sci-fi game interfaces — and built to be reusable in any C++ / OpenGL project.

```
 ┌─────────────────────────────────────────────────────────────────┐
 │  Atlas UI — Immediate-Mode Sci-Fi Interface Toolkit             │
 │                                                                 │
 │  ┌── Panel ─────────────┐   ┌── Ship HUD ──────────┐          │
 │  │ ▓ Header with title  │   │  Shield ████░░░  78%  │          │
 │  │ ───────────────────  │   │  Armor  ██████░  95%  │          │
 │  │  Row 1: Frigate      │   │  Hull   ████████ 100% │          │
 │  │  Row 2: Cruiser      │   │                       │          │
 │  │  Row 3: Station      │   │  ◎ Capacitor ring     │          │
 │  │  [Scrollbar]         │   │  [F1][F2][F3][F4]     │          │
 │  └──────────────────────┘   │  Speed: 142 m/s       │          │
 │                              └───────────────────────┘          │
 │  [Button]  ☑ Checkbox  ▼ ComboBox  ═══●══ Slider              │
 └─────────────────────────────────────────────────────────────────┘
```

## Overview

Atlas UI is the custom UI framework built for the Atlas space simulator. It provides:

- **Immediate-mode API** — no widget trees to maintain; call widget functions each frame
- **Single draw-call batching** — the renderer accumulates all geometry into one GPU draw
- **EVE-style widget library** — panels, status arcs, capacitor rings, module racks, overview tables, target cards
- **Full interactivity** — click, hover, drag-to-move panels, tab switching, scrolling, text input
- **Themeable** — complete color scheme customization via the `Theme` struct
- **Portable** — only requires OpenGL 3.3; no other dependencies

## Architecture

```
atlas_types.h       Core types (Vec2, Rect, Color, Theme, InputState, PanelState, WidgetID)
atlas_context.h/cpp Frame-level state: hot/active tracking, ID stack, input snapshot
atlas_renderer.h/cpp  OpenGL batch renderer (rects, lines, arcs, text, circles)
atlas_widgets.h/cpp   Widget library (panel, button, progress bar, overview row, etc.)
atlas_hud.h/cpp       Full HUD compositor (assembles widgets into EVE-style layout)
```

### Frame Lifecycle

```cpp
// 1. Create context (once)
atlas::AtlasContext ctx;
ctx.init();

// 2. Each frame:
atlas::InputState input;
input.windowW = windowWidth;
input.windowH = windowHeight;
input.mousePos = {mouseX, mouseY};
input.mouseDown[0] = leftButtonDown;
input.mouseClicked[0] = leftButtonJustPressed;
input.mouseReleased[0] = leftButtonJustReleased;

ctx.beginFrame(input);

// ... draw widgets here ...

ctx.endFrame();  // flushes draw commands to GPU
```

## Widget Reference

### Panel

Dark translucent rectangle with optional header, close/minimize buttons, and border.

```cpp
atlas::Rect bounds = {100, 100, 300, 200};
atlas::PanelFlags flags;
flags.showHeader = true;
flags.showClose = true;
flags.showMinimize = true;
flags.drawBorder = true;

bool open = true;
if (atlas::panelBegin(ctx, "My Panel", bounds, flags, &open)) {
    // draw content inside panel...
}
atlas::panelEnd(ctx);
```

**Stateful panels** (support drag-to-move and minimize persistence):

```cpp
atlas::PanelState state;
state.bounds = {100, 100, 300, 200};
state.open = true;

// Each frame:
if (atlas::panelBeginStateful(ctx, "Movable Panel", state)) {
    // content visible (not minimized)
}
atlas::panelEnd(ctx);
```

### Button

```cpp
atlas::Rect btnRect = {110, 140, 100, 28};
if (atlas::button(ctx, "Click Me", btnRect)) {
    // handle click
}
```

### Icon Button (Sidebar style)

```cpp
atlas::WidgetID id = atlas::hashID("my_icon");
atlas::Rect iconRect = {4, 8, 36, 36};
atlas::Color iconColor = {0.3f, 0.8f, 0.9f, 1.0f};
if (atlas::iconButton(ctx, id, iconRect, iconColor)) {
    // handle click
}
```

### Progress Bar

```cpp
atlas::Rect barRect = {110, 170, 200, 16};
atlas::progressBar(ctx, barRect, 0.75f, ctx.theme().shield, "Shield: 75%");
```

### Ship Status Arcs

Concentric semicircular arcs showing shield / armor / hull:

```cpp
atlas::Vec2 centre = {640, 600};
float outerRadius = 70.0f;
atlas::shipStatusArcs(ctx, centre, outerRadius,
                      0.85f,   // shield 85%
                      1.0f,    // armor 100%
                      1.0f);   // hull 100%
```

### Capacitor Ring

Segmented circular gauge:

```cpp
atlas::capacitorRing(ctx, centre, 40.0f, 48.0f, 0.72f, 16);
```

Animated version (smoothly lerps toward target):

```cpp
float displayFrac = 1.0f;  // persistent state
atlas::capacitorRingAnimated(ctx, centre, 40.0f, 48.0f,
                             targetFrac, displayFrac, deltaTime, 16);
```

### Module Slot

Circular button for fitted modules:

```cpp
atlas::Vec2 slotPos = {600, 680};
if (atlas::moduleSlot(ctx, slotPos, 14.0f, isActive, cooldownPct, color)) {
    // module clicked
}
```

Extended version with overheat:

```cpp
if (atlas::moduleSlotEx(ctx, slotPos, 14.0f, isActive, cooldownPct,
                        color, overheatPct, gameTime)) {
    // module clicked
}
```

### Overview Row

```cpp
atlas::OverviewEntry entry;
entry.name = "Pirate Frigate";
entry.type = "Frigate";
entry.distance = 12000.0f;
entry.selected = true;

atlas::Rect rowRect = {800, 200, 300, 22};
if (atlas::overviewRow(ctx, rowRect, entry, false)) {
    // row clicked
}
```

### Target Card

```cpp
atlas::TargetCardInfo info;
info.name = "Enemy Ship";
info.shieldPct = 0.6f;
info.armorPct = 0.3f;
info.hullPct = 0.9f;
info.distance = 12000.0f;
info.isActive = true;

atlas::Rect cardRect = {500, 8, 80, 80};
if (atlas::targetCard(ctx, cardRect, info)) {
    // card clicked
}
```

### Sidebar

```cpp
atlas::sidebarBar(ctx, 0.0f, 40.0f, windowHeight, 8,
    [](int iconIndex) {
        // handle sidebar icon click
    });
```

### Other Widgets

| Widget | Function | Description |
|--------|----------|-------------|
| **Label** | `label(ctx, pos, text, color)` | Simple text label |
| **Separator** | `separator(ctx, start, width)` | Horizontal rule |
| **TreeNode** | `treeNode(ctx, rect, label, &expanded)` | Collapsible tree entry |
| **Scrollbar** | `scrollbar(ctx, track, offset, contentH, viewH)` | Vertical scroll indicator |
| **Tooltip** | `tooltip(ctx, text)` | Floating tooltip near cursor |
| **Checkbox** | `checkbox(ctx, label, rect, &checked)` | Toggle checkbox |
| **ComboBox** | `comboBox(ctx, label, rect, items, &sel, &open)` | Dropdown selector |
| **Slider** | `slider(ctx, label, rect, &val, min, max, fmt)` | Horizontal slider |
| **TextInput** | `textInput(ctx, label, rect, state, placeholder)` | Single-line text field |
| **Notification** | `notification(ctx, text, color)` | Transient toast banner |
| **ModeIndicator** | `modeIndicator(ctx, pos, text, color)` | Movement mode label |
| **InfoPanel** | `infoPanelDraw(ctx, state, data)` | Entity info window |
| **SpeedIndicator** | `speedIndicator(ctx, pos, speed, maxSpeed)` | Speed readout with +/- buttons |

## Theming

The `Theme` struct controls all visual properties:

```cpp
atlas::Theme theme;

// Backgrounds
theme.bgPrimary   = {0.051f, 0.067f, 0.090f, 0.92f};
theme.bgPanel     = {0.031f, 0.047f, 0.071f, 0.95f};
theme.bgHeader    = {0.039f, 0.055f, 0.078f, 1.0f};

// Accents (teal by default)
theme.accentPrimary = {0.271f, 0.816f, 0.910f, 1.0f};

// Text
theme.textPrimary   = {0.902f, 0.929f, 0.953f, 1.0f};
theme.textSecondary = {0.545f, 0.580f, 0.620f, 1.0f};

// Health colors
theme.shield = {0.2f, 0.6f, 1.0f, 1.0f};    // Blue
theme.armor  = {1.0f, 0.816f, 0.251f, 1.0f}; // Gold
theme.hull   = {0.902f, 0.271f, 0.271f, 1.0f}; // Red

// Apply
ctx.setTheme(theme);
```

## Interaction Model

Atlas uses an immediate-mode hot/active widget pattern:

- **Hot** — mouse is hovering over the widget this frame
- **Active** — mouse was pressed on the widget and is still held
- **Clicked** — mouse was released while still over the widget

```cpp
// Manual interaction:
atlas::WidgetID id = ctx.currentID("my_widget");
bool clicked = ctx.buttonBehavior(rect, id);

// Or use ctx helpers:
bool hovered = ctx.isHovered(rect);
bool isHot = ctx.isHot(id);
bool isActive = ctx.isActive(id);
bool mouseDown = ctx.isMouseDown();
bool mouseClicked = ctx.isMouseClicked();
```

### ID System

Widgets are identified by hashed string IDs, scoped by a push/pop stack:

```cpp
ctx.pushID("panel_1");
  // widgets here get IDs scoped under "panel_1"
  atlas::WidgetID btnId = ctx.currentID("my_button");
ctx.popID();
```

## Using in Other Projects

### Minimum Files Required

Copy these files into your project:

```
include/ui/atlas/
  atlas_types.h       — Types, Theme, InputState
  atlas_context.h     — Context class
  atlas_renderer.h    — OpenGL renderer
  atlas_widgets.h     — Widget functions

src/ui/atlas/
  atlas_context.cpp   — Context implementation
  atlas_renderer.cpp  — Renderer implementation
  atlas_widgets.cpp   — Widget implementations
```

Optionally, for the full HUD compositor:
```
include/ui/atlas/atlas_hud.h
src/ui/atlas/atlas_hud.cpp
```

### Dependencies

- **OpenGL 3.3+** (core profile)
- **C++17** standard library
- No other external libraries required

### Integration Steps

1. Copy the Atlas files into your project
2. Include the headers:
   ```cpp
   #include "atlas_types.h"
   #include "atlas_context.h"
   #include "atlas_widgets.h"
   ```
3. Create and initialize the context after your OpenGL context is ready:
   ```cpp
   atlas::AtlasContext ctx;
   ctx.init();
   ```
4. Each frame, between your OpenGL scene rendering and swap buffers:
   ```cpp
   atlas::InputState input = /* fill from your input system */;
   ctx.beginFrame(input);
   // ... your widget calls ...
   ctx.endFrame();
   ```
5. At shutdown:
   ```cpp
   ctx.shutdown();
   ```

### Building

Add the `.cpp` files to your build system. No special CMake modules or find scripts needed — just compile and link with OpenGL.

## File Reference

| File | Purpose |
|------|---------|
| `atlas_types.h` | Vec2, Rect, Color, Theme, InputState, PanelState, WidgetID, hashID() |
| `atlas_context.h/cpp` | AtlasContext: frame lifecycle, interaction state, ID stack |
| `atlas_renderer.h/cpp` | AtlasRenderer: batched OpenGL drawing (rects, lines, arcs, text, circles) |
| `atlas_widgets.h/cpp` | All widget functions (panel, button, bar, slot, overview, target, etc.) |
| `atlas_hud.h/cpp` | AtlasHUD: full EVE-style HUD compositor using the widget functions |
