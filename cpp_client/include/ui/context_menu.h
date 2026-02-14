#ifndef CONTEXT_MENU_H
#define CONTEXT_MENU_H

#include <string>
#include <functional>
#include <vector>

namespace atlas {
    class AtlasContext;
}

namespace UI {

// Context menu action types
enum class ContextMenuAction {
    APPROACH,
    ORBIT,
    KEEP_AT_RANGE,
    WARP_TO,
    LOCK_TARGET,
    UNLOCK_TARGET,
    LOOK_AT,
    SHOW_INFO,
    NAVIGATE_TO,
    BOOKMARK,
    JUMP,
    CANCEL
};

// Orbit distance options
enum class OrbitDistance {
    ORBIT_500M = 500,
    ORBIT_1KM = 1000,
    ORBIT_5KM = 5000,
    ORBIT_10KM = 10000,
    ORBIT_20KM = 20000,
    ORBIT_50KM = 50000
};

// Keep at range distance options
enum class KeepAtRangeDistance {
    RANGE_1KM = 1000,
    RANGE_5KM = 5000,
    RANGE_10KM = 10000,
    RANGE_20KM = 20000,
    RANGE_50KM = 50000
};

// Warp to distance options
enum class WarpToDistance {
    WARP_0KM = 0,
    WARP_10KM = 10000,
    WARP_50KM = 50000,
    WARP_100KM = 100000
};

// Callback types for context menu actions
using ApproachCallback = std::function<void(const std::string& entity_id)>;
using OrbitCallback = std::function<void(const std::string& entity_id, int distance_m)>;
using KeepAtRangeCallback = std::function<void(const std::string& entity_id, int distance_m)>;
using WarpToCallback = std::function<void(const std::string& entity_id, int distance_m)>;
using LockTargetCallback = std::function<void(const std::string& entity_id)>;
using UnlockTargetCallback = std::function<void(const std::string& entity_id)>;
using LookAtCallback = std::function<void(const std::string& entity_id)>;
using ShowInfoCallback = std::function<void(const std::string& entity_id)>;
using NavigateToCallback = std::function<void(float x, float y, float z)>;
using BookmarkCallback = std::function<void(float x, float y, float z)>;
using JumpCallback = std::function<void(const std::string& entity_id)>;

// Context menu type
enum class ContextMenuType {
    NONE,
    ENTITY,      // Right-click on entity
    EMPTY_SPACE  // Right-click on empty space
};

/**
 * EVE-style context menu system
 * Handles right-click context menus for entities and empty space.
 * Renders via the Atlas UI renderer.
 */
class ContextMenu {
public:
    ContextMenu();
    ~ContextMenu() = default;
    
    /**
     * Show context menu for an entity
     * @param entity_id ID of the clicked entity
     * @param is_locked Whether the entity is currently locked as a target
     * @param is_stargate Whether the entity is a stargate (adds Jump option)
     */
    void ShowEntityMenu(const std::string& entity_id, bool is_locked = false, bool is_stargate = false);
    
    /**
     * Show context menu for empty space
     * @param world_x World X coordinate of click
     * @param world_y World Y coordinate of click
     * @param world_z World Z coordinate of click
     */
    void ShowEmptySpaceMenu(float world_x, float world_y, float world_z);
    
    /**
     * Render the context menu via Atlas (call each frame between beginFrame/endFrame)
     */
    void Render();

    /**
     * Render using the Atlas context (called from application render loop)
     */
    void RenderAtlas(atlas::AtlasContext& ctx);

    /**
     * Pre-consume the context menu area so panels don't steal clicks.
     * Call BEFORE rendering HUD panels; call RenderAtlas() AFTER panels.
     */
    void ReserveInputArea(atlas::AtlasContext& ctx);
    
    /**
     * Close the context menu
     */
    void Close();
    
    /**
     * Check if menu is open
     */
    bool IsOpen() const { return m_menuType != ContextMenuType::NONE; }

    /**
     * Set the screen position where the menu was invoked
     */
    void SetScreenPosition(float x, float y) { m_screenX = x; m_screenY = y; }
    
    // Set callbacks
    void SetApproachCallback(ApproachCallback callback) { m_onApproach = callback; }
    void SetOrbitCallback(OrbitCallback callback) { m_onOrbit = callback; }
    void SetKeepAtRangeCallback(KeepAtRangeCallback callback) { m_onKeepAtRange = callback; }
    void SetWarpToCallback(WarpToCallback callback) { m_onWarpTo = callback; }
    void SetLockTargetCallback(LockTargetCallback callback) { m_onLockTarget = callback; }
    void SetUnlockTargetCallback(UnlockTargetCallback callback) { m_onUnlockTarget = callback; }
    void SetLookAtCallback(LookAtCallback callback) { m_onLookAt = callback; }
    void SetShowInfoCallback(ShowInfoCallback callback) { m_onShowInfo = callback; }
    void SetNavigateToCallback(NavigateToCallback callback) { m_onNavigateTo = callback; }
    void SetBookmarkCallback(BookmarkCallback callback) { m_onBookmark = callback; }
    void SetJumpCallback(JumpCallback callback) { m_onJump = callback; }

private:
    // Render submenu for orbit distances
    void RenderOrbitSubmenu();
    
    // Render submenu for keep at range distances
    void RenderKeepAtRangeSubmenu();
    
    // Render submenu for warp to distances
    void RenderWarpToSubmenu();
    
    // Current menu state
    ContextMenuType m_menuType;
    std::string m_targetEntityId;
    bool m_targetIsLocked;
    bool m_targetIsStargate = false;
    float m_worldX, m_worldY, m_worldZ;
    float m_screenX = 0.0f, m_screenY = 0.0f;
    
    // Submenu state
    int m_activeSubmenu = -1;  // -1 = none, 0 = orbit, 1 = keeprange, 2 = warpto
    
    // Callbacks
    ApproachCallback m_onApproach;
    OrbitCallback m_onOrbit;
    KeepAtRangeCallback m_onKeepAtRange;
    WarpToCallback m_onWarpTo;
    LockTargetCallback m_onLockTarget;
    UnlockTargetCallback m_onUnlockTarget;
    LookAtCallback m_onLookAt;
    ShowInfoCallback m_onShowInfo;
    NavigateToCallback m_onNavigateTo;
    BookmarkCallback m_onBookmark;
    JumpCallback m_onJump;
};

} // namespace UI

#endif // CONTEXT_MENU_H
