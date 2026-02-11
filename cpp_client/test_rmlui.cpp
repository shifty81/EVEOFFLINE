/**
 * @file test_rmlui.cpp
 * @brief Test program for the RmlUi-based Photon UI implementation.
 *
 * This test creates a GLFW window with OpenGL 3.3 core profile and
 * renders the EVE Photon UI panels using RmlUi. It demonstrates:
 *   - Ship HUD with animated health bars, speed readout, module rack
 *   - Overview panel with entity table
 *   - Dynamic ship status updates (animated)
 *   - GLFW input forwarding to RmlUi
 *
 * Build: cmake .. -DUSE_RMLUI=ON -DBUILD_TESTS=ON
 * Run:   ./bin/test_rmlui
 *
 * Controls:
 *   F1 - Toggle fitting panel
 *   F8 - Toggle RmlUi visual debugger
 *   ESC - Exit
 */

#include <iostream>
#include <cmath>
#include <cstdio>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "ui/rml_ui_manager.h"

static bool g_showFitting = false;
static bool g_showInventory = false;
static bool g_showDScan = false;
static bool g_showDebugger = false;

static void glfwErrorCallback(int error, const char* description) {
    std::cerr << "GLFW Error " << error << ": " << description << "\n";
}

int main() {
    std::cout << "=== EVE OFFLINE — RmlUi Photon UI Test ===" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  F1  - Toggle fitting panel" << std::endl;
    std::cout << "  F2  - Toggle inventory panel" << std::endl;
    std::cout << "  F3  - Toggle D-Scan panel" << std::endl;
    std::cout << "  F8  - Toggle RmlUi debugger" << std::endl;
    std::cout << "  ESC - Exit" << std::endl;
    std::cout << std::endl;

    // Initialize GLFW
    glfwSetErrorCallback(glfwErrorCallback);
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    // Request OpenGL 3.3 Core Profile (required by RmlUi GL3 renderer)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    // Create window
    GLFWwindow* window = glfwCreateWindow(1440, 900,
        "EVE OFFLINE — Photon UI (RmlUi)", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);  // VSync

    // Initialize GLEW
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "GLEW Error: " << glewGetErrorString(err) << "\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    std::cout << "OpenGL: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;

    // Initialize RmlUi Manager
    auto rmlUi = std::make_unique<UI::RmlUiManager>();

    if (!rmlUi->Initialize(window, "ui_resources")) {
#ifdef USE_RMLUI
        std::cerr << "ERROR: RmlUi initialization failed.\n";
        std::cerr << "Check that ui_resources/ directory exists with RML/RCSS files,\n";
        std::cerr << "and that FreeType and font files are available.\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
#else
        std::cout << "\nNote: RmlUi integration requires building with -DUSE_RMLUI=ON\n";
        std::cout << "The stub (no-op) implementation was used.\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return 0;
#endif
    }

    glfwSetWindowUserPointer(window, rmlUi.get());
    glfwSetKeyCallback(window, [](GLFWwindow* w, int key, int, int action, int mods) {
        auto* ui = static_cast<UI::RmlUiManager*>(glfwGetWindowUserPointer(w));
        if (ui) {
            ui->HandleKey(key, action, mods);
        }
    });
    glfwSetCharCallback(window, [](GLFWwindow* w, unsigned int codepoint) {
        auto* ui = static_cast<UI::RmlUiManager*>(glfwGetWindowUserPointer(w));
        if (ui) {
            ui->HandleChar(codepoint);
        }
    });
    glfwSetCursorPosCallback(window, [](GLFWwindow* w, double xpos, double ypos) {
        auto* ui = static_cast<UI::RmlUiManager*>(glfwGetWindowUserPointer(w));
        if (ui) {
            ui->HandleCursorPos(xpos, ypos);
        }
    });
    glfwSetMouseButtonCallback(window, [](GLFWwindow* w, int button, int action, int mods) {
        auto* ui = static_cast<UI::RmlUiManager*>(glfwGetWindowUserPointer(w));
        if (ui) {
            ui->HandleMouseButton(button, action, mods);
        }
    });
    glfwSetScrollCallback(window, [](GLFWwindow* w, double, double yoffset) {
        auto* ui = static_cast<UI::RmlUiManager*>(glfwGetWindowUserPointer(w));
        if (ui) {
            int mods = 0;
            if (glfwGetKey(w, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
                glfwGetKey(w, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS) {
                mods |= GLFW_MOD_SHIFT;
            }
            if (glfwGetKey(w, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
                glfwGetKey(w, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS) {
                mods |= GLFW_MOD_CONTROL;
            }
            if (glfwGetKey(w, GLFW_KEY_LEFT_ALT) == GLFW_PRESS ||
                glfwGetKey(w, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS) {
                mods |= GLFW_MOD_ALT;
            }
            ui->HandleScroll(yoffset, mods);
        }
    });
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* w, int width, int height) {
        auto* ui = static_cast<UI::RmlUiManager*>(glfwGetWindowUserPointer(w));
        if (ui) {
            ui->HandleFramebufferSize(width, height);
        }
    });

    std::cout << "\nRmlUi initialized successfully!" << std::endl;
    std::cout << "Displaying EVE Photon UI panels:" << std::endl;
    std::cout << "  - Ship HUD (bottom center) with health bars, speed, modules" << std::endl;
    std::cout << "  - Overview (top right) with entity table" << std::endl;
    std::cout << "  - Target List (top center) with locked target cards" << std::endl;
    std::cout << "  - Neocom (left sidebar) with service icons" << std::endl;
    std::cout << "  - Fitting (hidden, toggle with F1)" << std::endl;
    std::cout << "  - Inventory (hidden, toggle with F2)" << std::endl;
    std::cout << "  - D-Scan (hidden, toggle with F3)" << std::endl;
    std::cout << std::endl;

    // Add initial combat log messages
    rmlUi->AddCombatLogMessage("[12:34:56] Undocked from station");
    rmlUi->AddCombatLogMessage("[12:34:58] Warp drive active");
    rmlUi->AddCombatLogMessage("[12:35:02] Arrived at asteroid belt");

    // Initial ship status
    UI::ShipStatusData shipData;
    shipData.shield_pct = 0.85f;
    shipData.armor_pct = 1.0f;
    shipData.hull_pct = 1.0f;
    shipData.capacitor_pct = 0.7f;
    shipData.velocity = 45.5f;
    shipData.max_velocity = 380.0f;

    // Demo locked targets
    rmlUi->SetTarget("t1", "Venom Syndicate Spy", 0.6f, 1.0f, 1.0f, 12400.0f, true, true);
    rmlUi->SetTarget("t2", "Iron Corsairs Scout", 0.3f, 0.8f, 1.0f, 24500.0f, true, false);
    rmlUi->SetTarget("t3", "Asteroid Belt I", 1.0f, 1.0f, 1.0f, 45200.0f, false, false);

    // Demo inventory data
    {
        std::vector<std::string> names = {"200mm AutoCannon I", "Dustite", "Ferrium", "1MN Afterburner I"};
        std::vector<std::string> types = {"Weapon", "Ore", "Mineral", "Propulsion"};
        std::vector<int> quantities = {2, 1500, 3200, 1};
        std::vector<float> volumes = {5.0f, 0.1f, 0.01f, 5.0f};
        rmlUi->UpdateInventoryData(names, types, quantities, volumes, 42.0f, 100.0f);
    }

    // Demo D-Scan results
    {
        std::vector<std::string> names = {"Venom Syndicate Hideaway", "Asteroid Belt II", "Stargate (Thyrkstad)"};
        std::vector<std::string> types = {"Combat Site", "Asteroid Belt", "Stargate"};
        std::vector<float> distances = {0.5f, 2.3f, 8.1f};
        rmlUi->UpdateDScanResults(names, types, distances);
    }

    float startTime = static_cast<float>(glfwGetTime());

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Check for key toggles
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        static bool f1WasPressed = false;
        bool f1Pressed = glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS;
        if (f1Pressed && !f1WasPressed) {
            g_showFitting = !g_showFitting;
            rmlUi->SetDocumentVisible("fitting", g_showFitting);
            std::cout << "[UI] Fitting panel " << (g_showFitting ? "shown" : "hidden") << "\n";
        }
        f1WasPressed = f1Pressed;

        static bool f2WasPressed = false;
        bool f2Pressed = glfwGetKey(window, GLFW_KEY_F2) == GLFW_PRESS;
        if (f2Pressed && !f2WasPressed) {
            g_showInventory = !g_showInventory;
            rmlUi->SetDocumentVisible("inventory", g_showInventory);
            std::cout << "[UI] Inventory panel " << (g_showInventory ? "shown" : "hidden") << "\n";
        }
        f2WasPressed = f2Pressed;

        static bool f3WasPressed = false;
        bool f3Pressed = glfwGetKey(window, GLFW_KEY_F3) == GLFW_PRESS;
        if (f3Pressed && !f3WasPressed) {
            g_showDScan = !g_showDScan;
            rmlUi->SetDocumentVisible("dscan", g_showDScan);
            std::cout << "[UI] D-Scan panel " << (g_showDScan ? "shown" : "hidden") << "\n";
        }
        f3WasPressed = f3Pressed;

        float currentTime = static_cast<float>(glfwGetTime());
        float elapsed = currentTime - startTime;

        // Animate ship status
        shipData.shield_pct = 0.5f + 0.5f * std::sin(elapsed * 0.3f);
        shipData.armor_pct = 0.7f + 0.3f * std::cos(elapsed * 0.2f);
        shipData.hull_pct = 0.85f + 0.15f * std::sin(elapsed * 0.15f);
        shipData.capacitor_pct = 0.4f + 0.4f * std::sin(elapsed * 0.5f);
        shipData.velocity = 190.0f + 120.0f * std::sin(elapsed * 0.25f);

        rmlUi->SetShipStatus(shipData);

        // Add periodic combat log messages
        static int lastMessageTime = 0;
        int messageTime = static_cast<int>(elapsed) / 5;
        if (messageTime > lastMessageTime) {
            lastMessageTime = messageTime;
            char msg[128];
            std::snprintf(msg, sizeof(msg), "[%02d:%02d:%02d] Shield: %.0f%% | Cap: %.0f%%",
                12, 35 + static_cast<int>(elapsed) / 60,
                static_cast<int>(elapsed) % 60,
                shipData.shield_pct * 100.0f,
                shipData.capacitor_pct * 100.0f);
            rmlUi->AddCombatLogMessage(msg);
        }

        // Clear screen with EVE-style dark background
        glClearColor(0.01f, 0.015f, 0.025f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        // Update and render RmlUi
        rmlUi->ProcessInput();
        rmlUi->Update();
        rmlUi->BeginFrame();
        rmlUi->Render();
        rmlUi->EndFrame();

        glfwSwapBuffers(window);
    }

    // Cleanup
    rmlUi->Shutdown();
    rmlUi.reset();

    glfwDestroyWindow(window);
    glfwTerminate();

    std::cout << "\n=== Test Complete ===" << std::endl;
    return 0;
}
