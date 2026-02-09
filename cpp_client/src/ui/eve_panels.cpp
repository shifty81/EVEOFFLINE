#include "ui/eve_panels.h"
#include <imgui.h>
#include <algorithm>
#include <cmath>

// ImGui PI constant (not defined in all ImGui versions)
#ifndef IM_PI
#define IM_PI 3.14159265358979323846f
#endif

namespace UI {
namespace EVEPanels {

void GetHealthColorForPercent(float percent, float out_color[4], const float base_color[4]) {
    // Copy base color
    out_color[0] = base_color[0];
    out_color[1] = base_color[1];
    out_color[2] = base_color[2];
    out_color[3] = base_color[3];
    
    // Modify intensity based on health percentage
    if (percent < 0.3f) {
        // Brighten and add red tint when low
        out_color[0] = std::min(1.0f, base_color[0] * 1.3f);
        out_color[1] = base_color[1] * 0.8f;
        out_color[2] = base_color[2] * 0.8f;
    } else if (percent < 0.5f) {
        // Slightly dim when moderate
        out_color[0] = base_color[0] * 1.1f;
        out_color[1] = base_color[1] * 0.9f;
        out_color[2] = base_color[2] * 0.9f;
    }
}

void RenderHealthBar(const char* label, float current, float max, const float color[4], float width) {
    if (max <= 0.0f) max = 1.0f;
    
    float percent = std::max(0.0f, std::min(1.0f, current / max));
    
    // Get adjusted color based on percentage
    float adjusted_color[4];
    GetHealthColorForPercent(percent, adjusted_color, color);
    
    // Label
    ImGui::Text("%s", label);
    ImGui::SameLine();
    
    // Show numeric values
    char value_text[64];
    snprintf(value_text, sizeof(value_text), "%.0f / %.0f (%.0f%%)", current, max, percent * 100.0f);
    ImGui::Text("%s", value_text);
    
    // Progress bar
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(adjusted_color[0], adjusted_color[1], adjusted_color[2], adjusted_color[3]));
    ImGui::ProgressBar(percent, ImVec2(width, 0.0f), "");
    ImGui::PopStyleColor();
}

void RenderPanelHeader(const char* title, const float accent_color[4]) {
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(accent_color[0], accent_color[1], accent_color[2], accent_color[3]));
    ImGui::Text("%s", title);
    ImGui::PopStyleColor();
    ImGui::Separator();
}

void RenderShipStatus(const ShipStatus& status) {
    RenderPanelHeader("SHIP STATUS", EVEColors::ACCENT_PRIMARY);
    
    ImGui::Spacing();
    
    // Shields (blue)
    RenderHealthBar("Shields", status.shields, status.shields_max, EVEColors::SHIELD_COLOR);
    
    ImGui::Spacing();
    
    // Armor (yellow/gold)
    RenderHealthBar("Armor", status.armor, status.armor_max, EVEColors::ARMOR_COLOR);
    
    ImGui::Spacing();
    
    // Hull (red)
    RenderHealthBar("Hull", status.hull, status.hull_max, EVEColors::HULL_COLOR);
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    // Capacitor
    float cap_percent = (status.capacitor_max > 0.0f) ? (status.capacitor / status.capacitor_max) : 0.0f;
    
    // Choose capacitor color based on percentage
    float cap_color[4] = {1.0f, 0.9f, 0.3f, 1.0f}; // Default yellow
    if (cap_percent < 0.25f) {
        cap_color[0] = 1.0f; cap_color[1] = 0.3f; cap_color[2] = 0.0f; // Red
    } else if (cap_percent < 0.5f) {
        cap_color[0] = 1.0f; cap_color[1] = 0.6f; cap_color[2] = 0.0f; // Orange
    }
    
    ImGui::Text("Capacitor");
    ImGui::SameLine();
    
    char cap_text[64];
    snprintf(cap_text, sizeof(cap_text), "%.0f / %.0f (%.0f%%)", 
             status.capacitor, status.capacitor_max, cap_percent * 100.0f);
    ImGui::Text("%s", cap_text);
    
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(cap_color[0], cap_color[1], cap_color[2], cap_color[3]));
    ImGui::ProgressBar(cap_percent, ImVec2(-1, 0.0f), "");
    ImGui::PopStyleColor();
}

