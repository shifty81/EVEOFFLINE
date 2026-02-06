#include "ui/docking_manager.h"
#include <imgui.h>
#include <algorithm>
#include <iostream>
#include <sstream>

namespace UI {

DockingManager::DockingManager() {
}

void DockingManager::RegisterPanel(const std::string& id, const std::string& title,
                                    std::function<void()> renderContents,
                                    ImVec2 initialPos, ImVec2 initialSize) {
    DockablePanel panel;
    panel.id = id;
    panel.title = title;
    panel.renderContents = renderContents;
    panel.position = initialPos;
    panel.size = initialSize;
    panel.visible = true;
    panel.docked = false;
    panel.position_set = false;
    m_panels[id] = std::move(panel);
}

std::string DockingManager::DockPanel(const std::string& panelId, const std::string& containerId) {
    auto panelIt = m_panels.find(panelId);
    if (panelIt == m_panels.end()) return "";

    auto& panel = panelIt->second;

    // If already docked somewhere, undock first
    if (panel.docked) {
        UndockPanel(panelId);
    }

    std::string targetId = containerId;

    // Create new container if needed
    if (targetId.empty() || m_containers.find(targetId) == m_containers.end()) {
        targetId = GenerateContainerId();
        DockContainer container;
        container.id = targetId;
        container.title = panel.title;
        container.position = panel.position;
        container.size = panel.size;
        container.position_set = false;
        m_containers[targetId] = std::move(container);
    }

    auto& container = m_containers[targetId];
    container.panel_ids.push_back(panelId);
    panel.docked = true;
    panel.dock_container_id = targetId;

    return targetId;
}

std::string DockingManager::DockPanelsTogether(const std::string& panelId1, const std::string& panelId2) {
    std::string containerId = DockPanel(panelId1);
    DockPanel(panelId2, containerId);
    return containerId;
}

void DockingManager::UndockPanel(const std::string& panelId) {
    auto panelIt = m_panels.find(panelId);
    if (panelIt == m_panels.end()) return;

    auto& panel = panelIt->second;
    if (!panel.docked) return;

    auto containerIt = m_containers.find(panel.dock_container_id);
    if (containerIt != m_containers.end()) {
        auto& container = containerIt->second;

        // Use container position as floating position for undocked panel
        panel.position = container.position;
        panel.position.x += 20.0f;  // Slight offset so it's visually distinct
        panel.position.y += 20.0f;
        panel.position_set = false;

        // Remove panel from container
        auto& ids = container.panel_ids;
        ids.erase(std::remove(ids.begin(), ids.end(), panelId), ids.end());

        // Clamp active tab
        if (container.active_tab >= static_cast<int>(ids.size())) {
            container.active_tab = std::max(0, static_cast<int>(ids.size()) - 1);
        }

        // If container has 0 or 1 panel left, dissolve it
        if (ids.empty()) {
            m_containers.erase(containerIt);
        } else if (ids.size() == 1) {
            // Undock the remaining panel too and destroy the container
            auto& lastPanelId = ids[0];
            auto lastPanelIt = m_panels.find(lastPanelId);
            if (lastPanelIt != m_panels.end()) {
                lastPanelIt->second.docked = false;
                lastPanelIt->second.dock_container_id.clear();
                lastPanelIt->second.position = container.position;
                lastPanelIt->second.size = container.size;
                lastPanelIt->second.position_set = false;
            }
            m_containers.erase(containerIt);
        }
    }

    panel.docked = false;
    panel.dock_container_id.clear();
}

void DockingManager::SetPanelVisible(const std::string& panelId, bool visible) {
    auto it = m_panels.find(panelId);
    if (it != m_panels.end()) {
        it->second.visible = visible;
    }
}

bool DockingManager::IsPanelVisible(const std::string& panelId) const {
    auto it = m_panels.find(panelId);
    if (it != m_panels.end()) {
        return it->second.visible;
    }
    return false;
}

void DockingManager::RenderAll() {
    // Render dock containers (tabbed windows)
    for (auto& [id, container] : m_containers) {
        RenderDockContainer(container);
    }

    // Render floating (undocked) panels
    for (auto& [id, panel] : m_panels) {
        if (!panel.docked && panel.visible) {
            RenderFloatingPanel(panel);
        }
    }

    // Render interface lock button
    RenderLockButton();
}

void DockingManager::RenderLockButton() {
    ImGuiIO& io = ImGui::GetIO();
    float buttonSize = 30.0f;
    ImVec2 pos(io.DisplaySize.x - buttonSize - 10.0f, 10.0f);

    ImGui::SetNextWindowPos(pos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(buttonSize, buttonSize), ImGuiCond_Always);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                             ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    if (ImGui::Begin("##LockButton", nullptr, flags)) {
        const char* icon = m_interfaceLocked ? "L" : "U";
        ImVec4 color = m_interfaceLocked
            ? ImVec4(0.8f, 0.2f, 0.2f, 1.0f)   // Red when locked
            : ImVec4(0.3f, 0.8f, 0.3f, 1.0f);   // Green when unlocked

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.1f, 0.15f, 0.8f));
        ImGui::PushStyleColor(ImGuiCol_Text, color);

