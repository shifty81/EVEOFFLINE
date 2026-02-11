/**
 * Test program for Phase 4.8 GUI panels: D-Scan and Neocom sidebar.
 * Also validates the enhanced module rack with data-bound slots.
 *
 * This is a visual/interactive test that renders the new panels in a
 * GLFW window. Press ESC to exit.
 *
 * Keyboard shortcuts:
 *   F1  — toggle D-Scan panel
 *   F2  — toggle Neocom sidebar
 *   V   — trigger a D-Scan scan
 *   ESC — exit
 */

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include "rendering/window.h"
#include "ui/ui_manager.h"
#include "ui/dscan_panel.h"
#include "ui/sidebar_panel.h"
#include "ui/hud_panels.h"
#include "ui/atlas/atlas_context.h"
#include <iostream>
#include <cmath>

using namespace eve;

// ---------------------------------------------------------------------------
// Demo data for D-Scan
// ---------------------------------------------------------------------------
static std::vector<UI::DScanResult> GenerateDemoResults() {
    return {
        {"Venom Syndicate Frigate",  "Frigate",    0.32f, "npc_001"},
        {"Asteroid Belt VII", "Asteroid",    1.25f, "belt_007"},
        {"Iron Corsairs Cruiser",  "Cruiser",     3.80f, "npc_002"},
        {"Customs Office",    "Structure",   0.05f, "struct_001"},
        {"Player Capsule",    "Capsule",     7.12f, "player_003"},
        {"Stargate",          "Stargate",   12.50f, "gate_001"},
        {"Crimson Order BS",   "Battleship",  0.78f, "npc_003"},
        {"Mobile Depot",      "Structure",   0.15f, "struct_002"},
    };
}

