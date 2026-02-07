#include "core/entity.h"
#include <algorithm>

namespace eve {

Entity::Entity(const std::string& id)
    : m_id(id)
{
}

void Entity::updateFromSpawn(const glm::vec3& position, const Health& health,
                             const Capacitor& capacitor,
                             const std::string& shipType,
                             const std::string& shipName,
                             const std::string& faction) {
    // Set initial position
    m_position = position;
    m_prevPosition = position;
    m_targetPosition = position;
    
    // Set health
    m_health = health;
    
    // Set capacitor
    m_capacitor = capacitor;
    
    // Set ship info
    m_shipType = shipType;
    m_shipName = shipName;
    m_faction = faction;
    
    // Reset interpolation
    m_interpolationProgress = 1.0f;
    m_needsUpdate = true;
}

void Entity::updateFromState(const glm::vec3& position, const glm::vec3& velocity,
                              float rotation, const Health& health,
                              const Capacitor& capacitor) {
    // Store previous position for interpolation
    m_prevPosition = m_position;
    
    // Update target state
    m_targetPosition = position;
    m_targetVelocity = velocity;
    m_targetRotation = rotation;
    m_health = health;
    m_capacitor = capacitor;
    
    // Reset interpolation progress
    m_interpolationProgress = 0.0f;
    m_needsUpdate = true;
}

void Entity::interpolate(float deltaTime, float interpolationTime) {
    if (m_interpolationProgress >= 1.0f) {
        // Already at target
        return;
    }
    
    // Advance interpolation
    m_interpolationProgress += deltaTime / interpolationTime;
    m_interpolationProgress = std::min(m_interpolationProgress, 1.0f);
    
    // Interpolate position using cubic ease-out for smooth deceleration
    float t = m_interpolationProgress;
    float smoothT = 1.0f - std::pow(1.0f - t, 3.0f);  // Cubic ease-out
    
    m_position = m_prevPosition + (m_targetPosition - m_prevPosition) * smoothT;
    m_velocity = m_targetVelocity;
    m_rotation = m_targetRotation;  // Could interpolate rotation too if needed
}

} // namespace eve
