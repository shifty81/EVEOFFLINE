#pragma once

/**
 * @file atlas_types.h
 * @brief Core types for the Atlas UI system
 *
 * Atlas UI is a custom EVE-style UI framework for EVEOFFLINE.
 * It renders translucent dark panels with teal accent highlights using
 * raw OpenGL, replacing ImGui for in-game HUD and panel rendering.
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

// ── Atlas Theme (Atlas UI palette) ────────────────────────────

struct Theme {
    // Backgrounds
    Color bgPrimary   {0.051f, 0.067f, 0.090f, 0.92f};   // #0D1117
    Color bgSecondary {0.086f, 0.106f, 0.133f, 0.90f};    // #161B22
    Color bgPanel     {0.031f, 0.047f, 0.071f, 0.95f};    // #080C12
    Color bgHeader    {0.039f, 0.055f, 0.078f, 1.0f};     // #0A0E14
    Color bgTooltip   {0.110f, 0.129f, 0.157f, 0.95f};    // #1C2128

    // Accents
    Color accentPrimary  {0.271f, 0.816f, 0.910f, 1.0f};  // #45D0E8
    Color accentSecondary{0.471f, 0.882f, 0.941f, 1.0f};   // #78E1F0
    Color accentDim      {0.165f, 0.353f, 0.416f, 1.0f};   // #2A5A6A

    // Selection / hover
    Color selection {0.102f, 0.227f, 0.290f, 0.80f};       // #1A3A4A
    Color hover     {0.102f, 0.227f, 0.290f, 0.50f};

    // Borders
    Color borderNormal   {0.157f, 0.220f, 0.282f, 0.6f};  // #283848
    Color borderHighlight{0.271f, 0.816f, 0.910f, 0.8f};   // #45D0E8
    Color borderSubtle   {0.118f, 0.165f, 0.212f, 0.5f};   // #1E2A36

    // Text
    Color textPrimary  {0.902f, 0.929f, 0.953f, 1.0f};    // #E6EDF3
    Color textSecondary{0.545f, 0.580f, 0.620f, 1.0f};     // #8B949E
    Color textDisabled {0.282f, 0.310f, 0.345f, 0.6f};     // #484F58

    // Health
    Color shield {0.2f,  0.6f,  1.0f,  1.0f};
    Color armor  {1.0f,  0.816f,0.251f,1.0f};
    Color hull   {0.902f,0.271f,0.271f,1.0f};
    Color capacitor{0.271f, 0.816f, 0.910f, 1.0f};

    // Standings
    Color hostile  {0.8f, 0.2f, 0.2f, 1.0f};
    Color friendly {0.2f, 0.6f, 1.0f, 1.0f};
    Color neutral  {0.667f,0.667f,0.667f,1.0f};

    // Feedback
    Color success {0.2f, 0.8f, 0.4f, 1.0f};
    Color warning {1.0f, 0.722f,0.2f,1.0f};
    Color danger  {1.0f, 0.2f, 0.2f, 1.0f};

    // Panel metrics
    float panelCornerRadius  = 4.0f;
    float borderWidth        = 1.0f;
    float headerHeight       = 28.0f;
    float scrollbarWidth     = 6.0f;
    float itemSpacing        = 4.0f;
    float padding            = 8.0f;
};

/** Global default theme. */
inline const Theme& defaultTheme() {
    static Theme t;
    return t;
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
};

// ── Panel persistent state ──────────────────────────────────────────

struct PanelState {
    Rect bounds;
    bool open = true;        // false = closed via × button
    bool minimized = false;  // true = collapsed to header-only
    bool dragging = false;   // true while header is being dragged
    Vec2 dragOffset;         // offset from mouse to panel origin during drag
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
