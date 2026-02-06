#ifndef EVE_TARGET_LIST_H
#define EVE_TARGET_LIST_H

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <glm/glm.hpp>

namespace eve {
    class Entity;
}

namespace UI {

/**
 * EVE Online-style target information
 */
struct TargetData {
    std::string entityId;
    std::string shipName;
    std::string shipType;
    float distance;
    
    // Health data (0.0 to 1.0)
    float shieldPercent;
    float armorPercent;
    float hullPercent;
    
    // Max values for tooltip display
    float maxShield;
    float maxArmor;
    float maxHull;
    
    bool isActive;  // Is this the active target?
    float lockProgress;  // 0.0 to 1.0 for locking animation
    
    TargetData() 
        : distance(0.0f)
        , shieldPercent(1.0f)
        , armorPercent(1.0f)
        , hullPercent(1.0f)
        , maxShield(0.0f)
        , maxArmor(0.0f)
        , maxHull(0.0f)
        , isActive(false)
        , lockProgress(1.0f)
    {}
};

/**
 * EVE Online-style target list panel
 * Displays circular target icons with arc-based health indicators
 */
class EVETargetList {
public:
    EVETargetList();
    ~EVETargetList();
    
    /**
     * Render the target list
     */
    void render();
    
    /**
     * Update targets from entity list
     * @param entities Map of entity ID to entity
     */
    void updateTargets(const std::unordered_map<std::string, std::shared_ptr<eve::Entity>>& entities);
    
    /**
     * Add a target
     */
    void addTarget(const std::string& entityId);
    
    /**
     * Remove a target
     */
    void removeTarget(const std::string& entityId);
    
    /**
     * Set active target
     */
    void setActiveTarget(const std::string& entityId);
    
    /**
     * Get active target ID
     */
    const std::string& getActiveTarget() const { return m_activeTargetId; }
    
    /**
     * Check if entity is targeted
     */
    bool isTargeted(const std::string& entityId) const;
    
    /**
     * Set panel position
     */
    void setPosition(float x, float y) { m_posX = x; m_posY = y; }
    
    /**
     * Set icon size
     */
    void setIconSize(float size) { m_iconSize = size; }
    
private:
    void renderTargetIcon(const TargetData& target, float x, float y);
    void renderHealthArc(float centerX, float centerY, float radius, 
                         float startAngle, float endAngle, 
                         float percent, const float color[4]);
    
    std::vector<TargetData> m_targets;
    std::string m_activeTargetId;
    
    float m_posX;
    float m_posY;
    float m_iconSize;
    float m_iconSpacing;
};

} // namespace UI

#endif // EVE_TARGET_LIST_H
