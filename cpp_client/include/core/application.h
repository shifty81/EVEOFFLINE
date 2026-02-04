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
class EmbeddedServer;
class SessionManager;

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

    /**
     * Get embedded server (nullptr if not hosting)
     */
    EmbeddedServer* getEmbeddedServer() { return m_embeddedServer.get(); }

    /**
     * Get session manager
     */
    SessionManager* getSessionManager() { return m_sessionManager.get(); }

    /**
     * Host a multiplayer game
     */
    bool hostMultiplayerGame(const std::string& sessionName, int maxPlayers = 20);

    /**
     * Join a multiplayer game
     */
    bool joinMultiplayerGame(const std::string& host, int port);

    /**
     * Check if hosting a game
     */
    bool isHosting() const;

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
    std::unique_ptr<EmbeddedServer> m_embeddedServer;
    std::unique_ptr<SessionManager> m_sessionManager;

    bool m_running;
    float m_lastFrameTime;
};

} // namespace eve
