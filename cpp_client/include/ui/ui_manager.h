#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <glm/glm.hpp>
#include "ui/photon/photon_context.h"
#include "ui/photon/photon_hud.h"
#include "ui/photon/photon_widgets.h"

namespace eve {
    class Entity;
}

namespace UI {

// Forward declarations — legacy panel objects kept for data storage / API compat
class EVETargetList;
class InventoryPanel;
class FittingPanel;
class MissionPanel;
class OverviewPanel;
class MarketPanel;
class DScanPanel;
class NeocomPanel;
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

    // Initialize Photon UI context and HUD layout
    bool Initialize(int windowW, int windowH);

    // Cleanup Photon GPU resources
    void Shutdown();

    // Frame management — caller must fill InputState from GLFW each frame
    void BeginFrame(const photon::InputState& input);
    void EndFrame();

    // Render all UI panels via Photon
    void Render();

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

    // Panel accessors (legacy panel objects kept for data storage)
    InventoryPanel* GetInventoryPanel() { return m_inventoryPanel.get(); }
    FittingPanel* GetFittingPanel() { return m_fittingPanel.get(); }
    MissionPanel* GetMissionPanel() { return m_missionPanel.get(); }
    OverviewPanel* GetOverviewPanel() { return m_overviewPanel.get(); }
    MarketPanel* GetMarketPanel() { return m_marketPanel.get(); }
    DScanPanel* GetDScanPanel() { return m_dscanPanel.get(); }
    NeocomPanel* GetNeocomPanel() { return m_neocomPanel.get(); }
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

    // Access Photon context for advanced / external usage
    photon::PhotonContext& GetPhotonContext() { return m_ctx; }

private:
    // Photon UI core
    photon::PhotonContext m_ctx;
    photon::PhotonHUD     m_hud;

    // UI state
    ShipStatus ship_status_;
    TargetInfo target_info_;
    std::vector<std::string> combat_log_;
    static constexpr size_t MAX_COMBAT_LOG_MESSAGES = 10;
    glm::vec3 m_playerPosition{0.0f};

    // Legacy panel objects (kept for data storage and API compatibility)
    std::unique_ptr<EVETargetList> m_targetList;
    std::unique_ptr<InventoryPanel> m_inventoryPanel;
    std::unique_ptr<FittingPanel> m_fittingPanel;
    std::unique_ptr<MissionPanel> m_missionPanel;
    std::unique_ptr<OverviewPanel> m_overviewPanel;
    std::unique_ptr<MarketPanel> m_marketPanel;
    std::unique_ptr<DScanPanel> m_dscanPanel;
    std::unique_ptr<NeocomPanel> m_neocomPanel;
    std::unique_ptr<ChatPanel> m_chatPanel;
    std::unique_ptr<DroneControlPanel> m_droneControlPanel;
    std::unique_ptr<NotificationManager> m_notificationManager;
    std::unique_ptr<ProbeScannerPanel> m_probeScannerPanel;

    // Photon panel states (replaces DockingManager)
    struct PanelConfig {
        photon::PanelState state;
        std::string title;
    };
    std::unordered_map<std::string, PanelConfig> m_panelConfigs;

    // Star map (toggled by Map button in Neocom)
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

    // Panel initialization helper
    void InitPanelConfigs(int windowW, int windowH);

    // Photon-based render helpers
    void RenderCombatLogPanel();
    void RenderStarMapPanel();
    void RenderAlertStack();
    void RenderDockablePanel(const std::string& id);
};

} // namespace UI

#endif // UI_MANAGER_H
