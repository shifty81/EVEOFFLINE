#include "ui/input_handler.h"
#include <GLFW/glfw3.h>

namespace eve {

InputHandler::InputHandler()
    : m_lastMouseX(0.0)
    , m_lastMouseY(0.0)
    , m_firstMouse(true)
{
}

void InputHandler::handleKey(int key, int action) {
    if (m_keyCallback) {
        m_keyCallback(key, action);
    }
}

void InputHandler::handleMouse(double xpos, double ypos) {
    if (m_firstMouse) {
        m_lastMouseX = xpos;
        m_lastMouseY = ypos;
        m_firstMouse = false;
        return;
    }
    
    // Calculate delta for future use
    double deltaX = xpos - m_lastMouseX;
    double deltaY = ypos - m_lastMouseY;
    
    m_lastMouseX = xpos;
    m_lastMouseY = ypos;
}

bool InputHandler::isKeyPressed(int key) const {
    // This would need to track key states
    return false;
}

} // namespace eve
