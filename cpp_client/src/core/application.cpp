#include "core/application.h"
#include "rendering/window.h"
#include "rendering/renderer.h"
#include "rendering/camera.h"
#include "core/game_client.h"
#include "core/entity.h"
#include "core/embedded_server.h"
#include "core/session_manager.h"
#include "core/solar_system_scene.h"
#include "core/ship_physics.h"
#include "ui/input_handler.h"
#include "ui/rml_ui_manager.h"
#include "ui/entity_picker.h"
#include "ui/context_menu.h"
#include "ui/radial_menu.h"
#include <GLFW/glfw3.h>
#include "ui/atlas/atlas_context.h"
#include "ui/atlas/atlas_hud.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <algorithm>
#include <cmath>

namespace atlas {

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
    m_uiManager = std::make_unique<UI::RmlUiManager>();
    m_entityPicker = std::make_unique<UI::EntityPicker>();
    m_solarSystem = std::make_unique<SolarSystemScene>();
    m_shipPhysics = std::make_unique<ShipPhysics>();
    m_atlasCtx = std::make_unique<atlas::AtlasContext>();
    m_atlasHUD = std::make_unique<atlas::AtlasHUD>();
    m_contextMenu = std::make_unique<UI::ContextMenu>();
    m_radialMenu = std::make_unique<UI::RadialMenu>();
    
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
        m_deltaTime = deltaTime;

        // Reset per-frame input state before polling events.
        // Clear previous frame's Atlas mouse consumption so stale state
        // doesn't block camera rotation during this frame's input phase.
        m_inputHandler->beginFrame();
        m_atlasConsumedMouse = false;

        // Poll events so transient input flags (clicked, released) are
        // available during update and render within the same frame
        m_window->pollEvents();
        
        // Update
        update(deltaTime);
        
        // Render
        render();
        
        // Present the frame
        m_window->swapBuffers();
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
    
    // Initialize UI manager (non-fatal — Atlas HUD provides fallback UI)
    if (!m_uiManager->Initialize(m_window->getHandle(), "ui_resources")) {
        std::cerr << "Warning: RmlUi UI manager initialization failed. "
                  << "Falling back to Atlas HUD-only mode. Some UI panels may be unavailable." << std::endl;
    }
    
    // Initialize Atlas UI context
    m_atlasCtx->init();
    m_atlasHUD->init(m_window->getWidth(), m_window->getHeight());
    
    // Wire Atlas sidebar icon callbacks so clicking sidebar opens panels
    m_atlasHUD->setSidebarCallback([this](int icon) {
        std::cout << "[Neocom] Sidebar icon " << icon << " clicked" << std::endl;
        switch (icon) {
            case -1:
                std::cout << "[Neocom] Toggle Character Sheet" << std::endl;
                m_atlasHUD->toggleCharacter();
                break;
            case 0:
                std::cout << "[Neocom] Toggle Inventory" << std::endl;
                m_atlasHUD->toggleInventory();
                break;
            case 1:
                std::cout << "[Neocom] Toggle Fitting" << std::endl;
                m_atlasHUD->toggleFitting();
                break;
            case 2:
                std::cout << "[Neocom] Toggle Market" << std::endl;
                m_atlasHUD->toggleMarket();
                break;
            case 3:
                std::cout << "[Neocom] Toggle Missions" << std::endl;
                m_atlasHUD->toggleMission();
                break;
            case 4:
                std::cout << "[Neocom] Toggle D-Scan" << std::endl;
                m_atlasHUD->toggleDScan();
                break;
            case 5:
                std::cout << "[Neocom] Toggle Overview" << std::endl;
                m_atlasHUD->toggleOverview();
                break;
            case 6:
                std::cout << "[Neocom] Toggle Chat" << std::endl;
                m_atlasHUD->toggleChat();
                break;
            case 7:
                std::cout << "[Neocom] Toggle Drones" << std::endl;
                m_atlasHUD->toggleDronePanel();
                break;
        }
    });
    
