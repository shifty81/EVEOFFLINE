#pragma once

#include <functional>
#include <unordered_map>
#include <unordered_set>

namespace eve {

/**
 * Input handler for keyboard and mouse
 */
class InputHandler {
public:
    using KeyCallback = std::function<void(int key, int action, int mods)>;
    using MouseButtonCallback = std::function<void(int button, int action, int mods, double x, double y)>;
    using MouseMoveCallback = std::function<void(double x, double y, double deltaX, double deltaY)>;

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
     * Set callbacks
     */
    void setKeyCallback(KeyCallback callback) { m_keyCallback = callback; }
    void setMouseButtonCallback(MouseButtonCallback callback) { m_mouseButtonCallback = callback; }
    void setMouseMoveCallback(MouseMoveCallback callback) { m_mouseMoveCallback = callback; }

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
    
    /**
     * Get mouse position
     */
    double getMouseX() const { return m_lastMouseX; }
    double getMouseY() const { return m_lastMouseY; }

private:
    void updateModifiers(int mods);

    KeyCallback m_keyCallback;
    MouseButtonCallback m_mouseButtonCallback;
    MouseMoveCallback m_mouseMoveCallback;
    
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
};

} // namespace eve