        if (ImGui::Button(icon, ImVec2(buttonSize - 4, buttonSize - 4))) {
            ToggleInterfaceLock();
        }
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::Text(m_interfaceLocked ? "Interface LOCKED (click to unlock)" : "Interface UNLOCKED (click to lock)");
            ImGui::EndTooltip();
        }

        ImGui::PopStyleColor(2);
    }
    ImGui::End();
    ImGui::PopStyleVar();
}

void DockingManager::RenderFloatingPanel(DockablePanel& panel) {
    if (!panel.visible || !panel.renderContents) return;

    // Set initial position
    if (!panel.position_set) {
        ImGui::SetNextWindowPos(panel.position, ImGuiCond_Once);
        ImGui::SetNextWindowSize(panel.size, ImGuiCond_Once);
        panel.position_set = true;
    }

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse;
    if (m_interfaceLocked) {
        flags |= ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
    }

    if (ImGui::Begin(panel.title.c_str(), &panel.visible, flags)) {
        // Track position/size for snapping/docking
        panel.position = ImGui::GetWindowPos();
        panel.size = ImGui::GetWindowSize();

        // Render panel contents
        panel.renderContents();

        // Snap logic (only when not locked)
        if (!m_interfaceLocked) {
            TrySnapPanel(panel);
        }
    }
    ImGui::End();
}

void DockingManager::RenderDockContainer(DockContainer& container) {
    // Check if any panels in this container are visible
    bool anyVisible = false;
    for (const auto& pid : container.panel_ids) {
        auto it = m_panels.find(pid);
        if (it != m_panels.end() && it->second.visible) {
            anyVisible = true;
            break;
        }
    }
    if (!anyVisible) return;

    // Set initial position
    if (!container.position_set) {
        ImGui::SetNextWindowPos(container.position, ImGuiCond_Once);
        ImGui::SetNextWindowSize(container.size, ImGuiCond_Once);
        container.position_set = true;
    }

    // Build window title from docked panels
    std::string windowTitle = "##DockContainer_" + container.id;

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse;
    if (m_interfaceLocked) {
        flags |= ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
    }

    bool windowOpen = true;
    if (ImGui::Begin(windowTitle.c_str(), &windowOpen, flags)) {
        container.position = ImGui::GetWindowPos();
        container.size = ImGui::GetWindowSize();

        // Render tab bar
        if (ImGui::BeginTabBar(("##Tabs_" + container.id).c_str(), ImGuiTabBarFlags_Reorderable)) {
            for (int i = 0; i < static_cast<int>(container.panel_ids.size()); ++i) {
                const auto& panelId = container.panel_ids[i];
                auto panelIt = m_panels.find(panelId);
                if (panelIt == m_panels.end() || !panelIt->second.visible) continue;

                auto& panel = panelIt->second;
                bool tabOpen = true;

                ImGuiTabItemFlags tabFlags = ImGuiTabItemFlags_None;

                if (ImGui::BeginTabItem(panel.title.c_str(), &tabOpen, tabFlags)) {
                    container.active_tab = i;

                    // Handle tab dragging for undock (when not locked)
                    if (!m_interfaceLocked && ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
                        HandleTabDragUndock(container, i);
                    } else if (m_tabDragActive && m_draggingTabPanelId == panelId && !ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
                        // Drag ended without undocking
                        m_tabDragActive = false;
                        m_draggingTabPanelId.clear();
                    }

                    // Render panel contents
                    if (panel.renderContents) {
                        panel.renderContents();
                    }

                    ImGui::EndTabItem();
                }

                // If tab was closed via the X button
                if (!tabOpen) {
                    panel.visible = false;
                }
            }
            ImGui::EndTabBar();
        }
    }
    ImGui::End();
}