    // Set up input callbacks — EVE Online style controls
    // Left-click: select/target, Double-click: approach
    // Right-click: context menu
    // Left-drag: nothing (UI uses it for interaction)
    // Right-drag: orbit camera around ship
    // Scroll: zoom camera
    m_window->setKeyCallback([this](int key, int, int action, int mods) {
        // Handle ESC key to exit
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            shutdown();
        }

        if (m_uiManager) {
            m_uiManager->HandleKey(key, action, mods);
        }

        m_inputHandler->handleKey(key, action, mods);
    });

    m_window->setCharCallback([this](unsigned int codepoint) {
        if (m_uiManager) {
            m_uiManager->HandleChar(codepoint);
        }
    });
    
    m_window->setMouseCallback([this](double xpos, double ypos) {
        if (m_uiManager) {
            m_uiManager->HandleCursorPos(xpos, ypos);
        }
        m_inputHandler->handleMouse(xpos, ypos);
    });
    
    m_window->setMouseButtonCallback([this](int button, int action, int mods) {
        if (m_uiManager) {
            m_uiManager->HandleMouseButton(button, action, mods);
        }
        double x = m_inputHandler->getMouseX();
        double y = m_inputHandler->getMouseY();
        m_inputHandler->handleMouseButton(button, action, mods, x, y);
    });
    
    // Scroll callback — EVE uses mousewheel for camera zoom
    m_window->setScrollCallback([this](double xoffset, double yoffset) {
        int mods = m_inputHandler->getModifierMask();
        if (m_uiManager) {
            m_uiManager->HandleScroll(yoffset, mods);
        }
        m_inputHandler->handleScroll(xoffset, yoffset);
        handleScroll(xoffset, yoffset);
    });
    
    m_window->setResizeCallback([this](int width, int height) {
        m_renderer->setViewport(0, 0, width, height);
        if (m_uiManager) {
            m_uiManager->HandleFramebufferSize(width, height);
        }
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
    
    // Load the solar system (with sun, planets, stations etc.)
    m_solarSystem->loadTestSystem();
    
    // Set up sun rendering from solar system data
    const auto* sun = m_solarSystem->getSun();
    if (sun) {
        m_renderer->setSunState(sun->position, sun->lightColor, sun->radius);
        std::cout << "[PVE] Sun configured at origin with radius " << sun->radius << "m" << std::endl;
    }
    
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
    
    // === Setup Response Callbacks (Network → UI) ===
    networkMgr->setInventoryCallback([](const atlas::InventoryResponse& response) {
        if (response.success) {
            std::cout << "✓ Inventory operation succeeded: " << response.message << std::endl;
        } else {
            std::cerr << "✗ Inventory operation failed: " << response.message << std::endl;
        }
    });

    networkMgr->setFittingCallback([](const atlas::FittingResponse& response) {
        if (response.success) {
            std::cout << "✓ Fitting operation succeeded: " << response.message << std::endl;
        } else {
            std::cerr << "✗ Fitting operation failed: " << response.message << std::endl;
        }
    });

    networkMgr->setMarketCallback([](const atlas::MarketResponse& response) {
        if (response.success) {
            std::cout << "✓ Market transaction succeeded: " << response.message << std::endl;
        } else {
            std::cerr << "✗ Market transaction failed: " << response.message << std::endl;
        }
    });
    
    // Error response callback (general errors)
    networkMgr->setErrorCallback([](const std::string& message) {
        std::cerr << "✗ Server error: " << message << std::endl;
        // TODO: Could show a general error dialog here
    });
    
    std::cout << "  - Response callbacks wired for all panels" << std::endl;
    
    // === Setup Context Menu Callbacks ===
    m_contextMenu->SetApproachCallback([this](const std::string& entityId) {
        commandApproach(entityId);
    });
    
    m_contextMenu->SetOrbitCallback([this](const std::string& entityId, int distance_m) {
        commandOrbit(entityId, static_cast<float>(distance_m));
    });
    
    m_contextMenu->SetKeepAtRangeCallback([this](const std::string& entityId, int distance_m) {
        commandKeepAtRange(entityId, static_cast<float>(distance_m));
    });
    
    m_contextMenu->SetWarpToCallback([this](const std::string& entityId, int distance_m) {
        // For now, warp just treats it as approach
        std::cout << "[Movement] Warp to " << entityId << " at " << distance_m << "m distance" << std::endl;
        commandWarpTo(entityId);
    });
    
    m_contextMenu->SetLockTargetCallback([this](const std::string& entityId) {
        if (std::find(m_targetList.begin(), m_targetList.end(), entityId) == m_targetList.end()) {
            m_targetList.push_back(entityId);
            std::cout << "[Targeting] Locked target: " << entityId << std::endl;
        }
    });
    
    m_contextMenu->SetUnlockTargetCallback([this](const std::string& entityId) {
        auto it = std::find(m_targetList.begin(), m_targetList.end(), entityId);
        if (it != m_targetList.end()) {
            m_targetList.erase(it);
            std::cout << "[Targeting] Unlocked target: " << entityId << std::endl;
        }
    });
    
    m_contextMenu->SetLookAtCallback([this](const std::string& entityId) {
        auto entity = m_gameClient->getEntityManager().getEntity(entityId);
        if (entity) {
            m_camera->setTarget(entity->getPosition());
            std::cout << "[Camera] Looking at: " << entityId << std::endl;
        }
    });
    
    m_contextMenu->SetShowInfoCallback([this](const std::string& entityId) {
        std::cout << "[Info] Show info for: " << entityId << std::endl;
        openInfoPanelForEntity(entityId);
    });

    m_contextMenu->SetJumpCallback([this](const std::string& entityId) {
        commandJump(entityId);
    });
    
    std::cout << "  - Context menu callbacks wired" << std::endl;
    
    // === Wire RmlUi context menu button events to the same callbacks ===
    if (m_uiManager) {
        m_uiManager->SetOnLockTarget([this](const std::string& entityId) {
            if (std::find(m_targetList.begin(), m_targetList.end(), entityId) == m_targetList.end()) {
                m_targetList.push_back(entityId);
                std::cout << "[Targeting] Locked target: " << entityId << std::endl;
            }
        });
        m_uiManager->SetOnApproach([this](const std::string& entityId) {
            commandApproach(entityId);
        });
        m_uiManager->SetOnOrbit([this](const std::string& entityId, int dist) {
            commandOrbit(entityId, static_cast<float>(dist));
        });
        m_uiManager->SetOnKeepAtRange([this](const std::string& entityId, int dist) {
            commandKeepAtRange(entityId, static_cast<float>(dist));
        });
        m_uiManager->SetOnAlignTo([this](const std::string& entityId) {
            commandAlignTo(entityId);
        });
        m_uiManager->SetOnWarpTo([this](const std::string& entityId, int dist) {
            std::cout << "[Movement] Warp to " << entityId << " at " << dist << "m distance" << std::endl;
            commandWarpTo(entityId);
        });
        m_uiManager->SetOnShowInfo([this](const std::string& entityId) {
            std::cout << "[Info] Show info for: " << entityId << std::endl;
            openInfoPanelForEntity(entityId);
        });
        m_uiManager->SetOnLookAt([this](const std::string& entityId) {
            auto entity = m_gameClient->getEntityManager().getEntity(entityId);
            if (entity) {
                m_camera->setTarget(entity->getPosition());
                std::cout << "[Camera] Looking at: " << entityId << std::endl;
            }
        });
        std::cout << "  - RmlUi context menu events wired" << std::endl;
    }
    
    // === Setup Radial Menu Callbacks ===
    m_radialMenu->SetActionCallback([this](UI::RadialMenu::Action action, const std::string& entityId) {
        switch (action) {
            case UI::RadialMenu::Action::APPROACH:
                commandApproach(entityId);
                break;
            case UI::RadialMenu::Action::ORBIT:
                commandOrbit(entityId, 500.0f);  // Default orbit distance
                break;
            case UI::RadialMenu::Action::KEEP_AT_RANGE:
                commandKeepAtRange(entityId, 2500.0f);  // Default range
                break;
            case UI::RadialMenu::Action::WARP_TO:
                commandWarpTo(entityId);
                break;
            case UI::RadialMenu::Action::LOCK_TARGET:
                if (std::find(m_targetList.begin(), m_targetList.end(), entityId) == m_targetList.end()) {
                    m_targetList.push_back(entityId);
                    std::cout << "[Targeting] Locked target: " << entityId << std::endl;
                }
                break;
            case UI::RadialMenu::Action::ALIGN_TO:
                commandAlignTo(entityId);
                break;
            case UI::RadialMenu::Action::LOOK_AT:
                {
                    auto entity = m_gameClient->getEntityManager().getEntity(entityId);
                    if (entity) {
                        m_camera->setTarget(entity->getPosition());
                        std::cout << "[Camera] Looking at: " << entityId << std::endl;
                    }
                }
                break;
            case UI::RadialMenu::Action::SHOW_INFO:
                {
                    std::cout << "[Info] Show info for: " << entityId << std::endl;
                    openInfoPanelForEntity(entityId);
                }
                break;
            default:
                break;
        }
    });
    
    std::cout << "  - Radial menu callbacks wired" << std::endl;
    
    // === Setup Selected Item Panel Callbacks ===
    m_atlasHUD->setSelectedItemOrbitCb([this]() {
        if (!m_currentTargetId.empty()) {
            commandOrbit(m_currentTargetId);
        }
    });
    m_atlasHUD->setSelectedItemApproachCb([this]() {
        if (!m_currentTargetId.empty()) {
            commandApproach(m_currentTargetId);
        }
    });
    m_atlasHUD->setSelectedItemWarpCb([this]() {
        if (!m_currentTargetId.empty()) {
            commandWarpTo(m_currentTargetId);
        }
    });
    m_atlasHUD->setSelectedItemInfoCb([this]() {
        if (!m_currentTargetId.empty()) {
            openInfoPanelForEntity(m_currentTargetId);
        }
    });
    std::cout << "  - Selected item panel callbacks wired" << std::endl;

    // === Setup Overview Interaction Callbacks ===
    m_atlasHUD->setOverviewSelectCb([this](const std::string& entityId) {
        targetEntity(entityId, false);
        std::cout << "[Overview] Selected entity: " << entityId << std::endl;
    });

    m_atlasHUD->setOverviewRightClickCb([this](const std::string& entityId, float screenX, float screenY) {
        bool isLocked = std::find(m_targetList.begin(), m_targetList.end(), entityId) != m_targetList.end();
        bool isStargate = false;
        float distToTarget = 0.0f;
        if (m_solarSystem) {
            const auto* cel = m_solarSystem->findCelestial(entityId);
            if (cel && cel->type == atlas::Celestial::Type::STARGATE)
                isStargate = true;
            if (cel) {
                auto playerEntity = m_gameClient->getEntityManager().getEntity(m_localPlayerId);
                if (playerEntity)
                    distToTarget = glm::distance(playerEntity->getPosition(), cel->position);
            }
        }
        if (distToTarget == 0.0f) {
            auto targetEntity = m_gameClient->getEntityManager().getEntity(entityId);
            auto playerEntity = m_gameClient->getEntityManager().getEntity(m_localPlayerId);
            if (targetEntity && playerEntity)
                distToTarget = glm::distance(playerEntity->getPosition(), targetEntity->getPosition());
        }
        m_contextMenu->ShowEntityMenu(entityId, isLocked, isStargate, distToTarget);
        m_contextMenu->SetScreenPosition(screenX, screenY);
        std::cout << "[Overview] Right-click context menu for: " << entityId << std::endl;
    });

    m_atlasHUD->setOverviewBgRightClickCb([this](float screenX, float screenY) {
        m_contextMenu->ShowEmptySpaceMenu(0.0f, 0.0f, 0.0f);
        m_contextMenu->SetScreenPosition(screenX, screenY);
        std::cout << "[Overview] Right-click empty space context menu" << std::endl;
    });

    // Ctrl+Click on overview row = lock target (EVE standard)
    m_atlasHUD->setOverviewCtrlClickCb([this](const std::string& entityId) {
        targetEntity(entityId, true);  // addToTargets = true for lock
        std::cout << "[Overview] Ctrl+Click lock target: " << entityId << std::endl;
    });

    std::cout << "  - Overview interaction callbacks wired" << std::endl;
    
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
    
    // Update solar system scene (engine trail, warp visual state)
    if (m_solarSystem && m_shipPhysics) {
        m_solarSystem->update(deltaTime, m_shipPhysics.get());
    }
    
    // Update ship status in the HUD
    auto playerEntity = m_gameClient->getEntityManager().getEntity(m_localPlayerId);
    if (playerEntity) {
        UI::ShipStatusData status;
        const auto& health = playerEntity->getHealth();
        status.shield_pct = health.maxShield > 0 ? health.currentShield / static_cast<float>(health.maxShield) : 0.0f;
        status.armor_pct = health.maxArmor > 0 ? health.currentArmor / static_cast<float>(health.maxArmor) : 0.0f;
        status.hull_pct = health.maxHull > 0 ? health.currentHull / static_cast<float>(health.maxHull) : 0.0f;
        
        // Use real capacitor data from entity
        const auto& capacitor = playerEntity->getCapacitor();
        status.capacitor_pct = capacitor.max > 0.0f ? capacitor.current / capacitor.max : 0.0f;
        
        status.velocity = m_playerSpeed;
        status.max_velocity = m_playerMaxSpeed;
        m_uiManager->SetShipStatus(status);

        // Update player position for UI calculations (e.g., distance in overview/targets)
        const auto playerPosition = playerEntity->getPosition();
        m_uiManager->UpdateOverviewData(m_gameClient->getEntityManager().getAllEntities(), playerPosition);
        updateTargetListUi(playerPosition);

        // Camera follows player ship
        m_camera->setTarget(playerPosition);
    } else if (m_uiManager) {
        m_uiManager->ClearTargets();
    }
}

void Application::updateTargetListUi(const glm::vec3& playerPosition) {
    if (!m_uiManager) return;

    m_uiManager->ClearTargets();
    if (m_targetList.empty()) {
        return;
    }

    for (const auto& targetId : m_targetList) {
        auto entity = m_gameClient->getEntityManager().getEntity(targetId);
        if (!entity) continue;

        const auto& health = entity->getHealth();
        const auto& position = entity->getPosition();
        float shieldPct = health.maxShield > 0 ? health.currentShield / static_cast<float>(health.maxShield) : 0.0f;
        float armorPct = health.maxArmor > 0 ? health.currentArmor / static_cast<float>(health.maxArmor) : 0.0f;
        float hullPct = health.maxHull > 0 ? health.currentHull / static_cast<float>(health.maxHull) : 0.0f;
        float distance = glm::distance(playerPosition, position);

        std::string displayName = entity->getShipName().empty() ? entity->getId() : entity->getShipName();
        bool isActive = targetId == m_currentTargetId;
        m_uiManager->SetTarget(targetId, displayName, shieldPct, armorPct, hullPct, distance, false, isActive);
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
    
    // Render warp tunnel overlay (after 3D scene, before UI)
    // Only update and render the warp tunnel when actively warping to avoid
    // stale state and unnecessary full-screen draws.
    if (m_solarSystem) {
        const auto& ws = m_solarSystem->getWarpVisualState();
        if (ws.active) {
            m_renderer->updateWarpEffect(ws.phase, ws.progress, 1.0f,
                                          ws.direction,
                                          m_deltaTime);
            m_renderer->renderWarpEffect();
        }
    }
    
    // Render UI
    m_uiManager->Update();
    m_uiManager->BeginFrame();
    m_uiManager->Render();
    m_uiManager->EndFrame();
    
    // Render Atlas HUD overlay
    {
        atlas::InputState atlasInput;
        atlasInput.windowW = m_window->getWidth();
        atlasInput.windowH = m_window->getHeight();
        // Forward mouse state from InputHandler to Atlas for interactive widgets
        atlasInput.mousePos = {static_cast<float>(m_inputHandler->getMouseX()),
                                static_cast<float>(m_inputHandler->getMouseY())};
        atlasInput.mouseDown[0] = m_inputHandler->isMouseDown(0);
        atlasInput.mouseDown[1] = m_inputHandler->isMouseDown(1);
        atlasInput.mouseDown[2] = m_inputHandler->isMouseDown(2);
        atlasInput.mouseClicked[0]  = m_inputHandler->isMouseClicked(0);
        atlasInput.mouseClicked[1]  = m_inputHandler->isMouseClicked(1);
        atlasInput.mouseClicked[2]  = m_inputHandler->isMouseClicked(2);
        atlasInput.mouseReleased[0] = m_inputHandler->isMouseReleased(0);
        atlasInput.mouseReleased[1] = m_inputHandler->isMouseReleased(1);
        atlasInput.mouseReleased[2] = m_inputHandler->isMouseReleased(2);
        atlasInput.scrollY = m_inputHandler->getScrollDeltaY();
        
        m_atlasCtx->beginFrame(atlasInput);
        
        atlas::ShipHUDData shipData;
        // Connect to actual ship state from game client
        auto playerEntity = m_gameClient->getEntityManager().getEntity(m_localPlayerId);
        if (playerEntity) {
            const auto& health = playerEntity->getHealth();
            shipData.shieldPct = health.maxShield > 0 ? health.currentShield / static_cast<float>(health.maxShield) : 0.0f;
            shipData.armorPct = health.maxArmor > 0 ? health.currentArmor / static_cast<float>(health.maxArmor) : 0.0f;
            shipData.hullPct = health.maxHull > 0 ? health.currentHull / static_cast<float>(health.maxHull) : 0.0f;
            const auto& capacitor = playerEntity->getCapacitor();
            shipData.capacitorPct = capacitor.max > 0.0f ? capacitor.current / capacitor.max : 0.0f;
            shipData.shipName = playerEntity->getShipType();
        }
        shipData.currentSpeed = m_playerSpeed;
        shipData.maxSpeed = m_playerMaxSpeed;
        
        // Feed warp state into HUD
        if (m_solarSystem) {
            const auto& ws = m_solarSystem->getWarpVisualState();
            shipData.warpActive   = ws.active;
            shipData.warpPhase    = ws.phase;
            shipData.warpProgress = ws.progress;
            shipData.warpSpeedAU  = ws.speedAU;
        }
        
        // Build Atlas target cards from target list
        std::vector<atlas::TargetCardInfo> atlasTargets;
        if (playerEntity) {
            const auto playerPos = playerEntity->getPosition();
            for (const auto& targetId : m_targetList) {
                auto targetEntity = m_gameClient->getEntityManager().getEntity(targetId);
                if (!targetEntity) continue;
                atlas::TargetCardInfo card;
                card.name = targetEntity->getShipName().empty() ? targetEntity->getId() : targetEntity->getShipName();
                const auto& th = targetEntity->getHealth();
                card.shieldPct = th.maxShield > 0 ? th.currentShield / static_cast<float>(th.maxShield) : 0.0f;
                card.armorPct = th.maxArmor > 0 ? th.currentArmor / static_cast<float>(th.maxArmor) : 0.0f;
                card.hullPct = th.maxHull > 0 ? th.currentHull / static_cast<float>(th.maxHull) : 0.0f;
                card.distance = glm::distance(playerPos, targetEntity->getPosition());
                card.isActive = (targetId == m_currentTargetId);
                atlasTargets.push_back(card);
            }
        }
        
        // Build Atlas overview entries from entity manager
        std::vector<atlas::OverviewEntry> atlasOverview;
        if (playerEntity) {
            const auto playerPos = playerEntity->getPosition();
            for (const auto& pair : m_gameClient->getEntityManager().getAllEntities()) {
                if (pair.first == m_localPlayerId) continue;
                atlas::OverviewEntry entry;
                entry.entityId = pair.first;
                entry.name = pair.second->getShipName().empty() ? pair.first : pair.second->getShipName();
                entry.type = pair.second->getShipType();
                entry.distance = glm::distance(playerPos, pair.second->getPosition());
                entry.selected = (pair.first == m_currentTargetId);
                atlasOverview.push_back(entry);
            }
            
            // Add solar system celestials (planets, stations, gates, belts)
            if (m_solarSystem) {
                for (const auto& c : m_solarSystem->getCelestials()) {
                    if (c.type == atlas::Celestial::Type::SUN) continue;
                    atlas::OverviewEntry entry;
                    entry.name = c.name;
                    entry.distance = glm::distance(playerPos, c.position);
                    entry.selected = false;
                    switch (c.type) {
                        case atlas::Celestial::Type::PLANET:        entry.type = "Planet";        break;
                        case atlas::Celestial::Type::MOON:          entry.type = "Moon";          break;
                        case atlas::Celestial::Type::STATION:       entry.type = "Station";       break;
                        case atlas::Celestial::Type::STARGATE:      entry.type = "Stargate";      break;
                        case atlas::Celestial::Type::ASTEROID_BELT: entry.type = "Asteroid Belt"; break;
                        case atlas::Celestial::Type::WORMHOLE:      entry.type = "Wormhole";      break;
                        case atlas::Celestial::Type::DYSON_RING:    entry.type = "Dyson Ring";    break;
                        default:                                  entry.type = "Celestial";     break;
                    }
                    atlasOverview.push_back(entry);
                }
            }
        }
        
        // Build selected item info
        atlas::SelectedItemInfo atlasSelected;
        if (!m_currentTargetId.empty() && playerEntity) {
            auto targetEntity = m_gameClient->getEntityManager().getEntity(m_currentTargetId);
            if (targetEntity) {
                atlasSelected.name = targetEntity->getShipName().empty() ? m_currentTargetId : targetEntity->getShipName();
                float dist = glm::distance(playerEntity->getPosition(), targetEntity->getPosition());
                if (dist >= 1000.0f) {
                    atlasSelected.distance = dist / 1000.0f;
                    atlasSelected.distanceUnit = "km";
                } else {
                    atlasSelected.distance = dist;
                    atlasSelected.distanceUnit = "m";
                }
            }
        }
        
        // Update mode indicator text on the HUD
        m_atlasHUD->setModeIndicator(m_activeModeText);
        
        // Reserve context menu / radial menu input areas BEFORE panels
        // so their clicks aren't stolen by panel body consumption.
        if (m_contextMenu && m_contextMenu->IsOpen()) {
            m_contextMenu->ReserveInputArea(*m_atlasCtx);
        }

        // Render HUD panels (overview, selected item, ship HUD, etc.)
        m_atlasHUD->update(*m_atlasCtx, shipData, atlasTargets, atlasOverview, atlasSelected);

        // Render Context Menu AFTER panels so it draws on top visually
        if (m_contextMenu && m_contextMenu->IsOpen()) {
            m_contextMenu->RenderAtlas(*m_atlasCtx);
        }

        // Render Radial Menu on top of everything
        if (m_radialMenu && m_radialMenuOpen) {
            m_radialMenu->RenderAtlas(*m_atlasCtx);
        }
        
        m_atlasCtx->endFrame();

        // Record whether Atlas UI consumed the mouse this frame so that
        // game-world interaction handlers can avoid click-through
        m_atlasConsumedMouse = m_atlasCtx->isMouseConsumed();
    }
    
    // Legacy context/radial menu render stubs (retained for RmlUi fallback)
    if (m_contextMenu) {
        m_contextMenu->Render();
    }
    
    if (m_radialMenu && m_radialMenuOpen) {
        m_radialMenu->Render();
    }
    
    // End rendering
    m_renderer->endFrame();
}

void Application::cleanup() {
    std::cout << "Cleaning up application..." << std::endl;
    
    // Shutdown Atlas UI
    if (m_atlasCtx) {
        m_atlasCtx->shutdown();
    }
    
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

    if (m_uiManager && m_uiManager->WantsKeyboardInput()) {
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
    // D + click = Dock/Jump Through
    if (key == GLFW_KEY_Q) {
        m_approachActive = true;
        m_orbitActive = false;
        m_keepRangeActive = false;
        m_dockingModeActive = false;
        m_warpModeActive = false;
        m_activeModeText = "APPROACH - click a target";
        std::cout << "[Controls] Approach mode active — click a target" << std::endl;
    } else if (key == GLFW_KEY_W) {
        m_approachActive = false;
        m_orbitActive = true;
        m_keepRangeActive = false;
        m_dockingModeActive = false;
        m_warpModeActive = false;
        m_activeModeText = "ORBIT - click a target";
        std::cout << "[Controls] Orbit mode active — click a target" << std::endl;
    } else if (key == GLFW_KEY_E) {
        m_approachActive = false;
        m_orbitActive = false;
        m_keepRangeActive = true;
        m_dockingModeActive = false;
        m_warpModeActive = false;
        m_activeModeText = "KEEP AT RANGE - click a target";
        std::cout << "[Controls] Keep at Range mode active — click a target" << std::endl;
    } else if (key == GLFW_KEY_D) {
        m_approachActive = false;
        m_orbitActive = false;
        m_keepRangeActive = false;
        m_dockingModeActive = true;
        m_warpModeActive = false;
        m_activeModeText = "DOCK / JUMP - click a station or gate";
        std::cout << "[Controls] Docking mode active — click a station or gate" << std::endl;
    } else if (key == GLFW_KEY_S && (mods & GLFW_MOD_CONTROL)) {
        // Ctrl+S = stop ship (EVE standard) — checked before bare S
        commandStopShip();
    } else if (key == GLFW_KEY_S) {
        // S + Click = Warp To (EVE standard)
        m_approachActive = false;
        m_orbitActive = false;
        m_keepRangeActive = false;
        m_dockingModeActive = false;
        m_warpModeActive = true;
        m_activeModeText = "WARP TO - click a target";
        std::cout << "[Controls] Warp mode active — click a target" << std::endl;
    } else if (key == GLFW_KEY_F) {
        // F = Engage/Recall drones (EVE standard)
        std::cout << "[Controls] Drone command: engage/recall" << std::endl;
        // TODO: Send drone engage/recall command to server
    }
    
    // Panel toggles
    if (key == GLFW_KEY_I && (mods & GLFW_MOD_ALT)) {
        if (m_uiManager) m_uiManager->ToggleDocument("inventory");
    } else if (key == GLFW_KEY_F && (mods & GLFW_MOD_ALT)) {
        if (m_uiManager) m_uiManager->ToggleDocument("fitting");
    } else if (key == GLFW_KEY_O && (mods & GLFW_MOD_ALT)) {
        if (m_uiManager) m_uiManager->ToggleDocument("overview");
        m_atlasHUD->toggleOverview();
    } else if (key == GLFW_KEY_R && (mods & GLFW_MOD_ALT)) {
        if (m_uiManager) m_uiManager->ToggleDocument("market");
    } else if (key == GLFW_KEY_J && (mods & GLFW_MOD_ALT)) {
        if (m_uiManager) m_uiManager->ToggleDocument("mission");
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
            // Skip if UI already captured the mouse (e.g. overview panel handled it)
            // to prevent two context menus appearing simultaneously
            if (m_rightMouseDown) {
                if (!m_atlasConsumedMouse) {
                    double dx = x - m_lastMouseDragX;
                    double dy = y - m_lastMouseDragY;
                    double dist = std::sqrt(dx * dx + dy * dy);
                    if (dist < 5.0) {
                        // Quick right-click — show EVE context menu
                        // Pick entity at mouse position
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
                        
                        if (!pickedId.empty()) {
                            // Show entity context menu (Atlas only — no RmlUi duplicate)
                            bool isLocked = std::find(m_targetList.begin(), m_targetList.end(), pickedId) != m_targetList.end();
                            bool isStargate = false;
                            float distToTarget = 0.0f;
                            if (m_solarSystem) {
                                const auto* cel = m_solarSystem->findCelestial(pickedId);
                                if (cel && cel->type == atlas::Celestial::Type::STARGATE)
                                    isStargate = true;
                            }
                            auto playerEntity = m_gameClient->getEntityManager().getEntity(m_localPlayerId);
                            auto targetEntity = m_gameClient->getEntityManager().getEntity(pickedId);
                            if (playerEntity && targetEntity)
                                distToTarget = glm::distance(playerEntity->getPosition(), targetEntity->getPosition());
                            m_contextMenu->ShowEntityMenu(pickedId, isLocked, isStargate, distToTarget);
                            m_contextMenu->SetScreenPosition(static_cast<float>(x), static_cast<float>(y));
                        } else {
                            // Show empty space context menu
                            m_contextMenu->ShowEmptySpaceMenu(0.0f, 0.0f, 0.0f);
                            m_contextMenu->SetScreenPosition(static_cast<float>(x), static_cast<float>(y));
                        }
                    }
                }
            }
            m_rightMouseDown = false;
        }
    }
    
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            m_leftMouseDown = true;
            m_radialMenuStartX = x;
            m_radialMenuStartY = y;
            m_radialMenuHoldStartTime = glfwGetTime();

            // Close context menu when clicking elsewhere (EVE behaviour)
            if (m_contextMenu && m_contextMenu->IsOpen()) {
                m_contextMenu->Close();
                if (m_uiManager) m_uiManager->HideContextMenu();
            }
        } else if (action == GLFW_RELEASE) {
            // Check if radial menu is open
            if (m_radialMenuOpen) {
                // Confirm selection
                auto confirmedAction = m_radialMenu->Confirm();
                m_radialMenuOpen = false;
                m_radialMenu->Close();
                if (m_uiManager) m_uiManager->HideRadialMenu();
            }
            m_leftMouseDown = false;
        }
    }
    
    // Left-click: select entity / apply movement command
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        // Don't process clicks that UI captured
        // Atlas UI consumption is the primary gate; also respect RmlUI
        // only when it has a focused interactive element (not just hover)
        if (m_atlasConsumedMouse) return;
        if (m_uiManager && m_uiManager->WantsKeyboardInput()) return;
        
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
            if (m_approachActive) {
                commandApproach(pickedEntityId);
                m_approachActive = false;
                m_activeModeText.clear();
            } else if (m_orbitActive) {
                commandOrbit(pickedEntityId);
                m_orbitActive = false;
                m_activeModeText.clear();
            } else if (m_keepRangeActive) {
                commandKeepAtRange(pickedEntityId);
                m_keepRangeActive = false;
                m_activeModeText.clear();
            } else if (m_dockingModeActive) {
                // D+Click for docking/jump through
                std::cout << "[Movement] Dock/Jump through " << pickedEntityId << std::endl;
                // TODO: Send dock/jump command to server
                m_dockingModeActive = false;
                m_activeModeText.clear();
            } else if (m_warpModeActive) {
                // S+Click for warp to (EVE standard)
                commandWarpTo(pickedEntityId);
                m_warpModeActive = false;
                m_activeModeText.clear();
            } else {
                // Default: select / CTRL+click to lock target / double-click to approach
                if (m_inputHandler->isDoubleClick()) {
                    // EVE-style double-click: approach the entity
                    commandApproach(pickedEntityId);
                } else {
                    bool addToTargets = (mods & GLFW_MOD_CONTROL) != 0;
                    targetEntity(pickedEntityId, addToTargets);
                }
            }
        }
    }
}

