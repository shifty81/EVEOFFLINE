#pragma once

#include <string>
#include <functional>

struct GLFWwindow;

namespace eve {

/**
 * Window management class using GLFW
 */
class Window {
public:
    using KeyCallback = std::function<void(int key, int action)>;
    using MouseCallback = std::function<void(double xpos, double ypos)>;
    using MouseButtonCallback = std::function<void(int button, int action, int mods)>;
    using ScrollCallback = std::function<void(double xoffset, double yoffset)>;
    using ResizeCallback = std::function<void(int width, int height)>;

    Window(const std::string& title, int width, int height);
    ~Window();

    // Delete copy constructor and assignment
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    /**
     * Check if window should close
     */
    bool shouldClose() const;

    /**
     * Swap buffers and poll events
     */
    void update();

    /**
     * Get window dimensions
     */
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    float getAspectRatio() const { return static_cast<float>(m_width) / static_cast<float>(m_height); }

    /**
     * Set callbacks
     */
    void setKeyCallback(KeyCallback callback) { m_keyCallback = callback; }
    void setMouseCallback(MouseCallback callback) { m_mouseCallback = callback; }
    void setMouseButtonCallback(MouseButtonCallback callback) { m_mouseButtonCallback = callback; }
    void setScrollCallback(ScrollCallback callback) { m_scrollCallback = callback; }
    void setResizeCallback(ResizeCallback callback) { m_resizeCallback = callback; }

    /**
     * Get GLFW window handle
     */
    GLFWwindow* getHandle() const { return m_window; }

private:
    static void keyCallbackStatic(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void cursorPosCallbackStatic(GLFWwindow* window, double xpos, double ypos);
    static void mouseButtonCallbackStatic(GLFWwindow* window, int button, int action, int mods);
    static void scrollCallbackStatic(GLFWwindow* window, double xoffset, double yoffset);
    static void framebufferSizeCallbackStatic(GLFWwindow* window, int width, int height);

    GLFWwindow* m_window;
    std::string m_title;
    int m_width;
    int m_height;

    KeyCallback m_keyCallback;
    MouseCallback m_mouseCallback;
    MouseButtonCallback m_mouseButtonCallback;
    ScrollCallback m_scrollCallback;
    ResizeCallback m_resizeCallback;
};

} // namespace eve
