#include "core/application.h"
#include "rendering/window.h"
#include "rendering/renderer.h"
#include "rendering/camera.h"
#include "core/game_client.h"
#include "core/entity.h"
#include "core/embedded_server.h"
#include "core/session_manager.h"
#include "ui/input_handler.h"
#include "ui/ui_manager.h"
#include "ui/entity_picker.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <algorithm>

namespace eve {

Application* Application::s_instance = nullptr;

Application::Application(const std::string& title, int width, int height)
    : m_running(false)
    , m_lastFrameTime(0.0f)
    , m_currentTargetIndex(-1)
{
    if (s_instance != nullptr) {
        throw std::runtime_error("Application already exists");
    }
    s_instance = this;
    
    std::cout << "Creating application: " << title << std::endl;
    
    // Create window
    m_window = std::make_unique<Window>(title, width, height);
    
    // Create subsystems
    m_renderer = std::make_unique<Renderer>();
    m_gameClient = std::make_unique<GameClient>();
    m_inputHandler = std::make_unique<InputHandler>();
    m_camera = std::make_unique<Camera>(45.0f, static_cast<float>(width) / height, 0.1f, 10000.0f);
    m_embeddedServer = std::make_unique<EmbeddedServer>();
    m_sessionManager = std::make_unique<SessionManager>();
    m_uiManager = std::make_unique<UI::UIManager>();
    m_entityPicker = std::make_unique<UI::EntityPicker>();
    
    // Initialize
    initialize();
}

Application::~Application() {
    cleanup();
    s_instance = nullptr;
    std::cout << "Application destroyed" << std::endl;
}

void Application::run() {
    std::cout << "Starting main loop..." << std::endl;
    m_running = true;
    m_lastFrameTime = static_cast<float>(glfwGetTime());
    
    // Main game loop
    while (m_running && !m_window->shouldClose()) {
        // Calculate delta time
        float currentTime = static_cast<float>(glfwGetTime());
        float deltaTime = currentTime - m_lastFrameTime;
        m_lastFrameTime = currentTime;
        
        // Update
        update(deltaTime);
        
        // Render
        render();
        
        // Swap buffers and poll events
        m_window->update();
    }
    
    std::cout << "Main loop ended" << std::endl;
}

void Application::shutdown() {
    std::cout << "Shutdown requested" << std::endl;
    m_running = false;
}

void Application::initialize() {
    std::cout << "Initializing application..." << std::endl;
    
    // Initialize renderer
    if (!m_renderer->initialize()) {
        throw std::runtime_error("Failed to initialize renderer");
    }
    
    // Initialize UI manager
    if (!m_uiManager->Initialize(m_window->getHandle())) {
        throw std::runtime_error("Failed to initialize UI manager");
    }
    
    // Set up input callbacks using lambdas to bind to member functions
    m_window->setKeyCallback([this](int key, int action) {
        // Handle ESC key to exit
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            shutdown();
        }
        
        // Forward to input handler (will be updated to handle mods)
        m_inputHandler->handleKey(key, action, 0);
    });
    
    m_window->setMouseCallback([this](double xpos, double ypos) {
        m_inputHandler->handleMouse(xpos, ypos);
    });
    
    m_window->setMouseButtonCallback([this](int button, int action, int mods) {
        double x = m_inputHandler->getMouseX();
        double y = m_inputHandler->getMouseY();
        m_inputHandler->handleMouseButton(button, action, mods, x, y);
    });
    
    m_window->setResizeCallback([this](int width, int height) {
        m_renderer->setViewport(0, 0, width, height);
    });
    
    // Register input handler callbacks
    m_inputHandler->setKeyCallback([this](int key, int action, int mods) {
        handleKeyInput(key, action, mods);
    });
    
    m_inputHandler->setMouseButtonCallback([this](int button, int action, int mods, double x, double y) {
        handleMouseButton(button, action, mods, x, y);
    });
    
    m_inputHandler->setMouseMoveCallback([this](double x, double y, double deltaX, double deltaY) {
        handleMouseMove(x, y, deltaX, deltaY);
    });
    
    // Set initial viewport
    m_renderer->setViewport(0, 0, m_window->getWidth(), m_window->getHeight());
    
    // Set up entity event callbacks
    m_gameClient->setOnEntitySpawned([this](const std::shared_ptr<Entity>& entity) {
        std::cout << "Application: Entity spawned event received" << std::endl;
        m_renderer->createEntityVisual(entity);
    });
    
    m_gameClient->setOnEntityDestroyed([this](const std::shared_ptr<Entity>& entity) {
        std::cout << "Application: Entity destroyed event received" << std::endl;
        m_renderer->removeEntityVisual(entity->getId());
    });
    
    // Setup UI callbacks for network integration
    setupUICallbacks();
    
    std::cout << "Application initialized successfully" << std::endl;
}

