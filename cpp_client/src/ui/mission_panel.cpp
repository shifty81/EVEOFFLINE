#include "ui/mission_panel.h"
#include "ui/ui_manager.h"
#include <imgui.h>

namespace UI {

MissionPanel::MissionPanel()
    : m_visible(false)
{
}

void MissionPanel::Render() {
    if (!m_visible) return;
    
    // Set window size and position
    ImGui::SetNextWindowSize(ImVec2(450, 500), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(200, 150), ImGuiCond_FirstUseEver);
    
    // EVE-style window flags
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse;
    
    if (!ImGui::Begin("Mission Tracker", &m_visible, flags)) {
        ImGui::End();
        return;
    }
    
    if (!m_data.is_active) {
        // No active mission
        ImGui::TextColored(ImVec4(
            EVEColors::TEXT_SECONDARY[0], EVEColors::TEXT_SECONDARY[1],
            EVEColors::TEXT_SECONDARY[2], EVEColors::TEXT_SECONDARY[3]),
            "No active mission");
        ImGui::Text("Accept a mission from an agent to get started.");
    } else {
        // Render mission details
        RenderMissionInfo();
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        RenderObjectivesList();
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        RenderRewards();
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        RenderProgressBar();
        ImGui::Spacing();
        RenderActionButtons();
    }
    
    ImGui::End();
}

void MissionPanel::SetMissionData(const MissionData& data) {
    m_data = data;
}

void MissionPanel::RenderMissionInfo() {
    // Mission name
    float color[4];
    GetMissionTypeColor(color);
    ImGui::TextColored(ImVec4(color[0], color[1], color[2], color[3]), 
                       "%s", m_data.mission_name.c_str());
    
    // Mission details
    ImGui::Text("Agent: %s", m_data.agent_name.c_str());
    ImGui::Text("Location: %s", m_data.location.c_str());
    ImGui::Text("Type: %s", m_data.mission_type.c_str());
    ImGui::Text("Level: %d", m_data.level);
    
    // Time limit (if any)
    if (m_data.time_limit > 0.0f) {
        float remaining = m_data.time_limit - m_data.time_elapsed;
        ImGui::TextColored(remaining < 1.0f ? 
            ImVec4(EVEColors::DANGER[0], EVEColors::DANGER[1],
                   EVEColors::DANGER[2], EVEColors::DANGER[3]) : 
            ImVec4(EVEColors::TEXT_SECONDARY[0], EVEColors::TEXT_SECONDARY[1],
                   EVEColors::TEXT_SECONDARY[2], EVEColors::TEXT_SECONDARY[3]),
                           "Time Remaining: %.1f hours", remaining);
    }
}

void MissionPanel::RenderObjectivesList() {
    ImGui::TextColored(ImVec4(
        EVEColors::ACCENT_PRIMARY[0], EVEColors::ACCENT_PRIMARY[1],
        EVEColors::ACCENT_PRIMARY[2], EVEColors::ACCENT_PRIMARY[3]), "OBJECTIVES");
    ImGui::Spacing();
    
    // Display objectives in a scrollable list
    ImGui::BeginChild("ObjectivesList", ImVec2(0, 150), true);
    
    for (size_t i = 0; i < m_data.objectives.size(); ++i) {
        const auto& objective = m_data.objectives[i];
        
        // Checkbox for completed objectives
        if (objective.completed) {
            ImGui::TextColored(ImVec4(
                EVEColors::SUCCESS[0], EVEColors::SUCCESS[1],
                EVEColors::SUCCESS[2], EVEColors::SUCCESS[3]), "[✓]");
        } else {
            ImGui::TextColored(ImVec4(
                EVEColors::TEXT_SECONDARY[0], EVEColors::TEXT_SECONDARY[1],
                EVEColors::TEXT_SECONDARY[2], EVEColors::TEXT_SECONDARY[3]), "[ ]");
        }
        
        ImGui::SameLine();
        ImGui::Text("%s", objective.description.c_str());
    }
    
    if (m_data.objectives.empty()) {
        ImGui::TextColored(ImVec4(
            EVEColors::TEXT_SECONDARY[0], EVEColors::TEXT_SECONDARY[1],
            EVEColors::TEXT_SECONDARY[2], EVEColors::TEXT_SECONDARY[3]),
            "No objectives");
    }
    
    ImGui::EndChild();
}

void MissionPanel::RenderRewards() {
    ImGui::TextColored(ImVec4(
        EVEColors::ACCENT_PRIMARY[0], EVEColors::ACCENT_PRIMARY[1],
        EVEColors::ACCENT_PRIMARY[2], EVEColors::ACCENT_PRIMARY[3]), "REWARDS");
    ImGui::Spacing();
    
    // ISK reward
    if (m_data.isk_reward > 0.0f) {
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "ISK:");
        ImGui::SameLine();
        ImGui::Text("%.0f", m_data.isk_reward);
    }
    
