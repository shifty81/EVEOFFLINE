#include "ui/probe_scanner_panel.h"
#include "ui/hud_panels.h"
#include <imgui.h>
#include <algorithm>

namespace UI {

ProbeScannerPanel::ProbeScannerPanel() = default;

void ProbeScannerPanel::RenderContents() {
    // Probe controls header
    ImGui::TextColored(ImVec4(SpaceColors::ACCENT_PRIMARY[0], SpaceColors::ACCENT_PRIMARY[1],
                              SpaceColors::ACCENT_PRIMARY[2], SpaceColors::ACCENT_PRIMARY[3]),
                       "PROBE SCANNER");
    ImGui::Separator();
    ImGui::Spacing();

    // Probe count and range controls
    ImGui::Text("Probes: %d / 8", m_probeCount);
    ImGui::SameLine(160);
    ImGui::Text("Range: %.1f AU", m_probeRange);

    // Range slider
    ImGui::PushItemWidth(200);
    float range = m_probeRange;
    if (ImGui::SliderFloat("##ProbeRange", &range, 0.25f, 32.0f, "%.2f AU")) {
        m_probeRange = range;
    }
    ImGui::PopItemWidth();

    ImGui::SameLine();

    // Scan button
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.35f, 0.45f, 0.9f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.5f, 0.6f, 1.0f));
    if (ImGui::Button("Analyze", ImVec2(80, 0))) {
        m_scanRequested = true;
        if (m_scanCallback) {
            m_scanCallback();
        }
    }
    ImGui::PopStyleColor(2);

    ImGui::Spacing();

    // Filter checkboxes
    ImGui::Checkbox("Anomalies", &m_filterAnomalies);
    ImGui::SameLine();
    ImGui::Checkbox("Signatures", &m_filterSignatures);
    ImGui::SameLine();
    ImGui::Checkbox("Ships", &m_filterShips);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Results table
    if (m_results.empty()) {
        ImGui::TextColored(ImVec4(SpaceColors::TEXT_SECONDARY[0], SpaceColors::TEXT_SECONDARY[1],
                                  SpaceColors::TEXT_SECONDARY[2], SpaceColors::TEXT_SECONDARY[3]),
                           "No scan results. Deploy probes and click Analyze.");
        return;
    }

    // Table header
    ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                            ImGuiTableFlags_Sortable | ImGuiTableFlags_ScrollY;
    if (ImGui::BeginTable("ProbeScanResults", 5, flags, ImVec2(0, 0))) {
        ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, 60.0f);
        ImGui::TableSetupColumn("Group", ImGuiTableColumnFlags_WidthFixed, 120.0f);
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 100.0f);
        ImGui::TableSetupColumn("Signal", ImGuiTableColumnFlags_WidthFixed, 70.0f);
        ImGui::TableHeadersRow();

        for (const auto& result : m_results) {
            // Filter check
            bool show = false;
            if (m_filterAnomalies && result.group == "Cosmic Anomaly") show = true;
            if (m_filterSignatures && result.group == "Cosmic Signature") show = true;
            if (m_filterShips && result.group == "Ship") show = true;
            if (!m_filterAnomalies && !m_filterSignatures && !m_filterShips) show = true;
            if (!show) continue;

            ImGui::TableNextRow();

            // ID column
            ImGui::TableNextColumn();
            ImGui::TextColored(ImVec4(0.5f, 0.7f, 0.9f, 1.0f), "%s", result.id.c_str());

            // Group column
            ImGui::TableNextColumn();
            if (result.group == "Cosmic Anomaly") {
                ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.4f, 1.0f), "%s", result.group.c_str());
            } else if (result.group == "Cosmic Signature") {
                ImGui::TextColored(ImVec4(0.8f, 0.6f, 0.2f, 1.0f), "%s", result.group.c_str());
            } else {
                ImGui::Text("%s", result.group.c_str());
            }

            // Name column
            ImGui::TableNextColumn();
            if (result.signal_strength >= 100.0f) {
                ImGui::Text("%s", result.name.c_str());
            } else if (result.signal_strength >= 25.0f) {
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "%s", result.name.c_str());
            } else {
                ImGui::TextColored(ImVec4(0.4f, 0.4f, 0.4f, 1.0f), "Unknown");
            }

            // Type column
            ImGui::TableNextColumn();
            if (result.signal_strength >= 25.0f) {
                ImGui::Text("%s", result.type.c_str());
            } else {
                ImGui::TextColored(ImVec4(0.4f, 0.4f, 0.4f, 1.0f), "---");
            }

            // Signal strength column (with colored bar)
            ImGui::TableNextColumn();
            ImVec4 sigColor;
            if (result.signal_strength >= 100.0f) {
                sigColor = ImVec4(0.2f, 0.8f, 0.4f, 1.0f);  // Green
            } else if (result.signal_strength >= 75.0f) {
                sigColor = ImVec4(0.6f, 0.8f, 0.2f, 1.0f);  // Yellow-green
            } else if (result.signal_strength >= 25.0f) {
                sigColor = ImVec4(0.8f, 0.6f, 0.2f, 1.0f);  // Orange
            } else {
                sigColor = ImVec4(0.8f, 0.2f, 0.2f, 1.0f);  // Red
            }
            ImGui::TextColored(sigColor, "%.0f%%", result.signal_strength);
        }

        ImGui::EndTable();
    }
}

void ProbeScannerPanel::SetResults(const std::vector<ProbeScanResult>& results) {
    m_results = results;
    SortResults();
}

void ProbeScannerPanel::ClearResults() {
    m_results.clear();
}

bool ProbeScannerPanel::ConsumesScanRequest() {
    bool req = m_scanRequested;
    m_scanRequested = false;
    return req;
}

void ProbeScannerPanel::SetProbeCount(int count) {
    m_probeCount = std::clamp(count, 0, 8);
}

void ProbeScannerPanel::SetProbeRange(float au) {
    m_probeRange = std::clamp(au, 0.25f, 32.0f);
}

void ProbeScannerPanel::SortResults() {
    std::sort(m_results.begin(), m_results.end(),
              [this](const ProbeScanResult& a, const ProbeScanResult& b) {
        switch (m_sortColumn) {
            case 0: return m_sortAscending ? a.id < b.id : a.id > b.id;
            case 1: return m_sortAscending ? a.group < b.group : a.group > b.group;
            case 2: return m_sortAscending ? a.name < b.name : a.name > b.name;
            case 3: return m_sortAscending ? a.type < b.type : a.type > b.type;
            case 4: return m_sortAscending ? a.signal_strength < b.signal_strength
                                           : a.signal_strength > b.signal_strength;
            default: return false;
        }
    });
}

} // namespace UI
