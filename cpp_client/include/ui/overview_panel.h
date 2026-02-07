#ifndef OVERVIEW_PANEL_H
#define OVERVIEW_PANEL_H

#include <string>
#include <vector>
#include <functional>
#include <unordered_map>
#include <memory>
#include <glm/glm.hpp>

namespace eve {
    class Entity;
}

namespace UI {

// Overview entity entry
struct OverviewEntry {
    std::string entity_id;
    std::string name;
    std::string ship_type;
    std::string corporation;
    float distance;  // in meters
    int standing;  // -10 to +10, 0 = neutral
    bool is_player;
    
    // Health for sorting/display
    float shield_percent;
    float armor_percent;
    float hull_percent;
    
    OverviewEntry() 
        : distance(0.0f), standing(0), is_player(false)
        , shield_percent(1.0f), armor_percent(1.0f), hull_percent(1.0f) {}
};

// Filter settings
struct OverviewFilter {
    bool show_hostile = true;
    bool show_friendly = true;
    bool show_neutral = true;
    bool show_players = true;
    bool show_npcs = true;
    
    // Distance filter (in km, 0 = no limit)
    float max_distance_km = 0.0f;
    
    // Type filters
    std::vector<std::string> show_ship_types;  // Empty = show all
    
    std::string name = "All";
};

// Column sort settings
enum class OverviewSortColumn {
    NONE = 0,
    NAME,
    DISTANCE,
    TYPE,
    CORPORATION,
    STANDING
};

// Callback types
using SelectEntityCallback = std::function<void(const std::string& entity_id, bool ctrl_held)>;
using AlignToCallback = std::function<void(const std::string& entity_id)>;
using WarpToCallback = std::function<void(const std::string& entity_id, int distance_m)>;
using ApproachCallback = std::function<void(const std::string& entity_id)>;
using OrbitCallback = std::function<void(const std::string& entity_id, int distance_m)>;
using KeepAtRangeCallback = std::function<void(const std::string& entity_id, int distance_m)>;
using LockTargetCallback = std::function<void(const std::string& entity_id)>;
using UnlockTargetCallback = std::function<void(const std::string& entity_id)>;
using LookAtCallback = std::function<void(const std::string& entity_id)>;
using ShowInfoCallback = std::function<void(const std::string& entity_id)>;

class OverviewPanel {
public:
    OverviewPanel();
    ~OverviewPanel() = default;
    
    // Render the overview panel
    void Render();
    
    // Render just the panel contents (no Begin/End) — used by docking manager
    void RenderContents();
    
    // Update overview from entity list
    void UpdateEntities(const std::unordered_map<std::string, std::shared_ptr<eve::Entity>>& entities,
                        const glm::vec3& playerPosition = glm::vec3(0.0f));
    
    // Visibility
    void SetVisible(bool visible) { m_visible = visible; }
    bool IsVisible() const { return m_visible; }
    
    // Callbacks
    void SetSelectCallback(SelectEntityCallback callback) { m_onSelect = callback; }
    void SetAlignToCallback(AlignToCallback callback) { m_onAlignTo = callback; }
    void SetWarpToCallback(WarpToCallback callback) { m_onWarpTo = callback; }
    void SetApproachCallback(ApproachCallback callback) { m_onApproach = callback; }
    void SetOrbitCallback(OrbitCallback callback) { m_onOrbit = callback; }
    void SetKeepAtRangeCallback(KeepAtRangeCallback callback) { m_onKeepAtRange = callback; }
    void SetLockTargetCallback(LockTargetCallback callback) { m_onLockTarget = callback; }
    void SetUnlockTargetCallback(UnlockTargetCallback callback) { m_onUnlockTarget = callback; }
    void SetLookAtCallback(LookAtCallback callback) { m_onLookAt = callback; }
    void SetShowInfoCallback(ShowInfoCallback callback) { m_onShowInfo = callback; }
    
    // Filter management
    void SetFilter(const OverviewFilter& filter);
    const OverviewFilter& GetFilter() const { return m_currentFilter; }
    void AddFilter(const std::string& name, const OverviewFilter& filter);
    void SelectFilter(const std::string& name);
    
    // Get selected entity
    const std::string& GetSelectedEntity() const { return m_selectedEntity; }
    
    // Sort settings
    void SetSortColumn(OverviewSortColumn column, bool ascending = true);

private:
    bool m_visible;
    std::vector<OverviewEntry> m_entries;
    std::vector<OverviewEntry> m_filteredEntries;
    std::string m_selectedEntity;
    
    // Filter settings
    OverviewFilter m_currentFilter;
    std::unordered_map<std::string, OverviewFilter> m_savedFilters;
    
    // Sort settings
    OverviewSortColumn m_sortColumn;
    bool m_sortAscending;
    
    // Callbacks
    SelectEntityCallback m_onSelect;
    AlignToCallback m_onAlignTo;
    WarpToCallback m_onWarpTo;
    ApproachCallback m_onApproach;
    OrbitCallback m_onOrbit;
    KeepAtRangeCallback m_onKeepAtRange;
    LockTargetCallback m_onLockTarget;
    UnlockTargetCallback m_onUnlockTarget;
    LookAtCallback m_onLookAt;
    ShowInfoCallback m_onShowInfo;
    
    // Helper functions
    void RenderFilterTabs();
    void RenderTableHeader();
    void RenderEntityRow(const OverviewEntry& entry, int row_index);
    void ApplyFilter();
    void SortEntries();
    
    // Distance formatting
    std::string FormatDistance(float meters) const;
    
    // Standing colors
    void GetStandingColor(int standing, float color[4]) const;
};

} // namespace UI

#endif // OVERVIEW_PANEL_H
