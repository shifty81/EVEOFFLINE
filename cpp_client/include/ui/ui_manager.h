#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

// Forward declarations
struct GLFWwindow;
struct ImGuiContext;

namespace eve {
    class Entity;
}

namespace UI {

class EVETargetList;
class InventoryPanel;
class FittingPanel;
class MissionPanel;
class OverviewPanel;
class MarketPanel;

struct EVEColors {
    // Background colors (RGB 0-1)
    static constexpr float BG_PRIMARY[4] = {0.05f, 0.07f, 0.09f, 0.95f};
    static constexpr float BG_SECONDARY[4] = {0.09f, 0.11f, 0.14f, 0.9f};
    static constexpr float BG_PANEL[4] = {0.02f, 0.03f, 0.05f, 0.95f};
    
    // Accent colors (teal/cyan)
    static constexpr float ACCENT_PRIMARY[4] = {0.35f, 0.65f, 1.0f, 1.0f};
    static constexpr float ACCENT_SECONDARY[4] = {0.47f, 0.76f, 1.0f, 1.0f};
    
    // Border colors
    static constexpr float BORDER_NORMAL[4] = {0.2f, 0.3f, 0.4f, 0.6f};
    static constexpr float BORDER_HIGHLIGHT[4] = {0.35f, 0.65f, 1.0f, 0.8f};
    
    // Text colors
    static constexpr float TEXT_PRIMARY[4] = {0.9f, 0.95f, 1.0f, 1.0f};
    static constexpr float TEXT_SECONDARY[4] = {0.7f, 0.75f, 0.8f, 1.0f};
    static constexpr float TEXT_DISABLED[4] = {0.4f, 0.45f, 0.5f, 0.6f};
    
    // Health colors
    static constexpr float SHIELD_COLOR[4] = {0.2f, 0.6f, 1.0f, 1.0f};
    static constexpr float ARMOR_COLOR[4] = {1.0f, 0.8f, 0.2f, 1.0f};
    static constexpr float HULL_COLOR[4] = {0.9f, 0.3f, 0.3f, 1.0f};
    
    // Target colors
    static constexpr float TARGET_HOSTILE[4] = {1.0f, 0.2f, 0.2f, 1.0f};
    static constexpr float TARGET_FRIENDLY[4] = {0.2f, 1.0f, 0.2f, 1.0f};
    static constexpr float TARGET_NEUTRAL[4] = {0.8f, 0.8f, 0.8f, 1.0f};
};

struct ShipStatus {
    float shields = 100.0f;
    float shields_max = 100.0f;
    float armor = 100.0f;
    float armor_max = 100.0f;
    float hull = 100.0f;
    float hull_max = 100.0f;
    float capacitor = 100.0f;
    float capacitor_max = 100.0f;
    float velocity = 0.0f;
    float max_velocity = 100.0f;
};

struct TargetInfo {
    std::string name = "No Target";
    float shields = 0.0f;
    float shields_max = 100.0f;
    float armor = 0.0f;
    float armor_max = 100.0f;
    float hull = 0.0f;
    float hull_max = 100.0f;
    float distance = 0.0f;
    bool is_hostile = false;
    bool is_locked = false;
};

class UIManager {
public:
    UIManager();
    ~UIManager();
    
    // Initialize ImGui with GLFW and OpenGL3
    bool Initialize(GLFWwindow* window);
    
    // Cleanup
    void Shutdown();
    
    // Frame management
    void BeginFrame();
    void EndFrame();
    
    // Render all UI panels
    void Render();
    
    // Set up EVE-style theme
    void SetupEVEStyle();
    
    // Data setters
    void SetShipStatus(const ShipStatus& status);
    void SetTargetInfo(const TargetInfo& target);
    void AddCombatLogMessage(const std::string& message);
    
    // Target list management
    void UpdateTargets(const std::unordered_map<std::string, std::shared_ptr<eve::Entity>>& entities);
    void AddTarget(const std::string& entityId);
    void RemoveTarget(const std::string& entityId);
    
    // Panel visibility toggles
    void SetPanelVisible(const std::string& panel_name, bool visible);
    bool IsPanelVisible(const std::string& panel_name) const;
    
    // Get target list
    EVETargetList* GetTargetList() { return m_targetList.get(); }
    
    // Get new panels (Phase 4.5)
    InventoryPanel* GetInventoryPanel() { return m_inventoryPanel.get(); }
    FittingPanel* GetFittingPanel() { return m_fittingPanel.get(); }
    MissionPanel* GetMissionPanel() { return m_missionPanel.get(); }
    OverviewPanel* GetOverviewPanel() { return m_overviewPanel.get(); }
    MarketPanel* GetMarketPanel() { return m_marketPanel.get(); }
    
    // Panel visibility shortcuts (Phase 4.5)
    void ToggleInventory();
    void ToggleFitting();
    void ToggleMission();
    void ToggleOverview();
    void ToggleMarket();

private:
    ImGuiContext* context_;
    GLFWwindow* window_;
    
    // UI state
    ShipStatus ship_status_;
    TargetInfo target_info_;
    std::vector<std::string> combat_log_;
    static constexpr size_t MAX_COMBAT_LOG_MESSAGES = 10;
    
    // EVE-style target list
    std::unique_ptr<EVETargetList> m_targetList;
    
    // Phase 4.5 panels
    std::unique_ptr<InventoryPanel> m_inventoryPanel;
    std::unique_ptr<FittingPanel> m_fittingPanel;
    std::unique_ptr<MissionPanel> m_missionPanel;
    std::unique_ptr<OverviewPanel> m_overviewPanel;
    std::unique_ptr<MarketPanel> m_marketPanel;
    
    // Panel visibility
    bool show_ship_status_;
    bool show_target_info_;
    bool show_speed_panel_;
    bool show_combat_log_;
    bool show_target_list_;
    
    // Helper functions for rendering panels
    void RenderShipStatusPanel();
    void RenderTargetInfoPanel();
    void RenderSpeedPanel();
    void RenderCombatLogPanel();
    
    // Helper for health bars
    void RenderHealthBar(const char* label, float current, float max, const float color[4]);
};

} // namespace UI

#endif // UI_MANAGER_H
