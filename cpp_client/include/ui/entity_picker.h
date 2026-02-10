#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>

namespace eve {
    class Camera;
    class Entity;
}

namespace UI {

/**
 * EntityPicker - Utility for selecting entities in 3D space via raycasting
 * 
 * Performs mouse picking by:
 * 1. Converting screen coordinates to world space ray
 * 2. Testing ray intersection with entity bounding spheres
 * 3. Returning the closest intersected entity
 */
class EntityPicker {
public:
    EntityPicker();
    ~EntityPicker();
    
    /**
     * Pick entity at screen coordinates
     * @param mouseX Screen X coordinate (pixels)
     * @param mouseY Screen Y coordinate (pixels)
     * @param screenWidth Viewport width
     * @param screenHeight Viewport height
     * @param camera Camera for view/projection matrices
     * @param entities List of entities to test
     * @return Picked entity ID, or empty string if none
     */
    std::string pickEntity(
        double mouseX,
        double mouseY,
        int screenWidth,
        int screenHeight,
        const ::eve::Camera& camera,
        const std::vector<std::shared_ptr<::eve::Entity>>& entities
    );
    
    /**
     * Set picking radius (default: 20.0 for ships, 5.0 for frigates)
     */
    void setPickingRadius(float radius) { m_pickingRadius = radius; }
    
    /**
     * Get last computed ray for debugging
     */
    glm::vec3 getLastRayOrigin() const { return m_lastRayOrigin; }
    glm::vec3 getLastRayDirection() const { return m_lastRayDirection; }
    
private:
    /**
     * Convert screen coordinates to world space ray
     */
    void screenToWorldRay(
        double mouseX,
        double mouseY,
        int screenWidth,
        int screenHeight,
        const glm::mat4& viewMatrix,
        const glm::mat4& projectionMatrix,
        glm::vec3& outRayOrigin,
        glm::vec3& outRayDirection
    );
    
    /**
     * Test ray-sphere intersection
     * @return Distance along ray if hit, or -1.0 if miss
     */
    float raySphereIntersection(
        const glm::vec3& rayOrigin,
        const glm::vec3& rayDirection,
        const glm::vec3& sphereCenter,
        float sphereRadius
    );
    
    float m_pickingRadius;
    
    // Debug info
    glm::vec3 m_lastRayOrigin;
    glm::vec3 m_lastRayDirection;
};

} // namespace UI