void RenderTargetInfo(const TargetInfo& target) {
    if (!target.is_locked) {
        RenderPanelHeader("TARGET INFO", EVEColors::ACCENT_PRIMARY);
        ImGui::Spacing();
        ImGui::TextColored(
            ImVec4(EVEColors::TEXT_SECONDARY[0], EVEColors::TEXT_SECONDARY[1], 
                   EVEColors::TEXT_SECONDARY[2], EVEColors::TEXT_SECONDARY[3]),
            "No target locked"
        );
        return;
    }
    
    // Choose header color based on target type
    const float* header_color = target.is_hostile ? EVEColors::TARGET_HOSTILE : EVEColors::TARGET_FRIENDLY;
    
    RenderPanelHeader("TARGET INFO", header_color);
    
    ImGui::Spacing();
    
    // Target name
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(header_color[0], header_color[1], header_color[2], header_color[3]));
    ImGui::Text("%s", target.name.c_str());
    ImGui::PopStyleColor();
    
    ImGui::Spacing();
    
    // Distance
    ImGui::Text("Distance: %.0f m", target.distance);
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    // Target health bars
    if (target.shields_max > 0.0f) {
        RenderHealthBar("Shields", target.shields, target.shields_max, EVEColors::SHIELD_COLOR, 250.0f);
        ImGui::Spacing();
    }
    
    if (target.armor_max > 0.0f) {
        RenderHealthBar("Armor", target.armor, target.armor_max, EVEColors::ARMOR_COLOR, 250.0f);
        ImGui::Spacing();
    }
    
    if (target.hull_max > 0.0f) {
        RenderHealthBar("Hull", target.hull, target.hull_max, EVEColors::HULL_COLOR, 250.0f);
    }
}

void RenderSpeedDisplay(float current_speed, float max_speed) {
    RenderPanelHeader("SPEED", EVEColors::ACCENT_PRIMARY);
    
    ImGui::Spacing();
    
    // Current speed (large text)
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(EVEColors::ACCENT_SECONDARY[0], 
                                                  EVEColors::ACCENT_SECONDARY[1], 
                                                  EVEColors::ACCENT_SECONDARY[2], 1.0f));
    
    char speed_text[64];
    snprintf(speed_text, sizeof(speed_text), "%.1f m/s", current_speed);
    ImGui::SetWindowFontScale(1.5f);
    ImGui::Text("%s", speed_text);
    ImGui::SetWindowFontScale(1.0f);
    
    ImGui::PopStyleColor();
    
    ImGui::Spacing();
    
    // Max speed indicator
    ImGui::Text("Max: %.1f m/s", max_speed);
    
    // Speed bar
    float speed_percent = (max_speed > 0.0f) ? std::min(1.0f, current_speed / max_speed) : 0.0f;
    
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, 
                          ImVec4(EVEColors::ACCENT_PRIMARY[0], 
                                 EVEColors::ACCENT_PRIMARY[1], 
                                 EVEColors::ACCENT_PRIMARY[2], 1.0f));
    ImGui::ProgressBar(speed_percent, ImVec2(-1, 0.0f), "");
    ImGui::PopStyleColor();
}

// ============================================================================
// EVE Online-style HUD using PathArcTo block-based gauges
// Reference: https://wiki.eveuniversity.org/Capacitor
// Reference: https://www.eveonline.com/eve-academy/ships/flying-your-ship
//
// EXACT EVE LAYOUT (center-bottom of screen):
//   - CAPACITOR = innermost, full circle, 32 brick segments, golden yellow
//   - HULL      = next ring out, red
//   - ARMOR     = middle ring, yellow/gold
//   - SHIELD    = outermost ring, blue
//   - MODULE RACK = horizontal bar BELOW the circular display
//   - Speed readout = center of the circle
//
// Segments deplete counter-clockwise from right side.
// Each ring uses PathArcTo + PathStroke with gaps between blocks.
// ============================================================================

