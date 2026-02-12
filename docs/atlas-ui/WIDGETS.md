# Atlas UI — Widget Reference

Complete API reference for every Atlas UI widget. All functions live in the `atlas` namespace and take `AtlasContext&` as their first parameter.

---

## Core Types

### `Vec2`
```cpp
struct Vec2 {
    float x, y;
    Vec2 operator+(const Vec2& o) const;
    Vec2 operator-(const Vec2& o) const;
    Vec2 operator*(float s) const;
};
```

### `Rect`
```cpp
struct Rect {
    float x, y, w, h;
    float right() const;   // x + w
    float bottom() const;  // y + h
    Vec2  center() const;
    bool  contains(Vec2 p) const;
};
```

### `Color`
```cpp
struct Color {
    float r, g, b, a;
    static Color fromRGBA(int r, int g, int b, int a = 255);
    Color withAlpha(float a) const;
};
```

### `InputState`
Filled by the host application each frame.
```cpp
struct InputState {
    Vec2 mousePos;
    bool mouseDown[3];      // left, right, middle
    bool mouseClicked[3];   // true the frame a button goes down
    bool mouseReleased[3];  // true the frame a button goes up
    float scrollY;
    int windowW, windowH;
};
```

### `PanelState`
Persistent per-panel state for drag and minimize.
```cpp
struct PanelState {
    Rect bounds;
    bool open;
    bool minimized;
    bool dragging;
    Vec2 dragOffset;
};
```

### `Theme`
Complete visual configuration — backgrounds, accents, text colors, health colors, standings, feedback colors, and panel metrics. See `atlas_types.h` for all fields.

### `WidgetID`
`uint32_t` hash for widget identity. Use `hashID(const char*)` to generate.

---

## Panels

### `panelBegin` / `panelEnd`
```cpp
bool panelBegin(AtlasContext& ctx, const char* title,
                Rect& bounds, const PanelFlags& flags = {},
                bool* open = nullptr);
void panelEnd(AtlasContext& ctx);
```
Returns `true` if panel is open. The `×` button writes `false` to `*open`.

### `panelBeginStateful`
```cpp
bool panelBeginStateful(AtlasContext& ctx, const char* title,
                        PanelState& state, const PanelFlags& flags = {});
```
Supports drag-to-move (via header bar) and minimize/restore. Returns `true` if content area should be drawn (not minimized).

### `PanelFlags`
```cpp
struct PanelFlags {
    bool showHeader   = true;
    bool showClose    = true;
    bool showMinimize = true;
    bool compactMode  = false;
    bool locked       = false;   // prevent drag
    bool drawBorder   = true;
};
```

---

## Buttons

### `button`
```cpp
bool button(AtlasContext& ctx, const char* label, const Rect& r);
```
Rectangular text button. Returns `true` on click.

### `iconButton`
```cpp
bool iconButton(AtlasContext& ctx, WidgetID id, const Rect& r, const Color& iconColor);
```
Small colored square button (sidebar style). Returns `true` on click.

---

## Bars & Gauges

### `progressBar`
```cpp
void progressBar(AtlasContext& ctx, const Rect& r, float fraction,
                 const Color& fillColor, const char* label = nullptr);
```

### `shipStatusArcs`
```cpp
void shipStatusArcs(AtlasContext& ctx, Vec2 centre, float outerR,
                    float shieldPct, float armorPct, float hullPct);
```

### `capacitorRing`
```cpp
void capacitorRing(AtlasContext& ctx, Vec2 centre,
                   float innerR, float outerR,
                   float fraction, int segments = 16);
```

### `capacitorRingAnimated`
```cpp
void capacitorRingAnimated(AtlasContext& ctx, Vec2 centre,
                           float innerR, float outerR,
                           float targetFrac, float& displayFrac,
                           float dt, int segments = 16,
                           float lerpSpeed = 5.0f);
```

### `speedIndicator`
```cpp
void speedIndicator(AtlasContext& ctx, Vec2 pos,
                    float currentSpeed, float maxSpeed);
```

---

## Module Slots

