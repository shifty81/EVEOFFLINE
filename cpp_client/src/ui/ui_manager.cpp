#include "ui/ui_manager.h"
#include "ui/eve_panels.h"
#include "ui/eve_target_list.h"
#include "ui/inventory_panel.h"
#include "ui/fitting_panel.h"
#include "ui/mission_panel.h"
#include "ui/overview_panel.h"
#include "ui/market_panel.h"
#include "ui/docking_manager.h"
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
    m_dockingManager = std::make_unique<DockingManager>();
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
    
    // Setup dockable panels in the docking manager
    SetupDockablePanels();
    
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
    // === Ship HUD (bottom-center, like EVE Online) ===
    // The ship HUD is always rendered directly, not through the docking system
    if (show_ship_status_) {
        RenderShipStatusPanel();
    }
    
    if (show_speed_panel_) {
        RenderSpeedPanel();
    }
    
    if (show_target_info_) {
        RenderTargetInfoPanel();
    }
    
    if (show_combat_log_) {
        RenderCombatLogPanel();
    }
    
    // Render EVE-style target list
    if (show_target_list_ && m_targetList) {
        m_targetList->render();
    }
    
    // Render dockable panels (Inventory, Fitting, Mission, Overview, Market)
    // through the docking manager
    if (m_dockingManager) {
        m_dockingManager->RenderAll();
    }
}

void UIManager::SetupDockablePanels() {
    if (!m_dockingManager) return;
    
    // Register each panel with the docking manager
    // These panels can be docked together into tabbed containers
    
    if (m_inventoryPanel) {
        m_dockingManager->RegisterPanel("inventory", "Inventory",
            [this]() { m_inventoryPanel->RenderContents(); },
            ImVec2(50, 300), ImVec2(350, 400));
    }
    
    if (m_fittingPanel) {
        m_dockingManager->RegisterPanel("fitting", "Fitting",
            [this]() { m_fittingPanel->RenderContents(); },
            ImVec2(420, 300), ImVec2(400, 450));
    }
    
    if (m_missionPanel) {
        m_dockingManager->RegisterPanel("mission", "Missions",
            [this]() { m_missionPanel->RenderContents(); },
            ImVec2(50, 50), ImVec2(400, 350));
    }
    
    if (m_overviewPanel) {
        m_dockingManager->RegisterPanel("overview", "Overview",
            [this]() { m_overviewPanel->RenderContents(); },
            ImVec2(880, 50), ImVec2(380, 400));
    }
    
    if (m_marketPanel) {
        m_dockingManager->RegisterPanel("market", "Market",
            [this]() { m_marketPanel->RenderContents(); },
            ImVec2(420, 50), ImVec2(450, 500));
    }
    
    // Set default visibility — Overview visible, others hidden by default
    m_dockingManager->SetPanelVisible("overview", true);
    m_dockingManager->SetPanelVisible("inventory", false);
    m_dockingManager->SetPanelVisible("fitting", false);
    m_dockingManager->SetPanelVisible("mission", false);
    m_dockingManager->SetPanelVisible("market", false);
    
    std::cout << "[UIManager] Dockable panels registered" << std::endl;
}

