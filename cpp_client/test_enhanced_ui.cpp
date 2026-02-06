/**
 * Test program for Phase 4.5 Enhanced UI panels
 * Tests inventory, fitting, and mission panels with demo data
 */

#include <GL/glew.h>
#include "rendering/window.h"
#include "ui/ui_manager.h"
#include "ui/inventory_panel.h"
#include "ui/fitting_panel.h"
#include "ui/mission_panel.h"
#include <iostream>
#include <cmath>

using namespace eve;

int main() {
    std::cout << "[Test] Phase 4.5 Enhanced UI Test Program" << std::endl;
    
    // Create window
    Window window("Phase 4.5 Enhanced UI Test", 1280, 720);
    
    // Initialize GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "[Test] Failed to initialize GLEW" << std::endl;
        return -1;
    }
    
    // Create UI manager
    auto uiManager = std::make_unique<UI::UIManager>();
    if (!uiManager->Initialize(window.getHandle())) {
        std::cerr << "[Test] Failed to initialize UI Manager" << std::endl;
        return -1;
    }
    
    std::cout << "[Test] UI Manager initialized successfully" << std::endl;
    
    // Get panels
    auto* inventoryPanel = uiManager->GetInventoryPanel();
    auto* fittingPanel = uiManager->GetFittingPanel();
    auto* missionPanel = uiManager->GetMissionPanel();
    
    // Setup demo inventory data
    UI::InventoryData invData;
    invData.cargo_capacity = 100.0f;
    invData.cargo_used = 45.5f;
    invData.hangar_capacity = 10000.0f;
    invData.hangar_used = 2350.0f;
    
    // Add cargo items
    invData.cargo_items.push_back(UI::InventoryItem("ore_veldspar", "Veldspar", 1000, 0.01f, "ore", "mining"));
    invData.cargo_items.push_back(UI::InventoryItem("ore_plagioclase", "Plagioclase", 500, 0.035f, "ore", "mining"));
    invData.cargo_items.push_back(UI::InventoryItem("ammo_emp_s", "EMP S", 2000, 0.0003f, "ammo", "weapon"));
    invData.cargo_items.push_back(UI::InventoryItem("module_shield_extender", "Medium Shield Extender II", 1, 5.0f, "module", "shield"));
    
    // Add hangar items
    invData.hangar_items.push_back(UI::InventoryItem("ship_rifter", "Rifter", 1, 2500.0f, "ship", "frigate"));
    invData.hangar_items.push_back(UI::InventoryItem("ship_thrasher", "Thrasher", 1, 3500.0f, "ship", "destroyer"));
    invData.hangar_items.push_back(UI::InventoryItem("module_shield_booster", "Large Shield Booster II", 5, 50.0f, "module", "shield"));
    invData.hangar_items.push_back(UI::InventoryItem("ore_kernite", "Kernite", 10000, 1.2f, "ore", "mining"));
    invData.hangar_items.push_back(UI::InventoryItem("mineral_tritanium", "Tritanium", 50000, 0.01f, "mineral", "materials"));
    
    inventoryPanel->SetInventoryData(invData);
    
    // Setup demo fitting data
    UI::FittingData fittingData;
    fittingData.ship_name = "My Rifter";
    fittingData.ship_type = "Frigate (Minmatar)";
    fittingData.cpu_used = 85.3f;
    fittingData.cpu_max = 120.0f;
    fittingData.powergrid_used = 42.5f;
    fittingData.powergrid_max = 50.0f;
    
    // Add some fitted modules
    fittingData.high_slots[0] = UI::ModuleInfo("weapon_200mm_ac", "200mm AutoCannon II", "weapon", 12.0f, 8.0f, true, false);
    fittingData.high_slots[1] = UI::ModuleInfo("weapon_200mm_ac", "200mm AutoCannon II", "weapon", 12.0f, 8.0f, true, false);
    fittingData.high_slots[2] = UI::ModuleInfo("weapon_200mm_ac", "200mm AutoCannon II", "weapon", 12.0f, 8.0f, true, false);
    
    fittingData.mid_slots[0] = UI::ModuleInfo("prop_ab", "1MN Afterburner II", "propulsion", 18.0f, 10.0f, true, false);
    fittingData.mid_slots[1] = UI::ModuleInfo("ewar_web", "Stasis Webifier II", "ewar", 10.0f, 5.0f, false, false);
    
    fittingData.low_slots[0] = UI::ModuleInfo("damage_gyro", "Gyrostabilizer II", "damage", 20.0f, 1.0f, true, false);
    fittingData.low_slots[1] = UI::ModuleInfo("tank_nano", "Nanofiber Internal Structure II", "tank", 5.0f, 0.5f, true, false);
    
    fittingData.rig_slots[0] = UI::ModuleInfo("rig_burst_aerator", "Small Burst Aerator I", "rig", 0.0f, 0.0f, true, false);
    
    fittingPanel->SetFittingData(fittingData);
    
    // Setup demo mission data
    UI::MissionData missionData;
    missionData.is_active = true;
    missionData.mission_id = "mission_001";
    missionData.mission_name = "Clear the Serpentis Base";
    missionData.mission_type = "combat";
    missionData.agent_name = "Agent Smith";
    missionData.location = "Rens VII - Moon 17";
    missionData.level = 2;
    missionData.isk_reward = 450000.0f;
    missionData.lp_reward = 225.0f;
    missionData.item_rewards.push_back("150mm AutoCannon II Blueprint Copy");
    missionData.time_limit = 4.0f;  // 4 hours
    missionData.time_elapsed = 1.25f;  // 1.25 hours elapsed
    
    // Add objectives
    missionData.objectives.push_back(UI::MissionObjective("Warp to mission location", true));
    missionData.objectives.push_back(UI::MissionObjective("Destroy 10 Serpentis Frigates", true));
    missionData.objectives.push_back(UI::MissionObjective("Destroy 5 Serpentis Cruisers", false));
    missionData.objectives.push_back(UI::MissionObjective("Destroy mission boss 'Serpentis Commander'", false));
    missionData.objectives.push_back(UI::MissionObjective("Return to agent", false));
    
    missionPanel->SetMissionData(missionData);
    
    // Show all panels on startup
    inventoryPanel->SetVisible(true);
    fittingPanel->SetVisible(true);
    missionPanel->SetVisible(true);
    
    // Setup callbacks
    inventoryPanel->SetTransferCallback([](const std::string& item_id, bool to_hangar) {
        std::cout << "[Test] Transfer item: " << item_id << " to " 
                  << (to_hangar ? "hangar" : "cargo") << std::endl;
    });
    
    inventoryPanel->SetJettisonCallback([](const std::string& item_id, int quantity) {
        std::cout << "[Test] Jettison item: " << item_id << " x" << quantity << std::endl;
    });
    
    fittingPanel->SetUnfitModuleCallback([](const std::string& slot_type, int slot_index) {
        std::cout << "[Test] Unfit module from " << slot_type << " slot " << slot_index << std::endl;
    });
    
    fittingPanel->SetOnlineModuleCallback([](const std::string& slot_type, int slot_index, bool online) {
        std::cout << "[Test] Set module " << slot_type << " slot " << slot_index 
                  << " to " << (online ? "online" : "offline") << std::endl;
    });
    
    missionPanel->SetCompleteCallback([](const std::string& mission_id) {
        std::cout << "[Test] Complete mission: " << mission_id << std::endl;
    });
    
    missionPanel->SetDeclineCallback([](const std::string& mission_id) {
        std::cout << "[Test] Decline mission: " << mission_id << std::endl;
    });
    
    std::cout << "[Test] Starting render loop. Press ESC to exit." << std::endl;
    std::cout << "[Test] Panel visibility can be toggled via panel close buttons." << std::endl;
    
    // Animation state
    double time = 0.0;
    
    // Main loop
    while (!window.shouldClose()) {
        time += 0.016;  // ~60 FPS
        
        // Clear screen
        glClearColor(0.05f, 0.05f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        // Update UI
        uiManager->BeginFrame();
        uiManager->Render();
        uiManager->EndFrame();
        
        // Update window
        window.update();
        
        // Check for ESC key
        if (glfwGetKey(window.getHandle(), GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            break;
        }
        
        // Toggle panels with F-keys
        static bool f1_pressed = false;
        if (glfwGetKey(window.getHandle(), GLFW_KEY_F1) == GLFW_PRESS) {
            if (!f1_pressed) {
                inventoryPanel->SetVisible(!inventoryPanel->IsVisible());
                f1_pressed = true;
            }
        } else {
            f1_pressed = false;
        }
        
        static bool f2_pressed = false;
        if (glfwGetKey(window.getHandle(), GLFW_KEY_F2) == GLFW_PRESS) {
            if (!f2_pressed) {
                fittingPanel->SetVisible(!fittingPanel->IsVisible());
                f2_pressed = true;
            }
        } else {
            f2_pressed = false;
        }
        
        static bool f3_pressed = false;
        if (glfwGetKey(window.getHandle(), GLFW_KEY_F3) == GLFW_PRESS) {
            if (!f3_pressed) {
                missionPanel->SetVisible(!missionPanel->IsVisible());
                f3_pressed = true;
            }
        } else {
            f3_pressed = false;
        }
    }
    
    // Cleanup
    std::cout << "[Test] Shutting down..." << std::endl;
    uiManager->Shutdown();
    
    std::cout << "[Test] Test completed successfully!" << std::endl;
    return 0;
}