// Draw a segmented block-arc gauge using PathArcTo + PathStroke.
// Full circle version with 'segments' blocks and gaps between them.
// Segments deplete counter-clockwise: filled segments count down from
// the right side (angle 0) going counter-clockwise.
static void DrawBlockRingGauge(ImDrawList* drawList, ImVec2 center, float radius,
                                float pct, ImU32 filledColor, ImU32 emptyColor,
                                float thickness, int segments = 32,
                                float gapFraction = 0.08f) {
    float fullCircle = 2.0f * IM_PI;
    float angle_step = fullCircle / static_cast<float>(segments);
    float gap = angle_step * gapFraction;

    // Start from top (-PI/2) and go clockwise
    // Filled segments are drawn from the start; as pct decreases,
    // fewer segments are filled.
    int filledCount = static_cast<int>(pct * segments + 0.5f);

    for (int i = 0; i < segments; i++) {
        float a1 = -IM_PI / 2.0f + angle_step * i + gap;
        float a2 = -IM_PI / 2.0f + angle_step * (i + 1) - gap;

        // Segments fill clockwise from top; deplete counter-clockwise
        ImU32 color = (i < filledCount) ? filledColor : emptyColor;

        drawList->PathArcTo(center, radius, a1, a2, 10);
        drawList->PathStroke(color, 0, thickness);
    }
}

// Draw a half-circle (bottom half) block gauge — used for shield/armor/hull
// in EVE style. Arcs from left (PI) to right (2*PI), so the bottom half.
static void DrawHalfCircleBlockGauge(ImDrawList* drawList, ImVec2 center, float radius,
                                      float pct, ImU32 filledColor, ImU32 emptyColor,
                                      float thickness, int segments = 20,
                                      float gapFraction = 0.06f) {
    // Bottom half-circle: PI (left) to 2*PI (right)
    float angle_min = IM_PI;
    float angle_max = 2.0f * IM_PI;
    float angle_step = (angle_max - angle_min) / static_cast<float>(segments);
    float gap = angle_step * gapFraction;

    float fillCount = pct * segments;

    for (int i = 0; i < segments; i++) {
        float a1 = angle_min + angle_step * i + gap;
        float a2 = angle_min + angle_step * (i + 1) - gap;

        ImU32 color = (static_cast<float>(i) < fillCount) ? filledColor : emptyColor;

        drawList->PathArcTo(center, radius, a1, a2, 10);
        drawList->PathStroke(color, 0, thickness);
    }
}

// Draw a single module slot (circular button) — for the horizontal rack
static bool DrawModuleSlot(ImDrawList* drawList, ImVec2 pos, float radius,
                            int slotNum, bool isActive, ImU32 activeColor,
                            ImU32 inactiveColor, ImU32 borderColor) {
    ImU32 bgColor = isActive ? activeColor : inactiveColor;
    drawList->AddCircleFilled(pos, radius, bgColor, 32);
    drawList->AddCircle(pos, radius, borderColor, 32, 1.0f);

    // F-key label
    char label[4];
    snprintf(label, sizeof(label), "F%d", slotNum);
    ImVec2 textSize = ImGui::CalcTextSize(label);
    drawList->AddText(ImVec2(pos.x - textSize.x / 2, pos.y - textSize.y / 2),
                      IM_COL32(200, 220, 240, 220), label);

    // Invisible button for click detection
    ImVec2 topLeft(pos.x - radius, pos.y - radius);
    ImGui::SetCursorScreenPos(topLeft);
    char btnId[32];
    snprintf(btnId, sizeof(btnId), "##mod_%d", slotNum);
    return ImGui::InvisibleButton(btnId, ImVec2(radius * 2, radius * 2));
}

