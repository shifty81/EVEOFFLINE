#pragma once

/**
 * @file atlas_types.h
 * @brief Core types for the Atlas UI system
 *
 * Atlas UI is a custom EVE-style UI framework for EVEOFFLINE.
 * It renders translucent dark panels with teal accent highlights using
 * raw OpenGL for in-game HUD and panel rendering.
 */

#include <string>
#include <vector>
#include <functional>
#include <cstdint>

namespace atlas {

// ── Geometry ────────────────────────────────────────────────────────

struct Vec2 {
    float x = 0.0f, y = 0.0f;
    Vec2() = default;
    Vec2(float x_, float y_) : x(x_), y(y_) {}
    Vec2 operator+(const Vec2& o) const { return {x + o.x, y + o.y}; }
    Vec2 operator-(const Vec2& o) const { return {x - o.x, y - o.y}; }
    Vec2 operator*(float s) const { return {x * s, y * s}; }
};

struct Rect {
    float x = 0.0f, y = 0.0f, w = 0.0f, h = 0.0f;
    Rect() = default;
    Rect(float x_, float y_, float w_, float h_)
        : x(x_), y(y_), w(w_), h(h_) {}

    float right()  const { return x + w; }
    float bottom() const { return y + h; }
    Vec2  center() const { return {x + w * 0.5f, y + h * 0.5f}; }
    bool  contains(Vec2 p) const {
        return p.x >= x && p.x <= x + w && p.y >= y && p.y <= y + h;
    }
};

// ── Color ───────────────────────────────────────────────────────────

struct Color {
    float r = 1.0f, g = 1.0f, b = 1.0f, a = 1.0f;
    Color() = default;
    Color(float r_, float g_, float b_, float a_ = 1.0f)
        : r(r_), g(g_), b(b_), a(a_) {}

    /** Construct from 0–255 integers. */
    static Color fromRGBA(int r, int g, int b, int a = 255) {
        return {r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f};
    }

    /** Return a copy with modified alpha. */
    Color withAlpha(float a_) const { return {r, g, b, a_}; }
};

// ── Photon Motion Timing Constants ───────────────────────────────
//
// EVE Photon UI principle: nothing snaps instantly. All transitions
// use these locked timings so the UI feels calm and predictable.
// Widgets MUST use these values — never invent custom durations.

namespace MotionTiming {
    constexpr float Instant        = 0.0f;
    constexpr float HoverDelay     = 0.06f;   // seconds before hover effect
    constexpr float HoverFade      = 0.10f;   // hover highlight fade duration
    constexpr float PanelOpen      = 0.16f;   // panel slide+fade in
    constexpr float PanelClose     = 0.12f;   // panel slide+fade out
    constexpr float FocusGain      = 0.14f;   // accent glow on focus
    constexpr float FocusLoss      = 0.10f;   // accent dim on unfocus
    constexpr float SelectionMove  = 0.12f;   // selection bar slide
    constexpr float TooltipDelay   = 0.25f;   // tooltip appear delay
    constexpr float TooltipFade    = 0.10f;   // tooltip fade in/out
    constexpr float TabSwitch      = 0.14f;   // tab content slide
    constexpr float RowHover       = 0.10f;   // list row hover highlight
    constexpr float RowSelect      = 0.12f;   // list row selection
}

// ── Atlas Theme (Photon Dark — EVE Online palette) ──────────────
//
// Based on the Photon UI design principles:
//   - Dark sci-fi panels with translucent overlays
//   - Sparse, meaningful accent colors (blue=navigation, orange=combat)
//   - Sharp edges, skeletal frames, no rounded blobs
//   - Clean typography with consistent hierarchy

struct Theme {
    // Backgrounds (near-black blues / gunmetal)
    Color bgPrimary   {0.05f,  0.06f,  0.08f,  0.96f};   // root background
    Color bgSecondary {0.08f,  0.10f,  0.13f,  0.94f};    // panel alt
    Color bgPanel     {0.06f,  0.08f,  0.11f,  0.96f};    // panel fill
    Color bgHeader    {0.039f, 0.055f, 0.078f, 1.0f};     // header bar
    Color bgTooltip   {0.110f, 0.129f, 0.157f, 0.95f};    // tooltip fill

    // Accents (semantic — navigation=blue/teal, combat=orange, danger=red)
    Color accentPrimary  {0.40f, 0.58f, 0.86f, 1.0f};    // navigation blue
    Color accentSecondary{0.28f, 0.72f, 0.82f, 1.0f};     // info/scanning cyan
    Color accentDim      {0.15f, 0.18f, 0.22f, 1.0f};     // subdued frame
    Color accentCombat   {0.88f, 0.46f, 0.24f, 1.0f};     // combat orange

    // Selection / hover
    Color selection {0.102f, 0.227f, 0.290f, 0.80f};
    Color hover     {0.102f, 0.227f, 0.290f, 0.50f};

