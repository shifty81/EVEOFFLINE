#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <glm/glm.hpp>
#include "ui/atlas/atlas_context.h"
#include "ui/atlas/atlas_hud.h"
#include "ui/atlas/atlas_widgets.h"
#include "ui/layout_manager.h"

namespace atlas {
    class Entity;
}

namespace UI {

// Forward declarations — legacy panel objects kept for data storage / API compat
class TargetList;
class InventoryPanel;
class FittingPanel;
class MissionPanel;
class OverviewPanel;
class MarketPanel;
class DScanPanel;
class SidebarPanel;
class ChatPanel;
class DroneControlPanel;
class NotificationManager;
class ProbeScannerPanel;

/**
 * Module slot state for the HUD module rack.
 * Holds the data needed to render a single module slot with
 * proper active/inactive/cooldown visuals.
 */
struct ModuleSlotState {
    bool fitted = false;          // true if a module is in this slot
    bool active = false;          // true if the module is currently activated
    bool overheated = false;      // true if overheating
    float cooldown_pct = 0.0f;    // 0.0 = ready, 1.0 = full cooldown
    std::string name;             // Module short name (e.g. "AC II")
    enum SlotType { HIGH, MID, LOW } slotType = HIGH;
};

/**
 * HUD alert priority levels (higher = more urgent).
 * Alerts are displayed above the ship HUD in a stack.
 */
enum class HUDAlertPriority {
    INFO = 0,      // General info (e.g. "Warp Drive Active")
    WARNING = 1,   // Warning (e.g. "CAP LOW", "SHIELD LOW")
    CRITICAL = 2   // Critical (e.g. "STRUCTURE CRITICAL", "SCRAMBLED")
};

/**
 * A single HUD alert entry shown above the ship status display.
 * Modelled after EVE Online's alert stack (CAP LOW, SCRAMBLED, etc.).
 */
struct HUDAlert {
    std::string message;
    HUDAlertPriority priority = HUDAlertPriority::INFO;
    float duration = 5.0f;        // Total display time (seconds)
    float elapsed = 0.0f;         // Time since alert was created

    HUDAlert() = default;
    HUDAlert(const std::string& msg, HUDAlertPriority prio, float dur = 5.0f)
        : message(msg), priority(prio), duration(dur), elapsed(0.0f) {}
};

/**
 * Selected item info for the "Selected Item" panel (top-right).
 * Shows name, type, distance, and quick-action buttons for the
 * currently selected entity in space.
 */
struct SelectedItemData {
    std::string name;
    std::string type;             // e.g. "Frigate", "Asteroid Belt", "Station"
    float distance = 0.0f;        // Distance in meters
    float shields_pct = 0.0f;     // 0-1 shield remaining
    float armor_pct = 0.0f;       // 0-1 armor remaining
    float hull_pct = 0.0f;        // 0-1 hull remaining
    float velocity = 0.0f;        // Target velocity m/s
    float angular_velocity = 0.0f;// Angular velocity rad/s
    bool is_hostile = false;
    bool is_locked = false;
    bool has_health = false;       // true if health bars should be shown

    bool isEmpty() const { return name.empty(); }
};

/**
 * Probe scanner result for the probe scanner panel.
 */
struct ProbeScanResult {
    std::string id;
    std::string name;
    std::string group;        // e.g. "Cosmic Signature", "Cosmic Anomaly", "Ship"
    std::string type;         // e.g. "Combat Site", "Relic Site", "Data Site", "Gas Site"
    float signal_strength = 0.0f;  // 0-100% scan completion
    float distance = 0.0f;         // Distance in AU

    ProbeScanResult() = default;
    ProbeScanResult(const std::string& id_, const std::string& name_,
                    const std::string& group_, const std::string& type_,
                    float signal, float dist)
        : id(id_), name(name_), group(group_), type(type_),
          signal_strength(signal), distance(dist) {}
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

    // Initialize Atlas UI context and HUD layout
    bool Initialize(int windowW, int windowH);

    // Cleanup Atlas GPU resources
    void Shutdown();

