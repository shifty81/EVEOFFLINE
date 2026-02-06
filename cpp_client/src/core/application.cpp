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
#include "ui/inventory_panel.h"
#include "ui/fitting_panel.h"
#include "ui/market_panel.h"
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <algorithm>
#include <cmath>

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
    
    // Set up input callbacks — EVE Online style controls
    // Left-click: select/target, Double-click: approach
    // Right-click: context menu
    // Left-drag: nothing (ImGui uses it for UI interaction)
    // Right-drag: orbit camera around ship
    // Scroll: zoom camera
    m_window->setKeyCallback([this](int key, int action) {
        // Handle ESC key to exit
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            shutdown();
        }
        
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
    
    // Scroll callback — EVE uses mousewheel for camera zoom
    m_window->setScrollCallback([this](double xoffset, double yoffset) {
        handleScroll(xoffset, yoffset);
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
    
    // Spawn local player entity so ship is always visible (PVE mode)
    spawnLocalPlayerEntity();
    spawnDemoNPCEntities();
    
    // Set initial camera to orbit around player
    m_camera->setDistance(200.0f);
    m_camera->rotate(45.0f, 0.0f);
    
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
    
    // Get UI panels
    auto* inventoryPanel = m_uiManager->GetInventoryPanel();
    auto* fittingPanel = m_uiManager->GetFittingPanel();
    auto* marketPanel = m_uiManager->GetMarketPanel();
    
    // === Setup Request Callbacks (UI → Network) ===
    
    // Inventory panel drag-drop callback
    if (inventoryPanel) {
        inventoryPanel->SetDragDropCallback([networkMgr, inventoryPanel](
            const std::string& item_id, int quantity,
            bool from_cargo, bool to_cargo, bool to_space) {
            
            // Set pending state
            inventoryPanel->SetPendingOperation(true);
            
            if (to_space) {
                // Jettison operation
                networkMgr->sendInventoryJettison(item_id, quantity);
            } else {
                // Transfer operation
                networkMgr->sendInventoryTransfer(item_id, quantity, from_cargo, to_cargo);
            }
        });
        
        std::cout << "  - Inventory panel request callbacks wired" << std::endl;
    }
    
    // Fitting panel callbacks (placeholder for future fitting operations)
    if (fittingPanel) {
        // TODO: Wire fitting callbacks when fitting operations are implemented
        std::cout << "  - Fitting panel request callbacks (ready for wiring)" << std::endl;
    }
    
    // Market panel callbacks (placeholder for future market operations)
    if (marketPanel) {
        // TODO: Wire market callbacks when market operations are implemented
        std::cout << "  - Market panel request callbacks (ready for wiring)" << std::endl;
    }
    
    // === Setup Response Callbacks (Network → UI) ===
    
    // Inventory response callback
    networkMgr->setInventoryCallback([inventoryPanel](const eve::InventoryResponse& response) {
        if (!inventoryPanel) return;
        
        if (response.success) {
            std::cout << "✓ Inventory operation succeeded: " << response.message << std::endl;
            inventoryPanel->ShowSuccess(response.message);
        } else {
            std::cerr << "✗ Inventory operation failed: " << response.message << std::endl;
            inventoryPanel->ShowError(response.message);
        }
    });
    
    // Fitting response callback
    networkMgr->setFittingCallback([fittingPanel](const eve::FittingResponse& response) {
        if (!fittingPanel) return;
        
        if (response.success) {
            std::cout << "✓ Fitting operation succeeded: " << response.message << std::endl;
            fittingPanel->ShowSuccess(response.message);
        } else {
            std::cerr << "✗ Fitting operation failed: " << response.message << std::endl;
            fittingPanel->ShowError(response.message);
        }
    });
    
    // Market response callback
    networkMgr->setMarketCallback([marketPanel](const eve::MarketResponse& response) {
        if (!marketPanel) return;
        
        if (response.success) {
            std::cout << "✓ Market transaction succeeded: " << response.message << std::endl;
            marketPanel->ShowSuccess(response.message);
        } else {
            std::cerr << "✗ Market transaction failed: " << response.message << std::endl;
            marketPanel->ShowError(response.message);
        }
    });
    
    // Error response callback (general errors)
    networkMgr->setErrorCallback([](const std::string& message) {
        std::cerr << "✗ Server error: " << message << std::endl;
        // TODO: Could show a general error dialog here
    });
    
    std::cout << "  - Response callbacks wired for all panels" << std::endl;
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
    
    // Update local movement (PVE mode — EVE-style movement commands)
    updateLocalMovement(deltaTime);
    
    // Update ship status in the HUD
    auto playerEntity = m_gameClient->getEntityManager().getEntity(m_localPlayerId);
    if (playerEntity) {
        UI::ShipStatus status;
        const auto& health = playerEntity->getHealth();
        status.shields = health.currentShield;
        status.shields_max = static_cast<float>(health.maxShield);
        status.armor = health.currentArmor;
        status.armor_max = static_cast<float>(health.maxArmor);
        status.hull = health.currentHull;
        status.hull_max = static_cast<float>(health.maxHull);
        status.capacitor = 85.0f;     // Demo value
        status.capacitor_max = 100.0f;
        status.velocity = m_playerSpeed;
        status.max_velocity = m_playerMaxSpeed;
        m_uiManager->SetShipStatus(status);
        
        // Camera follows player ship
        m_camera->setTarget(playerEntity->getPosition());
    }
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
    
    // EVE-style right-click context menu
    if (m_showContextMenu) {
        ImGui::OpenPopup("##SpaceContextMenu");
        m_showContextMenu = false;
    }
    
    if (ImGui::BeginPopup("##SpaceContextMenu")) {
        if (!m_contextMenuEntityId.empty()) {
            // Entity context menu — PVE actions
            auto entity = m_gameClient->getEntityManager().getEntity(m_contextMenuEntityId);
            if (entity) {
                ImGui::TextColored(ImVec4(0.27f, 0.82f, 0.91f, 1.0f), "%s",
                                   entity->getShipName().empty() ? entity->getId().c_str() : entity->getShipName().c_str());
                ImGui::Separator();
                
                if (ImGui::MenuItem("Approach")) {
                    commandApproach(m_contextMenuEntityId);
                }
                if (ImGui::MenuItem("Orbit (500m)")) {
                    commandOrbit(m_contextMenuEntityId, 500.0f);
                }
                if (ImGui::MenuItem("Keep at Range (2500m)")) {
                    commandKeepAtRange(m_contextMenuEntityId, 2500.0f);
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Lock Target")) {
                    targetEntity(m_contextMenuEntityId, true);
                }
                if (ImGui::MenuItem("Align To")) {
                    commandAlignTo(m_contextMenuEntityId);
                }
                if (ImGui::MenuItem("Warp To")) {
                    commandWarpTo(m_contextMenuEntityId);
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Show Info")) {
                    // Placeholder for info panel
                    std::cout << "[Info] Entity: " << m_contextMenuEntityId << std::endl;
                }
            }
        } else {
            // Empty space context menu
            ImGui::TextColored(ImVec4(0.55f, 0.58f, 0.62f, 1.0f), "Space");
            ImGui::Separator();
            if (ImGui::MenuItem("Stop Ship")) {
                commandStopShip();
            }
            ImGui::Separator();
            if (ImGui::BeginMenu("Panels")) {
                if (ImGui::MenuItem("Overview")) { m_uiManager->ToggleOverview(); }
                if (ImGui::MenuItem("Inventory")) { m_uiManager->ToggleInventory(); }
                if (ImGui::MenuItem("Fitting")) { m_uiManager->ToggleFitting(); }
                if (ImGui::MenuItem("Market")) { m_uiManager->ToggleMarket(); }
                if (ImGui::MenuItem("Missions")) { m_uiManager->ToggleMission(); }
                ImGui::EndMenu();
            }
            if (ImGui::MenuItem("Lock Interface")) {
                m_uiManager->ToggleInterfaceLock();
            }
        }
        ImGui::EndPopup();
    }
    
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
    
    // Module activation (F1-F8) — EVE standard
    if (key >= GLFW_KEY_F1 && key <= GLFW_KEY_F8) {
        int slot = key - GLFW_KEY_F1 + 1;  // F1 = slot 1
        activateModule(slot);
        return;
    }
    
    // Tab — cycle targets (EVE standard)
    if (key == GLFW_KEY_TAB) {
        cycleTarget();
        return;
    }
    
    // CTRL+SPACE — stop ship (EVE standard)
    if (key == GLFW_KEY_SPACE && (mods & GLFW_MOD_CONTROL)) {
        commandStopShip();
        return;
    }
    
    // EVE-style shortcut keys with modifier:
    // Q + click = Approach (we just toggle approach mode)
    // W + click = Orbit
    // E + click = Keep at Range
    // A + click = Align To
    if (key == GLFW_KEY_Q) {
        m_uiManager->approach_active = true;
        m_uiManager->orbit_active = false;
        m_uiManager->keep_range_active = false;
        std::cout << "[Controls] Approach mode active — click a target" << std::endl;
    } else if (key == GLFW_KEY_W) {
        m_uiManager->approach_active = false;
        m_uiManager->orbit_active = true;
        m_uiManager->keep_range_active = false;
        std::cout << "[Controls] Orbit mode active — click a target" << std::endl;
    } else if (key == GLFW_KEY_E) {
        m_uiManager->approach_active = false;
        m_uiManager->orbit_active = false;
        m_uiManager->keep_range_active = true;
        std::cout << "[Controls] Keep at Range mode active — click a target" << std::endl;
    } else if (key == GLFW_KEY_S && (mods & GLFW_MOD_CONTROL)) {
        commandStopShip();
    }
    
    // Panel toggles
    if (key == GLFW_KEY_I && (mods & GLFW_MOD_ALT)) {
        m_uiManager->ToggleInventory();
    } else if (key == GLFW_KEY_F && (mods & GLFW_MOD_ALT)) {
        m_uiManager->ToggleFitting();
    } else if (key == GLFW_KEY_O && (mods & GLFW_MOD_ALT)) {
        m_uiManager->ToggleOverview();
    } else if (key == GLFW_KEY_R && (mods & GLFW_MOD_ALT)) {
        m_uiManager->ToggleMarket();
    } else if (key == GLFW_KEY_J && (mods & GLFW_MOD_ALT)) {
        m_uiManager->ToggleMission();
    }
    
    // L — Toggle interface lock
    if (key == GLFW_KEY_L && (mods & GLFW_MOD_CONTROL)) {
        m_uiManager->ToggleInterfaceLock();
    }
}

void Application::handleMouseButton(int button, int action, int mods, double x, double y) {
    // Track button state for camera control
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) {
            m_rightMouseDown = true;
            m_lastMouseDragX = x;
            m_lastMouseDragY = y;
        } else if (action == GLFW_RELEASE) {
            // If right-click was a quick click (not a drag), show context menu
            if (m_rightMouseDown) {
                double dx = x - m_lastMouseDragX;
                double dy = y - m_lastMouseDragY;
                double dist = std::sqrt(dx * dx + dy * dy);
                if (dist < 5.0) {
                    // Quick right-click — show EVE context menu
                    showSpaceContextMenu(x, y);
                }
            }
            m_rightMouseDown = false;
        }
    }
    
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            m_leftMouseDown = true;
        } else if (action == GLFW_RELEASE) {
            m_leftMouseDown = false;
        }
    }
    
    // Left-click: select entity / apply movement command
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        // Don't process clicks that ImGui captured
        ImGuiIO& io = ImGui::GetIO();
        if (io.WantCaptureMouse) return;
        
        // Pick entity at mouse position
        auto entities = m_gameClient->getEntityManager().getAllEntities();
        std::vector<std::shared_ptr<Entity>> entityList;
        for (const auto& pair : entities) {
            if (pair.first != m_localPlayerId) {  // Don't pick yourself
                entityList.push_back(pair.second);
            }
        }
        
        std::string pickedEntityId = m_entityPicker->pickEntity(
            x, y,
            m_window->getWidth(), m_window->getHeight(),
            *m_camera,
            entityList
        );
        
        if (!pickedEntityId.empty()) {
            // EVE-style: Apply pending movement command if one is active
            if (m_uiManager->approach_active) {
                commandApproach(pickedEntityId);
                m_uiManager->approach_active = false;
            } else if (m_uiManager->orbit_active) {
                commandOrbit(pickedEntityId);
                m_uiManager->orbit_active = false;
            } else if (m_uiManager->keep_range_active) {
                commandKeepAtRange(pickedEntityId);
                m_uiManager->keep_range_active = false;
            } else {
                // Default: select / CTRL+click to lock target
                bool addToTargets = (mods & GLFW_MOD_CONTROL) != 0;
                targetEntity(pickedEntityId, addToTargets);
            }
        }
    }
}