void Application::handleMouseMove(double x, double y, double deltaX, double deltaY) {
    // Update radial menu if open
    if (m_radialMenuOpen && m_radialMenu) {
        m_radialMenu->UpdateMousePosition(static_cast<float>(x), static_cast<float>(y));
    }
    
    // Check if we should open radial menu (left mouse held for RADIAL_MENU_HOLD_TIME)
    if (m_leftMouseDown && !m_radialMenuOpen) {
        double holdTime = glfwGetTime() - m_radialMenuHoldStartTime;
        if (holdTime >= RADIAL_MENU_HOLD_TIME) {
            // Check distance moved
            double dx = x - m_radialMenuStartX;
            double dy = y - m_radialMenuStartY;
            double dist = std::sqrt(dx * dx + dy * dy);
            
            // Only open if not dragging significantly
            if (dist < 10.0) {
                // Pick entity at hold position (3D raycasting)
                auto entities = m_gameClient->getEntityManager().getAllEntities();
                std::vector<std::shared_ptr<Entity>> entityList;
                for (const auto& pair : entities) {
                    if (pair.first != m_localPlayerId) {
                        entityList.push_back(pair.second);
                    }
                }
                
                std::string pickedId = m_entityPicker->pickEntity(
                    m_radialMenuStartX, m_radialMenuStartY,
                    m_window->getWidth(), m_window->getHeight(),
                    *m_camera, entityList);
                
                if (!pickedId.empty()) {
                    // Compute distance to target for warp eligibility check
                    float distToTarget = 0.0f;
                    if (m_shipPhysics) {
                        auto targetEntity = m_gameClient->getEntityManager().getEntity(pickedId);
                        if (targetEntity) {
                            distToTarget = glm::distance(m_shipPhysics->getPosition(), targetEntity->getPosition());
                        }
                    }
                    m_radialMenu->Open(static_cast<float>(m_radialMenuStartX), 
                                      static_cast<float>(m_radialMenuStartY), 
                                      pickedId, distToTarget);
                    m_radialMenuOpen = true;
                    std::cout << "[Radial Menu] Opened for entity: " << pickedId
                              << " (distance: " << (distToTarget / 1000.0f) << " km)" << std::endl;
                }
            }
        }
    }
    
    // EVE-style camera: Right-click drag to orbit camera around ship
    if (m_rightMouseDown) {
        if (!m_atlasConsumedMouse) {
            float sensitivity = 0.15f;
            m_camera->rotate(static_cast<float>(deltaX) * sensitivity,
                           static_cast<float>(-deltaY) * sensitivity);
        }
    }
}

