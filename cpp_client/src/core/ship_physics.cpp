#include "core/ship_physics.h"
#include <algorithm>
#include <cmath>

namespace eve {

ShipPhysics::ShipPhysics()
    : m_position(0.0f)
    , m_velocity(0.0f)
    , m_desiredDirection(0.0f, 0.0f, 1.0f)
    , m_heading(0.0f, 0.0f, 1.0f)
    , m_navMode(NavigationMode::MANUAL)
    , m_navTarget(0.0f)
    , m_navRange(0.0f)
    , m_warpPhase(WarpPhase::NONE)
    , m_warpProgress(0.0f)
    , m_warpDistanceTotal(0.0f)
    , m_warpDistanceTraveled(0.0f)
    , m_currentWarpSpeedAU(0.0f)
    , m_baseWarpSpeedAU(5.0f)
    , m_warpPhaseTimer(0.0f)
    , m_warpStartPos(0.0f)
    , m_warpDirection(0.0f)
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

void ShipPhysics::alignTo(const glm::vec3& destination) {
    glm::vec3 toTarget = destination - m_position;
    float distance = glm::length(toTarget);
    if (distance < 0.001f) return;

    m_navMode = NavigationMode::ALIGN_TO;
    m_navTarget = destination;
    m_desiredDirection = glm::normalize(toTarget);
}

void ShipPhysics::warpTo(const glm::vec3& destination) {
    glm::vec3 toTarget = destination - m_position;
    float distance = glm::length(toTarget);
    
    // Must be at least 150km to warp
    if (distance < MIN_WARP_DISTANCE) {
        // Too close to warp, just approach instead
        approach(destination, 0.0f);
        return;
    }
    
    m_navMode = NavigationMode::WARPING;
    m_navTarget = destination;
    m_desiredDirection = glm::normalize(toTarget);
    
    // Initialize warp state — start with alignment phase
    m_warpPhase = WarpPhase::ALIGNING;
    m_warpProgress = 0.0f;
    m_warpDistanceTotal = distance;
    m_warpDistanceTraveled = 0.0f;
    m_currentWarpSpeedAU = 0.0f;
    m_warpPhaseTimer = 0.0f;
    m_warpStartPos = m_position;
    m_warpDirection = glm::normalize(toTarget);
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
        
        case NavigationMode::ALIGN_TO: {
            // Align to destination: accelerate toward it to 75% max velocity
            // This prepares the ship for warp without actually warping
            glm::vec3 toTarget = m_navTarget - m_position;
            float distance = glm::length(toTarget);
            if (distance > 0.001f) {
                m_desiredDirection = glm::normalize(toTarget);
            }
            break;
        }
        
        case NavigationMode::WARPING: {
            // Warp is handled by updateWarp() — don't use normal physics
            updateWarp(deltaTime);
            // Update heading to match warp direction
            if (m_warpPhase != WarpPhase::NONE && glm::length(m_warpDirection) > 0.001f) {
                m_heading = m_warpDirection;
            }
            return;  // Skip normal acceleration/friction during warp
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
    
    // Update heading direction (ship gradually turns toward velocity)
    float speed = glm::length(m_velocity);
    if (speed > 1.0f) {
        glm::vec3 velDir = m_velocity / speed;
        float turnRate = 2.0f * deltaTime;  // Smooth turn
        m_heading = glm::normalize(glm::mix(m_heading, velDir, std::min(turnRate, 1.0f)));
    }
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

void ShipPhysics::updateWarp(float deltaTime) {
    m_warpPhaseTimer += deltaTime;
    
    switch (m_warpPhase) {
        case WarpPhase::ALIGNING: {
            // Phase 1: Align and accelerate to 75% max subwarp speed
            // Normal acceleration happens in this phase
            updateAcceleration(deltaTime);
            m_position += m_velocity * deltaTime;
            
            if (isAlignedForWarp()) {
                // Aligned! Enter warp acceleration phase
                m_warpPhase = WarpPhase::ACCELERATING;
                m_warpPhaseTimer = 0.0f;
                m_warpStartPos = m_position;
                m_warpDistanceTotal = glm::length(m_navTarget - m_position);
                m_warpDistanceTraveled = 0.0f;
            }
            break;
        }
        
        case WarpPhase::ACCELERATING: {
            // Phase 2: Accelerate from subwarp to max warp speed
            // Covers first ~33% of warp distance, or ~1 AU equivalent
            float accelDuration = 3.0f;  // ~3 seconds to reach max warp
            float t = std::min(m_warpPhaseTimer / accelDuration, 1.0f);
            
            // Logarithmic acceleration curve
            float speedFraction = t * t;  // Smooth ramp up
            m_currentWarpSpeedAU = m_baseWarpSpeedAU * speedFraction;
            
            // Convert AU/s to m/s for position update
            float warpSpeedMeters = m_currentWarpSpeedAU * AU_IN_METERS;
            float distanceThisFrame = warpSpeedMeters * deltaTime;
            m_warpDistanceTraveled += distanceThisFrame;
            
            // Move along warp direction
            m_position = m_warpStartPos + m_warpDirection * m_warpDistanceTraveled;
            m_velocity = m_warpDirection * warpSpeedMeters;
            
            // Update progress
            m_warpProgress = m_warpDistanceTraveled / m_warpDistanceTotal;
            
            // Transition to cruise when at max speed or past 33% distance
            if (t >= 1.0f || m_warpProgress >= 0.33f) {
                m_warpPhase = WarpPhase::CRUISING;
                m_warpPhaseTimer = 0.0f;
                m_currentWarpSpeedAU = m_baseWarpSpeedAU;
            }
            break;
        }
        
        case WarpPhase::CRUISING: {
            // Phase 3: Travel at max warp speed (warp tunnel effect)
            m_currentWarpSpeedAU = m_baseWarpSpeedAU;
            float warpSpeedMeters = m_currentWarpSpeedAU * AU_IN_METERS;
            float distanceThisFrame = warpSpeedMeters * deltaTime;
            m_warpDistanceTraveled += distanceThisFrame;
            
            m_position = m_warpStartPos + m_warpDirection * m_warpDistanceTraveled;
            m_velocity = m_warpDirection * warpSpeedMeters;
            
            m_warpProgress = m_warpDistanceTraveled / m_warpDistanceTotal;
            
            // Begin deceleration at ~67% of total distance
            if (m_warpProgress >= 0.67f) {
                m_warpPhase = WarpPhase::DECELERATING;
                m_warpPhaseTimer = 0.0f;
            }
            break;
        }
        
        case WarpPhase::DECELERATING: {
            // Phase 4: Decelerate from warp speed to subwarp
            float decelDuration = 3.0f;  // ~3 seconds to decelerate
            float t = std::min(m_warpPhaseTimer / decelDuration, 1.0f);
            
            // Smooth deceleration curve
            float speedFraction = 1.0f - (t * t);
            m_currentWarpSpeedAU = m_baseWarpSpeedAU * std::max(speedFraction, 0.01f);
            
            float warpSpeedMeters = m_currentWarpSpeedAU * AU_IN_METERS;
            float distanceThisFrame = warpSpeedMeters * deltaTime;
            m_warpDistanceTraveled += distanceThisFrame;
            
            m_position = m_warpStartPos + m_warpDirection * m_warpDistanceTraveled;
            m_velocity = m_warpDirection * warpSpeedMeters;
            
            m_warpProgress = m_warpDistanceTraveled / m_warpDistanceTotal;
            
            // Check if arrived (within 2500m or exceeded total distance)
            float remainingDistance = m_warpDistanceTotal - m_warpDistanceTraveled;
            if (remainingDistance <= WARP_EXIT_DISTANCE || t >= 1.0f) {
                // Exit warp — land offset behind destination (half of exit distance)
                m_position = m_navTarget - m_warpDirection * (WARP_EXIT_DISTANCE * 0.5f);
                m_velocity = m_warpDirection * m_stats.maxVelocity * WARP_EXIT_SPEED_FRACTION;
                m_warpPhase = WarpPhase::NONE;
                m_currentWarpSpeedAU = 0.0f;
                m_warpProgress = 1.0f;
                m_navMode = NavigationMode::STOPPED;
                m_desiredDirection = glm::vec3(0.0f);
            }
            break;
        }
        
        case WarpPhase::NONE:
        default:
            break;
    }
}

float ShipPhysics::getEngineThrottle() const {
    if (m_navMode == NavigationMode::STOPPED) {
        return 0.0f;
    }
    
    if (m_warpPhase == WarpPhase::CRUISING || m_warpPhase == WarpPhase::ACCELERATING) {
        return 1.0f;  // Full thrust during warp
    }
    
    if (m_warpPhase == WarpPhase::DECELERATING) {
        return 0.3f;  // Reduced during decel
    }
    
    // Normal flight: throttle based on speed vs max
    float speed = glm::length(m_velocity);
    if (m_stats.maxVelocity > 0.0f) {
        return std::min(speed / m_stats.maxVelocity, 1.0f);
    }
    return 0.0f;
}

bool ShipPhysics::isWarpPathBlocked(const glm::vec3& from, const glm::vec3& to,
                                     const std::vector<CelestialCollisionZone>& zones) const {
    glm::vec3 warpDir = to - from;
    float warpLength = glm::length(warpDir);
    if (warpLength < 0.001f) return false;

    glm::vec3 warpDirNorm = warpDir / warpLength;

    for (const auto& zone : zones) {
        // Ray-sphere intersection test
        // Check if the warp line segment passes within collisionRadius of the zone center
        glm::vec3 oc = from - zone.position;
        float b = glm::dot(oc, warpDirNorm);
        float c = glm::dot(oc, oc) - zone.collisionRadius * zone.collisionRadius;

        float discriminant = b * b - c;
        if (discriminant < 0.0f) continue;  // No intersection

        float sqrtDisc = std::sqrt(discriminant);
        float t1 = -b - sqrtDisc;
        float t2 = -b + sqrtDisc;

        // Check if intersection is within the warp segment [0, warpLength]
        if (t1 <= warpLength && t2 >= 0.0f) {
            return true;  // Warp path is blocked
        }
    }
    return false;
}

bool ShipPhysics::isInsideCollisionZone(const std::vector<CelestialCollisionZone>& zones) const {
    for (const auto& zone : zones) {
        float dist = glm::length(m_position - zone.position);
        if (dist < zone.collisionRadius) {
            return true;
        }
    }
    return false;
}

glm::vec3 ShipPhysics::resolveCollision(const std::vector<CelestialCollisionZone>& zones) {
    for (const auto& zone : zones) {
        glm::vec3 toShip = m_position - zone.position;
        float dist = glm::length(toShip);
        if (dist < zone.collisionRadius) {
            // Push ship to the edge of the collision zone
            glm::vec3 pushDir;
            if (dist > 0.001f) {
                pushDir = toShip / dist;
            } else {
                // Ship is exactly at center — push upward (arbitrary but deterministic)
                pushDir = glm::vec3(0.0f, 1.0f, 0.0f);
            }
            m_position = zone.position + pushDir * (zone.collisionRadius + COLLISION_PUSH_MARGIN);

            // Kill velocity toward the celestial (bounce effect)
            float velToward = glm::dot(m_velocity, -pushDir);
            if (velToward > 0.0f) {
                m_velocity += pushDir * velToward;
            }
            return m_position;
        }
    }
    return m_position;
}

} // namespace eve
