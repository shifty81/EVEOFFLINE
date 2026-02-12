#pragma once

#include <functional>
#include <unordered_map>
#include <unordered_set>

namespace atlas {

/**
 * Input handler for keyboard and mouse.
 *
 * Tracks per-frame button transitions (clicked/released), scroll
 * deltas, and double-click detection — all needed to drive EVE-style
 * camera orbit, zoom, and the Atlas immediate-mode UI widgets.
 */
class InputHandler {
public:
    using KeyCallback = std::function<void(int key, int action, int mods)>;
    using MouseButtonCallback = std::function<void(int button, int action, int mods, double x, double y)>;
    using MouseMoveCallback = std::function<void(double x, double y, double deltaX, double deltaY)>;
    using ScrollCallback = std::function<void(double xoffset, double yoffset)>;

    InputHandler();

    /**
     * Handle key input
     */
    void handleKey(int key, int action, int mods);

    /**
     * Handle mouse button input
     */
    void handleMouseButton(int button, int action, int mods, double xpos, double ypos);

    /**
     * Handle mouse movement
     */
    void handleMouse(double xpos, double ypos);

    /**
     * Handle scroll wheel input
     */
    void handleScroll(double xoffset, double yoffset);

    /**
     * Reset per-frame transient state (clicked, released, scroll delta).
     * Call once at the start of each frame before polling events.
     */
    void beginFrame();

    /**
     * Set callbacks
     */
    void setKeyCallback(KeyCallback callback) { m_keyCallback = callback; }
    void setMouseButtonCallback(MouseButtonCallback callback) { m_mouseButtonCallback = callback; }
    void setMouseMoveCallback(MouseMoveCallback callback) { m_mouseMoveCallback = callback; }
    void setScrollCallback(ScrollCallback callback) { m_scrollCallback = callback; }

    /**
     * Check if key is pressed
     */
    bool isKeyPressed(int key) const;
    
    /**
     * Check if key modifiers are active
     */
    bool isCtrlPressed() const { return m_ctrlPressed; }
    bool isShiftPressed() const { return m_shiftPressed; }
    bool isAltPressed() const { return m_altPressed; }
    int getModifierMask() const;
    
    /**
     * Get mouse position
     */
    double getMouseX() const { return m_lastMouseX; }
    double getMouseY() const { return m_lastMouseY; }

    // ── Per-frame button state (for Atlas UI) ───────────────────────
    bool isMouseDown(int button) const    { return (button >= 0 && button < 3) ? m_mouseDown[button] : false; }
    bool isMouseClicked(int button) const { return (button >= 0 && button < 3) ? m_mouseClicked[button] : false; }
    bool isMouseReleased(int button) const{ return (button >= 0 && button < 3) ? m_mouseReleased[button] : false; }

    // ── Scroll delta (accumulated over one frame) ───────────────────
    float getScrollDeltaY() const { return m_scrollDeltaY; }

    // ── Double-click detection ──────────────────────────────────────
    bool isDoubleClick() const { return m_doubleClick; }

private:
    void updateModifiers(int mods);

    KeyCallback m_keyCallback;
    MouseButtonCallback m_mouseButtonCallback;
    MouseMoveCallback m_mouseMoveCallback;
    ScrollCallback m_scrollCallback;
    
    double m_lastMouseX;
    double m_lastMouseY;
    double m_prevMouseX;
    double m_prevMouseY;
    bool m_firstMouse;
    
    // Track pressed keys
    std::unordered_set<int> m_pressedKeys;
    
    // Track modifiers
    bool m_ctrlPressed;
    bool m_shiftPressed;
    bool m_altPressed;

    // Per-frame mouse button state
    bool m_mouseDown[3]     = {};
    bool m_mouseClicked[3]  = {};   // true on the frame a button goes down
    bool m_mouseReleased[3] = {};   // true on the frame a button goes up

    // Per-frame scroll accumulator
    float m_scrollDeltaY = 0.0f;

    // Double-click detection
    bool   m_doubleClick = false;
    double m_lastClickTime = 0.0;
    double m_lastClickX = 0.0;
    double m_lastClickY = 0.0;
    static constexpr double DOUBLE_CLICK_TIME = 0.35;
    static constexpr double DOUBLE_CLICK_DIST = 8.0;
};

} // namespace atlas
