#include "ui/drone_control_panel.h"
#include "ui/ui_manager.h"
#include "ui/eve_colors.h"
#include <imgui.h>
#include <cstdio>

namespace UI {

DroneControlPanel::DroneControlPanel() = default;

void DroneControlPanel::Render() {
    if (!m_visible) return;

    ImGui::SetNextWindowSize(ImVec2(320, 400), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(60, 300), ImGuiCond_FirstUseEver);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse;
    if (!ImGui::Begin("Drones", &m_visible, flags)) {
        ImGui::End();
        return;
    }

    RenderContents();
    ImGui::End();
}

void DroneControlPanel::RenderContents() {
    RenderBandwidthBar();
    RenderBayCapacityBar();
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    RenderDronesInSpace();
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    RenderDronesInBay();
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    RenderDroneActions();
}

void DroneControlPanel::SetDroneBayData(const DroneBayData& data) {
    m_data = data;
}

void DroneControlPanel::RenderBandwidthBar() {
    float pct = (m_data.max_bandwidth > 0)
        ? static_cast<float>(m_data.used_bandwidth) / static_cast<float>(m_data.max_bandwidth)
        : 0.0f;

    char label[64];
    std::snprintf(label, sizeof(label), "Bandwidth: %d / %d Mbit/s",
                  m_data.used_bandwidth, m_data.max_bandwidth);

    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(
        EVEColors::ACCENT_PRIMARY[0], EVEColors::ACCENT_PRIMARY[1],
        EVEColors::ACCENT_PRIMARY[2], 0.8f));
    ImGui::ProgressBar(pct, ImVec2(-1, 16), label);
    ImGui::PopStyleColor();
}

void DroneControlPanel::RenderBayCapacityBar() {
    float pct = (m_data.bay_capacity > 0.0f)
        ? m_data.bay_used / m_data.bay_capacity
        : 0.0f;

    char label[64];
    std::snprintf(label, sizeof(label), "Bay: %.0f / %.0f m3",
                  m_data.bay_used, m_data.bay_capacity);

    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(
        EVEColors::SHIELD_COLOR[0], EVEColors::SHIELD_COLOR[1],
        EVEColors::SHIELD_COLOR[2], 0.8f));
    ImGui::ProgressBar(pct, ImVec2(-1, 16), label);
    ImGui::PopStyleColor();
}

void DroneControlPanel::RenderDronesInSpace() {
    ImGui::TextColored(ImVec4(
        EVEColors::ACCENT_PRIMARY[0], EVEColors::ACCENT_PRIMARY[1],
        EVEColors::ACCENT_PRIMARY[2], 1.0f),
        "Drones in Space (%zu)", m_data.space_drones.size());

    if (m_data.space_drones.empty()) {
        ImGui::TextColored(ImVec4(
            EVEColors::TEXT_DISABLED[0], EVEColors::TEXT_DISABLED[1],
            EVEColors::TEXT_DISABLED[2], 1.0f),
            "  No drones deployed");
        return;
    }

    ImGui::BeginChild("SpaceDrones", ImVec2(0, 120), true);
    for (size_t i = 0; i < m_data.space_drones.size(); ++i) {
        RenderDroneRow(m_data.space_drones[i], static_cast<int>(i), true,
                       m_selectedSpaceDrone == static_cast<int>(i));
    }
    ImGui::EndChild();
}

void DroneControlPanel::RenderDronesInBay() {
    ImGui::TextColored(ImVec4(
        EVEColors::TEXT_SECONDARY[0], EVEColors::TEXT_SECONDARY[1],
        EVEColors::TEXT_SECONDARY[2], 1.0f),
        "Drone Bay (%zu)", m_data.bay_drones.size());

    if (m_data.bay_drones.empty()) {
        ImGui::TextColored(ImVec4(
            EVEColors::TEXT_DISABLED[0], EVEColors::TEXT_DISABLED[1],
            EVEColors::TEXT_DISABLED[2], 1.0f),
            "  Drone bay empty");
        return;
    }

    ImGui::BeginChild("BayDrones", ImVec2(0, 120), true);
    for (size_t i = 0; i < m_data.bay_drones.size(); ++i) {
        RenderDroneRow(m_data.bay_drones[i], static_cast<int>(i), false,
                       m_selectedBayDrone == static_cast<int>(i));
    }
    ImGui::EndChild();
}