void UIManager::SetupEVEStyle() {
    ImGuiStyle& style = ImGui::GetStyle();
    
    // Window styling — EVE Photon UI uses near-square corners, compact spacing
    style.WindowRounding = 2.0f;
    style.FrameRounding = 1.0f;
    style.ScrollbarRounding = 1.0f;
    style.GrabRounding = 1.0f;
    style.TabRounding = 1.0f;
    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;  // Frames rely on color, not borders
    style.WindowPadding = ImVec2(8.0f, 8.0f);
    style.FramePadding = ImVec2(6.0f, 3.0f);
    style.ItemSpacing = ImVec2(6.0f, 4.0f);
    style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);
    style.IndentSpacing = 16.0f;
    style.ScrollbarSize = 10.0f;
    style.GrabMinSize = 8.0f;
    
    // Color scheme — EVE Online Photon UI dark theme
    // Reference: docs/design/EVE_UI_STYLE_REFERENCE.md
    ImVec4* colors = style.Colors;
    
    // Window background — dark blue-black, semi-transparent
    colors[ImGuiCol_WindowBg] = ImVec4(
        EVEColors::BG_PRIMARY[0], EVEColors::BG_PRIMARY[1],
        EVEColors::BG_PRIMARY[2], EVEColors::BG_PRIMARY[3]);
    
    colors[ImGuiCol_ChildBg] = ImVec4(
        EVEColors::BG_PANEL[0], EVEColors::BG_PANEL[1],
        EVEColors::BG_PANEL[2], 0.5f);
    
    colors[ImGuiCol_PopupBg] = ImVec4(
        EVEColors::BG_TOOLTIP[0], EVEColors::BG_TOOLTIP[1],
        EVEColors::BG_TOOLTIP[2], EVEColors::BG_TOOLTIP[3]);
    
    // Title bar — darkest shade
    colors[ImGuiCol_TitleBg] = ImVec4(
        EVEColors::BG_HEADER[0], EVEColors::BG_HEADER[1],
        EVEColors::BG_HEADER[2], EVEColors::BG_HEADER[3]);
    colors[ImGuiCol_TitleBgActive] = ImVec4(
        EVEColors::ACCENT_DIM[0] * 0.5f, EVEColors::ACCENT_DIM[1] * 0.5f,
        EVEColors::ACCENT_DIM[2] * 0.5f, 1.0f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(
        EVEColors::BG_HEADER[0], EVEColors::BG_HEADER[1],
        EVEColors::BG_HEADER[2], 0.6f);
    
    // Borders — subtle dark border, teal on highlight
    colors[ImGuiCol_Border] = ImVec4(
        EVEColors::BORDER_NORMAL[0], EVEColors::BORDER_NORMAL[1],
        EVEColors::BORDER_NORMAL[2], EVEColors::BORDER_NORMAL[3]);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    
    // Frame backgrounds — slightly lighter than window for contrast
    colors[ImGuiCol_FrameBg] = ImVec4(
        EVEColors::BG_SECONDARY[0], EVEColors::BG_SECONDARY[1],
        EVEColors::BG_SECONDARY[2], 0.7f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(
        EVEColors::SELECTION[0], EVEColors::SELECTION[1],
        EVEColors::SELECTION[2], EVEColors::SELECTION[3]);
    colors[ImGuiCol_FrameBgActive] = ImVec4(
        EVEColors::ACCENT_DIM[0], EVEColors::ACCENT_DIM[1],
        EVEColors::ACCENT_DIM[2], 0.8f);
    
    // Text
    colors[ImGuiCol_Text] = ImVec4(
        EVEColors::TEXT_PRIMARY[0], EVEColors::TEXT_PRIMARY[1],
        EVEColors::TEXT_PRIMARY[2], EVEColors::TEXT_PRIMARY[3]);
    colors[ImGuiCol_TextDisabled] = ImVec4(
        EVEColors::TEXT_DISABLED[0], EVEColors::TEXT_DISABLED[1],
        EVEColors::TEXT_DISABLED[2], EVEColors::TEXT_DISABLED[3]);
    
    // Buttons — dim accent background, brighter on hover, full accent on press
    colors[ImGuiCol_Button] = ImVec4(
        EVEColors::ACCENT_DIM[0] * 0.6f, EVEColors::ACCENT_DIM[1] * 0.6f,
        EVEColors::ACCENT_DIM[2] * 0.6f, 0.7f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(
        EVEColors::ACCENT_DIM[0], EVEColors::ACCENT_DIM[1],
        EVEColors::ACCENT_DIM[2], 0.9f);
    colors[ImGuiCol_ButtonActive] = ImVec4(
        EVEColors::ACCENT_PRIMARY[0] * 0.8f, EVEColors::ACCENT_PRIMARY[1] * 0.8f,
        EVEColors::ACCENT_PRIMARY[2] * 0.8f, 1.0f);
    
    // Headers (collapsing headers, tree nodes)
    colors[ImGuiCol_Header] = ImVec4(
        EVEColors::SELECTION[0], EVEColors::SELECTION[1],
        EVEColors::SELECTION[2], 0.7f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(
        EVEColors::ACCENT_DIM[0] * 0.7f, EVEColors::ACCENT_DIM[1] * 0.7f,
        EVEColors::ACCENT_DIM[2] * 0.7f, 0.8f);
    colors[ImGuiCol_HeaderActive] = ImVec4(
        EVEColors::ACCENT_DIM[0], EVEColors::ACCENT_DIM[1],
        EVEColors::ACCENT_DIM[2], 1.0f);
    
    // Tabs — match EVE's filter tab style
    colors[ImGuiCol_Tab] = ImVec4(
        EVEColors::BG_HEADER[0], EVEColors::BG_HEADER[1],
        EVEColors::BG_HEADER[2], 1.0f);
    colors[ImGuiCol_TabHovered] = ImVec4(
        EVEColors::ACCENT_DIM[0] * 0.6f, EVEColors::ACCENT_DIM[1] * 0.6f,
        EVEColors::ACCENT_DIM[2] * 0.6f, 0.8f);
    
    // Separator
    colors[ImGuiCol_Separator] = ImVec4(
        EVEColors::BORDER_SUBTLE[0], EVEColors::BORDER_SUBTLE[1],
        EVEColors::BORDER_SUBTLE[2], EVEColors::BORDER_SUBTLE[3]);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(
        EVEColors::ACCENT_DIM[0], EVEColors::ACCENT_DIM[1],
        EVEColors::ACCENT_DIM[2], 0.8f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(
        EVEColors::ACCENT_PRIMARY[0], EVEColors::ACCENT_PRIMARY[1],
        EVEColors::ACCENT_PRIMARY[2], 1.0f);
    
    // Scrollbar — slim, dark
    colors[ImGuiCol_ScrollbarBg] = ImVec4(
        EVEColors::BG_PANEL[0], EVEColors::BG_PANEL[1],
        EVEColors::BG_PANEL[2], 0.3f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(
        EVEColors::ACCENT_DIM[0] * 0.6f, EVEColors::ACCENT_DIM[1] * 0.6f,
        EVEColors::ACCENT_DIM[2] * 0.6f, 0.6f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(
        EVEColors::ACCENT_DIM[0], EVEColors::ACCENT_DIM[1],
        EVEColors::ACCENT_DIM[2], 0.8f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(
        EVEColors::ACCENT_PRIMARY[0], EVEColors::ACCENT_PRIMARY[1],
        EVEColors::ACCENT_PRIMARY[2], 1.0f);
    
    // Resize grip
    colors[ImGuiCol_ResizeGrip] = ImVec4(
        EVEColors::ACCENT_DIM[0] * 0.4f, EVEColors::ACCENT_DIM[1] * 0.4f,
        EVEColors::ACCENT_DIM[2] * 0.4f, 0.4f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(
        EVEColors::ACCENT_PRIMARY[0], EVEColors::ACCENT_PRIMARY[1],
        EVEColors::ACCENT_PRIMARY[2], 0.6f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(
        EVEColors::ACCENT_PRIMARY[0], EVEColors::ACCENT_PRIMARY[1],
        EVEColors::ACCENT_PRIMARY[2], 0.9f);
    
    // Table colors
    colors[ImGuiCol_TableHeaderBg] = ImVec4(
        EVEColors::BG_HEADER[0], EVEColors::BG_HEADER[1],
        EVEColors::BG_HEADER[2], 1.0f);
    colors[ImGuiCol_TableBorderStrong] = ImVec4(
        EVEColors::BORDER_NORMAL[0], EVEColors::BORDER_NORMAL[1],
        EVEColors::BORDER_NORMAL[2], 0.8f);
    colors[ImGuiCol_TableBorderLight] = ImVec4(
        EVEColors::BORDER_SUBTLE[0], EVEColors::BORDER_SUBTLE[1],
        EVEColors::BORDER_SUBTLE[2], 0.4f);
    colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    colors[ImGuiCol_TableRowBgAlt] = ImVec4(
        EVEColors::BG_SECONDARY[0], EVEColors::BG_SECONDARY[1],
        EVEColors::BG_SECONDARY[2], 0.3f);
    
    // Check mark
    colors[ImGuiCol_CheckMark] = ImVec4(
        EVEColors::ACCENT_PRIMARY[0], EVEColors::ACCENT_PRIMARY[1],
        EVEColors::ACCENT_PRIMARY[2], 1.0f);
    
    // Slider
    colors[ImGuiCol_SliderGrab] = ImVec4(
        EVEColors::ACCENT_DIM[0], EVEColors::ACCENT_DIM[1],
        EVEColors::ACCENT_DIM[2], 0.9f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(
        EVEColors::ACCENT_PRIMARY[0], EVEColors::ACCENT_PRIMARY[1],
        EVEColors::ACCENT_PRIMARY[2], 1.0f);
    
    std::cout << "[UIManager] EVE Photon UI style applied" << std::endl;
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
    } else if (m_dockingManager) {
        // Try docking manager for dockable panels
        m_dockingManager->SetPanelVisible(panel_name, visible);
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
    } else if (m_dockingManager) {
        return m_dockingManager->IsPanelVisible(panel_name);
    }
    return false;
}

void UIManager::RenderShipStatusPanel() {
    // EVE Online positions the ship HUD at bottom-center of screen
    ImGuiIO& io = ImGui::GetIO();
    float hudWidth = 400.0f;
    float hudHeight = 310.0f;
    float posX = (io.DisplaySize.x - hudWidth) * 0.5f;
    float posY = io.DisplaySize.y - hudHeight - 10.0f;
    
    ImGui::SetNextWindowPos(ImVec2(posX, posY), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(hudWidth, hudHeight), ImGuiCond_FirstUseEver);
    
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
                             ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBackground;
    if (m_dockingManager && m_dockingManager->IsInterfaceLocked()) {
        flags |= ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
    }
    
    ImGui::Begin("##ShipHUD", &show_ship_status_, flags);
    
    // Use the EVE Online-style circular gauge display
    EVEPanels::RenderShipStatusCircular(ship_status_);
    
    ImGui::End();
}

void UIManager::RenderTargetInfoPanel() {
    ImGui::SetNextWindowPos(ImVec2(890, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(300, 220), ImGuiCond_FirstUseEver);
    
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse;
    if (m_dockingManager && m_dockingManager->IsInterfaceLocked()) {
        flags |= ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
    }
    
    ImGui::Begin("Target Info", &show_target_info_, flags);
    
    EVEPanels::RenderTargetInfo(target_info_);
    
    ImGui::End();
}

void UIManager::RenderSpeedPanel() {
    // EVE positions speed below the ship HUD, but we put it to the left for space
    ImGuiIO& io = ImGui::GetIO();
    float posX = (io.DisplaySize.x * 0.5f) - 350.0f;
    float posY = io.DisplaySize.y - 200.0f;
    
    ImGui::SetNextWindowPos(ImVec2(posX, posY), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(250, 200), ImGuiCond_FirstUseEver);
    
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
                             ImGuiWindowFlags_NoBackground;
    if (m_dockingManager && m_dockingManager->IsInterfaceLocked()) {
        flags |= ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
    }
    
    ImGui::Begin("##SpeedGauge", &show_speed_panel_, flags);
    
    // Use the EVE Online-style radial speed gauge
    EVEPanels::RenderSpeedGauge(ship_status_.velocity, ship_status_.max_velocity,
                                 &approach_active, &orbit_active, &keep_range_active);
    
    ImGui::End();
}

void UIManager::RenderCombatLogPanel() {
    ImGui::SetNextWindowPos(ImVec2(320, 550), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(450, 200), ImGuiCond_FirstUseEver);
    
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse;
    if (m_dockingManager && m_dockingManager->IsInterfaceLocked()) {
        flags |= ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
    }
    
    ImGui::Begin("Combat Log", &show_combat_log_, flags);
    
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

// Phase 4.5 panel visibility toggles — now go through docking manager
void UIManager::ToggleInventory() {
    if (m_dockingManager) {
        bool vis = m_dockingManager->IsPanelVisible("inventory");
        m_dockingManager->SetPanelVisible("inventory", !vis);
    }
}

void UIManager::ToggleFitting() {
    if (m_dockingManager) {
        bool vis = m_dockingManager->IsPanelVisible("fitting");
        m_dockingManager->SetPanelVisible("fitting", !vis);
    }
}

void UIManager::ToggleMission() {
    if (m_dockingManager) {
        bool vis = m_dockingManager->IsPanelVisible("mission");
        m_dockingManager->SetPanelVisible("mission", !vis);
    }
}

void UIManager::ToggleOverview() {
    if (m_dockingManager) {
        bool vis = m_dockingManager->IsPanelVisible("overview");
        m_dockingManager->SetPanelVisible("overview", !vis);
    }
}

void UIManager::ToggleMarket() {
    if (m_dockingManager) {
        bool vis = m_dockingManager->IsPanelVisible("market");
        m_dockingManager->SetPanelVisible("market", !vis);
    }
}

// Interface lock delegated to docking manager
void UIManager::SetInterfaceLocked(bool locked) {
    if (m_dockingManager) {
        m_dockingManager->SetInterfaceLocked(locked);
    }
}

bool UIManager::IsInterfaceLocked() const {
    if (m_dockingManager) {
        return m_dockingManager->IsInterfaceLocked();
    }
    return false;
}

void UIManager::ToggleInterfaceLock() {
    if (m_dockingManager) {
        m_dockingManager->ToggleInterfaceLock();
    }
}

} // namespace UI
