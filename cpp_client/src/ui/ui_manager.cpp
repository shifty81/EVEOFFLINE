#include "ui/ui_manager.h"
#include "ui/eve_panels.h"
#include "ui/eve_target_list.h"
#include "ui/inventory_panel.h"
#include "ui/fitting_panel.h"
#include "ui/mission_panel.h"
#include "ui/overview_panel.h"
#include "ui/market_panel.h"
#include "core/entity.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <algorithm>
#include <iostream>

namespace UI {

UIManager::UIManager()
    : context_(nullptr)
    , window_(nullptr)
    , show_ship_status_(true)
    , show_target_info_(true)
    , show_speed_panel_(true)
    , show_combat_log_(true)
    , show_target_list_(true)
{
    m_targetList = std::make_unique<EVETargetList>();
    m_inventoryPanel = std::make_unique<InventoryPanel>();
    m_fittingPanel = std::make_unique<FittingPanel>();
    m_missionPanel = std::make_unique<MissionPanel>();
    m_overviewPanel = std::make_unique<OverviewPanel>();
    m_marketPanel = std::make_unique<MarketPanel>();
}

UIManager::~UIManager() {
    Shutdown();
}

bool UIManager::Initialize(GLFWwindow* window) {
    if (!window) {
        std::cerr << "[UIManager] Error: Invalid GLFW window" << std::endl;
        return false;
    }
    
    window_ = window;
    
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    context_ = ImGui::CreateContext();
    if (!context_) {
        std::cerr << "[UIManager] Error: Failed to create ImGui context" << std::endl;
        return false;
    }
    
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    // Setup Platform/Renderer backends
    if (!ImGui_ImplGlfw_InitForOpenGL(window_, true)) {
        std::cerr << "[UIManager] Error: Failed to initialize ImGui GLFW backend" << std::endl;
        return false;
    }
    
    // OpenGL 3.3 GLSL version
    const char* glsl_version = "#version 330";
    if (!ImGui_ImplOpenGL3_Init(glsl_version)) {
        std::cerr << "[UIManager] Error: Failed to initialize ImGui OpenGL3 backend" << std::endl;
        return false;
    }
    
    // Setup EVE-style theme
    SetupEVEStyle();
    
    // Initialize default ship status
    ship_status_ = ShipStatus();
    target_info_ = TargetInfo();
    
    std::cout << "[UIManager] ImGui initialized successfully" << std::endl;
    return true;
}

void UIManager::Shutdown() {
    if (context_) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext(context_);
        context_ = nullptr;
    }
}

void UIManager::BeginFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void UIManager::EndFrame() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void UIManager::Render() {
    if (show_ship_status_) {
        RenderShipStatusPanel();
    }
    
    if (show_target_info_) {
        RenderTargetInfoPanel();
    }
    
    if (show_speed_panel_) {
        RenderSpeedPanel();
    }
    
    if (show_combat_log_) {
        RenderCombatLogPanel();
    }
    
    // Render EVE-style target list
    if (show_target_list_ && m_targetList) {
        m_targetList->render();
    }
    
    // Render Phase 4.5 panels
    if (m_inventoryPanel) {
        m_inventoryPanel->Render();
    }
    if (m_fittingPanel) {
        m_fittingPanel->Render();
    }
    if (m_missionPanel) {
        m_missionPanel->Render();
    }
    if (m_overviewPanel) {
        m_overviewPanel->Render();
    }
    if (m_marketPanel) {
        m_marketPanel->Render();
    }
}