void RenderShipStatusCircular(const ShipStatus& status) {
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 windowPos = ImGui::GetCursorScreenPos();

    // === Gauge sizing ===
    float outerRadius = 100.0f;      // Shield (outermost)
    float barThickness = 10.0f;      // Thickness of each health ring stroke
    float ringGap = 4.0f;            // Gap between concentric rings
    float capThickness = 7.0f;       // Capacitor brick thickness

    ImVec2 center(windowPos.x + outerRadius + 15, windowPos.y + outerRadius + 10);

    // === Dark background circle ===
    drawList->AddCircleFilled(center, outerRadius + 8, IM_COL32(8, 12, 18, 230), 64);
    drawList->AddCircle(center, outerRadius + 8, IM_COL32(35, 50, 65, 100), 64, 1.5f);

    // === SHIELD — outermost ring (blue) ===
    float shieldPct = (status.shields_max > 0) ?
        std::clamp(status.shields / status.shields_max, 0.0f, 1.0f) : 0.0f;
    DrawHalfCircleBlockGauge(drawList, center, outerRadius, shieldPct,
                              IM_COL32(0, 180, 255, 240),     // Filled: EVE shield blue
                              IM_COL32(20, 45, 80, 70),       // Empty: dark blue
                              barThickness, 20, 0.06f);

    // === ARMOR — middle ring (yellow/gold) ===
    float armorPct = (status.armor_max > 0) ?
        std::clamp(status.armor / status.armor_max, 0.0f, 1.0f) : 0.0f;
    float armorRadius = outerRadius - barThickness - ringGap;
    DrawHalfCircleBlockGauge(drawList, center, armorRadius, armorPct,
                              IM_COL32(255, 210, 60, 240),    // Filled: EVE armor gold
                              IM_COL32(80, 65, 15, 70),       // Empty: dark gold
                              barThickness, 20, 0.06f);

    // === HULL — innermost health ring (red) ===
    float hullPct = (status.hull_max > 0) ?
        std::clamp(status.hull / status.hull_max, 0.0f, 1.0f) : 0.0f;
    float hullRadius = armorRadius - barThickness - ringGap;
    DrawHalfCircleBlockGauge(drawList, center, hullRadius, hullPct,
                              IM_COL32(230, 55, 55, 240),     // Filled: EVE hull red
                              IM_COL32(80, 20, 20, 70),       // Empty: dark red
                              barThickness, 20, 0.06f);

    // === CAPACITOR — center, full circle, 32 brick segments (EVE standard) ===
    float capPct = (status.capacitor_max > 0) ?
        std::clamp(status.capacitor / status.capacitor_max, 0.0f, 1.0f) : 0.0f;
    float capRadius = hullRadius - barThickness - 8.0f;
    DrawBlockRingGauge(drawList, center, capRadius, capPct,
                       IM_COL32(255, 225, 70, 220),   // Filled: banana yellow
                       IM_COL32(45, 40, 15, 90),       // Empty: very dark gold
                       capThickness, 32, 0.10f);

    // === Center text: speed readout ===
    // In EVE, the center shows the ship model; we show speed instead
    char speedText[32];
    snprintf(speedText, sizeof(speedText), "%.0f", status.velocity);
    ImVec2 speedSize = ImGui::CalcTextSize(speedText);

    // Larger speed text
    ImGui::SetWindowFontScale(1.4f);
    ImVec2 bigSpeedSize = ImGui::CalcTextSize(speedText);
    drawList->AddText(ImVec2(center.x - bigSpeedSize.x / 2, center.y - bigSpeedSize.y / 2 - 6),
                      IM_COL32(210, 235, 250, 245), speedText);
    ImGui::SetWindowFontScale(1.0f);

    // "m/s" unit below
    const char* unitLabel = "m/s";
    ImVec2 unitSize = ImGui::CalcTextSize(unitLabel);
    drawList->AddText(ImVec2(center.x - unitSize.x / 2, center.y + 6),
                      IM_COL32(130, 150, 170, 170), unitLabel);

    // === HP/CAP readout text above the gauge (small) ===
    float infoY = center.y - outerRadius - 22;
    float infoX = center.x - 60;

    char hpText[96];
    snprintf(hpText, sizeof(hpText), "S:%.0f  A:%.0f  H:%.0f  C:%.0f%%",
             status.shields, status.armor, status.hull, capPct * 100.0f);
    drawList->AddText(ImVec2(infoX, infoY), IM_COL32(160, 180, 200, 180), hpText);

    // === MODULE RACK — horizontal bar BELOW the gauge (EVE layout) ===
    float rackY = center.y + outerRadius + 18;
    float slotRadius = 13.0f;
    float slotSpacing = 30.0f;
    int totalSlots = 8;  // F1-F8
    float rackWidth = totalSlots * slotSpacing;
    float rackStartX = center.x - rackWidth / 2.0f + slotSpacing / 2.0f;

    // Rack background bar
    drawList->AddRectFilled(
        ImVec2(rackStartX - slotRadius - 4, rackY - slotRadius - 6),
        ImVec2(rackStartX + (totalSlots - 1) * slotSpacing + slotRadius + 4, rackY + slotRadius + 6),
        IM_COL32(12, 16, 22, 200), 3.0f);
    drawList->AddRect(
        ImVec2(rackStartX - slotRadius - 4, rackY - slotRadius - 6),
        ImVec2(rackStartX + (totalSlots - 1) * slotSpacing + slotRadius + 4, rackY + slotRadius + 6),
        IM_COL32(40, 55, 72, 120), 3.0f, 0, 1.0f);

    // Rack section labels
    drawList->AddText(ImVec2(rackStartX - 4, rackY - slotRadius - 20),
                      IM_COL32(120, 150, 180, 140), "HIGH         MID          LOW");

    // Draw module slots F1-F8
    for (int i = 0; i < totalSlots; i++) {
        ImVec2 slotPos(rackStartX + i * slotSpacing, rackY);

        // Color-code by slot type: High=green, Mid=blue, Low=orange
        ImU32 activeCol, inactiveCol;
        if (i < 3) {
            // High slots (F1-F3)
            activeCol = IM_COL32(50, 140, 50, 210);
            inactiveCol = IM_COL32(22, 38, 28, 190);
        } else if (i < 6) {
            // Mid slots (F4-F6)
            activeCol = IM_COL32(50, 100, 160, 210);
            inactiveCol = IM_COL32(22, 30, 45, 190);
        } else {
            // Low slots (F7-F8)
            activeCol = IM_COL32(160, 110, 40, 210);
            inactiveCol = IM_COL32(45, 32, 18, 190);
        }

        DrawModuleSlot(drawList, slotPos, slotRadius, i + 1, false,
                       activeCol, inactiveCol, IM_COL32(55, 75, 95, 140));
    }

    // === Reserve ImGui layout space ===
    float totalWidth = (outerRadius + 15) * 2 + 10;
    float totalHeight = outerRadius + 10 + outerRadius + 18 + slotRadius + 30;
    ImGui::Dummy(ImVec2(totalWidth, totalHeight));
}

