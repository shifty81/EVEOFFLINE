#pragma once

/**
 * @file photon_context.h
 * @brief Frame-level state manager for the Photon UI system
 *
 * PhotonContext holds the per-frame input state, active/hot widget IDs
 * (for click/hover tracking), and provides the bridge between the host
 * application's GLFW input and the Photon widget layer.
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

#include "photon_types.h"
#include "photon_renderer.h"
#include <string>

namespace photon {

/**
 * PhotonContext — per-frame UI state and the main entry point for
 * immediate-mode-style widget calls.
 *
 * Widgets query the context for hot/active state (hover, pressed)
 * and push draw commands through the embedded PhotonRenderer.
 */
class PhotonContext {
public:
    PhotonContext();
    ~PhotonContext();

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

    PhotonRenderer& renderer() { return m_renderer; }
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

    /** Begin dragging from the given position. Returns drag delta. */
    Vec2 getDragDelta() const;

    /** Check if the left mouse is currently held down. */
    bool isMouseDown() const { return m_input.mouseDown[0]; }

    /** Check if left mouse was just clicked this frame. */
    bool isMouseClicked() const { return m_input.mouseClicked[0]; }

private:
    PhotonRenderer m_renderer;
    Theme          m_theme;
    InputState     m_input;

    WidgetID m_hotID    = 0;
    WidgetID m_activeID = 0;

    // ID stack for scoped widget naming
    std::vector<WidgetID> m_idStack;
};

} // namespace photon