void UIManager::SetupEVEStyle() {
    ImGuiStyle& style = ImGui::GetStyle();
    
    // Window styling
    style.WindowRounding = 4.0f;
    style.FrameRounding = 2.0f;
    style.ScrollbarRounding = 2.0f;
    style.GrabRounding = 2.0f;
    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;
    style.WindowPadding = ImVec2(12.0f, 12.0f);
    style.FramePadding = ImVec2(8.0f, 4.0f);
    style.ItemSpacing = ImVec2(8.0f, 6.0f);
    style.ItemInnerSpacing = ImVec2(6.0f, 4.0f);
    style.IndentSpacing = 20.0f;
    style.ScrollbarSize = 14.0f;
    style.GrabMinSize = 10.0f;
    
    // Color scheme - EVE Online Photon UI style
    ImVec4* colors = style.Colors;
    
    // Window background - dark blue-black
    colors[ImGuiCol_WindowBg] = ImVec4(
        EVEColors::BG_PRIMARY[0],
        EVEColors::BG_PRIMARY[1],
        EVEColors::BG_PRIMARY[2],
        EVEColors::BG_PRIMARY[3]
    );
    
    // Title bar
    colors[ImGuiCol_TitleBg] = ImVec4(
        EVEColors::BG_PANEL[0],
        EVEColors::BG_PANEL[1],
        EVEColors::BG_PANEL[2],
        1.0f
    );
    colors[ImGuiCol_TitleBgActive] = ImVec4(
        EVEColors::ACCENT_SECONDARY[0] * 0.3f,
        EVEColors::ACCENT_SECONDARY[1] * 0.3f,
        EVEColors::ACCENT_SECONDARY[2] * 0.3f,
        1.0f
    );
    
    // Borders
    colors[ImGuiCol_Border] = ImVec4(
        EVEColors::BORDER_NORMAL[0],
        EVEColors::BORDER_NORMAL[1],
        EVEColors::BORDER_NORMAL[2],
        EVEColors::BORDER_NORMAL[3]
    );
    
    // Frame/backgrounds
    colors[ImGuiCol_FrameBg] = ImVec4(
        EVEColors::BG_SECONDARY[0],
        EVEColors::BG_SECONDARY[1],
        EVEColors::BG_SECONDARY[2],
        0.8f
    );
    colors[ImGuiCol_FrameBgHovered] = ImVec4(
        EVEColors::ACCENT_SECONDARY[0] * 0.4f,
        EVEColors::ACCENT_SECONDARY[1] * 0.4f,
        EVEColors::ACCENT_SECONDARY[2] * 0.4f,
        0.8f
    );
    colors[ImGuiCol_FrameBgActive] = ImVec4(
        EVEColors::ACCENT_PRIMARY[0] * 0.5f,
        EVEColors::ACCENT_PRIMARY[1] * 0.5f,
        EVEColors::ACCENT_PRIMARY[2] * 0.5f,
        0.9f
    );
    
    // Text
    colors[ImGuiCol_Text] = ImVec4(
        EVEColors::TEXT_PRIMARY[0],
        EVEColors::TEXT_PRIMARY[1],
        EVEColors::TEXT_PRIMARY[2],
        EVEColors::TEXT_PRIMARY[3]
    );
    colors[ImGuiCol_TextDisabled] = ImVec4(
        EVEColors::TEXT_DISABLED[0],
        EVEColors::TEXT_DISABLED[1],
        EVEColors::TEXT_DISABLED[2],
        EVEColors::TEXT_DISABLED[3]
    );
    
    // Buttons
    colors[ImGuiCol_Button] = ImVec4(
        EVEColors::ACCENT_SECONDARY[0] * 0.4f,
        EVEColors::ACCENT_SECONDARY[1] * 0.4f,
        EVEColors::ACCENT_SECONDARY[2] * 0.4f,
        0.8f
    );
    colors[ImGuiCol_ButtonHovered] = ImVec4(
        EVEColors::ACCENT_SECONDARY[0] * 0.6f,
        EVEColors::ACCENT_SECONDARY[1] * 0.6f,
        EVEColors::ACCENT_SECONDARY[2] * 0.6f,
        0.9f
    );
    colors[ImGuiCol_ButtonActive] = ImVec4(
        EVEColors::ACCENT_PRIMARY[0],
        EVEColors::ACCENT_PRIMARY[1],
        EVEColors::ACCENT_PRIMARY[2],
        1.0f
    );
    
    // Headers
    colors[ImGuiCol_Header] = ImVec4(
        EVEColors::ACCENT_SECONDARY[0] * 0.3f,
        EVEColors::ACCENT_SECONDARY[1] * 0.3f,
        EVEColors::ACCENT_SECONDARY[2] * 0.3f,
        0.8f
    );
    colors[ImGuiCol_HeaderHovered] = ImVec4(
        EVEColors::ACCENT_SECONDARY[0] * 0.5f,
        EVEColors::ACCENT_SECONDARY[1] * 0.5f,
        EVEColors::ACCENT_SECONDARY[2] * 0.5f,
        0.9f
    );
    colors[ImGuiCol_HeaderActive] = ImVec4(
        EVEColors::ACCENT_PRIMARY[0] * 0.7f,
        EVEColors::ACCENT_PRIMARY[1] * 0.7f,
        EVEColors::ACCENT_PRIMARY[2] * 0.7f,
        1.0f
    );
    
    // Scrollbar
    colors[ImGuiCol_ScrollbarBg] = ImVec4(
        EVEColors::BG_PANEL[0],
        EVEColors::BG_PANEL[1],
        EVEColors::BG_PANEL[2],
        0.5f
    );
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(
        EVEColors::ACCENT_SECONDARY[0] * 0.5f,
        EVEColors::ACCENT_SECONDARY[1] * 0.5f,
        EVEColors::ACCENT_SECONDARY[2] * 0.5f,
        0.8f
    );
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(
        EVEColors::ACCENT_SECONDARY[0] * 0.7f,
        EVEColors::ACCENT_SECONDARY[1] * 0.7f,
        EVEColors::ACCENT_SECONDARY[2] * 0.7f,
        0.9f
    );
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(
        EVEColors::ACCENT_PRIMARY[0],
        EVEColors::ACCENT_PRIMARY[1],
        EVEColors::ACCENT_PRIMARY[2],
        1.0f
    );
    
    std::cout << "[UIManager] EVE-style theme applied" << std::endl;
}