void RenderSpeedGauge(float current_speed, float max_speed,
                      bool* approach_active, bool* orbit_active, bool* keep_range_active) {
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 windowPos = ImGui::GetCursorScreenPos();

    float gaugeRadius = 50.0f;
    ImVec2 center(windowPos.x + gaugeRadius + 10, windowPos.y + gaugeRadius + 10);

    // Background circle
    drawList->AddCircleFilled(center, gaugeRadius, IM_COL32(13, 17, 23, 200), 48);
    drawList->AddCircle(center, gaugeRadius, IM_COL32(40, 56, 72, 150), 48, 1.0f);

    // Speed gauge - segmented arc (270 degrees)
    float speedPct = (max_speed > 0) ? std::clamp(current_speed / max_speed, 0.0f, 1.0f) : 0.0f;
    int segments = 15;
    float arcStart = 0.75f * IM_PI;    // 135 degrees
    float arcRange = 1.5f * IM_PI;     // 270 degrees
    float stepAngle = arcRange / static_cast<float>(segments);
    float pctFill = speedPct * segments;
    float gap = stepAngle * 0.08f;

    for (int i = 0; i < segments; i++) {
        float a1 = arcStart + stepAngle * i + gap;
        float a2 = arcStart + stepAngle * (i + 1) - gap;

        ImU32 color;
        if (static_cast<float>(i) < pctFill) {
            color = IM_COL32(69, 208, 232, 220);   // Teal (EVE accent)
        } else {
            color = IM_COL32(30, 50, 70, 100);      // Empty
        }

        drawList->PathArcTo(center, gaugeRadius - 3.0f, a1, a2, 8);
        drawList->PathStroke(color, 0, 6.0f);
    }

    // Center speed text
    char speedText[32];
    snprintf(speedText, sizeof(speedText), "%.0f", current_speed);
    ImVec2 textSize = ImGui::CalcTextSize(speedText);
    drawList->AddText(ImVec2(center.x - textSize.x / 2, center.y - textSize.y / 2 - 5),
                      IM_COL32(200, 230, 245, 240), speedText);

    // "m/s" label
    const char* unitLabel = "m/s";
    ImVec2 unitSize = ImGui::CalcTextSize(unitLabel);
    drawList->AddText(ImVec2(center.x - unitSize.x / 2, center.y - unitSize.y / 2 + 10),
                      IM_COL32(140, 160, 180, 180), unitLabel);

    // Reserve space
    ImGui::Dummy(ImVec2(gaugeRadius * 2 + 20, gaugeRadius * 2 + 20));

    // Motion command buttons below the gauge
    ImGui::Spacing();

    float buttonWidth = 70.0f;
    float buttonHeight = 22.0f;

    // Approach button
    bool approachState = approach_active ? *approach_active : false;
    if (approachState) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.4f, 0.5f, 0.9f));
    }
    if (ImGui::Button("Approach", ImVec2(buttonWidth, buttonHeight))) {
        if (approach_active) *approach_active = !*approach_active;
    }
    if (approachState) {
        ImGui::PopStyleColor();
    }

    ImGui::SameLine();

    // Orbit button
    bool orbitState = orbit_active ? *orbit_active : false;
    if (orbitState) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.4f, 0.5f, 0.9f));
    }
    if (ImGui::Button("Orbit", ImVec2(buttonWidth, buttonHeight))) {
        if (orbit_active) *orbit_active = !*orbit_active;
    }
    if (orbitState) {
        ImGui::PopStyleColor();
    }

    ImGui::SameLine();

    // Keep At Range button
    bool keepState = keep_range_active ? *keep_range_active : false;
    if (keepState) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.4f, 0.5f, 0.9f));
    }
    if (ImGui::Button("Keep", ImVec2(buttonWidth, buttonHeight))) {
        if (keep_range_active) *keep_range_active = !*keep_range_active;
    }
    if (keepState) {
        ImGui::PopStyleColor();
    }

    // Stop button (CTRL+SPACE in EVE)
    if (ImGui::Button("STOP", ImVec2(buttonWidth * 3 + ImGui::GetStyle().ItemSpacing.x * 2, buttonHeight))) {
        if (approach_active) *approach_active = false;
        if (orbit_active) *orbit_active = false;
        if (keep_range_active) *keep_range_active = false;
    }

    // Max speed label
    ImGui::TextColored(ImVec4(0.55f, 0.58f, 0.62f, 1.0f), "Max: %.0f m/s", max_speed);
}

