/**
 * Test program for Phase 4.6 Advanced Features
 * Tests drag-and-drop inventory, module browser, and market panels
 */

#include <GL/glew.h>
#include "rendering/window.h"
#include "ui/ui_manager.h"
#include "ui/inventory_panel.h"
#include "ui/module_browser_panel.h"
#include "ui/market_panel.h"
#include <iostream>

using namespace eve;

int main() {
    std::cout << "[Test] Phase 4.6 Advanced Features Test Program" << std::endl;
    
    // Create window
    Window window("Phase 4.6 Advanced Features Test", 1600, 900);
    
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
    
    // Create new panels for Phase 4.6
    auto inventoryPanel = std::make_unique<UI::InventoryPanel>();
    auto moduleBrowser = std::make_unique<UI::ModuleBrowserPanel>();
    auto marketPanel = std::make_unique<UI::MarketPanel>();
    
    // Setup demo inventory data with drag-and-drop enabled
    UI::InventoryData invData;
    invData.cargo_capacity = 150.0f;
    invData.cargo_used = 67.8f;
    invData.hangar_capacity = 10000.0f;
    invData.hangar_used = 3421.5f;
    
    // Add cargo items
    invData.cargo_items.push_back(UI::InventoryItem("ore_dustite", "Dustite", 2000, 0.01f, "ore", "mining"));
    invData.cargo_items.push_back(UI::InventoryItem("ore_plagioclase", "Plagioclase", 800, 0.035f, "ore", "mining"));
    invData.cargo_items.push_back(UI::InventoryItem("ammo_emp_s", "EMP S", 5000, 0.0003f, "ammo", "weapon"));
    invData.cargo_items.push_back(UI::InventoryItem("module_shield_ext", "Medium Shield Extender II", 2, 5.0f, "module", "shield"));
    invData.cargo_items.push_back(UI::InventoryItem("module_damage_ctrl", "Damage Control II", 1, 5.0f, "module", "tank"));
    
    // Add hangar items
    invData.hangar_items.push_back(UI::InventoryItem("ship_fang", "Fang", 1, 2500.0f, "ship", "frigate"));
    invData.hangar_items.push_back(UI::InventoryItem("ship_thrasher", "Thrasher", 1, 3500.0f, "ship", "destroyer"));
    invData.hangar_items.push_back(UI::InventoryItem("ship_stabber", "Stabber", 1, 15000.0f, "ship", "cruiser"));
    invData.hangar_items.push_back(UI::InventoryItem("module_shield_booster", "Large Shield Booster II", 10, 50.0f, "module", "shield"));
    invData.hangar_items.push_back(UI::InventoryItem("module_armor_plate", "1600mm Steel Plates II", 5, 120.0f, "module", "armor"));
    invData.hangar_items.push_back(UI::InventoryItem("ore_kernite", "Kernite", 15000, 1.2f, "ore", "mining"));
    invData.hangar_items.push_back(UI::InventoryItem("mineral_ferrium", "Ferrium", 100000, 0.01f, "mineral", "materials"));
    invData.hangar_items.push_back(UI::InventoryItem("mineral_ignium", "Ignium", 25000, 0.01f, "mineral", "materials"));
    
    inventoryPanel->SetInventoryData(invData);
    inventoryPanel->SetVisible(true);
    inventoryPanel->SetDragDropEnabled(true);
    
    // Setup drag-drop callback
    inventoryPanel->SetDragDropCallback([](const std::string& item_id, int quantity, 
                                           bool from_cargo, bool to_cargo, bool to_space) {
        if (to_space) {
            std::cout << "[Test] Jettisoned " << quantity << "x " << item_id << " into space" << std::endl;
        } else {
            std::cout << "[Test] Transferred " << quantity << "x " << item_id 
                     << " from " << (from_cargo ? "cargo" : "hangar")
                     << " to " << (to_cargo ? "cargo" : "hangar") << std::endl;
        }
    });
    
    // Populate module browser with demo modules
    std::vector<UI::ModuleBrowserEntry> modules;
    
    // Weapons (high slots)
    modules.push_back(UI::ModuleBrowserEntry("weapon_200mm_ac_i", "200mm AutoCannon I", 
                                            "weapon", "projectile", 8.0f, 6.0f, "high"));
    modules.back().damage = 45.0f;
    modules.back().activation_time = 3.5f;
    modules.back().capacitor_use = 1.5f;
    modules.back().description = "Small projectile turret, rapid fire rate.";
    
    modules.push_back(UI::ModuleBrowserEntry("weapon_200mm_ac_ii", "200mm AutoCannon II", 
                                            "weapon", "projectile", 12.0f, 8.0f, "high"));
    modules.back().damage = 62.0f;
    modules.back().activation_time = 3.0f;
    modules.back().capacitor_use = 1.5f;
    modules.back().meta_level = 5.0f;
    modules.back().description = "Tech II projectile turret with enhanced damage.";
    
    modules.push_back(UI::ModuleBrowserEntry("weapon_light_missile", "Light Missile Launcher I", 
                                            "weapon", "missile", 10.0f, 5.0f, "high"));
    modules.back().damage = 55.0f;
    modules.back().activation_time = 7.0f;
    modules.back().description = "Guided missile launcher for frigates.";
    
    // Shield modules (mid slots)
    modules.push_back(UI::ModuleBrowserEntry("shield_ext_medium_i", "Medium Shield Extender I", 
                                            "shield", "passive", 18.0f, 12.0f, "mid"));
    modules.back().shield_hp = 1200.0f;
    modules.back().description = "Increases maximum shield capacity.";
    
    modules.push_back(UI::ModuleBrowserEntry("shield_ext_medium_ii", "Medium Shield Extender II", 
                                            "shield", "passive", 24.0f, 15.0f, "mid"));
    modules.back().shield_hp = 1600.0f;
    modules.back().meta_level = 5.0f;
    modules.back().description = "Tech II shield extender with increased capacity.";
    
    modules.push_back(UI::ModuleBrowserEntry("shield_booster_medium", "Medium Shield Booster II", 
                                            "shield", "active", 22.0f, 18.0f, "mid"));
    modules.back().shield_hp = 120.0f;
    modules.back().activation_time = 5.0f;
    modules.back().capacitor_use = 45.0f;
    modules.back().meta_level = 5.0f;
    modules.back().description = "Active shield repair module.";
    
    // Propulsion (mid slots)
    modules.push_back(UI::ModuleBrowserEntry("prop_1mn_ab_i", "1MN Afterburner I", 
                                            "propulsion", "speed", 12.0f, 8.0f, "mid"));
    modules.back().speed_bonus = 200.0f;
    modules.back().activation_time = 10.0f;
    modules.back().capacitor_use = 12.0f;
    modules.back().description = "Increases ship velocity significantly.";
    
    modules.push_back(UI::ModuleBrowserEntry("prop_1mn_ab_ii", "1MN Afterburner II", 
                                            "propulsion", "speed", 18.0f, 10.0f, "mid"));
    modules.back().speed_bonus = 250.0f;
    modules.back().activation_time = 10.0f;
    modules.back().capacitor_use = 10.0f;
    modules.back().meta_level = 5.0f;
    modules.back().description = "Tech II afterburner with improved efficiency.";
    
    // EWAR (mid slots)
    modules.push_back(UI::ModuleBrowserEntry("ewar_web_i", "Stasis Webifier I", 
                                            "ewar", "web", 8.0f, 4.0f, "mid"));
    modules.back().description = "Reduces target's velocity by 50%.";
    
    modules.push_back(UI::ModuleBrowserEntry("ewar_scram", "Warp Scrambler II", 
                                            "ewar", "tackle", 10.0f, 5.0f, "mid"));
    modules.back().meta_level = 5.0f;
    modules.back().description = "Prevents target from warping.";
    
    // Damage mods (low slots)
    modules.push_back(UI::ModuleBrowserEntry("damage_gyro_i", "Gyrostabilizer I", 
                                            "damage", "projectile", 12.0f, 1.0f, "low"));
    modules.back().description = "Increases projectile weapon damage by 10%.";
    
    modules.push_back(UI::ModuleBrowserEntry("damage_gyro_ii", "Gyrostabilizer II", 
                                            "damage", "projectile", 20.0f, 1.0f, "low"));
    modules.back().meta_level = 5.0f;
    modules.back().description = "Increases projectile weapon damage by 15%.";
    
    // Armor (low slots)
    modules.push_back(UI::ModuleBrowserEntry("armor_plate_800", "800mm Steel Plates I", 
                                            "armor", "passive", 10.0f, 5.0f, "low"));
    modules.back().armor_hp = 2400.0f;
    modules.back().description = "Increases maximum armor capacity.";
    
    modules.push_back(UI::ModuleBrowserEntry("armor_plate_1600", "1600mm Steel Plates II", 
                                            "armor", "passive", 15.0f, 8.0f, "low"));
    modules.back().armor_hp = 5200.0f;
    modules.back().meta_level = 5.0f;
    modules.back().description = "Large armor plating for cruisers.";
    
    // Rigs
    modules.push_back(UI::ModuleBrowserEntry("rig_burst_aerator", "Small Burst Aerator I", 
                                            "rig", "weapon", 0.0f, 0.0f, "rig"));
    modules.back().description = "Reduces projectile weapon activation time by 10%.";
    
    modules.push_back(UI::ModuleBrowserEntry("rig_anti_em", "Small Anti-EM Screen Reinforcer I", 
                                            "rig", "shield", 0.0f, 0.0f, "rig"));
    modules.back().description = "Increases shield EM resistance by 15%.";
    
    moduleBrowser->SetModules(modules);
    moduleBrowser->SetVisible(true);
    
    // Setup module browser callbacks
    moduleBrowser->SetBrowseCallback([](const std::string& module_id) {
        std::cout << "[Test] Browsing module: " << module_id << std::endl;
    });
    
    moduleBrowser->SetFitCallback([](const std::string& module_id) {
        std::cout << "[Test] Fitting module: " << module_id << std::endl;
    });
    
    // Populate market with demo items and orders
    std::vector<UI::MarketItem> items;
    items.push_back(UI::MarketItem("ore_dustite", "Dustite", "Ore", "Common Ore", 10.5f));
    items.push_back(UI::MarketItem("ore_plagioclase", "Plagioclase", "Ore", "Common Ore", 28.0f));
    items.push_back(UI::MarketItem("ore_kernite", "Kernite", "Ore", "Uncommon Ore", 125.0f));
    items.push_back(UI::MarketItem("mineral_ferrium", "Ferrium", "Mineral", "Common Mineral", 5.2f));
    items.push_back(UI::MarketItem("mineral_ignium", "Ignium", "Mineral", "Common Mineral", 7.8f));
    items.push_back(UI::MarketItem("mineral_allonium", "Allonium", "Mineral", "Uncommon Mineral", 45.0f));
    items.push_back(UI::MarketItem("ammo_emp_s", "EMP S", "Ammunition", "Small Ammo", 15.5f));
    items.push_back(UI::MarketItem("module_shield_ext", "Medium Shield Extender II", "Module", "Shield", 2500000.0f));
    items.push_back(UI::MarketItem("module_damage_gyro", "Gyrostabilizer II", "Module", "Damage", 1800000.0f));
    items.push_back(UI::MarketItem("ship_fang", "Fang", "Ship", "Frigate", 450000.0f));
    
    marketPanel->SetAvailableItems(items);
    
    // Add some buy orders
    std::vector<UI::MarketOrder> buyOrders;
    buyOrders.push_back(UI::MarketOrder("buy_01", "Dustite", "ore_dustite", true, 10.2f, 50000, "Thyrkstad Station"));
    buyOrders.push_back(UI::MarketOrder("buy_02", "Dustite", "ore_dustite", true, 9.8f, 100000, "Thyrkstad Station"));
    buyOrders.push_back(UI::MarketOrder("buy_03", "Ferrium", "mineral_ferrium", true, 5.0f, 1000000, "Solari Station"));
    buyOrders.push_back(UI::MarketOrder("buy_04", "Ferrium", "mineral_ferrium", true, 4.8f, 500000, "Aurendis"));
    
    // Add some sell orders
    std::vector<UI::MarketOrder> sellOrders;
    sellOrders.push_back(UI::MarketOrder("sell_01", "Dustite", "ore_dustite", false, 10.8f, 30000, "Thyrkstad Station"));
    sellOrders.push_back(UI::MarketOrder("sell_02", "Dustite", "ore_dustite", false, 11.2f, 75000, "Thyrkstad Station"));
    sellOrders.push_back(UI::MarketOrder("sell_03", "Ferrium", "mineral_ferrium", false, 5.5f, 800000, "Solari Station"));
    sellOrders.push_back(UI::MarketOrder("sell_04", "Ferrium", "mineral_ferrium", false, 5.8f, 250000, "Kelheim Station"));
    
    marketPanel->SetBuyOrders(buyOrders);
    marketPanel->SetSellOrders(sellOrders);
    marketPanel->SetVisible(true);
    
    // Setup market callbacks
    marketPanel->SetQuickBuyCallback([](const std::string& item_id, int quantity) {
        std::cout << "[Test] Quick buy: " << quantity << "x " << item_id << std::endl;
    });
    
    marketPanel->SetQuickSellCallback([](const std::string& item_id, int quantity) {
        std::cout << "[Test] Quick sell: " << quantity << "x " << item_id << std::endl;
    });
    
    std::cout << "[Test] All panels initialized with demo data" << std::endl;
    std::cout << "[Test] ====================" << std::endl;
    std::cout << "[Test] Features to test:" << std::endl;
    std::cout << "[Test] 1. Drag items between cargo and hangar in Inventory" << std::endl;
    std::cout << "[Test] 2. Drag items to jettison zone to drop into space" << std::endl;
    std::cout << "[Test] 3. Search and filter modules in Module Browser" << std::endl;
    std::cout << "[Test] 4. Double-click modules to fit them" << std::endl;
    std::cout << "[Test] 5. Browse market items and view order book" << std::endl;
    std::cout << "[Test] 6. Use Quick Trade tab for instant buy/sell" << std::endl;
    std::cout << "[Test] ====================" << std::endl;
    
    // Main loop
    while (!window.shouldClose()) {
        // Process events (handled by window.update() at end of loop)
        
        // Clear screen
        glClearColor(0.05f, 0.08f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Start ImGui frame
        uiManager->BeginFrame();
        
        // Render all panels
        inventoryPanel->Render();
        moduleBrowser->Render();
        marketPanel->Render();
        
        // End ImGui frame and render
        uiManager->EndFrame();
        
        // Update window (swap buffers + poll events)
        window.update();
    }
    
    std::cout << "[Test] Shutting down..." << std::endl;
    uiManager->Shutdown();
    
    return 0;
}