    // LP reward
    if (m_data.lp_reward > 0.0f) {
        ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "LP:");
        ImGui::SameLine();
        ImGui::Text("%.0f", m_data.lp_reward);
    }
    
    // Item rewards
    if (!m_data.item_rewards.empty()) {
        ImGui::Text("Items:");
        for (const auto& item : m_data.item_rewards) {
            ImGui::BulletText("%s", item.c_str());
        }
    }
}

void MissionPanel::RenderActionButtons() {
    // Complete button (only if all objectives done)
    bool allComplete = true;
    for (const auto& obj : m_data.objectives) {
        if (!obj.completed) {
            allComplete = false;
            break;
        }
    }
    
    if (!allComplete) {
        ImGui::BeginDisabled();
    }
    
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(
        EVEColors::SUCCESS[0] * 0.5f, EVEColors::SUCCESS[1] * 0.5f,
        EVEColors::SUCCESS[2] * 0.5f, 0.8f));
    if (ImGui::Button("Complete Mission", ImVec2(150, 35))) {
        if (m_onComplete) {
            m_onComplete(m_data.mission_id);
        }
    }
    ImGui::PopStyleColor();
    
    if (!allComplete) {
        ImGui::EndDisabled();
    }
    
    ImGui::SameLine();
    
    // Decline button (can decline at any time, but with penalty)
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(
        EVEColors::DANGER[0] * 0.5f, EVEColors::DANGER[1] * 0.5f,
        EVEColors::DANGER[2] * 0.5f, 0.8f));
    if (ImGui::Button("Decline Mission", ImVec2(150, 35))) {
        if (m_onDecline) {
            m_onDecline(m_data.mission_id);
        }
    }
    ImGui::PopStyleColor();
}

void MissionPanel::RenderProgressBar() {
    // Calculate progress percentage
    if (m_data.objectives.empty()) return;
    
    int completed = 0;
    for (const auto& obj : m_data.objectives) {
        if (obj.completed) completed++;
    }
    
    float progress = static_cast<float>(completed) / static_cast<float>(m_data.objectives.size());
    
    ImGui::Text("Progress:");
    ImGui::ProgressBar(progress, ImVec2(-1.0f, 0.0f));
}

void MissionPanel::GetMissionTypeColor(float color[4]) const {
    if (m_data.mission_type == "combat") {
        color[0] = 1.0f; color[1] = 0.3f; color[2] = 0.3f; color[3] = 1.0f;  // Red
    } else if (m_data.mission_type == "courier") {
        color[0] = 0.3f; color[1] = 0.7f; color[2] = 1.0f; color[3] = 1.0f;  // Blue
    } else if (m_data.mission_type == "mining") {
        color[0] = 1.0f; color[1] = 0.8f; color[2] = 0.2f; color[3] = 1.0f;  // Gold
    } else if (m_data.mission_type == "exploration") {
        color[0] = 0.6f; color[1] = 0.3f; color[2] = 1.0f; color[3] = 1.0f;  // Purple
    } else {
        color[0] = 0.9f; color[1] = 0.95f; color[2] = 1.0f; color[3] = 1.0f;  // White
    }
}

void MissionPanel::RenderContents() {
    if (!m_data.is_active) {
        // No active mission
        ImGui::TextColored(ImVec4(
            EVEColors::TEXT_SECONDARY[0], EVEColors::TEXT_SECONDARY[1],
            EVEColors::TEXT_SECONDARY[2], EVEColors::TEXT_SECONDARY[3]),
            "No active mission");
        ImGui::Text("Accept a mission from an agent to get started.");
    } else {
        // Render mission details
        RenderMissionInfo();
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        RenderObjectivesList();
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        RenderRewards();
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        RenderProgressBar();
        ImGui::Spacing();
        RenderActionButtons();
    }
}

} // namespace UI
