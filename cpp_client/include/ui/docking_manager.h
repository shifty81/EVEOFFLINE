#ifndef DOCKING_MANAGER_H
#define DOCKING_MANAGER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <imgui.h>

namespace UI {

/**
 * Represents a single dockable panel that can be rendered independently
 * or as a tab inside a dock container.
 */
struct DockablePanel {
    std::string id;           // Unique panel identifier
    std::string title;        // Display title (used as tab label)
    bool visible = true;
    bool docked = false;
    std::string dock_container_id;  // ID of the container this panel is docked into ("" if floating)

    // Render callback - draws the panel contents (without Begin/End)
    std::function<void()> renderContents;

    // Floating window state
    ImVec2 position = {0, 0};
    ImVec2 size = {300, 400};
    bool position_set = false;  // Whether initial position has been applied

    // Panel controls
    bool collapsed = false;     // Whether the panel body is collapsed (header only)
    bool pinned = false;        // Whether the panel size is locked (pinned)
    float opacity = 0.92f;      // Background opacity (0.0 - 1.0); text stays readable
};

/**
 * A container that holds multiple docked panels as tabs.
 */
struct DockContainer {
    std::string id;
    std::string title;          // Container window title (updated based on active tab)
    std::vector<std::string> panel_ids;  // Ordered list of docked panel IDs
    int active_tab = 0;

    ImVec2 position = {50, 50};
    ImVec2 size = {600, 400};
    bool position_set = false;
};

/**
 * DockingManager — Manages panel docking, undocking, snapping, and interface locking.
 *
 * Features:
 *  - Panels can be docked together into tabbed containers
 *  - Tabs can be dragged out to undock
 *  - Interface can be locked to prevent accidental moves
 *  - Panels snap to edges and to each other
 */
class DockingManager {
public:
    DockingManager();
    ~DockingManager() = default;

    /**
     * Register a dockable panel.
     * @param id Unique identifier
     * @param title Display title
     * @param renderContents Callback that renders the panel body
     * @param initialPos Initial floating position
     * @param initialSize Initial floating size
     */
    void RegisterPanel(const std::string& id, const std::string& title,
                       std::function<void()> renderContents,
                       ImVec2 initialPos = {0, 0}, ImVec2 initialSize = {300, 400});

    /**
     * Dock a panel into an existing container (or create a new one).
     * @param panelId The panel to dock
     * @param containerId Target container ("" to create new)
     * @return The container ID
     */
    std::string DockPanel(const std::string& panelId, const std::string& containerId = "");

    /**
     * Dock two panels together into a new container.
     */
    std::string DockPanelsTogether(const std::string& panelId1, const std::string& panelId2);

    /**
     * Undock a panel from its container.
     */
    void UndockPanel(const std::string& panelId);

    /**
     * Set panel visibility.
     */
    void SetPanelVisible(const std::string& panelId, bool visible);
    bool IsPanelVisible(const std::string& panelId) const;

    /**
     * Toggle interface lock (prevents panel moving/resizing).
     */
    void SetInterfaceLocked(bool locked) { m_interfaceLocked = locked; }
    bool IsInterfaceLocked() const { return m_interfaceLocked; }
    void ToggleInterfaceLock() { m_interfaceLocked = !m_interfaceLocked; }

    /**
     * Render all panels and containers.
     * Call between ImGui NewFrame and Render.
     */
    void RenderAll();

    /**
     * Render the interface lock toggle button (small overlay).
     */
    void RenderLockButton();

private:
    void RenderFloatingPanel(DockablePanel& panel);
    void RenderDockContainer(DockContainer& container);
    void HandleTabDragUndock(DockContainer& container, int tabIndex);
    void TrySnapPanel(DockablePanel& panel);

    // Generate unique container ID
    std::string GenerateContainerId();

    std::unordered_map<std::string, DockablePanel> m_panels;
    std::unordered_map<std::string, DockContainer> m_containers;

    bool m_interfaceLocked = false;
    int m_nextContainerId = 0;

    // Drag state for tab undocking
    std::string m_draggingTabPanelId;
    bool m_tabDragActive = false;
    ImVec2 m_tabDragStartPos = {0, 0};
    static constexpr float TAB_UNDOCK_DISTANCE = 30.0f;  // Pixels to drag before undocking
    static constexpr float SNAP_DISTANCE = 15.0f;        // Edge snap distance
};

} // namespace UI

#endif // DOCKING_MANAGER_H