    // Frame management — caller must fill InputState from GLFW each frame
    void BeginFrame(const atlas::InputState& input);
    void EndFrame();

    // Render all UI panels via Atlas
    void Render();

    // Data setters
    void SetShipStatus(const ShipStatus& status);
    void SetTargetInfo(const TargetInfo& target);
    void AddCombatLogMessage(const std::string& message);
    void SetPlayerPosition(const glm::vec3& position);

    // Target list management
    void UpdateTargets(const std::unordered_map<std::string, std::shared_ptr<::atlas::Entity>>& entities);
    void AddTarget(const std::string& entityId);
    void RemoveTarget(const std::string& entityId);

    // Panel visibility toggles
    void SetPanelVisible(const std::string& panel_name, bool visible);
    bool IsPanelVisible(const std::string& panel_name) const;

    // Get target list
    TargetList* GetTargetList() { return m_targetList.get(); }

    // Panel accessors (legacy panel objects kept for data storage)
    InventoryPanel* GetInventoryPanel() { return m_inventoryPanel.get(); }
    FittingPanel* GetFittingPanel() { return m_fittingPanel.get(); }
    MissionPanel* GetMissionPanel() { return m_missionPanel.get(); }
    OverviewPanel* GetOverviewPanel() { return m_overviewPanel.get(); }
    MarketPanel* GetMarketPanel() { return m_marketPanel.get(); }
    DScanPanel* GetDScanPanel() { return m_dscanPanel.get(); }
    SidebarPanel* GetSidebarPanel() { return m_sidebarPanel.get(); }
    ChatPanel* GetChatPanel() { return m_chatPanel.get(); }
    DroneControlPanel* GetDroneControlPanel() { return m_droneControlPanel.get(); }
    NotificationManager* GetNotificationManager() { return m_notificationManager.get(); }
    ProbeScannerPanel* GetProbeScannerPanel() { return m_probeScannerPanel.get(); }

    // Panel visibility shortcuts
    void ToggleInventory();
    void ToggleFitting();
    void ToggleMission();
    void ToggleOverview();
    void ToggleMarket();
    void ToggleDScan();
    void ToggleMap();
    void ToggleChat();
    void ToggleDrones();
    void ToggleProbeScanner();

    // Interface lock
    void SetInterfaceLocked(bool locked);
    bool IsInterfaceLocked() const;
    void ToggleInterfaceLock();

    // Motion command states (for EVE-style movement)
    bool approach_active = false;
    bool orbit_active = false;
    bool keep_range_active = false;

    // Selected item management
    void SetSelectedItem(const SelectedItemData& item);
    const SelectedItemData& GetSelectedItem() const { return m_selectedItem; }
    void ClearSelectedItem();

    // HUD alert management
    void AddAlert(const std::string& message, HUDAlertPriority priority, float duration = 5.0f);
    void ClearAlerts();
    void UpdateAlerts(float deltaTime);

    // Module rack data binding
    void SetModuleSlots(const ModuleSlotState slots[], int count);

    // Compact mode
    void SetCompactMode(bool enabled);
    bool IsCompactMode() const { return m_compactMode; }
    void ToggleCompactMode();

    // Access Atlas context for advanced / external usage
    atlas::AtlasContext& GetAtlasContext() { return m_ctx; }

    // ── Layout management (Phase 4.10) ──────────────────────────────

    /** Save current panel layout to a named preset. */
    bool SaveLayout(const std::string& presetName);

    /** Load a named preset and apply it to all panels. */
    bool LoadLayout(const std::string& presetName);

    /** Get list of available layout presets. */
    std::vector<std::string> GetAvailableLayouts() const;

    /** Reset all panels to the default layout. */
    void ResetToDefaultLayout();

    /** Get the active layout preset name. */
    const std::string& GetActiveLayoutName() const { return m_activeLayoutName; }

    // Per-panel opacity
    void SetPanelOpacity(const std::string& panel_name, float opacity);
    float GetPanelOpacity(const std::string& panel_name) const;

    // Access layout manager
    LayoutManager& GetLayoutManager() { return m_layoutManager; }