void DockingManager::HandleTabDragUndock(DockContainer& container, int tabIndex) {
    if (tabIndex < 0 || tabIndex >= static_cast<int>(container.panel_ids.size())) return;
    if (container.panel_ids.size() <= 1) return;  // Can't undock the only tab

    const std::string& panelId = container.panel_ids[tabIndex];

    if (!m_tabDragActive || m_draggingTabPanelId != panelId) {
        // Start tracking drag
        m_tabDragActive = true;
        m_draggingTabPanelId = panelId;
        m_tabDragStartPos = ImGui::GetMousePos();
        return;
    }

    // Check drag distance
    ImVec2 mousePos = ImGui::GetMousePos();
    float dx = mousePos.x - m_tabDragStartPos.x;
    float dy = mousePos.y - m_tabDragStartPos.y;
    float dist = std::sqrt(dx * dx + dy * dy);

    if (dist > TAB_UNDOCK_DISTANCE) {
        // Undock the panel
        auto panelIt = m_panels.find(panelId);
        if (panelIt != m_panels.end()) {
            panelIt->second.position = mousePos;
            panelIt->second.position_set = false;
        }
        UndockPanel(panelId);
        m_tabDragActive = false;
        m_draggingTabPanelId.clear();
    }
}

void DockingManager::TrySnapPanel(DockablePanel& panel) {
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 pos = panel.position;
    ImVec2 size = panel.size;
    bool snapped = false;

    // Snap to screen edges
    if (pos.x < SNAP_DISTANCE) { pos.x = 0; snapped = true; }
    if (pos.y < SNAP_DISTANCE) { pos.y = 0; snapped = true; }
    if (pos.x + size.x > io.DisplaySize.x - SNAP_DISTANCE) {
        pos.x = io.DisplaySize.x - size.x;
        snapped = true;
    }
    if (pos.y + size.y > io.DisplaySize.y - SNAP_DISTANCE) {
        pos.y = io.DisplaySize.y - size.y;
        snapped = true;
    }

    // Snap to other floating panels
    for (const auto& [otherId, other] : m_panels) {
        if (otherId == panel.id || other.docked || !other.visible) continue;

        ImVec2 otherEnd(other.position.x + other.size.x, other.position.y + other.size.y);
        ImVec2 panelEnd(pos.x + size.x, pos.y + size.y);

        // Right edge of panel to left edge of other
        if (std::abs(panelEnd.x - other.position.x) < SNAP_DISTANCE) {
            pos.x = other.position.x - size.x;
            snapped = true;
        }
        // Left edge of panel to right edge of other
        if (std::abs(pos.x - otherEnd.x) < SNAP_DISTANCE) {
            pos.x = otherEnd.x;
            snapped = true;
        }
        // Bottom edge of panel to top edge of other
        if (std::abs(panelEnd.y - other.position.y) < SNAP_DISTANCE) {
            pos.y = other.position.y - size.y;
            snapped = true;
        }
        // Top edge of panel to bottom edge of other
        if (std::abs(pos.y - otherEnd.y) < SNAP_DISTANCE) {
            pos.y = otherEnd.y;
            snapped = true;
        }
    }

    if (snapped) {
        ImGui::SetWindowPos(pos);
        panel.position = pos;
    }
}

std::string DockingManager::GenerateContainerId() {
    std::ostringstream oss;
    oss << "dock_" << m_nextContainerId++;
    return oss.str();
}

} // namespace UI
