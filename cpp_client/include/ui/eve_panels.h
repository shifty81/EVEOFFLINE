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
void RenderShipStatus(const ShipStatus& status);

// Render target information panel
void RenderTargetInfo(const TargetInfo& target);

// Render speed/velocity panel
void RenderSpeedDisplay(float current_speed, float max_speed);

// Render combat log with scrolling messages
void RenderCombatLog(const std::vector<std::string>& messages);

// Helper to get color based on health percentage
void GetHealthColorForPercent(float percent, float out_color[4], const float base_color[4]);

} // namespace EVEPanels

} // namespace UI

#endif // EVE_PANELS_H