void Application::handleMouseMove(double x, double y, double deltaX, double deltaY) {
    // EVE-style camera: Right-click drag to orbit camera around ship
    if (m_rightMouseDown) {
        ImGuiIO& io = ImGui::GetIO();
        if (!io.WantCaptureMouse) {
            float sensitivity = 0.3f;
            m_camera->rotate(static_cast<float>(deltaX) * sensitivity,
                           static_cast<float>(-deltaY) * sensitivity);
        }
    }
}

void Application::handleScroll(double xoffset, double yoffset) {
    // EVE-style: mousewheel zooms camera
    ImGuiIO& io = ImGui::GetIO();
    if (!io.WantCaptureMouse) {
        m_camera->zoom(static_cast<float>(yoffset));
    }
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

// === EVE-style movement commands (PVE) ===

void Application::showSpaceContextMenu(double x, double y) {
    // Check if clicking on an entity
    auto entities = m_gameClient->getEntityManager().getAllEntities();
    std::vector<std::shared_ptr<Entity>> entityList;
    for (const auto& pair : entities) {
        if (pair.first != m_localPlayerId) {
            entityList.push_back(pair.second);
        }
    }
    
    std::string pickedId = m_entityPicker->pickEntity(
        x, y, m_window->getWidth(), m_window->getHeight(),
        *m_camera, entityList);
    
    m_contextMenuEntityId = pickedId;
    m_contextMenuX = x;
    m_contextMenuY = y;
    m_showContextMenu = true;
}

void Application::showEntityContextMenu(const std::string& entityId, double x, double y) {
    m_contextMenuEntityId = entityId;
    m_contextMenuX = x;
    m_contextMenuY = y;
    m_showContextMenu = true;
}

void Application::commandApproach(const std::string& entityId) {
    m_currentMoveCommand = MoveCommand::Approach;
    m_moveTargetId = entityId;
    std::cout << "[Movement] Approaching " << entityId << std::endl;
}

void Application::commandOrbit(const std::string& entityId, float distance) {
    m_currentMoveCommand = MoveCommand::Orbit;
    m_moveTargetId = entityId;
    m_orbitDistance = distance;
    std::cout << "[Movement] Orbiting " << entityId << " at " << distance << "m" << std::endl;
}

void Application::commandKeepAtRange(const std::string& entityId, float distance) {
    m_currentMoveCommand = MoveCommand::KeepAtRange;
    m_moveTargetId = entityId;
    m_keepAtRangeDistance = distance;
    std::cout << "[Movement] Keeping at range " << distance << "m from " << entityId << std::endl;
}

void Application::commandAlignTo(const std::string& entityId) {
    m_currentMoveCommand = MoveCommand::AlignTo;
    m_moveTargetId = entityId;
    std::cout << "[Movement] Aligning to " << entityId << std::endl;
}

void Application::commandWarpTo(const std::string& entityId) {
    m_currentMoveCommand = MoveCommand::WarpTo;
    m_moveTargetId = entityId;
    std::cout << "[Movement] Warping to " << entityId << std::endl;
}

void Application::commandStopShip() {
    m_currentMoveCommand = MoveCommand::None;
    m_moveTargetId.clear();
    m_playerVelocity = glm::vec3(0.0f);
    m_playerSpeed = 0.0f;
    m_uiManager->approach_active = false;
    m_uiManager->orbit_active = false;
    m_uiManager->keep_range_active = false;
    std::cout << "[Movement] Ship stopped" << std::endl;
}

void Application::spawnLocalPlayerEntity() {
    if (m_localPlayerSpawned) return;
    
    std::cout << "[PVE] Spawning local player ship..." << std::endl;
    
    // Create player entity at origin with a Rifter (Minmatar frigate)
    Health playerHealth(1500, 800, 500);  // Shield, Armor, Hull
    m_gameClient->getEntityManager().spawnEntity(
        m_localPlayerId,
        glm::vec3(0.0f, 0.0f, 0.0f),
        playerHealth,
        "Rifter",
        "Your Ship",
        "Minmatar"
    );
    
    m_localPlayerSpawned = true;
    std::cout << "[PVE] Local player ship spawned as Rifter" << std::endl;
}

void Application::spawnDemoNPCEntities() {
    std::cout << "[PVE] Spawning demo NPC entities..." << std::endl;
    
    // Spawn some NPC enemies for the PVE demo
    // These would normally come from the server in missions/anomalies
    
    // Blood Raider pirate (hostile NPC)
    Health npc1Health(800, 600, 400);
    m_gameClient->getEntityManager().spawnEntity(
        "npc_raider_1",
        glm::vec3(300.0f, 10.0f, 200.0f),
        npc1Health,
        "Cruiser",
        "Blood Raider",
        "Blood Raiders"
    );
    
    // Serpentis frigate
    Health npc2Health(400, 300, 200);
    m_gameClient->getEntityManager().spawnEntity(
        "npc_serp_1",
        glm::vec3(-250.0f, -5.0f, 350.0f),
        npc2Health,
        "Frigate",
        "Serpentis Scout",
        "Serpentis"
    );
    
    // Guristas destroyer
    Health npc3Health(600, 500, 350);
    m_gameClient->getEntityManager().spawnEntity(
        "npc_gur_1",
        glm::vec3(150.0f, 20.0f, -300.0f),
        npc3Health,
        "Destroyer",
        "Guristas Watchman",
        "Guristas"
    );
    
    std::cout << "[PVE] 3 NPC entities spawned" << std::endl;
}

void Application::updateLocalMovement(float deltaTime) {
    auto playerEntity = m_gameClient->getEntityManager().getEntity(m_localPlayerId);
    if (!playerEntity) return;
    
    glm::vec3 playerPos = playerEntity->getPosition();
    float accel = 80.0f;  // Acceleration m/s²
    float decel = 40.0f;  // Deceleration when stopping
    
    if (m_currentMoveCommand == MoveCommand::None) {
        // Decelerate to stop
        if (m_playerSpeed > 0.0f) {
            m_playerSpeed = std::max(0.0f, m_playerSpeed - decel * deltaTime);
            playerPos += m_playerVelocity * deltaTime;
        }
    } else {
        // Get target position
        auto targetEntity = m_gameClient->getEntityManager().getEntity(m_moveTargetId);
        if (!targetEntity) {
            m_currentMoveCommand = MoveCommand::None;
            return;
        }
        
        glm::vec3 targetPos = targetEntity->getPosition();
        glm::vec3 toTarget = targetPos - playerPos;
        float dist = glm::length(toTarget);
        
        if (dist < 0.01f) return;  // Already at target
        
        glm::vec3 dir = glm::normalize(toTarget);
        
        switch (m_currentMoveCommand) {
            case MoveCommand::Approach: {
                // Accelerate towards target, slow down when close
                float targetSpeed = m_playerMaxSpeed;
                if (dist < 50.0f) {
                    targetSpeed = m_playerMaxSpeed * (dist / 50.0f);
                }
                m_playerSpeed = std::min(m_playerMaxSpeed,
                                         m_playerSpeed + accel * deltaTime);
                m_playerSpeed = std::min(m_playerSpeed, targetSpeed);
                m_playerVelocity = dir * m_playerSpeed;
                break;
            }
            case MoveCommand::Orbit: {
                // Orbit around target at set distance
                m_playerSpeed = std::min(m_playerMaxSpeed,
                                         m_playerSpeed + accel * deltaTime);
                if (dist > m_orbitDistance + 10.0f) {
                    m_playerVelocity = dir * m_playerSpeed;
                } else if (dist < m_orbitDistance - 10.0f) {
                    m_playerVelocity = -dir * m_playerSpeed * 0.5f;
                } else {
                    // Orbit tangent
                    glm::vec3 tangent(-dir.z, 0.0f, dir.x);
                    m_playerVelocity = tangent * m_playerSpeed;
                }
                break;
            }
            case MoveCommand::KeepAtRange: {
                m_playerSpeed = std::min(m_playerMaxSpeed,
                                         m_playerSpeed + accel * deltaTime);
                if (dist > m_keepAtRangeDistance + 20.0f) {
                    m_playerVelocity = dir * m_playerSpeed;
                } else if (dist < m_keepAtRangeDistance - 20.0f) {
                    m_playerVelocity = -dir * m_playerSpeed * 0.3f;
                } else {
                    m_playerSpeed = std::max(0.0f, m_playerSpeed - decel * deltaTime);
                    m_playerVelocity = dir * m_playerSpeed;
                }
                break;
            }
            case MoveCommand::AlignTo: {
                m_playerSpeed = std::min(m_playerMaxSpeed * 0.75f,
                                         m_playerSpeed + accel * deltaTime);
                m_playerVelocity = dir * m_playerSpeed;
                break;
            }
            case MoveCommand::WarpTo: {
                // Warp = very fast movement, simulated
                float warpSpeed = 5000.0f;
                m_playerSpeed = std::min(warpSpeed,
                                         m_playerSpeed + warpSpeed * deltaTime);
                m_playerVelocity = dir * m_playerSpeed;
                if (dist < 100.0f) {
                    m_currentMoveCommand = MoveCommand::None;
                    m_playerSpeed = 0.0f;
                    m_playerVelocity = glm::vec3(0.0f);
                    std::cout << "[Movement] Warp complete" << std::endl;
                }
                break;
            }
            default:
                break;
        }
        
        playerPos += m_playerVelocity * deltaTime;
    }
    
    // Update player entity position
    float rotation = 0.0f;
    if (glm::length(m_playerVelocity) > 0.1f) {
        rotation = std::atan2(m_playerVelocity.x, m_playerVelocity.z);
    }
    
    Health currentHealth = playerEntity->getHealth();
    m_gameClient->getEntityManager().updateEntityState(
        m_localPlayerId, playerPos, m_playerVelocity, rotation, currentHealth);
}

} // namespace eve
