#ifndef HUD_PANELS_H
#define HUD_PANELS_H

#include "ui_manager.h"
#include "space_colors.h"
#include "ui/atlas/atlas_context.h"
#include <string>
#include <vector>

namespace UI {

/**
 * @file hud_panels.h
 */
// Utility functions for rendering HUD-styled panels via Atlas renderer
namespace HUDPanels {

// Render a styled health bar
void RenderHealthBar(
    aatlas::AtlasContext& ctx,
    const char* label,
    float current,
    float max,
    const float color[4],
    float width = 200.0f
);

// Render a stylized panel header
void RenderPanelHeader(aatlas::AtlasContext& ctx, const char* title, const float accent_color[4]);

// Render ship status panel with shield/armor/hull
// Original linear bar layout
void RenderShipStatus(aatlas::AtlasContext& ctx, const ShipStatus& status);

// Render ship status in circular gauge style
// Includes nested shield/armor/hull arcs and central capacitor ring
void RenderShipStatusCircular(aatlas::AtlasContext& ctx, const ShipStatus& status);

// Render target information panel
void RenderTargetInfo(aatlas::AtlasContext& ctx, const TargetInfo& target);

// Render speed/velocity panel (original linear)
void RenderSpeedDisplay(aatlas::AtlasContext& ctx, float current_speed, float max_speed);

// Render speed with radial gauge and approach/orbit/stop controls
void RenderSpeedGauge(aatlas::AtlasContext& ctx, float current_speed, float max_speed,
                      bool* approach_active = nullptr,
                      bool* orbit_active = nullptr,
                      bool* keep_range_active = nullptr);

// Render combat log with scrolling messages
void RenderCombatLog(aatlas::AtlasContext& ctx, const std::vector<std::string>& messages);

// Helper to get color based on health percentage (pure logic, no rendering)
void GetHealthColorForPercent(float percent, float out_color[4], const float base_color[4]);

} // namespace HUDPanels

// ModuleSlotState, HUDAlertPriority, HUDAlert, SelectedItemData, ProbeScanResult
// are now defined in ui_manager.h (included above) to break the circular dependency.

/** Render the module rack using an array of slot states (up to 8). */
namespace HUDPanels {
void RenderModuleRack(aatlas::AtlasContext& ctx, const ModuleSlotState slots[], int count);
} // namespace HUDPanels

/**
 * Render the HUD alert/warning stack above the ship status display.
 * Alerts are displayed in priority order (critical on top), fade out
 * as they expire, and pulse when critical.
 */
namespace HUDPanels {
void RenderAlertStack(aatlas::AtlasContext& ctx, const std::vector<HUDAlert>& alerts, float centerX, float baseY);
} // namespace HUDPanels

/**
 * Render the Selected Item panel contents.
 */
namespace HUDPanels {
void RenderSelectedItem(aatlas::AtlasContext& ctx, const SelectedItemData& item,
                        bool* approach_clicked = nullptr,
                        bool* orbit_clicked = nullptr,
                        bool* lock_clicked = nullptr,
                        bool* warp_clicked = nullptr);
} // namespace HUDPanels

} // namespace UI

#endif // HUD_PANELS_H
