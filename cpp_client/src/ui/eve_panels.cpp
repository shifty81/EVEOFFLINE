#include "ui/eve_panels.h"
#include <imgui/imgui.h>
#include <algorithm>
#include <cmath>

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
        
        // Auto-scroll to bottom
        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
            ImGui::SetScrollHereY(1.0f);
        }
    }
    
    ImGui::EndChild();
}

} // namespace EVEPanels
} // namespace UI