void Application::handleScroll(double xoffset, double yoffset) {
    // EVE-style: mousewheel zooms camera
    if (!m_atlasConsumedMouse) {
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
        }
    } else {
        // Replace current target
        m_currentTargetId = entityId;
        m_targetList.clear();
        m_targetList.push_back(entityId);
        m_currentTargetIndex = 0;
    }
}

void Application::clearTarget() {
    std::cout << "[Targeting] Clear target" << std::endl;
    
    m_currentTargetId.clear();
    m_targetList.clear();
    m_currentTargetIndex = -1;

    if (m_uiManager) {
        m_uiManager->ClearTargets();
    }
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

void Application::openInfoPanelForEntity(const std::string& entityId) {
    auto entity = m_gameClient->getEntityManager().getEntity(entityId);
    if (!entity || !m_atlasHUD) return;

    auto playerEntity = m_gameClient->getEntityManager().getEntity(m_localPlayerId);
    atlas::InfoPanelData info;
    info.name = entity->getShipName().empty() ? entityId : entity->getShipName();
    info.type = entity->getShipType();
    info.faction = entity->getFaction();
    const auto& health = entity->getHealth();
    info.shieldPct = health.maxShield > 0 ? health.currentShield / static_cast<float>(health.maxShield) : 0.0f;
    info.armorPct = health.maxArmor > 0 ? health.currentArmor / static_cast<float>(health.maxArmor) : 0.0f;
    info.hullPct = health.maxHull > 0 ? health.currentHull / static_cast<float>(health.maxHull) : 0.0f;
    info.hasHealth = true;
    if (playerEntity) {
        info.distance = glm::distance(playerEntity->getPosition(), entity->getPosition());
    }
    m_atlasHUD->showInfoPanel(info);
}

void Application::commandApproach(const std::string& entityId) {
    m_currentMoveCommand = MoveCommand::Approach;
    m_moveTargetId = entityId;
    m_activeModeText = "APPROACHING";
    std::cout << "[Movement] Approaching " << entityId << std::endl;
}

void Application::commandOrbit(const std::string& entityId, float distance) {
    m_currentMoveCommand = MoveCommand::Orbit;
    m_moveTargetId = entityId;
    m_orbitDistance = distance;
    m_activeModeText = "ORBITING";
    std::cout << "[Movement] Orbiting " << entityId << " at " << distance << "m" << std::endl;
}

void Application::commandKeepAtRange(const std::string& entityId, float distance) {
    m_currentMoveCommand = MoveCommand::KeepAtRange;
    m_moveTargetId = entityId;
    m_keepAtRangeDistance = distance;
    m_activeModeText = "KEEP AT RANGE";
    std::cout << "[Movement] Keeping at range " << distance << "m from " << entityId << std::endl;
}

void Application::commandAlignTo(const std::string& entityId) {
    m_currentMoveCommand = MoveCommand::AlignTo;
    m_moveTargetId = entityId;
    m_activeModeText = "ALIGNING";
    std::cout << "[Movement] Aligning to " << entityId << std::endl;
}

void Application::commandWarpTo(const std::string& entityId) {
    m_currentMoveCommand = MoveCommand::WarpTo;
    m_moveTargetId = entityId;
    m_activeModeText = "WARPING";
    std::cout << "[Movement] Warping to " << entityId << std::endl;

    // Use ShipPhysics + SolarSystemScene for proper 4-phase warp
    if (m_solarSystem && m_shipPhysics) {
        // Try to warp via celestial lookup first
        const Celestial* celestial = m_solarSystem->findCelestial(entityId);
        if (celestial) {
            m_solarSystem->warpTo(entityId, m_shipPhysics.get());
            return;
        }
        // Fallback: warp to an entity position
        auto targetEntity = m_gameClient->getEntityManager().getEntity(entityId);
        if (targetEntity) {
            m_shipPhysics->warpTo(targetEntity->getPosition());
        }
    }
}

void Application::commandStopShip() {
    m_currentMoveCommand = MoveCommand::None;
    m_moveTargetId.clear();
    m_playerVelocity = glm::vec3(0.0f);
    m_playerSpeed = 0.0f;
    m_approachActive = false;
    m_orbitActive = false;
    m_keepRangeActive = false;
    m_dockingModeActive = false;
    m_activeModeText.clear();
    std::cout << "[Movement] Ship stopped" << std::endl;
}

void Application::commandJump(const std::string& entityId) {
    if (!m_solarSystem) return;

    const auto* gate = m_solarSystem->findCelestial(entityId);
    if (!gate || gate->type != atlas::Celestial::Type::STARGATE) {
        std::cout << "[Jump] " << entityId << " is not a stargate" << std::endl;
        return;
    }

    std::string destination = gate->linkedSystem;
    if (destination.empty()) {
        std::cout << "[Jump] Stargate " << entityId << " has no linked system" << std::endl;
        return;
    }

    std::cout << "[Jump] Jumping through stargate " << entityId
              << " to system: " << destination << std::endl;

    // Generate the destination system from its name hash
    uint32_t destSeed = 0;
    for (char c : destination) {
        destSeed = destSeed * 31 + static_cast<uint32_t>(c);
    }
    m_solarSystem->generateSystem(destSeed, destination);

    // Reset player position to the arrival gate location
    const auto& celestials = m_solarSystem->getCelestials();
    glm::vec3 arrivalPos(0.0f);
    for (const auto& c : celestials) {
        if (c.type == atlas::Celestial::Type::STARGATE) {
            arrivalPos = c.position;
            break;  // arrive at the first gate in the new system
        }
    }

    glm::vec3 playerPos = arrivalPos + glm::vec3(2000.0f, 0.0f, 0.0f);  // offset from gate
    m_playerVelocity = glm::vec3(0.0f);
    m_playerSpeed = 0.0f;
    m_currentMoveCommand = MoveCommand::None;
    m_activeModeText.clear();

    // Update the player entity position through the entity manager
    auto playerEntity = m_gameClient->getEntityManager().getEntity(m_localPlayerId);
    if (playerEntity) {
        float rotation = playerEntity->getRotation();
        Health currentHealth = playerEntity->getHealth();
        m_gameClient->getEntityManager().updateEntityState(
            m_localPlayerId, playerPos, m_playerVelocity, rotation, currentHealth);
    }

    std::cout << "[Jump] Arrived in " << destination << " at position ("
              << playerPos.x << ", " << playerPos.y << ", "
              << playerPos.z << ")" << std::endl;
}

void Application::spawnLocalPlayerEntity() {
    if (m_localPlayerSpawned) return;
    
    std::cout << "[PVE] Spawning local player ship..." << std::endl;
    
    // Create player entity near the Dyson Ring Core Module so the sun
    // is visible in the distance.  The core module orbits at 0.5 AU.
    // Spawn 15 km away from it so the player can see and dock.
    Health playerHealth(1500, 800, 500);  // Shield, Armor, Hull
    Capacitor playerCapacitor(250.0f, 250.0f);  // Fang capacitor: 250 GJ
    
    // Dyson Ring Core Module is at (0.5 AU, 0, 0).  Spawn offset 15 km.
    glm::vec3 spawnPos(0.5f * 149597870700.0f + 15000.0f,
                       0.0f,
                       0.0f);
    
    m_gameClient->getEntityManager().spawnEntity(
        m_localPlayerId,
        spawnPos,
        playerHealth,
        playerCapacitor,
        "Fang",
        "Your Ship",
        "Keldari"
    );
    
    m_localPlayerSpawned = true;
    std::cout << "[PVE] Local player ship spawned as Fang" << std::endl;
}

void Application::spawnDemoNPCEntities() {
    std::cout << "[PVE] Spawning demo NPC entities..." << std::endl;
    
    // Spawn some NPC enemies for the PVE demo
    // These would normally come from the server in missions/anomalies
    
    // Crimson Order pirate (hostile NPC)
    Health npc1Health(800, 600, 400);
    Capacitor npc1Cap(500.0f, 500.0f);
    m_gameClient->getEntityManager().spawnEntity(
        "npc_raider_1",
        glm::vec3(300.0f, 10.0f, 200.0f),
        npc1Health,
        npc1Cap,
        "Cruiser",
        "Crimson Order",
        "Crimson Order"
    );
    
    // Venom Syndicate frigate
    Health npc2Health(400, 300, 200);
    Capacitor npc2Cap(300.0f, 300.0f);
    m_gameClient->getEntityManager().spawnEntity(
        "npc_serp_1",
        glm::vec3(-250.0f, -5.0f, 350.0f),
        npc2Health,
        npc2Cap,
        "Frigate",
        "Venom Syndicate Scout",
        "Venom Syndicate"
    );
    
    // Iron Corsairs destroyer
    Health npc3Health(600, 500, 350);
    Capacitor npc3Cap(400.0f, 400.0f);
    m_gameClient->getEntityManager().spawnEntity(
        "npc_gur_1",
        glm::vec3(150.0f, 20.0f, -300.0f),
        npc3Health,
        npc3Cap,
        "Destroyer",
        "Iron Corsairs Watchman",
        "Iron Corsairs"
    );
    
    std::cout << "[PVE] 3 NPC entities spawned" << std::endl;
}

void Application::updateLocalMovement(float deltaTime) {
    auto playerEntity = m_gameClient->getEntityManager().getEntity(m_localPlayerId);
    if (!playerEntity) return;
    
    // Movement physics constants — tuned for EVE-style feel with proper align time
    static constexpr float ACCELERATION = 25.0f;           // m/s² (reduced for gradual ramp-up)
    static constexpr float DECELERATION = 30.0f;            // m/s² when stopping (faster than accel for responsive stop)
    static constexpr float APPROACH_DECEL_DIST = 50.0f;     // Start slowing at this range
    static constexpr float WARP_SPEED = 5000.0f;            // Simulated warp speed m/s
    static constexpr float WARP_EXIT_DIST = 100.0f;         // Exit warp at this range
    static constexpr float ALIGN_TURN_RATE = 1.5f;          // rad/s — how fast ship rotates toward target
    static constexpr float ALIGN_SPEED_FRACTION = 0.75f;    // Must reach 75% max speed to warp
    
    glm::vec3 playerPos = playerEntity->getPosition();
    
    if (m_currentMoveCommand == MoveCommand::None) {
        // Decelerate to stop — exponential slowdown for smooth feel
        if (m_playerSpeed > 0.1f) {
            // Guard against negative factor when deltaTime is very large (e.g. lag spike)
            m_playerSpeed *= std::max(0.0f, 1.0f - DECELERATION * deltaTime / m_playerMaxSpeed);
            playerPos += m_playerVelocity * deltaTime;
            // Update velocity direction with reduced speed
            if (glm::length(m_playerVelocity) > 0.01f) {
                m_playerVelocity = glm::normalize(m_playerVelocity) * m_playerSpeed;
            }
        } else {
            m_playerSpeed = 0.0f;
            m_playerVelocity = glm::vec3(0.0f);
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
                // EVE-style exponential acceleration towards target
                // Ship gradually ramps up speed, giving time to align
                float targetSpeed = m_playerMaxSpeed;
                if (dist < APPROACH_DECEL_DIST) {
                    targetSpeed = m_playerMaxSpeed * (dist / APPROACH_DECEL_DIST);
                }
                // Exponential ramp: speed approaches target over time
                float speedDiff = targetSpeed - m_playerSpeed;
                m_playerSpeed += speedDiff * ACCELERATION * deltaTime / m_playerMaxSpeed;
                m_playerSpeed = std::clamp(m_playerSpeed, 0.0f, targetSpeed);
                m_playerVelocity = dir * m_playerSpeed;
                break;
            }
            case MoveCommand::Orbit: {
                // Orbit around target at set distance with gradual acceleration
                float speedDiff = m_playerMaxSpeed - m_playerSpeed;
                m_playerSpeed += speedDiff * ACCELERATION * deltaTime / m_playerMaxSpeed;
                m_playerSpeed = std::min(m_playerSpeed, m_playerMaxSpeed);
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
                float speedDiff = m_playerMaxSpeed - m_playerSpeed;
                m_playerSpeed += speedDiff * ACCELERATION * deltaTime / m_playerMaxSpeed;
                m_playerSpeed = std::min(m_playerSpeed, m_playerMaxSpeed);
                if (dist > m_keepAtRangeDistance + 20.0f) {
                    m_playerVelocity = dir * m_playerSpeed;
                } else if (dist < m_keepAtRangeDistance - 20.0f) {
                    m_playerVelocity = -dir * m_playerSpeed * 0.3f;
                } else {
                    m_playerSpeed = std::max(0.0f, m_playerSpeed - DECELERATION * deltaTime);
                    m_playerVelocity = dir * m_playerSpeed;
                }
                break;
            }
            case MoveCommand::AlignTo: {
                // Align to target: gradually accelerate to 75% max speed
                // giving the ship time to turn and align before reaching speed
                float alignTarget = m_playerMaxSpeed * ALIGN_SPEED_FRACTION;
                float speedDiff = alignTarget - m_playerSpeed;
                m_playerSpeed += speedDiff * ACCELERATION * deltaTime / m_playerMaxSpeed;
                m_playerSpeed = std::clamp(m_playerSpeed, 0.0f, alignTarget);
                m_playerVelocity = dir * m_playerSpeed;
                break;
            }
            case MoveCommand::WarpTo: {
                // Use ShipPhysics 4-phase warp when available
                if (m_shipPhysics && m_shipPhysics->isWarping()) {
                    m_shipPhysics->update(deltaTime);
                    playerPos = m_shipPhysics->getPosition();
                    m_playerVelocity = m_shipPhysics->getVelocity();
                    m_playerSpeed = m_shipPhysics->getCurrentSpeed();

                    // Update mode text with warp phase info
                    auto phase = m_shipPhysics->getWarpPhase();
                    float speedAU = m_shipPhysics->getWarpSpeedAU();
                    switch (phase) {
                        case ShipPhysics::WarpPhase::ALIGNING:
                            m_activeModeText = "ALIGNING";
                            break;
                        case ShipPhysics::WarpPhase::ACCELERATING: {
                            char buf[64];
                            std::snprintf(buf, sizeof(buf), "WARP  %.1f AU/s", speedAU);
                            m_activeModeText = buf;
                            break;
                        }
                        case ShipPhysics::WarpPhase::CRUISING: {
                            char buf[64];
                            std::snprintf(buf, sizeof(buf), "WARP  %.1f AU/s", speedAU);
                            m_activeModeText = buf;
                            break;
                        }
                        case ShipPhysics::WarpPhase::DECELERATING:
                            m_activeModeText = "DECELERATING";
                            break;
                        default:
                            break;
                    }

                    // Warp completed?
                    if (!m_shipPhysics->isWarping()) {
                        m_currentMoveCommand = MoveCommand::None;
                        m_playerSpeed = m_shipPhysics->getCurrentSpeed();
                        m_playerVelocity = m_shipPhysics->getVelocity();
                        m_activeModeText.clear();
                        std::cout << "[Movement] Warp complete" << std::endl;
                    }
                    // Update entity via EntityManager (ShipPhysics owns position)
                    float rotation = 0.0f;
                    if (glm::length(m_playerVelocity) > 0.1f) {
                        rotation = std::atan2(m_playerVelocity.x, m_playerVelocity.z);
                    }
                    Health currentHealth = playerEntity->getHealth();
                    m_gameClient->getEntityManager().updateEntityState(
                        m_localPlayerId, playerPos, m_playerVelocity, rotation, currentHealth);
                    return;
                }
                // Fallback: simple linear warp (legacy path)
                m_playerSpeed = std::min(WARP_SPEED,
                                         m_playerSpeed + WARP_SPEED * deltaTime);
                m_playerVelocity = dir * m_playerSpeed;
                if (dist < WARP_EXIT_DIST) {
                    m_currentMoveCommand = MoveCommand::None;
                    m_playerSpeed = 0.0f;
                    m_playerVelocity = glm::vec3(0.0f);
                    m_activeModeText.clear();
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

} // namespace atlas