### `moduleSlot`
```cpp
bool moduleSlot(AtlasContext& ctx, Vec2 centre, float radius,
                bool active, float cooldownPct, const Color& color);
```

### `moduleSlotEx`
```cpp
bool moduleSlotEx(AtlasContext& ctx, Vec2 centre, float radius,
                  bool active, float cooldownPct, const Color& color,
                  float overheatPct, float time);
```

---

## Overview & Target Widgets

### `overviewHeader`
```cpp
void overviewHeader(AtlasContext& ctx, const Rect& r,
                    const std::vector<std::string>& tabs, int activeTab);
```

### `overviewHeaderInteractive`
```cpp
int overviewHeaderInteractive(AtlasContext& ctx, const Rect& r,
                              const std::vector<std::string>& tabs, int activeTab);
```
Returns clicked tab index, or `-1` if none.

### `overviewRow`
```cpp
bool overviewRow(AtlasContext& ctx, const Rect& r,
                 const OverviewEntry& entry, bool isAlternate);
```

### `targetCard`
```cpp
bool targetCard(AtlasContext& ctx, const Rect& r,
                const TargetCardInfo& info);
```

### `selectedItemPanel`
```cpp
void selectedItemPanel(AtlasContext& ctx, const Rect& r,
                       const SelectedItemInfo& info);
```

---

## Sidebar

### `sidebarBar`
```cpp
void sidebarBar(AtlasContext& ctx, float x, float width, float height,
                int icons, const std::function<void(int)>& callback);
```

---

## Form Widgets

### `checkbox`
```cpp
bool checkbox(AtlasContext& ctx, const char* label,
              const Rect& r, bool* checked);
```

### `comboBox`
```cpp
bool comboBox(AtlasContext& ctx, const char* label,
              const Rect& r, const std::vector<std::string>& items,
              int* selected, bool* dropdownOpen);
```

### `slider`
```cpp
bool slider(AtlasContext& ctx, const char* label,
            const Rect& r, float* value,
            float minVal, float maxVal,
            const char* format = nullptr);
```

### `textInput`
```cpp
bool textInput(AtlasContext& ctx, const char* label,
               const Rect& r, TextInputState& state,
               const char* placeholder = nullptr);
```

---

## Utility Widgets

### `label`
```cpp
void label(AtlasContext& ctx, Vec2 pos, const std::string& text,
           const Color& color = {});
```

### `separator`
```cpp
void separator(AtlasContext& ctx, Vec2 start, float width);
```

### `treeNode`
```cpp
bool treeNode(AtlasContext& ctx, const Rect& r,
              const char* label, bool* expanded);
```

### `scrollbar`
```cpp
void scrollbar(AtlasContext& ctx, const Rect& track,
               float scrollOffset, float contentHeight, float viewHeight);
```

### `tooltip`
```cpp
void tooltip(AtlasContext& ctx, const std::string& text);
```

### `notification`
```cpp
void notification(AtlasContext& ctx, const std::string& text,
                  const Color& color = {});
```

### `modeIndicator`
```cpp
void modeIndicator(AtlasContext& ctx, Vec2 pos,
                   const char* modeText, const Color& color = {});
```

### `infoPanelDraw`
```cpp
void infoPanelDraw(AtlasContext& ctx, PanelState& state,
                   const InfoPanelData& data);
```

---

## Data Structures

### `OverviewEntry`
```cpp
struct OverviewEntry {
    std::string name, type;
    float distance, velocity;
    Color standingColor;
    bool selected;
};
```

### `TargetCardInfo`
```cpp
struct TargetCardInfo {
    std::string name;
    float shieldPct, armorPct, hullPct, distance;
    bool isHostile, isActive;
};
```

### `SelectedItemInfo`
```cpp
struct SelectedItemInfo {
    std::string name;
    float distance;
    std::string distanceUnit;
};
```

### `InfoPanelData`
```cpp
struct InfoPanelData {
    std::string name, type, faction;
    float shieldPct, armorPct, hullPct;
    float distance, velocity, signature;
    bool hasHealth;
    bool isEmpty() const;
};
```

### `TextInputState`
```cpp
struct TextInputState {
    std::string text;
    int cursorPos;
    bool focused;
};
```
