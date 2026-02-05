#ifndef MISSION_PANEL_H
#define MISSION_PANEL_H

#include <string>
#include <vector>
#include <functional>

namespace UI {

// Mission objective
struct MissionObjective {
    std::string description;
    bool completed;
    
    MissionObjective() : completed(false) {}
    MissionObjective(const std::string& desc, bool comp = false)
        : description(desc), completed(comp) {}
};

// Mission data
struct MissionData {
    std::string mission_id;
    std::string mission_name = "No Active Mission";
    std::string mission_type = "combat";  // combat, courier, mining, exploration
    std::string agent_name = "";
    std::string location = "";
    int level = 1;
    
    std::vector<MissionObjective> objectives;
    
    // Rewards
    float isk_reward = 0.0f;
    float lp_reward = 0.0f;
    std::vector<std::string> item_rewards;
    
    // Status
    bool is_active = false;
    bool is_completed = false;
    float time_limit = 0.0f;  // hours (0 = no limit)
    float time_elapsed = 0.0f;  // hours
    
    MissionData() = default;
};

// Callback types
using AcceptMissionCallback = std::function<void(const std::string& mission_id)>;
using CompleteMissionCallback = std::function<void(const std::string& mission_id)>;
using DeclineMissionCallback = std::function<void(const std::string& mission_id)>;

class MissionPanel {
public:
    MissionPanel();
    ~MissionPanel() = default;
    
    // Render the mission panel
    void Render();
    
    // Update mission data
    void SetMissionData(const MissionData& data);
    
    // Visibility
    void SetVisible(bool visible) { m_visible = visible; }
    bool IsVisible() const { return m_visible; }
    
    // Callbacks
    void SetAcceptCallback(AcceptMissionCallback callback) { m_onAccept = callback; }
    void SetCompleteCallback(CompleteMissionCallback callback) { m_onComplete = callback; }
    void SetDeclineCallback(DeclineMissionCallback callback) { m_onDecline = callback; }
    
private:
    bool m_visible;
    MissionData m_data;
    
    // Callbacks
    AcceptMissionCallback m_onAccept;
    CompleteMissionCallback m_onComplete;
    DeclineMissionCallback m_onDecline;
    
    // Helper functions
    void RenderMissionInfo();
    void RenderObjectivesList();
    void RenderRewards();
    void RenderActionButtons();
    void RenderProgressBar();
    
    // Get color for mission type
    void GetMissionTypeColor(float color[4]) const;
};

} // namespace UI

#endif // MISSION_PANEL_H
