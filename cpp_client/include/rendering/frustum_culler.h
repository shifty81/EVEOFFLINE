#pragma once

#include <glm/glm.hpp>
#include <array>

namespace eve {

/**
 * Represents a single plane in 3D space
 * Used for frustum culling calculations
 */
struct Plane {
    glm::vec3 normal;
    float distance;
    
    Plane() : normal(0.0f, 1.0f, 0.0f), distance(0.0f) {}
    Plane(const glm::vec3& n, float d) : normal(n), distance(d) {}
    
    /**
     * Calculate signed distance from a point to the plane
     * Positive = in front, Negative = behind
     */
    float distanceToPoint(const glm::vec3& point) const {
        return glm::dot(normal, point) + distance;
    }
};

/**
 * View frustum for culling off-screen entities
 * Contains 6 planes: near, far, left, right, top, bottom
 */
class Frustum {
public:
    enum FrustumPlane {
        NEAR = 0,
        FAR = 1,
        LEFT = 2,
        RIGHT = 3,
        TOP = 4,
        BOTTOM = 5
    };
    
    Frustum();
    ~Frustum() = default;
    
    /**
     * Extract frustum planes from view-projection matrix
     * @param viewProjection Combined view and projection matrix
     */
    void extractFromMatrix(const glm::mat4& viewProjection);
    
    /**
     * Test if a point is inside the frustum
     * @param point Point to test
     * @return true if point is inside frustum
     */
    bool containsPoint(const glm::vec3& point) const;
    
    /**
     * Test if a sphere intersects or is inside the frustum
     * @param center Center of the sphere
     * @param radius Radius of the sphere
     * @return true if sphere is visible (intersects or inside)
     */
    bool containsSphere(const glm::vec3& center, float radius) const;
    
    /**
     * Test if an axis-aligned bounding box intersects the frustum
     * @param min Minimum corner of the box
     * @param max Maximum corner of the box
     * @return true if box is visible
     */
    bool containsAABB(const glm::vec3& min, const glm::vec3& max) const;
    
    /**
     * Get a specific frustum plane
     * @param plane Plane to retrieve
     * @return Reference to the plane
     */
    const Plane& getPlane(FrustumPlane plane) const { return m_planes[plane]; }
    
    /**
     * Get all frustum planes
     * @return Array of 6 planes
     */
    const std::array<Plane, 6>& getPlanes() const { return m_planes; }

private:
    std::array<Plane, 6> m_planes;
    
    /**
     * Normalize a plane (ensure normal is unit length)
     * @param plane Plane to normalize
     */
    void normalizePlane(Plane& plane);
};

/**
 * Frustum culling manager for entity visibility
 * Integrates with LODManager for complete visibility control
 */
class FrustumCuller {
public:
    FrustumCuller();
    ~FrustumCuller() = default;
    
    /**
     * Update frustum from camera view-projection matrix
     * @param viewProjection Combined view and projection matrix
     */
    void update(const glm::mat4& viewProjection);
    
    /**
     * Test if an entity is visible
     * @param position Entity position
     * @param boundingRadius Entity bounding sphere radius
     * @return true if entity is visible
     */
    bool isVisible(const glm::vec3& position, float boundingRadius) const;
    
    /**
     * Test if an AABB is visible
     * @param min Minimum corner
     * @param max Maximum corner
     * @return true if AABB is visible
     */
    bool isVisible(const glm::vec3& min, const glm::vec3& max) const;
    
    /**
     * Get the current frustum
     * @return Reference to the frustum
     */
    const Frustum& getFrustum() const { return m_frustum; }
    
    /**
     * Enable or disable frustum culling
     * @param enabled true to enable culling
     */
    void setEnabled(bool enabled) { m_enabled = enabled; }
    
    /**
     * Check if culling is enabled
     * @return true if enabled
     */
    bool isEnabled() const { return m_enabled; }
    
    /**
     * Get statistics
     */
    struct Stats {
        unsigned int totalTests = 0;
        unsigned int visibleEntities = 0;
        unsigned int culledEntities = 0;
        
        void reset() {
            totalTests = 0;
            visibleEntities = 0;
            culledEntities = 0;
        }
        
        float getCullRate() const {
            if (totalTests == 0) return 0.0f;
            return static_cast<float>(culledEntities) / static_cast<float>(totalTests);
        }
    };
    
    const Stats& getStats() const { return m_stats; }
    Stats& getStats() { return m_stats; }
    void resetStats() { m_stats.reset(); }

private:
    Frustum m_frustum;
    bool m_enabled;
    mutable Stats m_stats;  // mutable so we can update stats in const methods
};

} // namespace eve