void DroneControlPanel::RenderDroneRow(const DroneDisplayInfo& drone, int index,
                                        bool is_space, bool selected) {
    ImGui::PushID(is_space ? (index + 1000) : index);

    // Selectable row
    if (ImGui::Selectable(("##drone_" + std::to_string(index)).c_str(), selected,
                          ImGuiSelectableFlags_SpanAllColumns, ImVec2(0, 24))) {
        if (is_space) {
            m_selectedSpaceDrone = index;
            m_selectedBayDrone = -1;
        } else {
            m_selectedBayDrone = index;
            m_selectedSpaceDrone = -1;
        }
    }

    ImGui::SameLine();

    // Drone name
    ImGui::Text("%s", drone.name.c_str());

    // Health bar on same line
    ImGui::SameLine(ImGui::GetContentRegionAvail().x - 80.0f);
    float hpPct = (drone.max_hitpoints > 0.0f)
        ? drone.hitpoints / drone.max_hitpoints
        : 0.0f;

    // Color based on health
    ImVec4 hpColor;
    if (hpPct > 0.5f) {
        hpColor = ImVec4(EVEColors::SUCCESS[0], EVEColors::SUCCESS[1],
                          EVEColors::SUCCESS[2], 0.8f);
    } else if (hpPct > 0.25f) {
        hpColor = ImVec4(EVEColors::WARNING[0], EVEColors::WARNING[1],
                          EVEColors::WARNING[2], 0.8f);
    } else {
        hpColor = ImVec4(EVEColors::DANGER[0], EVEColors::DANGER[1],
                          EVEColors::DANGER[2], 0.8f);
    }

    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, hpColor);
    ImGui::ProgressBar(hpPct, ImVec2(75, 14), "");
    ImGui::PopStyleColor();

    // Engaging indicator
    if (is_space && drone.is_engaging) {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(EVEColors::DANGER[0], EVEColors::DANGER[1],
                                   EVEColors::DANGER[2], 1.0f), "*");
    }

    ImGui::PopID();
}

void DroneControlPanel::RenderDroneActions() {
    float buttonWidth = (ImGui::GetContentRegionAvail().x - 8.0f) / 3.0f;

    // Group actions
    ImGui::TextColored(ImVec4(
        EVEColors::TEXT_SECONDARY[0], EVEColors::TEXT_SECONDARY[1],
        EVEColors::TEXT_SECONDARY[2], 1.0f), "Group Actions:");

    if (ImGui::Button("Launch All", ImVec2(buttonWidth, 28))) {
        if (m_onLaunchAll) m_onLaunchAll();
    }
    ImGui::SameLine();
    if (ImGui::Button("Return All", ImVec2(buttonWidth, 28))) {
        if (m_onReturnAll) m_onReturnAll();
    }
    ImGui::SameLine();
    if (ImGui::Button("Engage All", ImVec2(buttonWidth, 28))) {
        if (m_onEngageAll) m_onEngageAll();
    }

    ImGui::Spacing();

    // Single drone actions
    if (m_selectedBayDrone >= 0 && m_selectedBayDrone < static_cast<int>(m_data.bay_drones.size())) {
        const auto& drone = m_data.bay_drones[m_selectedBayDrone];
        ImGui::TextColored(ImVec4(
            EVEColors::TEXT_SECONDARY[0], EVEColors::TEXT_SECONDARY[1],
            EVEColors::TEXT_SECONDARY[2], 1.0f),
            "Selected: %s", drone.name.c_str());
        if (ImGui::Button("Launch", ImVec2(-1, 26))) {
            if (m_onLaunchDrone) m_onLaunchDrone(drone.drone_id);
        }
    }

    if (m_selectedSpaceDrone >= 0 && m_selectedSpaceDrone < static_cast<int>(m_data.space_drones.size())) {
        const auto& drone = m_data.space_drones[m_selectedSpaceDrone];
        ImGui::TextColored(ImVec4(
            EVEColors::TEXT_SECONDARY[0], EVEColors::TEXT_SECONDARY[1],
            EVEColors::TEXT_SECONDARY[2], 1.0f),
            "Selected: %s", drone.name.c_str());

        float halfWidth = (ImGui::GetContentRegionAvail().x - 4.0f) / 2.0f;
        if (ImGui::Button("Return", ImVec2(halfWidth, 26))) {
            if (m_onReturnDrone) m_onReturnDrone(drone.drone_id);
        }
        ImGui::SameLine();
        if (ImGui::Button("Engage", ImVec2(halfWidth, 26))) {
            if (m_onEngageDrone) m_onEngageDrone(drone.drone_id);
        }
    }

    // Keyboard shortcut hints
    ImGui::Spacing();
    ImGui::TextColored(ImVec4(
        EVEColors::TEXT_DISABLED[0], EVEColors::TEXT_DISABLED[1],
        EVEColors::TEXT_DISABLED[2], 0.7f),
        "Shift+F: Launch  Shift+R: Return  F: Engage");
}

} // namespace UI
