#pragma once

/**
 * @file atlas_context.h
 * @brief Frame-level state manager for the Atlas UI system
 *
 * AtlasContext holds the per-frame input state, active/hot widget IDs
 * (for click/hover tracking), and provides the bridge between the host
 * application's GLFW input and the Atlas widget layer.
 *
 * Typical frame flow:
 *   ctx.beginFrame(input);
 *   // … widget calls (panel, button, bar, etc.) …
 *   ctx.endFrame();
 *
 * Layout reference (from EVE Online screenshot analysis):
 *
 *   ┌─────────┬────────────────────────────────────────────┬──────────────┐
 *   │ Neocom  │  Locked Targets (top-center row)           │ Selected     │
 *   │ (left   │                                            │ Item panel   │
 *   │ 15-56px)│                                            │ (top-right)  │
 *   │         │         3D Space View                      │──────────────│
 *   │         │                                            │ Overview     │
 *   │         │  ┌─People & Places─┐                       │ panel (right │
 *   │         │  │  search / tree  │   Combat text floats  │ ~300px wide) │
 *   │         │  └─────────────────┘                       │              │
 *   │         │  ┌─Local Chat──────┐                       │              │
 *   │         │  │  channel msgs   │   "APPROACHING"       │              │
 *   │         │  └─────────────────┘   notification        │              │
 *   │         │                                            │              │
 *   │         │       ┌──────HUD──────────────────┐        │              │
 *   │         │       │ Shield/Armor/Hull arcs     │        │              │
 *   │         │       │ Capacitor ring (segments)  │        │              │
 *   │         │       │ Module rack (circles)      │        │              │
 *   │         │       │ Speed: 100.0 m/s  [- / +]  │        │              │
 *   │         │       └───────────────────────────┘        │              │
 *   └─────────┴────────────────────────────────────────────┴──────────────┘
 *     Clock
 */

#include "atlas_types.h"
#include "atlas_renderer.h"
#include <string>

namespace atlas {

/**
 * AtlasContext — per-frame UI state and the main entry point for
 * immediate-mode-style widget calls.
 *
 * Widgets query the context for hot/active state (hover, pressed)
 * and push draw commands through the embedded AtlasRenderer.
 */
class AtlasContext {
public:
    AtlasContext();
    ~AtlasContext();

    // ── Lifecycle ───────────────────────────────────────────────────

    /** Compile shaders and allocate GPU resources.  Call once. */
    bool init();

    /** Free GPU resources.  Call once at shutdown. */
    void shutdown();

    /** Begin a new UI frame.  Must be called before any widget calls. */
    void beginFrame(const InputState& input);

    /** Flush draw commands and reset per-frame state. */
    void endFrame();

    // ── Accessors ───────────────────────────────────────────────────

    AtlasRenderer& renderer() { return m_renderer; }
    const Theme&    theme()    const { return m_theme; }
    const InputState& input()  const { return m_input; }

    void setTheme(const Theme& t) { m_theme = t; }

    // ── Interaction helpers ─────────────────────────────────────────

    /** Test whether the mouse is inside a rectangle this frame. */
    bool isHovered(const Rect& r) const;

    /** Mark a widget as "hot" (hovered) this frame. */
    void setHot(WidgetID id);

    /** Mark a widget as "active" (pressed/dragging) this frame. */
    void setActive(WidgetID id);

    /** Release the active widget. */
    void clearActive();

    bool isHot(WidgetID id)    const { return m_hotID == id; }
    bool isActive(WidgetID id) const { return m_activeID == id; }

    /** Convenience: returns true if the left mouse button was clicked
     *  inside @p r this frame. Also sets hot/active state. */
    bool buttonBehavior(const Rect& r, WidgetID id);

    // ── ID stack (for panel scoping) ────────────────────────────────

    void pushID(const char* label);
    void popID();
    WidgetID currentID(const char* label) const;

    // ── Drag helpers ────────────────────────────────────────────────

    /** Per-frame mouse delta (current position minus previous frame position). */
    Vec2 getDragDelta() const;

    /** Check if the left mouse is currently held down. */
    bool isMouseDown() const { return m_input.mouseDown[0]; }

    /** Check if left mouse was just clicked this frame. */
    bool isMouseClicked() const { return m_input.mouseClicked[0]; }

    /** Check if right mouse was just clicked this frame. */
    bool isRightMouseClicked() const { return m_input.mouseClicked[1]; }

    // ── Mouse consumption (prevents click-through) ──────────────────

    /** Mark the mouse as consumed — subsequent widgets should ignore clicks. */
    void consumeMouse() { m_mouseConsumed = true; }

    /** Check if another widget already consumed the mouse this frame. */
    bool isMouseConsumed() const { return m_mouseConsumed; }

    // ── Layout margins (sidebar boundary) ───────────────────────────

    /** Set the sidebar width so panels clamp to it as a left boundary. */
    void setSidebarWidth(float w) { m_sidebarWidth = w; }

    /** Get the sidebar width (left margin for panel clamping). */
    float sidebarWidth() const { return m_sidebarWidth; }

private:
    AtlasRenderer m_renderer;
    Theme          m_theme;
    InputState     m_input;
    Vec2           m_prevMousePos;  // previous frame mouse position for drag delta

    WidgetID m_hotID    = 0;
    WidgetID m_activeID = 0;

    // Mouse consumed flag — set when a widget claims the mouse event
    // to prevent other widgets from also responding
    bool m_mouseConsumed = false;

    // Sidebar width — used as left margin for panel clamping
    float m_sidebarWidth = 0.0f;

    // ID stack for scoped widget naming
    std::vector<WidgetID> m_idStack;
};

} // namespace atlas
