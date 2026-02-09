#include "ui/dscan_panel.h"
#include "ui/ui_manager.h"
#include <imgui.h>
#include <algorithm>
#include <cstdio>
#include <cmath>

namespace UI {

DScanPanel::DScanPanel() = default;

void DScanPanel::SetResults(const std::vector<DScanResult>& results) {
    m_results = results;
    SortResults();
}

void DScanPanel::ClearResults() {
    m_results.clear();
}

void DScanPanel::SetScanAngle(float degrees) {
    m_scanAngle = std::clamp(degrees, 5.0f, 360.0f);
}

void DScanPanel::SetScanRange(float au) {
    m_scanRange = std::clamp(au, 0.1f, 14.3f);
}

bool DScanPanel::ConsumesScanRequest() {
    bool r = m_scanRequested;
    m_scanRequested = false;
    return r;
}

void DScanPanel::SortResults() {
    std::sort(m_results.begin(), m_results.end(),
        [this](const DScanResult& a, const DScanResult& b) {
            int cmp = 0;
            switch (m_sortColumn) {
                case 0: cmp = a.type.compare(b.type); break;
                case 1: cmp = a.name.compare(b.name); break;
                case 2: cmp = (a.distance < b.distance) ? -1 : (a.distance > b.distance) ? 1 : 0; break;
                default: break;
            }
            return m_sortAscending ? (cmp < 0) : (cmp > 0);
        });
}

void DScanPanel::RenderContents() {
    // ---- Scan controls ----
    ImGui::TextColored(
        ImVec4(EVEColors::ACCENT_PRIMARY[0], EVEColors::ACCENT_PRIMARY[1],
               EVEColors::ACCENT_PRIMARY[2], 1.0f),
        "Directional Scanner");
    ImGui::Separator();
    ImGui::Spacing();

    // Angle slider
    ImGui::Text("Scan Angle");
    ImGui::SameLine(100);
    char angleLabel[16];
    std::snprintf(angleLabel, sizeof(angleLabel), "%.0f deg", m_scanAngle);
    ImGui::SliderFloat("##angle", &m_scanAngle, 5.0f, 360.0f, angleLabel);

    // Range slider
    ImGui::Text("Range");
    ImGui::SameLine(100);
    char rangeLabel[16];
    std::snprintf(rangeLabel, sizeof(rangeLabel), "%.1f AU", m_scanRange);
    ImGui::SliderFloat("##range", &m_scanRange, 0.1f, 14.3f, rangeLabel);

    ImGui::Spacing();

    // Scan button — full width, EVE-teal accent
    ImGui::PushStyleColor(ImGuiCol_Button,
        ImVec4(EVEColors::ACCENT_DIM[0], EVEColors::ACCENT_DIM[1],
               EVEColors::ACCENT_DIM[2], 0.9f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
        ImVec4(EVEColors::ACCENT_PRIMARY[0] * 0.8f, EVEColors::ACCENT_PRIMARY[1] * 0.8f,
               EVEColors::ACCENT_PRIMARY[2] * 0.8f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,
        ImVec4(EVEColors::ACCENT_PRIMARY[0], EVEColors::ACCENT_PRIMARY[1],
               EVEColors::ACCENT_PRIMARY[2], 1.0f));

    if (ImGui::Button("Scan  (V)", ImVec2(-1, 28))) {
        m_scanRequested = true;
        if (m_scanCallback) {
            m_scanCallback(m_scanAngle, m_scanRange);
        }
    }
    ImGui::PopStyleColor(3);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // ---- Results table ----
    char header[64];
    std::snprintf(header, sizeof(header), "Results (%zu)", m_results.size());
    ImGui::Text("%s", header);
    ImGui::Spacing();

    if (m_results.empty()) {
        ImGui::TextColored(
            ImVec4(EVEColors::TEXT_SECONDARY[0], EVEColors::TEXT_SECONDARY[1],
                   EVEColors::TEXT_SECONDARY[2], 1.0f),
            "No results. Press Scan to scan.");
        return;
    }

    // Column headers
    const ImGuiTableFlags tableFlags =
        ImGuiTableFlags_Resizable | ImGuiTableFlags_Sortable |
        ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV |
        ImGuiTableFlags_ScrollY;

    if (ImGui::BeginTable("##dscan_results", 3, tableFlags, ImVec2(0, 0))) {
        ImGui::TableSetupColumn("Type",     ImGuiTableColumnFlags_DefaultSort, 100.0f);
        ImGui::TableSetupColumn("Name",     0, 140.0f);
        ImGui::TableSetupColumn("Distance", ImGuiTableColumnFlags_DefaultSort, 80.0f);
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableHeadersRow();

        // Handle sort specs
        if (ImGuiTableSortSpecs* sortSpecs = ImGui::TableGetSortSpecs()) {
            if (sortSpecs->SpecsDirty && sortSpecs->SpecsCount > 0) {
                m_sortColumn = sortSpecs->Specs[0].ColumnIndex;
                m_sortAscending = (sortSpecs->Specs[0].SortDirection == ImGuiSortDirection_Ascending);
                SortResults();
                sortSpecs->SpecsDirty = false;
            }
        }

        for (const auto& r : m_results) {
            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted(r.type.c_str());

            ImGui::TableSetColumnIndex(1);
            ImGui::TextUnformatted(r.name.c_str());

            ImGui::TableSetColumnIndex(2);
            char distText[32];
            if (r.distance < 1.0f) {
                constexpr float KM_PER_AU = 149597.871f;
                std::snprintf(distText, sizeof(distText), "%.0f km", r.distance * KM_PER_AU);
            } else {
                std::snprintf(distText, sizeof(distText), "%.2f AU", r.distance);
            }
            ImGui::TextUnformatted(distText);
        }

        ImGui::EndTable();
    }
}

} // namespace UI
