#include "ui/input_handler.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>

namespace atlas {

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

void InputHandler::beginFrame() {
    // Clear per-frame transient state
    for (int i = 0; i < 3; ++i) {
        m_mouseClicked[i]  = false;
        m_mouseReleased[i] = false;
    }
    m_scrollDeltaY = 0.0f;
    m_doubleClick = false;
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

    // Track per-frame button transitions
    if (button >= 0 && button < 3) {
        if (action == GLFW_PRESS) {
            m_mouseDown[button]    = true;
            m_mouseClicked[button] = true;

            // Double-click detection (left button only)
            if (button == 0) {
                double now = glfwGetTime();
                double dt  = now - m_lastClickTime;
                double dx  = xpos - m_lastClickX;
                double dy  = ypos - m_lastClickY;
                double dist = std::sqrt(dx * dx + dy * dy);
                if (dt < DOUBLE_CLICK_TIME && dist < DOUBLE_CLICK_DIST) {
                    m_doubleClick = true;
                }
                m_lastClickTime = now;
                m_lastClickX    = xpos;
                m_lastClickY    = ypos;
            }
        } else if (action == GLFW_RELEASE) {
            m_mouseDown[button]     = false;
            m_mouseReleased[button] = true;
        }
    }
    
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
    
    // Calculate delta from last known position to new position
    double deltaX = xpos - m_lastMouseX;
    double deltaY = ypos - m_lastMouseY;
    
    // Update previous and current
    m_prevMouseX = m_lastMouseX;
    m_prevMouseY = m_lastMouseY;
    m_lastMouseX = xpos;
    m_lastMouseY = ypos;
    
    // Call registered callback
    if (m_mouseMoveCallback) {
        m_mouseMoveCallback(xpos, ypos, deltaX, deltaY);
    }
}

void InputHandler::handleScroll(double xoffset, double yoffset) {
    m_scrollDeltaY += static_cast<float>(yoffset);

    if (m_scrollCallback) {
        m_scrollCallback(xoffset, yoffset);
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

} // namespace atlas
