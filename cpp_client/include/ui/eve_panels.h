#ifndef EVE_PANELS_H
#define EVE_PANELS_H

#include "ui_manager.h"
#include "eve_colors.h"
#include "ui/photon/photon_context.h"
#include <string>
#include <vector>

namespace UI {

// Utility functions for rendering EVE-styled panels via Photon renderer
namespace EVEPanels {

// Render a styled health bar with EVE colors
void RenderHealthBar(
    photon::PhotonContext& ctx,
    const char* label,
    float current,
    float max,
    const float color[4],
    float width = 200.0f
);

// Render a stylized panel header with EVE look
void RenderPanelHeader(photon::PhotonContext& ctx, const char* title, const float accent_color[4]);

// Render ship status panel with shield/armor/hull
// Original linear bar layout
void RenderShipStatus(photon::PhotonContext& ctx, const ShipStatus& status);

// Render ship status in EVE Online Photon UI style with circular gauges
// Includes nested shield/armor/hull arcs and central capacitor ring
void RenderShipStatusCircular(photon::PhotonContext& ctx, const ShipStatus& status);

// Render target information panel
void RenderTargetInfo(photon::PhotonContext& ctx, const TargetInfo& target);

// Render speed/velocity panel (original linear)
void RenderSpeedDisplay(photon::PhotonContext& ctx, float current_speed, float max_speed);

// Render speed in EVE Online style with radial gauge and approach/orbit/stop controls
void RenderSpeedGauge(photon::PhotonContext& ctx, float current_speed, float max_speed,
                      bool* approach_active = nullptr,
                      bool* orbit_active = nullptr,
                      bool* keep_range_active = nullptr);

// Render combat log with scrolling messages
void RenderCombatLog(photon::PhotonContext& ctx, const std::vector<std::string>& messages);

// Helper to get color based on health percentage (pure logic, no rendering)
void GetHealthColorForPercent(float percent, float out_color[4], const float base_color[4]);

} // namespace EVEPanels

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

/** Render the module rack using an array of slot states (up to 8). */
namespace EVEPanels {
void RenderModuleRack(photon::PhotonContext& ctx, const ModuleSlotState slots[], int count);
} // namespace EVEPanels

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
 * Render the HUD alert/warning stack above the ship status display.
 * Alerts are displayed in priority order (critical on top), fade out
 * as they expire, and pulse when critical.
 */
namespace EVEPanels {
void RenderAlertStack(photon::PhotonContext& ctx, const std::vector<HUDAlert>& alerts, float centerX, float baseY);
} // namespace EVEPanels

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
 * Render the Selected Item panel contents (EVE-style).
 */
namespace EVEPanels {
void RenderSelectedItem(photon::PhotonContext& ctx, const SelectedItemData& item,
                        bool* approach_clicked = nullptr,
                        bool* orbit_clicked = nullptr,
                        bool* lock_clicked = nullptr,
                        bool* warp_clicked = nullptr);
} // namespace EVEPanels

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

} // namespace UI

#endif // EVE_PANELS_H
