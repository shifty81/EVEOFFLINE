#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <glm/glm.hpp>

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
class DockingManager;
class DScanPanel;
class NeocomPanel;

namespace eve {
    class StarMap;
}

struct EVEColors {
    // Background colors — deep dark blue-black (Photon UI style)
    // See docs/design/EVE_UI_STYLE_REFERENCE.md for full palette
    static constexpr float BG_PRIMARY[4] = {0.051f, 0.067f, 0.090f, 0.92f};   // #0D1117
    static constexpr float BG_SECONDARY[4] = {0.086f, 0.106f, 0.133f, 0.90f}; // #161B22
    static constexpr float BG_PANEL[4] = {0.031f, 0.047f, 0.071f, 0.95f};     // #080C12
    static constexpr float BG_HEADER[4] = {0.039f, 0.055f, 0.078f, 1.0f};     // #0A0E14
    static constexpr float BG_TOOLTIP[4] = {0.110f, 0.129f, 0.157f, 0.95f};   // #1C2128
    
    // Accent colors — teal/cyan (EVE Photon default accent)
    static constexpr float ACCENT_PRIMARY[4] = {0.271f, 0.816f, 0.910f, 1.0f};  // #45D0E8
    static constexpr float ACCENT_SECONDARY[4] = {0.471f, 0.882f, 0.941f, 1.0f}; // #78E1F0
    static constexpr float ACCENT_DIM[4] = {0.165f, 0.353f, 0.416f, 1.0f};      // #2A5A6A
    
    // Selection / interaction
    static constexpr float SELECTION[4] = {0.102f, 0.227f, 0.290f, 0.80f};      // #1A3A4A
    
    // Border colors
    static constexpr float BORDER_NORMAL[4] = {0.157f, 0.220f, 0.282f, 0.6f};   // #283848
    static constexpr float BORDER_HIGHLIGHT[4] = {0.271f, 0.816f, 0.910f, 0.8f}; // #45D0E8
    static constexpr float BORDER_SUBTLE[4] = {0.118f, 0.165f, 0.212f, 0.5f};    // #1E2A36
    
    // Text colors
    static constexpr float TEXT_PRIMARY[4] = {0.902f, 0.929f, 0.953f, 1.0f};   // #E6EDF3
    static constexpr float TEXT_SECONDARY[4] = {0.545f, 0.580f, 0.620f, 1.0f}; // #8B949E
    static constexpr float TEXT_DISABLED[4] = {0.282f, 0.310f, 0.345f, 0.6f};  // #484F58
    
    // Health colors (matches EVE Online exactly)
    static constexpr float SHIELD_COLOR[4] = {0.2f, 0.6f, 1.0f, 1.0f};   // Blue
    static constexpr float ARMOR_COLOR[4] = {1.0f, 0.816f, 0.251f, 1.0f}; // Gold
    static constexpr float HULL_COLOR[4] = {0.902f, 0.271f, 0.271f, 1.0f}; // Red
    
    // Target / standing colors
    static constexpr float TARGET_HOSTILE[4] = {0.8f, 0.2f, 0.2f, 1.0f};   // Red
    static constexpr float TARGET_FRIENDLY[4] = {0.2f, 0.6f, 1.0f, 1.0f};  // Blue (EVE uses blue)
    static constexpr float TARGET_NEUTRAL[4] = {0.667f, 0.667f, 0.667f, 1.0f}; // Grey
    
    // Feedback colors
    static constexpr float SUCCESS[4] = {0.2f, 0.8f, 0.4f, 1.0f};  // Green
    static constexpr float WARNING[4] = {1.0f, 0.722f, 0.2f, 1.0f}; // Amber
    static constexpr float DANGER[4] = {1.0f, 0.2f, 0.2f, 1.0f};   // Red
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
    void SetPlayerPosition(const glm::vec3& position);
    
    // Target list management
    void UpdateTargets(const std::unordered_map<std::string, std::shared_ptr<::eve::Entity>>& entities);
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
    DScanPanel* GetDScanPanel() { return m_dscanPanel.get(); }
    NeocomPanel* GetNeocomPanel() { return m_neocomPanel.get(); }
    
    // Docking manager access
    DockingManager* GetDockingManager() { return m_dockingManager.get(); }
    
    // Panel visibility shortcuts (Phase 4.5)
    void ToggleInventory();
    void ToggleFitting();
    void ToggleMission();
    void ToggleOverview();
    void ToggleMarket();
    void ToggleDScan();
    void ToggleMap();
    
    // Interface lock
    void SetInterfaceLocked(bool locked);
    bool IsInterfaceLocked() const;
    void ToggleInterfaceLock();
    
    // Motion command states (for EVE-style movement)
    bool approach_active = false;
    bool orbit_active = false;
    bool keep_range_active = false;

private:
    ImGuiContext* context_;
    GLFWwindow* window_;
    
    // UI state
    ShipStatus ship_status_;
    TargetInfo target_info_;
    std::vector<std::string> combat_log_;
    static constexpr size_t MAX_COMBAT_LOG_MESSAGES = 10;
    glm::vec3 m_playerPosition{0.0f};  // Player position for distance calculations
    
    // EVE-style target list
    std::unique_ptr<EVETargetList> m_targetList;
    
    // Phase 4.5 panels
    std::unique_ptr<InventoryPanel> m_inventoryPanel;
    std::unique_ptr<FittingPanel> m_fittingPanel;
    std::unique_ptr<MissionPanel> m_missionPanel;
    std::unique_ptr<OverviewPanel> m_overviewPanel;
    std::unique_ptr<MarketPanel> m_marketPanel;
    
    // Docking manager for panel docking/undocking/locking
    std::unique_ptr<DockingManager> m_dockingManager;
    
    // Phase 4.8 panels
    std::unique_ptr<DScanPanel> m_dscanPanel;
    std::unique_ptr<NeocomPanel> m_neocomPanel;
    
    // Star map (toggled by Map button in Neocom)
    bool m_showStarMap = false;
    
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
    void RenderStarMapPanel();
    
    // Helper for health bars
    void RenderHealthBar(const char* label, float current, float max, const float color[4]);
    
    // Setup dockable panels in docking manager
    void SetupDockablePanels();
};

} // namespace UI

#endif // UI_MANAGER_H