void UIManager::SetShipStatus(const ShipStatus& status) {
    ship_status_ = status;
}

void UIManager::SetTargetInfo(const TargetInfo& target) {
    target_info_ = target;
}

void UIManager::AddCombatLogMessage(const std::string& message) {
    combat_log_.push_back(message);
    
    // Keep only the last N messages
    if (combat_log_.size() > MAX_COMBAT_LOG_MESSAGES) {
        combat_log_.erase(combat_log_.begin());
    }
}

void UIManager::SetPanelVisible(const std::string& panel_name, bool visible) {
    if (panel_name == "ship_status") {
        show_ship_status_ = visible;
    } else if (panel_name == "target_info") {
        show_target_info_ = visible;
    } else if (panel_name == "speed") {
        show_speed_panel_ = visible;
    } else if (panel_name == "combat_log") {
        show_combat_log_ = visible;
    }
}

bool UIManager::IsPanelVisible(const std::string& panel_name) const {
    if (panel_name == "ship_status") {
        return show_ship_status_;
    } else if (panel_name == "target_info") {
        return show_target_info_;
    } else if (panel_name == "speed") {
        return show_speed_panel_;
    } else if (panel_name == "combat_log") {
        return show_combat_log_;
    }
    return false;
}

void UIManager::RenderShipStatusPanel() {
    ImGui::SetNextWindowPos(ImVec2(10, 550), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(300, 200), ImGuiCond_FirstUseEver);
    
    ImGui::Begin("Ship Status", &show_ship_status_, ImGuiWindowFlags_NoCollapse);
    
    EVEPanels::RenderShipStatus(ship_status_);
    
    ImGui::End();
}

void UIManager::RenderTargetInfoPanel() {
    ImGui::SetNextWindowPos(ImVec2(890, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(300, 220), ImGuiCond_FirstUseEver);
    
    ImGui::Begin("Target Info", &show_target_info_, ImGuiWindowFlags_NoCollapse);
    
    EVEPanels::RenderTargetInfo(target_info_);
    
    ImGui::End();
}

void UIManager::RenderSpeedPanel() {
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(250, 120), ImGuiCond_FirstUseEver);
    
    ImGui::Begin("Speed", &show_speed_panel_, ImGuiWindowFlags_NoCollapse);
    
    EVEPanels::RenderSpeedDisplay(ship_status_.velocity, ship_status_.max_velocity);
    
    ImGui::End();
}

void UIManager::RenderCombatLogPanel() {
    ImGui::SetNextWindowPos(ImVec2(320, 550), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(450, 200), ImGuiCond_FirstUseEver);
    
    ImGui::Begin("Combat Log", &show_combat_log_, ImGuiWindowFlags_NoCollapse);
    
    EVEPanels::RenderCombatLog(combat_log_);
    
    ImGui::End();
}

void UIManager::RenderHealthBar(const char* label, float current, float max, const float color[4]) {
    EVEPanels::RenderHealthBar(label, current, max, color);
}

void UIManager::UpdateTargets(const std::unordered_map<std::string, std::shared_ptr<eve::Entity>>& entities) {
    if (m_targetList) {
        m_targetList->updateTargets(entities);
    }
    // Also update overview panel
    if (m_overviewPanel) {
        m_overviewPanel->UpdateEntities(entities);
    }
}

void UIManager::AddTarget(const std::string& entityId) {
    if (m_targetList) {
        m_targetList->addTarget(entityId);
    }
}

void UIManager::RemoveTarget(const std::string& entityId) {
    if (m_targetList) {
        m_targetList->removeTarget(entityId);
    }
}

// Phase 4.5 panel visibility toggles
void UIManager::ToggleInventory() {
    if (m_inventoryPanel) {
        m_inventoryPanel->SetVisible(!m_inventoryPanel->IsVisible());
    }
}

void UIManager::ToggleFitting() {
    if (m_fittingPanel) {
        m_fittingPanel->SetVisible(!m_fittingPanel->IsVisible());
    }
}

void UIManager::ToggleMission() {
    if (m_missionPanel) {
        m_missionPanel->SetVisible(!m_missionPanel->IsVisible());
    }
}

void UIManager::ToggleOverview() {
    if (m_overviewPanel) {
        m_overviewPanel->SetVisible(!m_overviewPanel->IsVisible());
    }
}

void UIManager::ToggleMarket() {
    if (m_marketPanel) {
        m_marketPanel->SetVisible(!m_marketPanel->IsVisible());
    }
}

} // namespace UI