void Application::setupUICallbacks() {
    std::cout << "Setting up UI callbacks for network integration..." << std::endl;
    
    // Get network manager from game client
    auto* networkMgr = m_gameClient->getNetworkManager();
    if (!networkMgr) {
        std::cout << "NetworkManager not available yet, skipping UI callback setup" << std::endl;
        return;
    }
    
    // Setup inventory panel callbacks
    auto* inventoryPanel = m_uiManager->GetInventoryPanel();
    if (inventoryPanel) {
        inventoryPanel->SetDragDropCallback([networkMgr](
            const std::string& item_id, int quantity,
            bool from_cargo, bool to_cargo, bool to_space) {
            
            if (to_space) {
                // Jettison operation
                networkMgr->sendInventoryJettison(item_id, quantity);
            } else {
                // Transfer operation
                networkMgr->sendInventoryTransfer(item_id, quantity, from_cargo, to_cargo);
            }
        });
        
        std::cout << "  - Inventory panel callbacks wired" << std::endl;
    }
    
    // Setup fitting panel callbacks
    auto* fittingPanel = m_uiManager->GetFittingPanel();
    if (fittingPanel) {
        // Note: FittingPanel would need callback methods added similar to inventory
        // This is a placeholder for future integration
        std::cout << "  - Fitting panel callbacks (ready for wiring)" << std::endl;
    }
    
    std::cout << "UI callbacks setup complete" << std::endl;
}

void Application::update(float deltaTime) {
    // Update embedded server if running
    if (m_embeddedServer) {
        m_embeddedServer->update(deltaTime);
    }
    
    // Update session manager
    if (m_sessionManager) {
        m_sessionManager->update(deltaTime);
    }
    
    // Update game client
    m_gameClient->update(deltaTime);
    
    // Can add more update logic here
}

void Application::render() {
    // Clear screen
    m_renderer->clear(glm::vec4(0.01f, 0.01f, 0.05f, 1.0f));
    
    // Begin rendering
    m_renderer->beginFrame();
    
    // Update camera aspect ratio
    m_camera->setAspectRatio(m_window->getAspectRatio());
    m_camera->update(0.016f);
    
    // Update entity visuals from game client
    m_renderer->updateEntityVisuals(m_gameClient->getEntityManager().getAllEntities());
    
    // Render scene
    m_renderer->renderScene(*m_camera);
    
    // Render UI
    m_uiManager->BeginFrame();
    m_uiManager->UpdateTargets(m_gameClient->getEntityManager().getAllEntities());
    m_uiManager->Render();
    m_uiManager->EndFrame();
    
    // End rendering
    m_renderer->endFrame();
}

void Application::cleanup() {
    std::cout << "Cleaning up application..." << std::endl;
    
    // Leave session and stop server if hosting
    if (m_sessionManager) {
        m_sessionManager->leaveSession();
    }
    
    if (m_embeddedServer && m_embeddedServer->isRunning()) {
        m_embeddedServer->stop();
    }
    
    // Disconnect from server if connected
    if (m_gameClient) {
        m_gameClient->disconnect();
    }
    
    std::cout << "Cleanup complete" << std::endl;
}

bool Application::hostMultiplayerGame(const std::string& sessionName, int maxPlayers) {
    std::cout << "Hosting multiplayer game: " << sessionName << std::endl;
    
    // Configure embedded server
    EmbeddedServer::Config serverConfig;
    serverConfig.server_name = sessionName;
    serverConfig.description = "EVE OFFLINE Hosted Game";
    serverConfig.port = 8765;
    serverConfig.max_players = maxPlayers;
    serverConfig.lan_only = true;
    serverConfig.persistent_world = false;
    
    // Start embedded server
    if (!m_embeddedServer->start(serverConfig)) {
        std::cerr << "Failed to start embedded server!" << std::endl;
        return false;
    }
    
    // Configure session
    SessionManager::SessionConfig sessionConfig;
    sessionConfig.session_name = sessionName;
    sessionConfig.max_players = maxPlayers;
    sessionConfig.lan_only = true;
    
    // Host session
    if (!m_sessionManager->hostSession(sessionConfig, m_embeddedServer.get())) {
        std::cerr << "Failed to host session!" << std::endl;
        m_embeddedServer->stop();
        return false;
    }
    
    // Auto-connect to own server
    std::string localAddress = m_embeddedServer->getLocalAddress();
    int port = m_embeddedServer->getPort();
    
    // Give server a moment to start
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    if (!m_gameClient->connect(localAddress, port)) {
        std::cerr << "Failed to connect to own server!" << std::endl;
        m_sessionManager->leaveSession();
        m_embeddedServer->stop();
        return false;
    }
    
    std::cout << "Successfully hosting multiplayer game!" << std::endl;
    std::cout << "Other players can connect to: " << localAddress << ":" << port << std::endl;
    
    return true;
}

bool Application::joinMultiplayerGame(const std::string& host, int port) {
    std::cout << "Joining multiplayer game at " << host << ":" << port << std::endl;
    
    // Connect to remote server
    if (!m_gameClient->connect(host, port)) {
        std::cerr << "Failed to connect to server!" << std::endl;
        return false;
    }
    
    // Join session
    if (!m_sessionManager->joinSession(host, port)) {
        std::cerr << "Failed to join session!" << std::endl;
        m_gameClient->disconnect();
        return false;
    }
    
    std::cout << "Successfully joined multiplayer game!" << std::endl;
    return true;
}

