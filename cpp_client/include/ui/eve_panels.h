#ifndef EVE_PANELS_H
#define EVE_PANELS_H

#include "ui_manager.h"
#include <string>

namespace UI {

// Utility functions for rendering EVE-styled panels
namespace EVEPanels {

// Render a styled health bar with EVE colors
void RenderHealthBar(
    const char* label,
    float current,
    float max,
    const float color[4],
    float width = 200.0f
);

// Render a stylized panel header with EVE look
void RenderPanelHeader(const char* title, const float accent_color[4]);

// Render ship status panel with shield/armor/hull
// Original linear bar layout
void RenderShipStatus(const ShipStatus& status);

// Render ship status in EVE Online Photon UI style with circular gauges
// Includes nested shield/armor/hull arcs and central capacitor ring
void RenderShipStatusCircular(const ShipStatus& status);

// Render target information panel
void RenderTargetInfo(const TargetInfo& target);

// Render speed/velocity panel (original linear)
void RenderSpeedDisplay(float current_speed, float max_speed);

// Render speed in EVE Online style with radial gauge and approach/orbit/stop controls
void RenderSpeedGauge(float current_speed, float max_speed,
                      bool* approach_active = nullptr,
                      bool* orbit_active = nullptr,
                      bool* keep_range_active = nullptr);

// Render combat log with scrolling messages
void RenderCombatLog(const std::vector<std::string>& messages);

// Helper to get color based on health percentage
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
void RenderModuleRack(const ModuleSlotState slots[], int count);
} // namespace EVEPanels

} // namespace UI

#endif // EVE_PANELS_H
