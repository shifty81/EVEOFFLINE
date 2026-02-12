#ifndef DRONE_CONTROL_PANEL_H
#define DRONE_CONTROL_PANEL_H

#include <string>
#include <vector>
#include <functional>

namespace UI {

// Drone info for display
struct DroneDisplayInfo {
    std::string drone_id;
    std::string name;
    std::string type;          // "light_combat", "medium_combat", "heavy_combat", "sentry", "mining", "salvage"
    float hitpoints = 100.0f;
    float max_hitpoints = 100.0f;
    int bandwidth_use = 5;
    float volume = 5.0f;       // m3
    bool is_deployed = false;
    bool is_engaging = false;

    DroneDisplayInfo() = default;
    DroneDisplayInfo(const std::string& id, const std::string& n, const std::string& t,
                     float hp, float max_hp, int bw, float vol)
        : drone_id(id), name(n), type(t), hitpoints(hp), max_hitpoints(max_hp),
          bandwidth_use(bw), volume(vol) {}
};

// Drone bay data
struct DroneBayData {
    std::vector<DroneDisplayInfo> bay_drones;      // Drones in bay (not deployed)
    std::vector<DroneDisplayInfo> space_drones;     // Drones in space (deployed)
    float bay_capacity = 25.0f;        // m3
    float bay_used = 0.0f;             // m3
    int max_bandwidth = 25;            // Mbit/s
    int used_bandwidth = 0;            // Mbit/s
};

// Callback types
using LaunchDroneCallback = std::function<void(const std::string& drone_id)>;
using ReturnDroneCallback = std::function<void(const std::string& drone_id)>;
using EngageDroneCallback = std::function<void(const std::string& drone_id)>;
using LaunchAllCallback = std::function<void()>;
using ReturnAllCallback = std::function<void()>;
using EngageAllCallback = std::function<void()>;

class DroneControlPanel {
public:
    DroneControlPanel();
    ~DroneControlPanel() = default;

    // Render the drone panel
    void Render();

    // Render just the panel contents (no Begin/End) — used by docking manager
    void RenderContents();

    // Set drone bay data
    void SetDroneBayData(const DroneBayData& data);

    // Visibility
    void SetVisible(bool visible) { m_visible = visible; }
    bool IsVisible() const { return m_visible; }
    const DroneBayData& GetData() const { return m_data; }

    // Callbacks
    void SetLaunchDroneCallback(LaunchDroneCallback cb) { m_onLaunchDrone = cb; }
    void SetReturnDroneCallback(ReturnDroneCallback cb) { m_onReturnDrone = cb; }
    void SetEngageDroneCallback(EngageDroneCallback cb) { m_onEngageDrone = cb; }
    void SetLaunchAllCallback(LaunchAllCallback cb) { m_onLaunchAll = cb; }
    void SetReturnAllCallback(ReturnAllCallback cb) { m_onReturnAll = cb; }
    void SetEngageAllCallback(EngageAllCallback cb) { m_onEngageAll = cb; }

private:
    bool m_visible = false;
    DroneBayData m_data;

    // UI state
    int m_selectedBayDrone = -1;
    int m_selectedSpaceDrone = -1;

    // Callbacks
    LaunchDroneCallback m_onLaunchDrone;
    ReturnDroneCallback m_onReturnDrone;
    EngageDroneCallback m_onEngageDrone;
    LaunchAllCallback m_onLaunchAll;
    ReturnAllCallback m_onReturnAll;
    EngageAllCallback m_onEngageAll;

    // Helper functions
    void RenderBandwidthBar();
    void RenderBayCapacityBar();
    void RenderDronesInSpace();
    void RenderDronesInBay();
    void RenderDroneRow(const DroneDisplayInfo& drone, int index, bool is_space, bool selected);
    void RenderDroneActions();
};

} // namespace UI

#endif // DRONE_CONTROL_PANEL_H
