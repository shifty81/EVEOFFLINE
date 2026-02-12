#include "rendering/frustum_culler.h"
#include <glm/gtc/type_ptr.hpp>
#include <cmath>
#include <algorithm>

namespace atlas {

// ============================================================================
// Frustum Implementation
// ============================================================================

Frustum::Frustum() {
    // Initialize with default planes
    m_planes[NEAR] = Plane(glm::vec3(0, 0, -1), -1.0f);
    m_planes[FAR] = Plane(glm::vec3(0, 0, 1), -1000.0f);
    m_planes[LEFT] = Plane(glm::vec3(1, 0, 0), -100.0f);
    m_planes[RIGHT] = Plane(glm::vec3(-1, 0, 0), -100.0f);
    m_planes[TOP] = Plane(glm::vec3(0, -1, 0), -100.0f);
    m_planes[BOTTOM] = Plane(glm::vec3(0, 1, 0), -100.0f);
}

void Frustum::normalizePlane(Plane& plane) {
    float length = glm::length(plane.normal);
    if (length > 0.0f) {
        plane.normal /= length;
        plane.distance /= length;
    }
}

void Frustum::extractFromMatrix(const glm::mat4& vp) {
    // Extract frustum planes using Gribb/Hartmann method
    // This extracts planes directly from the view-projection matrix
    
    // Left plane: vp[3] + vp[0]
    m_planes[LEFT].normal.x = vp[0][3] + vp[0][0];
    m_planes[LEFT].normal.y = vp[1][3] + vp[1][0];
    m_planes[LEFT].normal.z = vp[2][3] + vp[2][0];
    m_planes[LEFT].distance = vp[3][3] + vp[3][0];
    normalizePlane(m_planes[LEFT]);
    
    // Right plane: vp[3] - vp[0]
    m_planes[RIGHT].normal.x = vp[0][3] - vp[0][0];
    m_planes[RIGHT].normal.y = vp[1][3] - vp[1][0];
    m_planes[RIGHT].normal.z = vp[2][3] - vp[2][0];
    m_planes[RIGHT].distance = vp[3][3] - vp[3][0];
    normalizePlane(m_planes[RIGHT]);
    
    // Bottom plane: vp[3] + vp[1]
    m_planes[BOTTOM].normal.x = vp[0][3] + vp[0][1];
    m_planes[BOTTOM].normal.y = vp[1][3] + vp[1][1];
    m_planes[BOTTOM].normal.z = vp[2][3] + vp[2][1];
    m_planes[BOTTOM].distance = vp[3][3] + vp[3][1];
    normalizePlane(m_planes[BOTTOM]);
    
    // Top plane: vp[3] - vp[1]
    m_planes[TOP].normal.x = vp[0][3] - vp[0][1];
    m_planes[TOP].normal.y = vp[1][3] - vp[1][1];
    m_planes[TOP].normal.z = vp[2][3] - vp[2][1];
    m_planes[TOP].distance = vp[3][3] - vp[3][1];
    normalizePlane(m_planes[TOP]);
    
    // Near plane: vp[3] + vp[2]
    m_planes[NEAR].normal.x = vp[0][3] + vp[0][2];
    m_planes[NEAR].normal.y = vp[1][3] + vp[1][2];
    m_planes[NEAR].normal.z = vp[2][3] + vp[2][2];
    m_planes[NEAR].distance = vp[3][3] + vp[3][2];
    normalizePlane(m_planes[NEAR]);
    
    // Far plane: vp[3] - vp[2]
    m_planes[FAR].normal.x = vp[0][3] - vp[0][2];
    m_planes[FAR].normal.y = vp[1][3] - vp[1][2];
    m_planes[FAR].normal.z = vp[2][3] - vp[2][2];
    m_planes[FAR].distance = vp[3][3] - vp[3][2];
    normalizePlane(m_planes[FAR]);
}

bool Frustum::containsPoint(const glm::vec3& point) const {
    // Point is inside if it's in front of all planes
    for (const auto& plane : m_planes) {
        if (plane.distanceToPoint(point) < 0.0f) {
            return false;
        }
    }
    return true;
}

bool Frustum::containsSphere(const glm::vec3& center, float radius) const {
    // Sphere is visible if it's not completely behind any plane
    for (const auto& plane : m_planes) {
        float distance = plane.distanceToPoint(center);
        if (distance < -radius) {
            // Sphere is completely behind this plane
            return false;
        }
    }
    return true;
}

bool Frustum::containsAABB(const glm::vec3& min, const glm::vec3& max) const {
    // Test all 8 corners of the AABB
    // If any corner is inside, the box is visible
    // We use a more efficient method: check if box is completely outside any plane
    
    for (const auto& plane : m_planes) {
        // Find the positive vertex (the one furthest in the normal direction)
        glm::vec3 positive;
        positive.x = (plane.normal.x >= 0.0f) ? max.x : min.x;
        positive.y = (plane.normal.y >= 0.0f) ? max.y : min.y;
        positive.z = (plane.normal.z >= 0.0f) ? max.z : min.z;
        
        // If the positive vertex is behind the plane, the whole box is outside
        if (plane.distanceToPoint(positive) < 0.0f) {
            return false;
        }
    }
    return true;
}

// ============================================================================
// FrustumCuller Implementation
// ============================================================================

FrustumCuller::FrustumCuller()
    : m_enabled(true)
{
    m_stats.reset();
}

void FrustumCuller::update(const glm::mat4& viewProjection) {
    m_frustum.extractFromMatrix(viewProjection);
}

bool FrustumCuller::isVisible(const glm::vec3& position, float boundingRadius) const {
    m_stats.totalTests++;
    
    // If culling is disabled, everything is visible
    if (!m_enabled) {
        m_stats.visibleEntities++;
        return true;
    }
    
    // Test sphere against frustum
    bool visible = m_frustum.containsSphere(position, boundingRadius);
    
    if (visible) {
        m_stats.visibleEntities++;
    } else {
        m_stats.culledEntities++;
    }
    
    return visible;
}

bool FrustumCuller::isVisible(const glm::vec3& min, const glm::vec3& max) const {
    m_stats.totalTests++;
    
    // If culling is disabled, everything is visible
    if (!m_enabled) {
        m_stats.visibleEntities++;
        return true;
    }
    
    // Test AABB against frustum
    bool visible = m_frustum.containsAABB(min, max);
    
    if (visible) {
        m_stats.visibleEntities++;
    } else {
        m_stats.culledEntities++;
    }
    
    return visible;
}

} // namespace atlas
