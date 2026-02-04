/**
 * Test ImGui UI Integration
 * Simple test to demonstrate the UI system with EVE-styled panels
 */

#include <iostream>
#include <cmath>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "rendering/window.h"
#include "ui/ui_manager.h"

using namespace eve;

int main() {
    std::cout << "=== EVE UI System Test ===" << std::endl;
    
    try {
        // Create window
        Window window("EVE UI Test", 1280, 720);
        
        // Initialize UI Manager
        auto uiManager = std::make_unique<UI::UIManager>();
        if (!uiManager->Initialize(window.getHandle())) {
            std::cerr << "Failed to initialize UI Manager. Ensure GLFW window is valid and OpenGL context is current." << std::endl;
            return -1;
        }
        
        // Setup demo ship status with animated values
        UI::ShipStatus shipStatus;
        shipStatus.shields = 85.0f;
        shipStatus.shields_max = 100.0f;
        shipStatus.armor = 65.0f;
        shipStatus.armor_max = 100.0f;
        shipStatus.hull = 95.0f;
        shipStatus.hull_max = 100.0f;
        shipStatus.capacitor = 70.0f;
        shipStatus.capacitor_max = 100.0f;
        shipStatus.velocity = 45.5f;
        shipStatus.max_velocity = 120.0f;
        uiManager->SetShipStatus(shipStatus);
        
        // Setup target info
        UI::TargetInfo targetInfo;
        targetInfo.name = "Hostile Frigate";
        targetInfo.shields = 30.0f;
        targetInfo.shields_max = 100.0f;
        targetInfo.armor = 50.0f;
        targetInfo.armor_max = 100.0f;
        targetInfo.hull = 80.0f;
        targetInfo.hull_max = 100.0f;
        targetInfo.distance = 2450.0f;
        targetInfo.is_hostile = true;
        targetInfo.is_locked = true;
        uiManager->SetTargetInfo(targetInfo);
        
        // Add some combat log messages
        uiManager->AddCombatLogMessage("[12:34:56] Locked target: Hostile Frigate");
        uiManager->AddCombatLogMessage("[12:34:58] Activated weapons");
        uiManager->AddCombatLogMessage("[12:35:00] Hit! 250 damage dealt");
        uiManager->AddCombatLogMessage("[12:35:02] Target shields depleted");
        uiManager->AddCombatLogMessage("[12:35:04] Target armor taking damage");
        
        std::cout << "\nUI System initialized successfully!" << std::endl;
        std::cout << "Displaying EVE-styled HUD panels:" << std::endl;
        std::cout << "  - Ship Status (bottom left)" << std::endl;
        std::cout << "  - Target Info (top right)" << std::endl;
        std::cout << "  - Speed Panel (top left)" << std::endl;
        std::cout << "  - Combat Log (bottom center)" << std::endl;
        std::cout << "\nPress ESC to exit" << std::endl;
        
        // Main loop
        float startTime = glfwGetTime();
        
        while (!window.shouldClose()) {
            float currentTime = glfwGetTime();
            float elapsed = currentTime - startTime;
            
            // Clear screen with EVE-style dark background
            glClearColor(0.01f, 0.01f, 0.02f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            
            // Animate values for demo
            shipStatus.shields = 50.0f + 50.0f * std::max(0.0f, sinf(elapsed * 0.5f));
            shipStatus.velocity = 45.5f + 30.0f * sinf(elapsed * 0.3f);
            uiManager->SetShipStatus(shipStatus);
            
            // Animate target health
            targetInfo.shields = 30.0f * std::max(0.0f, cosf(elapsed * 0.4f));
            targetInfo.distance = 2450.0f + 500.0f * sinf(elapsed * 0.2f);
            uiManager->SetTargetInfo(targetInfo);
            
            // Add combat messages periodically
            if (static_cast<int>(elapsed) % 3 == 0 && 
                static_cast<int>(elapsed * 10) % 10 == 0) {
                char msg[64];
                snprintf(msg, sizeof(msg), "[%02d:%02d:%02d] Damage: %.0f", 
                        static_cast<int>(elapsed) / 60,
                        static_cast<int>(elapsed) % 60,
                        static_cast<int>(elapsed * 100) % 100,
                        100.0f + 50.0f * sinf(elapsed));
                uiManager->AddCombatLogMessage(msg);
            }
            
            // Render UI
            uiManager->BeginFrame();
            uiManager->Render();
            uiManager->EndFrame();
            
            // Update window
            window.update();
        }
        
        // Cleanup
        uiManager->Shutdown();
        
        std::cout << "\n=== Test Complete ===" << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }
}