    // ── UI Scale (Phase 4.10) ───────────────────────────────────────

    /** Set the global UI scale factor (0.5 – 2.0). */
    void SetUIScale(float scale);

    /** Get the current UI scale factor. */
    float GetUIScale() const { return m_uiScale; }

    // ── Color Scheme (Phase 4.10) ───────────────────────────────────

    /** Available color scheme names. */
    enum class ColorScheme { DEFAULT, CLASSIC, COLORBLIND };

    /** Set the active color scheme. */
    void SetColorScheme(ColorScheme scheme);

    /** Get the active color scheme. */
    ColorScheme GetColorScheme() const { return m_colorScheme; }

private:
    // Atlas UI core
    atlas::AtlasContext m_ctx;
    atlas::AtlasHUD     m_hud;

    // UI state
    ShipStatus ship_status_;
    TargetInfo target_info_;
    std::vector<std::string> combat_log_;
    static constexpr size_t MAX_COMBAT_LOG_MESSAGES = 10;
    glm::vec3 m_playerPosition{0.0f};

    // Legacy panel objects (kept for data storage and API compatibility)
    std::unique_ptr<TargetList> m_targetList;
    std::unique_ptr<InventoryPanel> m_inventoryPanel;
    std::unique_ptr<FittingPanel> m_fittingPanel;
    std::unique_ptr<MissionPanel> m_missionPanel;
    std::unique_ptr<OverviewPanel> m_overviewPanel;
    std::unique_ptr<MarketPanel> m_marketPanel;
    std::unique_ptr<DScanPanel> m_dscanPanel;
    std::unique_ptr<SidebarPanel> m_sidebarPanel;
    std::unique_ptr<ChatPanel> m_chatPanel;
    std::unique_ptr<DroneControlPanel> m_droneControlPanel;
    std::unique_ptr<NotificationManager> m_notificationManager;
    std::unique_ptr<ProbeScannerPanel> m_probeScannerPanel;

    // Atlas panel states (replaces DockingManager)
    struct PanelConfig {
        atlas::PanelState state;
        std::string title;
        float opacity = 0.92f;  // Per-panel opacity (0.0–1.0)
    };
    std::unordered_map<std::string, PanelConfig> m_panelConfigs;

    // Star map (toggled by Map button in Sidebar)
    bool m_showStarMap = false;

    // Panel visibility flags
    bool show_ship_status_;
    bool show_target_info_;
    bool show_speed_panel_;
    bool show_combat_log_;
    bool show_target_list_;

    // Interface lock
    bool m_interfaceLocked = false;

    // Selected item state
    SelectedItemData m_selectedItem;
    bool m_showSelectedItem = true;

    // HUD alert stack
    std::vector<HUDAlert> m_alerts;
    static constexpr size_t MAX_ALERTS = 5;

    // Module rack state (data-bound)
    static constexpr int MAX_MODULE_SLOTS = 8;
    ModuleSlotState m_moduleSlots[8];
    int m_moduleSlotCount = 8;
    bool m_showModuleRack = true;

    // Compact mode
    bool m_compactMode = false;

    // Layout management (Phase 4.10)
    LayoutManager m_layoutManager;
    std::string m_activeLayoutName = "default";
    int m_windowW = 1280;
    int m_windowH = 720;

    // UI Scale (Phase 4.10)
    float m_uiScale = 1.0f;

    // Color scheme (Phase 4.10)
    ColorScheme m_colorScheme = ColorScheme::DEFAULT;

    // Panel initialization helper
    void InitPanelConfigs(int windowW, int windowH);

    // Atlas-based render helpers
    void RenderCombatLogPanel();
    void RenderStarMapPanel();
    void RenderAlertStack();
    void RenderDockablePanel(const std::string& id);

    // Layout helper: convert between PanelConfig map and PanelLayout map
    std::unordered_map<std::string, PanelLayout> ExportPanelLayouts() const;
    void ImportPanelLayouts(const std::unordered_map<std::string, PanelLayout>& layouts);
};

} // namespace UI

#endif // UI_MANAGER_H
