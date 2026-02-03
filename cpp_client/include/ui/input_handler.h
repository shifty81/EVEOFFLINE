#pragma once

#include <functional>

namespace eve {

/**
 * Input handler for keyboard and mouse
 */
class InputHandler {
public:
    using KeyCallback = std::function<void(int key, int action)>;

    InputHandler();

    /**
     * Handle key input
     */
    void handleKey(int key, int action);

    /**
     * Handle mouse movement
     */
    void handleMouse(double xpos, double ypos);

    /**
     * Set key callback
     */
    void setKeyCallback(KeyCallback callback) { m_keyCallback = callback; }

    /**
     * Check if key is pressed
     */
    bool isKeyPressed(int key) const;

private:
    KeyCallback m_keyCallback;
    double m_lastMouseX;
    double m_lastMouseY;
    bool m_firstMouse;
};

} // namespace eve