void RenderCombatLog(const std::vector<std::string>& messages) {
    RenderPanelHeader("COMBAT LOG", EVEColors::ACCENT_PRIMARY);
    
    ImGui::Spacing();
    
    // Create scrolling region
    ImGui::BeginChild("CombatLogScroll", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
    
    if (messages.empty()) {
        ImGui::TextColored(
            ImVec4(EVEColors::TEXT_SECONDARY[0], EVEColors::TEXT_SECONDARY[1], 
                   EVEColors::TEXT_SECONDARY[2], EVEColors::TEXT_SECONDARY[3]),
            "No combat activity"
        );
    } else {
        // Display messages (most recent at bottom)
        for (const auto& message : messages) {
            ImGui::TextWrapped("%s", message.c_str());
        }
        
        // Auto-scroll to bottom for new messages
        ImGui::SetScrollHereY(1.0f);
    }
    
    ImGui::EndChild();
}

// ============================================================================
// RenderModuleRack — data-bound module rack with active/cooldown visuals
// ============================================================================
void RenderModuleRack(const ModuleSlotState slots[], int count) {
    if (count <= 0) return;

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 windowPos = ImGui::GetCursorScreenPos();

    float slotRadius = 16.0f;
    float slotSpacing = 36.0f;
    float rackWidth = count * slotSpacing;
    float startX = windowPos.x + 10.0f;
    float centerY = windowPos.y + slotRadius + 8.0f;

    // Rack background
    drawList->AddRectFilled(
        ImVec2(startX - slotRadius - 4, centerY - slotRadius - 8),
        ImVec2(startX + (count - 1) * slotSpacing + slotRadius + 4, centerY + slotRadius + 8),
        IM_COL32(12, 16, 22, 210), 3.0f);
    drawList->AddRect(
        ImVec2(startX - slotRadius - 4, centerY - slotRadius - 8),
        ImVec2(startX + (count - 1) * slotSpacing + slotRadius + 4, centerY + slotRadius + 8),
        IM_COL32(40, 55, 72, 120), 3.0f, 0, 1.0f);

    for (int i = 0; i < count; i++) {
        const auto& s = slots[i];
        ImVec2 slotPos(startX + i * slotSpacing, centerY);

        // Slot type color
        ImU32 activeCol, inactiveCol, emptyCol;
        switch (s.slotType) {
            case ModuleSlotState::HIGH:
                activeCol   = IM_COL32(50, 180, 50, 230);
                inactiveCol = IM_COL32(30, 65, 35, 200);
                emptyCol    = IM_COL32(22, 35, 25, 140);
                break;
            case ModuleSlotState::MID:
                activeCol   = IM_COL32(50, 120, 200, 230);
                inactiveCol = IM_COL32(25, 40, 75, 200);
                emptyCol    = IM_COL32(22, 30, 45, 140);
                break;
            case ModuleSlotState::LOW:
                activeCol   = IM_COL32(200, 140, 40, 230);
                inactiveCol = IM_COL32(70, 50, 20, 200);
                emptyCol    = IM_COL32(45, 32, 18, 140);
                break;
        }

        ImU32 bgColor;
        if (!s.fitted) {
            bgColor = emptyCol;
        } else if (s.active) {
            bgColor = activeCol;
        } else {
            bgColor = inactiveCol;
        }

        // Draw the slot circle
        drawList->AddCircleFilled(slotPos, slotRadius, bgColor, 32);

        ImU32 borderColor = IM_COL32(55, 75, 95, 160);
        if (s.overheated) {
            borderColor = IM_COL32(255, 100, 30, 220);  // Orange glow for overheat
        } else if (s.active) {
            borderColor = IM_COL32(0, 200, 230, 200);   // Teal glow for active
        }
        drawList->AddCircle(slotPos, slotRadius, borderColor, 32, s.active ? 2.0f : 1.0f);

        // Cooldown overlay — clockwise arc from top
        if (s.fitted && s.cooldown_pct > 0.0f) {
            float angle = s.cooldown_pct * 2.0f * IM_PI;
            drawList->PathArcTo(slotPos, slotRadius - 1.0f, -IM_PI / 2.0f, -IM_PI / 2.0f + angle, 24);
            drawList->PathStroke(IM_COL32(0, 200, 230, 130), 0, 4.0f);
        }

        // F-key label
        char fLabel[4];
        snprintf(fLabel, sizeof(fLabel), "F%d", i + 1);
        ImVec2 textSize = ImGui::CalcTextSize(fLabel);
        drawList->AddText(ImVec2(slotPos.x - textSize.x / 2, slotPos.y - textSize.y / 2),
                          IM_COL32(200, 220, 240, s.fitted ? 240 : 120), fLabel);

        // Invisible button for interaction
        ImVec2 topLeft(slotPos.x - slotRadius, slotPos.y - slotRadius);
        ImGui::SetCursorScreenPos(topLeft);
        char btnId[32];
        snprintf(btnId, sizeof(btnId), "##modrack_%d", i);
        ImGui::InvisibleButton(btnId, ImVec2(slotRadius * 2, slotRadius * 2));

        // Tooltip on hover
        if (s.fitted && ImGui::IsItemHovered()) {
            ImGui::SetTooltip("%s%s", s.name.c_str(), s.active ? " [ACTIVE]" : "");
        }
    }

    // Reserve layout space
    ImGui::SetCursorScreenPos(ImVec2(windowPos.x, centerY + slotRadius + 12));
    ImGui::Dummy(ImVec2(rackWidth + 20, 0));
}

} // namespace EVEPanels
} // namespace UI
