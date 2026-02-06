#include "core/ship_physics.h"
#include <algorithm>
#include <cmath>

namespace eve {

ShipPhysics::ShipPhysics()
    : m_position(0.0f)
    , m_velocity(0.0f)
    , m_desiredDirection(0.0f, 0.0f, 1.0f)
    , m_navMode(NavigationMode::MANUAL)
    , m_navTarget(0.0f)
    , m_navRange(0.0f)
    , m_propulsionActive(false)
    , m_propulsionMultiplier(1.0f)
{
    // Default frigate stats
    m_stats.mass = 1200000.0f;           // 1.2 million kg
    m_stats.inertiaModifier = 3.2f;
    m_stats.maxVelocity = 400.0f;        // 400 m/s
    m_stats.signatureRadius = 35.0f;     // 35m signature
}

void ShipPhysics::setShipStats(const ShipStats& stats) {
    m_stats = stats;
}

void ShipPhysics::setDesiredDirection(const glm::vec3& direction) {
    if (glm::length(direction) > 0.001f) {
        m_desiredDirection = glm::normalize(direction);
    }
    m_navMode = NavigationMode::MANUAL;
}

void ShipPhysics::approach(const glm::vec3& target, float approachRange) {
    m_navMode = NavigationMode::APPROACH;
    m_navTarget = target;
    m_navRange = approachRange;
}

void ShipPhysics::orbit(const glm::vec3& target, float orbitRange) {
    m_navMode = NavigationMode::ORBIT;
    m_navTarget = target;
    m_navRange = orbitRange;
}

void ShipPhysics::keepAtRange(const glm::vec3& target, float range) {
    m_navMode = NavigationMode::KEEP_AT_RANGE;
    m_navTarget = target;
    m_navRange = range;
}

void ShipPhysics::warpTo(const glm::vec3& destination) {
    m_navMode = NavigationMode::WARPING;
    m_navTarget = destination;
    m_desiredDirection = glm::normalize(destination - m_position);
}

void ShipPhysics::stop() {
    m_navMode = NavigationMode::STOPPED;
    m_desiredDirection = glm::vec3(0.0f);
}

void ShipPhysics::update(float deltaTime) {
    // Update navigation behavior
    switch (m_navMode) {
        case NavigationMode::APPROACH: {
            glm::vec3 toTarget = m_navTarget - m_position;
            float distance = glm::length(toTarget);
            
            if (distance > m_navRange + 10.0f) {
                m_desiredDirection = glm::normalize(toTarget);
            } else {
                m_navMode = NavigationMode::STOPPED;
                m_desiredDirection = glm::vec3(0.0f);
            }
            break;
        }
        
        case NavigationMode::ORBIT: {
            updateOrbit(deltaTime);
            break;
        }
        
        case NavigationMode::KEEP_AT_RANGE: {
            glm::vec3 toTarget = m_navTarget - m_position;
            float distance = glm::length(toTarget);
            float error = distance - m_navRange;
            
            if (std::fabs(error) > 50.0f) {
                if (error > 0) {
                    // Too far, move closer
                    m_desiredDirection = glm::normalize(toTarget);
                } else {
                    // Too close, move away
                    m_desiredDirection = -glm::normalize(toTarget);
                }
            } else {
                m_desiredDirection = glm::vec3(0.0f);
            }
            break;
        }
        
        case NavigationMode::WARPING: {
            // Simplified warp: check if aligned, then instant jump
            if (isAlignedForWarp()) {
                m_position = m_navTarget;
                m_velocity = glm::vec3(0.0f);
                m_navMode = NavigationMode::STOPPED;
            }
            break;
        }
        
        case NavigationMode::STOPPED: {
            m_desiredDirection = glm::vec3(0.0f);
            break;
        }
        
        case NavigationMode::MANUAL:
        default:
            // Manual control, direction already set
            break;
    }
    
    // Update acceleration and velocity
    updateAcceleration(deltaTime);
    
    // Apply space friction (ships slow down without thrust)
    applySpaceFriction(deltaTime);
    
    // Update position
    m_position += m_velocity * deltaTime;
}

void ShipPhysics::updateAcceleration(float deltaTime) {
    if (glm::length(m_desiredDirection) < 0.001f) {
        // No desired direction, ship will decelerate naturally
        return;
    }
    
    // EVE Online uses exponential acceleration
    // Formula: v(t) = v_max * (1 - e^(-t * k))
    // where k = 1 / (agility / acceleration_constant)
    
    float effectiveMaxVel = m_stats.maxVelocity;
    if (m_propulsionActive) {
        effectiveMaxVel *= m_propulsionMultiplier;
    }
    
    // Current speed in desired direction
    float currentSpeedInDirection = glm::dot(m_velocity, m_desiredDirection);
    
    // Target velocity
    glm::vec3 targetVelocity = m_desiredDirection * effectiveMaxVel;
    
    // Calculate acceleration factor based on agility
    float agility = m_stats.getAgility();
    float k = ACCELERATION_CONSTANT / agility;
    
    // Exponential approach to target velocity
    // The ship accelerates quickly at first, then more slowly
    float accelerationFactor = 1.0f - exp(-k * deltaTime);
    
    // Interpolate current velocity toward target velocity
    m_velocity = m_velocity + (targetVelocity - m_velocity) * accelerationFactor;
    
    // Clamp to max velocity
    float currentSpeed = glm::length(m_velocity);
    if (currentSpeed > effectiveMaxVel) {
        m_velocity = glm::normalize(m_velocity) * effectiveMaxVel;
    }
}

void ShipPhysics::updateOrbit(float deltaTime) {
    glm::vec3 toTarget = m_navTarget - m_position;
    float distance = glm::length(toTarget);
    
    if (distance < 0.1f) {
        m_desiredDirection = glm::vec3(0.0f);
        return;
    }
    
    glm::vec3 toTargetNorm = toTarget / distance;
    
    // Calculate orbital velocity direction (perpendicular to radial)
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 tangent = glm::normalize(glm::cross(toTargetNorm, up));
    
    // If orbit range is larger than current distance, move toward target
    // If orbit range is smaller, move away while maintaining tangential component
    float error = distance - m_navRange;
    
    if (std::fabs(error) > 10.0f) {
        // Need to adjust radius
        float radialComponent = error / distance;
        float tangentialComponent = sqrt(1.0f - radialComponent * radialComponent);
        
        m_desiredDirection = -toTargetNorm * radialComponent + tangent * tangentialComponent;
        m_desiredDirection = glm::normalize(m_desiredDirection);
    } else {
        // At correct range, pure tangential movement
        m_desiredDirection = tangent;
    }
}

void ShipPhysics::applySpaceFriction(float deltaTime) {
    // In EVE, ships experience "space friction" - they slow down without thrust
    // This is NOT realistic physics but makes gameplay better
    
    if (m_navMode == NavigationMode::STOPPED || glm::length(m_desiredDirection) < 0.001f) {
        // Apply stronger friction when actively stopped
        float frictionFactor = exp(-SPACE_FRICTION * 2.0f * deltaTime);
        m_velocity *= frictionFactor;
        
        // Full stop if velocity is very low
        if (glm::length(m_velocity) < 0.1f) {
            m_velocity = glm::vec3(0.0f);
        }
    } else {
        // Light friction to prevent infinite acceleration
        float frictionFactor = exp(-SPACE_FRICTION * 0.1f * deltaTime);
        
        // Only apply friction to velocity components perpendicular to desired direction
        glm::vec3 parallelVel = m_desiredDirection * glm::dot(m_velocity, m_desiredDirection);
        glm::vec3 perpVel = m_velocity - parallelVel;
        
        m_velocity = parallelVel + perpVel * frictionFactor;
    }
}

bool ShipPhysics::isAlignedForWarp() const {
    if (glm::length(m_desiredDirection) < 0.001f) {
        return false;
    }
    
    float currentSpeed = glm::length(m_velocity);
    float speedInDirection = glm::dot(m_velocity, m_desiredDirection);
    
    // Must be at 75% of max velocity in the desired direction
    return speedInDirection >= (m_stats.maxVelocity * WARP_ALIGN_THRESHOLD);
}

float ShipPhysics::getTimeToAlign() const {
    if (isAlignedForWarp()) {
        return 0.0f;
    }
    
    // Approximate time to align based on current velocity and agility
    float currentSpeedInDirection = glm::dot(m_velocity, m_desiredDirection);
    float targetSpeed = m_stats.maxVelocity * WARP_ALIGN_THRESHOLD;
    float speedDelta = targetSpeed - currentSpeedInDirection;
    
    if (speedDelta <= 0.0f) {
        return 0.0f;
    }
    
    // Use exponential formula: t = -ln((v_max - v) / v_max) * (agility / k)
    float agility = m_stats.getAgility();
    float k = ACCELERATION_CONSTANT / agility;
    
    float ratio = speedDelta / m_stats.maxVelocity;
    return -log(ratio) / k;
}

void ShipPhysics::applyPropulsionBonus(float velocityMultiplier) {
    m_propulsionActive = true;
    m_propulsionMultiplier = velocityMultiplier;
}

void ShipPhysics::removePropulsionBonus() {
    m_propulsionActive = false;
    m_propulsionMultiplier = 1.0f;
    
    // Cap velocity if over natural max
    float currentSpeed = glm::length(m_velocity);
    if (currentSpeed > m_stats.maxVelocity) {
        m_velocity = glm::normalize(m_velocity) * m_stats.maxVelocity;
    }
}

} // namespace eve
