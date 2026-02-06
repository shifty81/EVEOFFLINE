#pragma once

#include <memory>
#include <string>
#include <vector>
#include <glm/glm.hpp>

namespace eve {

// Forward declarations
class Window;
class GameClient;
class Renderer;
class InputHandler;
class Camera;
class EmbeddedServer;
class SessionManager;

} // namespace eve

namespace UI {
    class UIManager;
    class EntityPicker;
}

namespace eve {

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
    
    /**
     * Target an entity by ID
     */
    void targetEntity(const std::string& entityId, bool addToTargets = false);
    
    /**
     * Clear current target
     */
    void clearTarget();
    
    /**
     * Cycle to next target
     */
    void cycleTarget();
    
    /**
     * Activate module by slot (F1-F8)
     */
    void activateModule(int slotNumber);

private:
    void initialize();
    void setupUICallbacks();  // New method for wiring UI to network
    void update(float deltaTime);
    void render();
    void cleanup();
    
    // Input handlers
    void handleKeyInput(int key, int action, int mods);
    void handleMouseButton(int button, int action, int mods, double x, double y);
    void handleMouseMove(double x, double y, double deltaX, double deltaY);
    void handleScroll(double xoffset, double yoffset);
    
    // EVE-style right-click context menu
    void showSpaceContextMenu(double x, double y);
    void showEntityContextMenu(const std::string& entityId, double x, double y);
    
    // EVE-style movement commands
    void commandApproach(const std::string& entityId);
    void commandOrbit(const std::string& entityId, float distance = 500.0f);
    void commandKeepAtRange(const std::string& entityId, float distance = 2500.0f);
    void commandAlignTo(const std::string& entityId);
    void commandWarpTo(const std::string& entityId);
    void commandStopShip();
    
    // Spawn local player entity for offline/demo mode
    void spawnLocalPlayerEntity();
    void spawnDemoNPCEntities();
    void updateLocalMovement(float deltaTime);

    static Application* s_instance;

    std::unique_ptr<Window> m_window;
    std::unique_ptr<GameClient> m_gameClient;
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<InputHandler> m_inputHandler;
    std::unique_ptr<Camera> m_camera;
    std::unique_ptr<EmbeddedServer> m_embeddedServer;
    std::unique_ptr<SessionManager> m_sessionManager;
    std::unique_ptr<UI::UIManager> m_uiManager;
    std::unique_ptr<UI::EntityPicker> m_entityPicker;

    bool m_running;
    float m_lastFrameTime;
    
    // Targeting state
    std::string m_currentTargetId;
    std::vector<std::string> m_targetList;
    int m_currentTargetIndex;
    
    // EVE-style camera control state
    bool m_rightMouseDown = false;
    bool m_leftMouseDown = false;
    double m_lastMouseDragX = 0.0;
    double m_lastMouseDragY = 0.0;
    
    // EVE-style movement state
    enum class MoveCommand { None, Approach, Orbit, KeepAtRange, AlignTo, WarpTo };
    MoveCommand m_currentMoveCommand = MoveCommand::None;
    std::string m_moveTargetId;
    float m_orbitDistance = 500.0f;
    float m_keepAtRangeDistance = 2500.0f;
    glm::vec3 m_playerVelocity{0.0f};
    float m_playerSpeed = 0.0f;
    float m_playerMaxSpeed = 250.0f;
    
    // Context menu state
    bool m_showContextMenu = false;
    std::string m_contextMenuEntityId;
    double m_contextMenuX = 0.0;
    double m_contextMenuY = 0.0;
    
    // Local/demo mode
    bool m_localPlayerSpawned = false;
    std::string m_localPlayerId = "player_local";
};

} // namespace eve