bool Application::isHosting() const {
    return m_embeddedServer && m_embeddedServer->isRunning();
}

void Application::handleKeyInput(int key, int action, int mods) {
    // Only handle PRESS events for most keys
    if (action != GLFW_PRESS) {
        return;
    }
    
    // Module activation (F1-F8)
    if (key >= GLFW_KEY_F1 && key <= GLFW_KEY_F8) {
        int slot = key - GLFW_KEY_F1 + 1;  // F1 = slot 1
        activateModule(slot);
        return;
    }
    
    // Target cycling
    if (key == GLFW_KEY_TAB) {
        cycleTarget();
        return;
    }
    
    // Clear target
    if (key == GLFW_KEY_ESCAPE) {
        clearTarget();
        return;
    }
    
    // Movement keys (placeholder for future implementation)
    if (key == GLFW_KEY_W) {
        std::cout << "[Input] W pressed - Approach" << std::endl;
    } else if (key == GLFW_KEY_A) {
        std::cout << "[Input] A pressed - Orbit left" << std::endl;
    } else if (key == GLFW_KEY_D) {
        std::cout << "[Input] D pressed - Orbit right" << std::endl;
    } else if (key == GLFW_KEY_S) {
        std::cout << "[Input] S pressed - Stop ship" << std::endl;
    }
}

void Application::handleMouseButton(int button, int action, int mods, double x, double y) {
    // Only handle left click press
    if (button != GLFW_MOUSE_BUTTON_LEFT || action != GLFW_PRESS) {
        return;
    }
    
    // Get all entities
    auto entities = m_gameClient->getEntityManager().getAllEntities();
    std::vector<std::shared_ptr<Entity>> entityList;
    for (const auto& pair : entities) {
        entityList.push_back(pair.second);
    }
    
    // Pick entity at mouse position
    std::string pickedEntityId = m_entityPicker->pickEntity(
        x, y,
        m_window->getWidth(), m_window->getHeight(),
        *m_camera,
        entityList
    );
    
    if (!pickedEntityId.empty()) {
        // Check if CTRL is pressed for multi-target
        bool addToTargets = (mods & GLFW_MOD_CONTROL) != 0;
        targetEntity(pickedEntityId, addToTargets);
    }
}

void Application::handleMouseMove(double x, double y, double deltaX, double deltaY) {
    // Camera control could be added here
    // For now, just track mouse position (already done by InputHandler)
}

void Application::targetEntity(const std::string& entityId, bool addToTargets) {
    if (entityId.empty()) {
        return;
    }
    
    std::cout << "[Targeting] Target entity: " << entityId;
    if (addToTargets) {
        std::cout << " (add to targets)";
    }
    std::cout << std::endl;
    
    if (addToTargets) {
        // Add to target list if not already present
        auto it = std::find(m_targetList.begin(), m_targetList.end(), entityId);
        if (it == m_targetList.end()) {
            m_targetList.push_back(entityId);
            m_uiManager->AddTarget(entityId);
        }
    } else {
        // Replace current target
        m_currentTargetId = entityId;
        m_targetList.clear();
        m_targetList.push_back(entityId);
        m_currentTargetIndex = 0;
        
        // Update UI
        m_uiManager->RemoveTarget("");  // Clear all (empty string)
        m_uiManager->AddTarget(entityId);
    }
}

void Application::clearTarget() {
    std::cout << "[Targeting] Clear target" << std::endl;
    
    m_currentTargetId.clear();
    m_targetList.clear();
    m_currentTargetIndex = -1;
    
    // Update UI
    m_uiManager->RemoveTarget("");  // Clear all
}

void Application::cycleTarget() {
    if (m_targetList.empty()) {
        std::cout << "[Targeting] No targets to cycle" << std::endl;
        return;
    }
    
    // Move to next target
    m_currentTargetIndex = (m_currentTargetIndex + 1) % m_targetList.size();
    m_currentTargetId = m_targetList[m_currentTargetIndex];
    
    std::cout << "[Targeting] Cycle to target: " << m_currentTargetId 
              << " (" << (m_currentTargetIndex + 1) << "/" << m_targetList.size() << ")" << std::endl;
}

void Application::activateModule(int slotNumber) {
    if (slotNumber < 1 || slotNumber > 8) {
        return;
    }
    
    std::cout << "[Modules] Activate module in slot " << slotNumber;
    if (!m_currentTargetId.empty()) {
        std::cout << " on target: " << m_currentTargetId;
    }
    std::cout << std::endl;
    
    // Send module activation command to server
    auto* networkMgr = m_gameClient->getNetworkManager();
    if (networkMgr && networkMgr->isConnected()) {
        networkMgr->sendModuleActivate(slotNumber - 1);  // Convert to 0-based index
    } else {
        std::cout << "[Modules] Not connected to server, activation not sent" << std::endl;
    }
}

} // namespace eve
