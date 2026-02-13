#ifndef RADIAL_MENU_H
#define RADIAL_MENU_H

#include <string>
#include <functional>
#include <vector>
#include <cmath>

namespace atlas {
    class AtlasContext;
}

namespace UI {

/**
 * EVE Online-style radial menu for in-space interaction.
 *
 * Activated by holding left mouse button on an entity in space.
 * Shows a circular menu with options: Orbit, Approach, Warp To, Lock,
 * Keep at Range, Look At, and Show Info arranged in pie-slice segments.
 *
 * The player moves the mouse toward a segment to highlight it,
 * then releases to select. If released in the dead zone (center),
 * the menu cancels.
 */
class RadialMenu {
public:
    // Radial menu action options
    enum class Action {
        NONE = 0,
        ORBIT,
        APPROACH,
        WARP_TO,
        LOCK_TARGET,
        KEEP_AT_RANGE,
        LOOK_AT,
        SHOW_INFO,
        ALIGN_TO
    };

    // Callback for when an action is selected (with optional distance)
    using ActionCallback = std::function<void(Action action, const std::string& entityId)>;
    using RangedActionCallback = std::function<void(Action action, const std::string& entityId, int distance_m)>;

    RadialMenu();
    ~RadialMenu() = default;

    /**
     * Open the radial menu at screen position, targeting an entity.
     * Call when the user holds left-click on an entity.
     * @param distanceToTarget Distance in metres to the target entity (used to disable warp for nearby entities)
     */
    void Open(float screenX, float screenY, const std::string& entityId, float distanceToTarget = 0.0f);

    /**
     * Close/cancel the radial menu.
     */
    void Close();

    /**
     * Update mouse position while menu is open.
     * Determines which segment is highlighted.
     */
    void UpdateMousePosition(float mouseX, float mouseY);

    /**
     * Confirm selection (call on mouse release).
     * Returns the selected action.
     */
    Action Confirm();

    /**
     * Render the radial menu (legacy stub — no-op).
     */
    void Render();

    /**
     * Render the radial menu via Atlas (call between beginFrame/endFrame).
     */
    void RenderAtlas(atlas::AtlasContext& ctx);

    /**
     * Check if menu is currently open.
     */
    bool IsOpen() const { return m_open; }

    /**
     * Set callback for action selection.
     */
    void SetActionCallback(ActionCallback cb) { m_onAction = std::move(cb); }

    /**
     * Set callback for ranged actions (Orbit, Keep at Range).
     * Distance is determined by how far the mouse is dragged from center.
     */
    void SetRangedActionCallback(RangedActionCallback cb) { m_onRangedAction = std::move(cb); }

    /**
     * Get the currently highlighted action.
     */
    Action GetHighlightedAction() const { return m_highlightedAction; }

    /**
     * Get targeted entity ID.
     */
    const std::string& GetTargetEntity() const { return m_entityId; }

    /**
     * Get the drag-to-range distance (metres) for the current selection.
     * Only meaningful when highlighted action is ORBIT or KEEP_AT_RANGE.
     */
    int GetRangeDistance() const { return m_rangeDistance; }

private:
    // Menu segment layout
    struct Segment {
        Action action;
        const char* label;
        const char* icon;
        float startAngle;  // In radians
        float endAngle;
    };

    void SetupSegments();
    int GetSegmentAtAngle(float angle) const;
    void UpdateRangeDistance(float dist);

    // Minimum warp distance in metres (matches ShipPhysics::MIN_WARP_DISTANCE)
    static constexpr float MIN_WARP_DISTANCE = 150000.0f;

    bool m_open;
    float m_centerX, m_centerY;        // Screen center of the menu
    float m_mouseX, m_mouseY;          // Current mouse position
    std::string m_entityId;            // Target entity
    Action m_highlightedAction;        // Currently highlighted segment
    int m_rangeDistance = 0;            // Drag-to-range distance (metres)
    float m_distanceToTarget = 0.0f;   // Distance in metres to target entity

    /** Check if warp is disabled for the current target (too close). */
    bool isWarpDisabled() const {
        return m_distanceToTarget > 0.0f && m_distanceToTarget < MIN_WARP_DISTANCE;
    }

    std::vector<Segment> m_segments;

    ActionCallback m_onAction;
    RangedActionCallback m_onRangedAction;

    // Visual constants
    static constexpr float INNER_RADIUS = 30.0f;   // Dead zone radius
    static constexpr float OUTER_RADIUS = 100.0f;  // Menu outer radius
    static constexpr float ICON_RADIUS = 65.0f;    // Where icons/labels are drawn
    static constexpr float MAX_RANGE_RADIUS = 180.0f; // Max drag radius for range selection
};

} // namespace UI

#endif // RADIAL_MENU_H
