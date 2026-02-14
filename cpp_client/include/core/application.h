#pragma once

#include <memory>
#include <string>
#include <vector>
#include <glm/glm.hpp>

namespace atlas {

// Forward declarations
class Window;
class GameClient;
class Renderer;
class InputHandler;
class Camera;
class EmbeddedServer;
class SessionManager;
class SolarSystemScene;
class ShipPhysics;

} // namespace atlas

namespace UI {
    class EntityPicker;
    class ContextMenu;
    class RadialMenu;
}

namespace atlas {
    class AtlasHUD;
    class AtlasContext;
}

namespace atlas {

/**
 * Main application class for EVE OFFLINE client
 * 
 * Manages the game loop, window, and all core systems including:
 * - Rendering (3D graphics, UI)
 * - Input handling (keyboard, mouse)
 * - Networking (client-server or embedded server)
 * - Camera control (EVE-style right-click camera)
 * - Entity management and targeting
 * - Session management (singleplayer/multiplayer)
 * 
 * Lifecycle:
 * 1. Constructor: Create window and systems
 * 2. initialize(): Set up OpenGL, load resources, connect subsystems
 * 3. run(): Main game loop (update → render → present)
 * 4. cleanup(): Shutdown systems and free resources
 * 
 * Supports two modes:
 * - **Local Mode**: Single-player with demo NPCs for testing
 * - **Multiplayer Mode**: Connect to dedicated server or host embedded server
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
     * 
     * Starts an embedded server and connects the client to it. The server
     * runs in a separate thread and accepts connections from other clients.
     * 
     * @param sessionName Name for the multiplayer session
     * @param maxPlayers Maximum number of players (default 20)
     * @return true if server started successfully
     */
    bool hostMultiplayerGame(const std::string& sessionName, int maxPlayers = 20);

    /**
     * Join a multiplayer game
     * 
     * Connect to a remote dedicated server. Disconnects from any existing
     * session first.
     * 
     * @param host Server hostname or IP address
     * @param port Server port (default 7777)
     * @return true if connection successful
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
    /**
     * Initialize all subsystems
     * 
     * Sets up:
     * - OpenGL context and renderer
     * - Input handlers
     * - UI manager and panels
     * - Camera
     * - Network client (if connecting to server)
     * - Local player entity (if in demo mode)
     */
    void initialize();
    
    /**
     * Wire UI callbacks to network commands
     * 
     * Connects UI panel buttons and interactions to the appropriate
     * network message handlers (target lock, module activate, etc.)
     */
    void setupUICallbacks();
    
    /**
     * Update game state
     * 
     * Called each frame to:
     * - Update camera position
     * - Process network messages
     * - Update entity states
     * - Handle movement commands
     * - Update UI state
     * 
     * @param deltaTime Time since last frame in seconds
     */
    void update(float deltaTime);
    
    /**
     * Render the current frame
     * 
     * Rendering order:
     * 1. Clear screen
     * 2. Render 3D scene (starfield, entities, health bars)
     * 3. Render UI overlays (panels, menus, HUD)
     * 4. Present frame to window
     */
    void render();
    
    /**
     * Cleanup and shutdown
     * 
     * Shuts down all subsystems in reverse initialization order:
     * - Disconnect from network
     * - Stop embedded server (if hosting)
     * - Cleanup OpenGL resources
     * - Destroy window
     */
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
    void commandJump(const std::string& entityId);
    void commandStopShip();
    
    // Spawn local player entity for offline/demo mode
    void spawnLocalPlayerEntity();
    void spawnDemoNPCEntities();
    void updateLocalMovement(float deltaTime);
    void updateTargetListUi(const glm::vec3& playerPosition);
    
    // Build info panel data for an entity
    void openInfoPanelForEntity(const std::string& entityId);

    static Application* s_instance;

    std::unique_ptr<Window> m_window;
    std::unique_ptr<GameClient> m_gameClient;
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<InputHandler> m_inputHandler;
    std::unique_ptr<Camera> m_camera;
    std::unique_ptr<EmbeddedServer> m_embeddedServer;
    std::unique_ptr<SessionManager> m_sessionManager;
    std::unique_ptr<UI::EntityPicker> m_entityPicker;
    std::unique_ptr<SolarSystemScene> m_solarSystem;
    std::unique_ptr<ShipPhysics> m_shipPhysics;
    std::unique_ptr<atlas::AtlasContext> m_atlasCtx;
    std::unique_ptr<atlas::AtlasHUD> m_atlasHUD;
    std::unique_ptr<UI::ContextMenu> m_contextMenu;
    std::unique_ptr<UI::RadialMenu> m_radialMenu;

    bool m_running;
    float m_lastFrameTime;
    float m_deltaTime = 0.016f;
    
    // Targeting state
    std::string m_currentTargetId;
    std::vector<std::string> m_targetList;
    int m_currentTargetIndex;
    
    // EVE-style camera control state
    bool m_rightMouseDown = false;
    bool m_leftMouseDown = false;
    double m_lastMouseDragX = 0.0;
    double m_lastMouseDragY = 0.0;

    // Atlas UI mouse consumption — set after each Atlas frame, checked by
    // game-world interaction handlers to prevent click-through
    bool m_atlasConsumedMouse = false;
    
    // EVE-style movement state
    enum class MoveCommand { None, Approach, Orbit, KeepAtRange, AlignTo, WarpTo };
    MoveCommand m_currentMoveCommand = MoveCommand::None;
    std::string m_moveTargetId;
    float m_orbitDistance = 500.0f;
    float m_keepAtRangeDistance = 2500.0f;
    glm::vec3 m_playerVelocity{0.0f};
    float m_playerSpeed = 0.0f;
    float m_playerMaxSpeed = 250.0f;
    bool m_approachActive = false;
    bool m_orbitActive = false;
    bool m_keepRangeActive = false;
    
    // Context menu state
    bool m_showContextMenu = false;
    std::string m_contextMenuEntityId;
    double m_contextMenuX = 0.0;
    double m_contextMenuY = 0.0;
    
    // Radial menu state
    bool m_radialMenuOpen = false;
    double m_radialMenuStartX = 0.0;
    double m_radialMenuStartY = 0.0;
    double m_radialMenuHoldStartTime = 0.0;
    const double RADIAL_MENU_HOLD_TIME = 0.3;  // 300ms hold to open radial menu
    
    // D-key docking mode
    bool m_dockingModeActive = false;
    
    // S-key warp mode
    bool m_warpModeActive = false;
    
    // Active movement mode indicator text
    std::string m_activeModeText;
    
    // Local/demo mode
    bool m_localPlayerSpawned = false;
    std::string m_localPlayerId = "player_local";
};

} // namespace atlas
