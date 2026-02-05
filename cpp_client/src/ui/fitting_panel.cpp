#include "ui/fitting_panel.h"
#include <imgui.h>
#include <algorithm>

namespace UI {

FittingPanel::FittingPanel()
    : m_visible(false)
{
}

void FittingPanel::Render() {
    if (!m_visible) return;
    
    // Set window size and position
    ImGui::SetNextWindowSize(ImVec2(700, 600), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(150, 50), ImGuiCond_FirstUseEver);
    
    // EVE-style window flags
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse;
    
    if (!ImGui::Begin("Ship Fitting", &m_visible, flags)) {
        ImGui::End();
        return;
    }
    
    // Render UI elements
    RenderShipInfo();
    ImGui::Spacing();
    RenderResourceBars();
    ImGui::Separator();
    ImGui::Spacing();
    
    // Render slot sections
    ImGui::BeginChild("SlotSections", ImVec2(0, 0), false);
    
    RenderSlotSection("High Slots", m_data.high_slots, "high", 8);
    ImGui::Spacing();
    
    RenderSlotSection("Mid Slots", m_data.mid_slots, "mid", 8);
    ImGui::Spacing();
    
    RenderSlotSection("Low Slots", m_data.low_slots, "low", 8);
    ImGui::Spacing();
    
    RenderRigSection();
    
    ImGui::EndChild();
    
    ImGui::End();
}

void FittingPanel::SetFittingData(const FittingData& data) {
    m_data = data;
}

void FittingPanel::RenderShipInfo() {
    ImGui::TextColored(ImVec4(0.9f, 0.95f, 1.0f, 1.0f), "Ship: %s", m_data.ship_name.c_str());
    ImGui::TextColored(ImVec4(0.7f, 0.75f, 0.8f, 1.0f), "Type: %s", m_data.ship_type.c_str());
}

void FittingPanel::RenderResourceBars() {
    RenderResourceBar("CPU", m_data.cpu_used, m_data.cpu_max, 0.0f);
    RenderResourceBar("Powergrid", m_data.powergrid_used, m_data.powergrid_max, 50.0f);
}

void FittingPanel::RenderResourceBar(const char* label, float used, float max, float y_offset) {
    ImGui::Text("%s:", label);
    ImGui::SameLine(100);
    
    float percent = max > 0.0f ? used / max : 0.0f;
    
    // Color bar based on usage
    ImVec4 barColor;
    if (percent > 1.0f) {
        barColor = ImVec4(0.9f, 0.2f, 0.2f, 0.9f);  // Red if over capacity
    } else if (percent > 0.9f) {
        barColor = ImVec4(1.0f, 0.6f, 0.2f, 0.9f);  // Orange if near capacity
    } else {
        barColor = ImVec4(0.2f, 0.6f, 0.8f, 0.9f);  // Blue if OK
    }
    
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, barColor);
    ImGui::ProgressBar(percent, ImVec2(300, 20));
    ImGui::PopStyleColor();
    
    ImGui::SameLine();
    ImGui::Text("%.1f / %.1f", used, max);
}

void FittingPanel::RenderSlotSection(const char* title, const std::array<std::optional<ModuleInfo>, 8>& slots,
                                     const std::string& slot_type, int max_slots) {
    ImGui::TextColored(ImVec4(0.35f, 0.65f, 1.0f, 1.0f), "%s", title);
    ImGui::Spacing();
    
    // Display slots in a grid (2 columns)
    ImGui::Columns(2, nullptr, false);
    
    for (int i = 0; i < max_slots; ++i) {
        RenderModuleSlot(slots[i], slot_type, i, true);
        
        // Move to next column
        if (i % 2 == 0) {
            ImGui::NextColumn();
        }
    }
    
    ImGui::Columns(1);
}

void FittingPanel::RenderRigSection() {
    ImGui::TextColored(ImVec4(0.35f, 0.65f, 1.0f, 1.0f), "Rig Slots");
    ImGui::Spacing();
    
    // Rigs displayed in single column
    for (int i = 0; i < 3; ++i) {
        RenderModuleSlot(m_data.rig_slots[i], "rig", i, true);
    }
}

void FittingPanel::RenderModuleSlot(const std::optional<ModuleInfo>& module,
                                    const std::string& slot_type, int slot_index, bool enabled) {
    std::string slotId = slot_type + "_" + std::to_string(slot_index);
    
    if (!enabled) {
        ImGui::BeginDisabled();
    }
    
    // Create a frame for the slot
    ImGui::BeginGroup();
    
    if (module.has_value()) {
        // Render filled slot
        const auto& mod = module.value();
        
        // Module button (clickable)
        ImVec4 buttonColor = mod.is_online ? 
            ImVec4(0.2f, 0.6f, 0.3f, 0.8f) :   // Green if online
            ImVec4(0.3f, 0.3f, 0.3f, 0.8f);    // Gray if offline
        
        ImGui::PushStyleColor(ImGuiCol_Button, buttonColor);
        
        if (ImGui::Button(mod.name.c_str(), ImVec2(280, 40))) {
            // Right-click to unfit
            if (ImGui::IsItemClicked(1) && m_onUnfitModule) {
                m_onUnfitModule(slot_type, slot_index);
            }
            // Left-click to toggle online/offline (not for rigs)
            else if (slot_type != "rig" && m_onOnlineModule) {
                m_onOnlineModule(slot_type, slot_index, !mod.is_online);
            }
        }
        
        ImGui::PopStyleColor();
        
        // Show tooltip on hover
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::Text("%s", mod.name.c_str());
            ImGui::Separator();
            ImGui::Text("Type: %s", mod.type.c_str());
            ImGui::Text("CPU: %.1f", mod.cpu_cost);
            ImGui::Text("Powergrid: %.1f", mod.powergrid_cost);
            ImGui::Text("Status: %s", mod.is_online ? "Online" : "Offline");
            if (slot_type != "rig") {
                ImGui::Text("Right-click to unfit");
            }
            ImGui::EndTooltip();
        }
        
    } else {
        // Render empty slot
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.1f, 0.15f, 0.8f));
        
        if (ImGui::Button(("[Empty " + slot_type + " slot]").c_str(), ImVec2(280, 40))) {
            // Click to fit module (not implemented in UI, would show module browser)
        }
        
        ImGui::PopStyleColor();
        
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::Text("Empty %s slot", slot_type.c_str());
            ImGui::Text("Click to fit a module");
            ImGui::EndTooltip();
        }
    }
    
    ImGui::EndGroup();
    
    if (!enabled) {
        ImGui::EndDisabled();
    }
}

} // namespace UI
