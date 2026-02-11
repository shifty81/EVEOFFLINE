#include "rendering/window.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <stdexcept>

namespace eve {

Window::Window(const std::string& title, int width, int height)
    : m_window(nullptr)
    , m_title(title)
    , m_width(width)
    , m_height(height)
    , m_keyCallback(nullptr)
    , m_mouseCallback(nullptr)
    , m_mouseButtonCallback(nullptr)
    , m_scrollCallback(nullptr)
    , m_resizeCallback(nullptr)
{
    // Initialize GLFW
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW");
    }

    std::cout << "GLFW initialized successfully" << std::endl;

    // Configure GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Enable multisampling for anti-aliasing
    glfwWindowHint(GLFW_SAMPLES, 4);

    // Create window
    m_window = glfwCreateWindow(m_width, m_height, m_title.c_str(), nullptr, nullptr);
    if (!m_window) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }

    std::cout << "Window created: " << m_width << "x" << m_height << std::endl;

    // Set window user pointer to this instance for callbacks
    glfwSetWindowUserPointer(m_window, this);

    // Make OpenGL context current
    glfwMakeContextCurrent(m_window);
    
    // Initialize GLEW
    glewExperimental = GL_TRUE;
    GLenum glewError = glewInit();
    if (glewError != GLEW_OK) {
        glfwDestroyWindow(m_window);
        glfwTerminate();
        throw std::runtime_error(std::string("Failed to initialize GLEW: ") + 
                                reinterpret_cast<const char*>(glewGetErrorString(glewError)));
    }
    
    std::cout << "GLEW initialized successfully" << std::endl;

    // Enable VSync
    glfwSwapInterval(1);

    // Set callbacks
    glfwSetKeyCallback(m_window, keyCallbackStatic);
    glfwSetCharCallback(m_window, charCallbackStatic);
    glfwSetCursorPosCallback(m_window, cursorPosCallbackStatic);
    glfwSetMouseButtonCallback(m_window, mouseButtonCallbackStatic);
    glfwSetScrollCallback(m_window, scrollCallbackStatic);
    glfwSetFramebufferSizeCallback(m_window, framebufferSizeCallbackStatic);

    std::cout << "Window initialization complete" << std::endl;
}

Window::~Window() {
    if (m_window) {
        glfwDestroyWindow(m_window);
        std::cout << "Window destroyed" << std::endl;
    }
    glfwTerminate();
    std::cout << "GLFW terminated" << std::endl;
}

bool Window::shouldClose() const {
    return glfwWindowShouldClose(m_window);
}

void Window::update() {
    glfwSwapBuffers(m_window);
    glfwPollEvents();
}

void Window::keyCallbackStatic(GLFWwindow* window, int key, int scancode, int action, int mods) {
    Window* instance = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (instance && instance->m_keyCallback) {
        instance->m_keyCallback(key, scancode, action, mods);
    }
}

void Window::charCallbackStatic(GLFWwindow* window, unsigned int codepoint) {
    Window* instance = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (instance && instance->m_charCallback) {
        instance->m_charCallback(codepoint);
    }
}

void Window::cursorPosCallbackStatic(GLFWwindow* window, double xpos, double ypos) {
    Window* instance = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (instance && instance->m_mouseCallback) {
        instance->m_mouseCallback(xpos, ypos);
    }
}

void Window::framebufferSizeCallbackStatic(GLFWwindow* window, int width, int height) {
    Window* instance = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (instance) {
        instance->m_width = width;
        instance->m_height = height;
        if (instance->m_resizeCallback) {
            instance->m_resizeCallback(width, height);
        }
    }
}

void Window::mouseButtonCallbackStatic(GLFWwindow* window, int button, int action, int mods) {
    Window* instance = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (instance && instance->m_mouseButtonCallback) {
        instance->m_mouseButtonCallback(button, action, mods);
    }
}

void Window::scrollCallbackStatic(GLFWwindow* window, double xoffset, double yoffset) {
    Window* instance = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (instance && instance->m_scrollCallback) {
        instance->m_scrollCallback(xoffset, yoffset);
    }
}

} // namespace eve