    // Borders (thin, skeletal)
    Color borderNormal   {0.22f, 0.26f, 0.31f, 0.6f};    // frame edge
    Color borderHighlight{0.40f, 0.58f, 0.86f, 0.8f};     // focused frame
    Color borderSubtle   {0.15f, 0.18f, 0.22f, 0.5f};     // subdued edge

    // Text (off-white, never pure white; hierarchy via weight not size)
    Color textPrimary  {0.92f, 0.94f, 0.96f, 1.0f};
    Color textSecondary{0.70f, 0.74f, 0.79f, 1.0f};
    Color textMuted    {0.46f, 0.49f, 0.53f, 1.0f};
    Color textDisabled {0.282f, 0.310f, 0.345f, 0.6f};

    // Health
    Color shield    {0.2f,  0.6f,  1.0f,  1.0f};
    Color armor     {1.0f,  0.816f,0.251f,1.0f};
    Color hull      {0.902f,0.271f,0.271f,1.0f};
    Color capacitor {0.271f, 0.816f, 0.910f, 1.0f};

    // Standings
    Color hostile  {0.86f, 0.26f, 0.26f, 1.0f};
    Color friendly {0.40f, 0.58f, 0.86f, 1.0f};
    Color neutral  {0.667f,0.667f,0.667f,1.0f};

    // Feedback
    Color success {0.2f, 0.8f, 0.4f, 1.0f};
    Color warning {0.92f, 0.68f, 0.22f, 1.0f};
    Color danger  {0.86f, 0.26f, 0.26f, 1.0f};

    // Panel metrics (Photon: sharp edges, tight spacing, thin frames)
    float panelCornerRadius  = 0.0f;     // sharp corners (EVE-style)
    float borderWidth        = 1.0f;     // thin frame edges
    float headerHeight       = 22.0f;    // compact Photon headers
    float scrollbarWidth     = 6.0f;
    float itemSpacing        = 4.0f;
    float padding            = 8.0f;
    float rowHeight          = 18.0f;    // data list row height
    float selectionBarWidth  = 2.0f;     // thin left selection indicator
};

/** Global default theme. */
inline const Theme& defaultTheme() {
    static Theme t;
    return t;
}

// ── Key codes (mirrors GLFW values so Atlas stays GLFW-free) ────────

namespace Key {
    constexpr int F1  = 290;
    constexpr int F2  = 291;
    constexpr int F3  = 292;
    constexpr int F4  = 293;
    constexpr int F5  = 294;
    constexpr int F6  = 295;
    constexpr int F7  = 296;
    constexpr int F8  = 297;
    constexpr int F9  = 298;
    constexpr int F10 = 299;
    constexpr int F11 = 300;
    constexpr int F12 = 301;
    constexpr int V   = 86;   // D-Scan shortcut
}

// ── Input state snapshot (filled each frame by the host app) ────────

struct InputState {
    Vec2 mousePos;
    bool mouseDown[3] = {};      // left, right, middle
    bool mouseClicked[3] = {};   // true on the frame the button goes down
    bool mouseReleased[3] = {};  // true on the frame the button goes up
    float scrollY = 0.0f;        // vertical scroll delta this frame
    int  windowW = 1280;
    int  windowH = 720;

    // Keyboard state (for module hotkeys F1-F8, panel shortcuts, etc.)
    bool keyPressed[512] = {};   // true on the frame a key goes down (GLFW key codes)
    bool keyDown[512] = {};      // true while a key is held
};

// ── Panel persistent state ──────────────────────────────────────────

struct PanelState {
    Rect bounds;
    bool open = true;        // false = closed via × button
    bool minimized = false;  // true = collapsed to header-only
    bool dragging = false;   // true while header is being dragged
    Vec2 dragOffset;         // offset from mouse to panel origin during drag

    // Resize state
    bool resizing = false;       // true while an edge/corner is being dragged
    int  resizeEdge = 0;         // bitmask: 1=left, 2=right, 4=top, 8=bottom
    Vec2 resizeAnchor;           // mouse position at resize start
    Rect resizeOrigBounds;       // bounds at resize start
    float minW = 150.0f;         // minimum panel width
    float minH = 80.0f;          // minimum panel height

    // Lock state — prevents drag and resize when true
    bool locked = false;

    // Per-panel settings
    bool  settingsOpen = false;  // true when settings dropdown is visible
    float opacity = 1.0f;        // panel opacity (0.0–1.0)
    bool  compactRows = false;   // compact row display mode
};

// ── Widget IDs ──────────────────────────────────────────────────────

using WidgetID = uint32_t;

/** Simple FNV-1a hash for generating widget IDs from strings. */
inline WidgetID hashID(const char* s) {
    uint32_t h = 2166136261u;
    while (*s) {
        h ^= static_cast<uint32_t>(*s++);
        h *= 16777619u;
    }
    return h;
}

} // namespace atlas
