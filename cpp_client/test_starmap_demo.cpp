#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "ui/star_map.h"
#include "core/ship_physics.h"

// Global state for interaction
eve::StarMap* g_starMap = nullptr;
bool g_mousePressed = false;

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_ESCAPE) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
        else if (key == GLFW_KEY_F10) {
            // Toggle star map (EVE's default key)
            if (g_starMap) {
                g_starMap->toggle();
                std::cout << "Star map " << (g_starMap->isVisible() ? "opened" : "closed") << std::endl;
            }
        }
        else if (key == GLFW_KEY_1) {
            if (g_starMap) {
                g_starMap->setViewMode(eve::StarMap::ViewMode::GALAXY);
                std::cout << "Switched to galaxy view" << std::endl;
            }
        }
        else if (key == GLFW_KEY_2) {
            if (g_starMap) {
                g_starMap->setViewMode(eve::StarMap::ViewMode::SOLAR_SYSTEM);
                std::cout << "Switched to solar system view" << std::endl;
            }
        }
        else if (key == GLFW_KEY_R) {
            if (g_starMap) {
                g_starMap->resetCamera();
                std::cout << "Reset camera" << std::endl;
            }
        }
    }
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            g_mousePressed = true;
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            if (g_starMap) {
                g_starMap->handleMouseClick((int)xpos, (int)ypos);
            }
        } else if (action == GLFW_RELEASE) {
            g_mousePressed = false;
        }
    }
}

void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    if (g_starMap) {
        g_starMap->handleMouseMove((int)xpos, (int)ypos);
    }
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    if (g_starMap) {
        g_starMap->handleMouseScroll((float)yoffset);
    }
}

int main() {
    std::cout << "=== EVE OFFLINE - Star Map & Ship Physics Demo ===" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  F10 - Toggle star map" << std::endl;
    std::cout << "  1 - Galaxy view" << std::endl;
    std::cout << "  2 - Solar system view" << std::endl;
    std::cout << "  R - Reset camera" << std::endl;
    std::cout << "  Mouse drag - Rotate map" << std::endl;
    std::cout << "  Mouse scroll - Zoom" << std::endl;
    std::cout << "  ESC - Exit" << std::endl;
    std::cout << std::endl;

    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Create window
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    GLFWwindow* window = glfwCreateWindow(1280, 720, "EVE OFFLINE - Star Map Demo", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetScrollCallback(window, scrollCallback);

    // Initialize GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.05f, 1.0f);

    // Create star map
    eve::StarMap starMap;
    g_starMap = &starMap;
    starMap.initialize("../data/universe/systems.json");
    starMap.setVisible(true);  // Show by default for demo

    // Create ship physics example
    eve::ShipPhysics shipPhysics;
    eve::ShipPhysics::ShipStats frigateStats;
    frigateStats.mass = 1200000.0f;
    frigateStats.inertiaModifier = 3.2f;
    frigateStats.maxVelocity = 400.0f;
    frigateStats.signatureRadius = 35.0f;
    shipPhysics.setShipStats(frigateStats);

    std::cout << "\n=== Ship Physics Test ===" << std::endl;
    std::cout << "Frigate Stats:" << std::endl;
    std::cout << "  Mass: " << frigateStats.mass << " kg" << std::endl;
    std::cout << "  Inertia Modifier: " << frigateStats.inertiaModifier << std::endl;
    std::cout << "  Max Velocity: " << frigateStats.maxVelocity << " m/s" << std::endl;
    std::cout << "  Agility: " << frigateStats.getAgility() << std::endl;
    std::cout << "  Align Time: " << frigateStats.getAlignTime() << " seconds" << std::endl;

    // Test acceleration
    shipPhysics.setDesiredDirection(glm::vec3(0.0f, 0.0f, 1.0f));
    std::cout << "\nAccelerating to max velocity..." << std::endl;
    
    float totalTime = 0.0f;
    float lastTime = 0.0f;
    bool reached75 = false;
    
    for (int i = 0; i < 100; i++) {
        float dt = 0.1f;  // 100ms per step
        shipPhysics.update(dt);
        totalTime += dt;
        
        float speedPct = shipPhysics.getSpeedPercentage();
        
        if (!reached75 && speedPct >= 0.75f) {
            std::cout << "  Reached 75% velocity (warp align) at " << totalTime << " seconds" << std::endl;
            std::cout << "    (Calculated align time: " << frigateStats.getAlignTime() << " seconds)" << std::endl;
            reached75 = true;
        }
        
        if (i % 10 == 0) {
            std::cout << "  Time: " << totalTime << "s, Speed: " 
                     << shipPhysics.getCurrentSpeed() << " m/s (" 
                     << (speedPct * 100.0f) << "%)" << std::endl;
        }
    }

    // Main loop
    float lastFrameTime = (float)glfwGetTime();
    
    while (!glfwWindowShouldClose(window)) {
        float currentTime = (float)glfwGetTime();
        float deltaTime = currentTime - lastFrameTime;
        lastFrameTime = currentTime;

        // Poll events
        glfwPollEvents();

        // Update
        starMap.update(deltaTime);

        // Render
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        starMap.render();

        // Swap buffers
        glfwSwapBuffers(window);
    }

    // Cleanup
    g_starMap = nullptr;
    glfwDestroyWindow(window);
    glfwTerminate();

    std::cout << "\nDemo completed successfully!" << std::endl;
    return 0;
}
