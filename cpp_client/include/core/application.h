#pragma once

#include <memory>
#include <string>

namespace eve {

// Forward declarations
class Window;
class GameClient;
class Renderer;
class InputHandler;
class Camera;

/**
 * Main application class
 * Manages the game loop, window, and core systems
 */
class Application {
public:
    Application(const std::string& title, int width, int height);
    ~Application();

    // Delete copy constructor and assignment
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    /**
     * Run the main game loop
     */
    void run();

    /**
     * Request application shutdown
     */
    void shutdown();

    /**
     * Get application instance
     */
    static Application* getInstance() { return s_instance; }

private:
    void initialize();
    void update(float deltaTime);
    void render();
    void cleanup();

    static Application* s_instance;

    std::unique_ptr<Window> m_window;
    std::unique_ptr<GameClient> m_gameClient;
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<InputHandler> m_inputHandler;
    std::unique_ptr<Camera> m_camera;

    bool m_running;
    float m_lastFrameTime;
};

} // namespace eve
