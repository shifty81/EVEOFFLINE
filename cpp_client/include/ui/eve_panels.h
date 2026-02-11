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

// ModuleSlotState, HUDAlertPriority, HUDAlert, SelectedItemData, ProbeScanResult
// are now defined in ui_manager.h (included above) to break the circular dependency.

/** Render the module rack using an array of slot states (up to 8). */
namespace EVEPanels {
void RenderModuleRack(photon::PhotonContext& ctx, const ModuleSlotState slots[], int count);
} // namespace EVEPanels

/**
 * Render the HUD alert/warning stack above the ship status display.
 * Alerts are displayed in priority order (critical on top), fade out
 * as they expire, and pulse when critical.
 */
namespace EVEPanels {
void RenderAlertStack(photon::PhotonContext& ctx, const std::vector<HUDAlert>& alerts, float centerX, float baseY);
} // namespace EVEPanels

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

} // namespace UI

#endif // EVE_PANELS_H
