#include "ui/input_handler.h"
#include <GLFW/glfw3.h>
#include <iostream>

namespace eve {

InputHandler::InputHandler()
    : m_lastMouseX(0.0)
    , m_lastMouseY(0.0)
    , m_prevMouseX(0.0)
    , m_prevMouseY(0.0)
    , m_firstMouse(true)
    , m_ctrlPressed(false)
    , m_shiftPressed(false)
    , m_altPressed(false)
{
}

void InputHandler::handleKey(int key, int action, int mods) {
    // Update key state
    if (action == GLFW_PRESS) {
        m_pressedKeys.insert(key);
    } else if (action == GLFW_RELEASE) {
        m_pressedKeys.erase(key);
    }
    
    // Update modifiers
    updateModifiers(mods);
    
    // Call registered callback
    if (m_keyCallback) {
        m_keyCallback(key, action, mods);
    }
}

void InputHandler::handleMouseButton(int button, int action, int mods, double xpos, double ypos) {
    // Update modifiers
    updateModifiers(mods);
    
    // Call registered callback
    if (m_mouseButtonCallback) {
        m_mouseButtonCallback(button, action, mods, xpos, ypos);
    }
}

void InputHandler::handleMouse(double xpos, double ypos) {
    if (m_firstMouse) {
        m_lastMouseX = xpos;
        m_lastMouseY = ypos;
        m_prevMouseX = xpos;
        m_prevMouseY = ypos;
        m_firstMouse = false;
        return;
    }
    
    // Calculate delta
    double deltaX = xpos - m_prevMouseX;
    double deltaY = ypos - m_prevMouseY;
    
    // Update previous
    m_prevMouseX = m_lastMouseX;
    m_prevMouseY = m_lastMouseY;
    
    // Update current
    m_lastMouseX = xpos;
    m_lastMouseY = ypos;
    
    // Call registered callback
    if (m_mouseMoveCallback) {
        m_mouseMoveCallback(xpos, ypos, deltaX, deltaY);
    }
}

bool InputHandler::isKeyPressed(int key) const {
    return m_pressedKeys.find(key) != m_pressedKeys.end();
}

int InputHandler::getModifierMask() const {
    int mods = 0;
    if (m_ctrlPressed) {
        mods |= GLFW_MOD_CONTROL;
    }
    if (m_shiftPressed) {
        mods |= GLFW_MOD_SHIFT;
    }
    if (m_altPressed) {
        mods |= GLFW_MOD_ALT;
    }
    return mods;
}

void InputHandler::updateModifiers(int mods) {
    m_ctrlPressed = (mods & GLFW_MOD_CONTROL) != 0;
    m_shiftPressed = (mods & GLFW_MOD_SHIFT) != 0;
    m_altPressed = (mods & GLFW_MOD_ALT) != 0;
}

} // namespace eve
