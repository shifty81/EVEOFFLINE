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

} // namespace UI

#endif // EVE_PANELS_H