// ---------------------------------------------------------------------------
int main() {
    std::cout << "[Test] Phase 4.8 D-Scan + Neocom + Module Rack Test" << std::endl;

    // Create window
    Window window("Phase 4.8 D-Scan / Neocom Test", 1280, 720);

    // Initialize GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "[Test] GLEW init failed" << std::endl;
        return -1;
    }

    // Create UI manager (this now includes D-Scan and Neocom)
    auto uiManager = std::make_unique<UI::UIManager>();
    if (!uiManager->Initialize(window.getHandle())) {
        std::cerr << "[Test] UIManager init failed" << std::endl;
        return -1;
    }
    std::cout << "[Test] UIManager initialized" << std::endl;

    // ---- Setup Photon context for module rack rendering ----
    atlas::AtlasContext atlasCtx;

    // ---- Setup demo ship status ----
    UI::ShipStatus shipStatus;
    shipStatus.shields = 85.0f;
    shipStatus.shields_max = 100.0f;
    shipStatus.armor = 60.0f;
    shipStatus.armor_max = 100.0f;
    shipStatus.hull = 95.0f;
    shipStatus.hull_max = 100.0f;
    shipStatus.capacitor = 70.0f;
    shipStatus.capacitor_max = 100.0f;
    shipStatus.velocity = 55.5f;
    shipStatus.max_velocity = 120.0f;
    uiManager->SetShipStatus(shipStatus);

    // ---- Make D-Scan visible with demo data ----
    auto* dscan = uiManager->GetDScanPanel();
    dscan->SetResults(GenerateDemoResults());
    // Show the D-Scan panel in the docking manager
    uiManager->SetPanelVisible("dscan", true);

    // ---- D-Scan callback ----
    dscan->SetScanCallback([&](float angle, float range) {
        std::cout << "[Test] D-Scan triggered: angle=" << angle
                  << "° range=" << range << " AU" << std::endl;
        // Re-populate with demo data
        dscan->SetResults(GenerateDemoResults());
    });

    // ---- Neocom is visible by default ----
    auto* neocom = uiManager->GetSidebarPanel();
    neocom->SetVisible(true);

    // Wire up a few Neocom callbacks for testing
    neocom->SetCharacterSheetCallback([]() {
        std::cout << "[Test] Neocom: Character Sheet clicked" << std::endl;
    });
    neocom->SetSettingsCallback([]() {
        std::cout << "[Test] Neocom: Settings clicked" << std::endl;
    });

    // ---- Prepare module rack demo data ----
    UI::ModuleSlotState moduleSlots[8];
    // High slots
    moduleSlots[0] = {true, true,  false, 0.0f, "200mm AC II", UI::ModuleSlotState::HIGH};
    moduleSlots[1] = {true, true,  false, 0.0f, "200mm AC II", UI::ModuleSlotState::HIGH};
    moduleSlots[2] = {true, false, false, 0.6f, "Rocket Lnch", UI::ModuleSlotState::HIGH};
    // Mid slots
    moduleSlots[3] = {true, true,  false, 0.0f, "1MN AB II",   UI::ModuleSlotState::MID};
    moduleSlots[4] = {true, false, false, 0.0f, "Web II",      UI::ModuleSlotState::MID};
    moduleSlots[5] = {false, false, false, 0.0f, "",            UI::ModuleSlotState::MID};
    // Low slots
    moduleSlots[6] = {true, true,  true,  0.0f, "Gyro II",     UI::ModuleSlotState::LOW};
    moduleSlots[7] = {true, false, false, 0.3f, "DCU II",      UI::ModuleSlotState::LOW};

    std::cout << "[Test] Entering render loop. ESC to exit." << std::endl;
    std::cout << "  F1 = toggle D-Scan, F2 = toggle Neocom, V = scan" << std::endl;

    double time = 0.0;

    while (!window.shouldClose()) {
        time += 0.016;

        glClearColor(0.03f, 0.03f, 0.06f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Animate capacitor and velocity
        shipStatus.capacitor = 50.0f + 50.0f * std::max(0.0f, (float)sin(time * 0.4));
        shipStatus.velocity  = 55.5f + 30.0f * (float)sin(time * 0.3);
        uiManager->SetShipStatus(shipStatus);

        // Animate a cooldown timer on slot 2
        moduleSlots[2].cooldown_pct = 0.5f + 0.5f * (float)sin(time * 0.8);

        // Render UI
        uiManager->BeginFrame();
        uiManager->Render();

        // Render data-bound module rack in a separate window for demo
        {
            ImGui::SetNextWindowPos(ImVec2(400, 640), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(380, 60), ImGuiCond_FirstUseEver);
            ImGui::Begin("##ModuleRackDemo", nullptr,
                ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground |
                ImGuiWindowFlags_NoScrollbar);
            UI::HUDPanels::RenderModuleRack(atlasCtx, moduleSlots, 8);
            ImGui::End();
        }

        uiManager->EndFrame();

        window.update();

        // ---- Hotkeys ----
        if (glfwGetKey(window.getHandle(), GLFW_KEY_ESCAPE) == GLFW_PRESS) break;

        static bool f1_prev = false;
        bool f1 = glfwGetKey(window.getHandle(), GLFW_KEY_F1) == GLFW_PRESS;
        if (f1 && !f1_prev) uiManager->ToggleDScan();
        f1_prev = f1;

        static bool f2_prev = false;
        bool f2 = glfwGetKey(window.getHandle(), GLFW_KEY_F2) == GLFW_PRESS;
        if (f2 && !f2_prev) {
            neocom->SetVisible(!neocom->IsVisible());
        }
        f2_prev = f2;

        static bool v_prev = false;
        bool v = glfwGetKey(window.getHandle(), GLFW_KEY_V) == GLFW_PRESS;
        if (v && !v_prev) {
            std::cout << "[Test] V pressed — performing scan" << std::endl;
            dscan->SetResults(GenerateDemoResults());
        }
        v_prev = v;
    }

    uiManager->Shutdown();
    std::cout << "[Test] Test completed successfully!" << std::endl;
    return 0;
}
