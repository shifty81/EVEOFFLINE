#include "ui/sidebar_panel.h"
#include "ui/ui_manager.h"
#include "ui/space_colors.h"
#include <imgui.h>

namespace UI {

SidebarPanel::SidebarPanel() = default;

bool SidebarPanel::RenderButton(const char* icon, const char* label, const char* tooltip) {
    bool clicked = false;

    float buttonSize = 40.0f;
    float expandedWidth = 170.0f;

    // Push Photon UI hover highlight colors
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(
        SpaceColors::SELECTION[0], SpaceColors::SELECTION[1],
        SpaceColors::SELECTION[2], 0.9f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(
        SpaceColors::ACCENT_DIM[0], SpaceColors::ACCENT_DIM[1],
        SpaceColors::ACCENT_DIM[2], 1.0f));

    if (m_collapsed) {
        // Icon-only button
        clicked = ImGui::Button(icon, ImVec2(buttonSize, buttonSize));
    } else {
        // Icon + label
        char fullLabel[128];
        std::snprintf(fullLabel, sizeof(fullLabel), "%s  %s", icon, label);
        clicked = ImGui::Button(fullLabel, ImVec2(expandedWidth, buttonSize));
    }

    // Hover highlight border effect
    if (ImGui::IsItemHovered()) {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 min = ImGui::GetItemRectMin();
        ImVec2 max = ImGui::GetItemRectMax();
        drawList->AddRect(min, max, IM_COL32(69, 208, 232, 120), 2.0f, 0, 1.5f);
    }

    ImGui::PopStyleColor(2);

    if (ImGui::IsItemHovered() && tooltip) {
        ImGui::SetTooltip("%s", tooltip);
    }

    return clicked;
}

void SidebarPanel::Render() {
    if (!m_visible) return;

    // Position on the left edge, full height
    ImGuiIO& io = ImGui::GetIO();
    float barWidth = m_collapsed ? 56.0f : 200.0f;
    float barHeight = io.DisplaySize.y;

    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(barWidth, barHeight), ImGuiCond_Always);

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBringToFrontOnFocus;

    // Semi-transparent dark background — Sidebar panel
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(
        SpaceColors::BG_PANEL[0], SpaceColors::BG_PANEL[1],
        SpaceColors::BG_PANEL[2], 0.92f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(6, 8));

    ImGui::Begin("##Sidebar", nullptr, flags);

    // ---- Collapse / Expand toggle ----
    {
        const char* toggleIcon = m_collapsed ? ">>" : "<<";
        if (ImGui::Button(toggleIcon, ImVec2(m_collapsed ? 40.0f : 170.0f, 24.0f))) {
            ToggleCollapsed();
        }
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // ---- Service buttons ----

    if (RenderButton("[C]", "Character", "Character Sheet (C)")) {
        if (m_onCharacterSheet) m_onCharacterSheet();
    }

    ImGui::Spacing();

    if (RenderButton("[I]", "Inventory", "Inventory (Alt+T)")) {
        if (m_onInventory) m_onInventory();
    }

    ImGui::Spacing();

    if (RenderButton("[F]", "Fitting", "Fitting Window (Alt+F)")) {
        if (m_onFitting) m_onFitting();
    }

    ImGui::Spacing();

    if (RenderButton("[M]", "Market", "Market")) {
        if (m_onMarket) m_onMarket();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (RenderButton("[*]", "Map", "Star Map (F10)")) {
        if (m_onMap) m_onMap();
    }

    ImGui::Spacing();

    if (RenderButton("[D]", "D-Scan", "Directional Scanner (V)")) {
        if (m_onDScan) m_onDScan();
    }

    ImGui::Spacing();

    if (RenderButton("[J]", "Missions", "Mission Journal")) {
        if (m_onMissions) m_onMissions();
    }

    ImGui::Spacing();

    if (RenderButton("[H]", "Chat", "Chat Channels")) {
        if (m_onChat) m_onChat();
    }

    ImGui::Spacing();

    if (RenderButton("[R]", "Drones", "Drone Control (Shift+F)")) {
        if (m_onDrones) m_onDrones();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (RenderButton("[G]", "Corporation", "Corporation")) {
        if (m_onCorporation) m_onCorporation();
    }

    ImGui::Spacing();

    if (RenderButton("[S]", "Settings", "Settings")) {
        if (m_onSettings) m_onSettings();
    }

    ImGui::End();

    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}

} // namespace UI
