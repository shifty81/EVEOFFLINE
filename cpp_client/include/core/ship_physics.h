#pragma once

#include <glm/glm.hpp>

namespace eve {

/**
 * Ship Physics System
 * 
 * Implements EVE Online-style ship movement:
 * - Exponential acceleration toward max velocity
 * - Mass and inertia-based agility
 * - Align time mechanics
 * - No true Newtonian physics (space has "friction")
 * - Ships decelerate when engines off
 */
class ShipPhysics {
public:
    struct ShipStats {
        float mass;                 // Ship mass in kg
        float inertiaModifier;      // Inertia modifier (lower = more agile)
        float maxVelocity;          // Maximum velocity in m/s
        float signatureRadius;      // Ship size in meters
        
        // Calculated properties
        float getAgility() const { return mass * inertiaModifier; }
        
        // Align time: time to reach 75% max velocity
        float getAlignTime() const {
            return -log(0.25) * getAgility() / 1000000.0f;
        }
    };

    ShipPhysics();
    
    /**
     * Set ship statistics
     */
    void setShipStats(const ShipStats& stats);
    const ShipStats& getShipStats() const { return m_stats; }
    
    /**
     * Get current state
     */
    glm::vec3 getPosition() const { return m_position; }
    glm::vec3 getVelocity() const { return m_velocity; }
    float getCurrentSpeed() const { return glm::length(m_velocity); }
    float getSpeedPercentage() const { return getCurrentSpeed() / m_stats.maxVelocity; }
    
    /**
     * Set desired direction (normalized)
     * Ship will accelerate in this direction
     */
    void setDesiredDirection(const glm::vec3& direction);
    glm::vec3 getDesiredDirection() const { return m_desiredDirection; }
    
    /**
     * Navigation commands
     */
    void approach(const glm::vec3& target, float approachRange = 0.0f);
    void orbit(const glm::vec3& target, float orbitRange);
    void keepAtRange(const glm::vec3& target, float range);
    void warpTo(const glm::vec3& destination);
    void stop();
    
    /**
     * Update physics simulation
     * @param deltaTime Time step in seconds
     */
    void update(float deltaTime);
    
    /**
     * Check if ship is aligned for warp (75% max velocity in desired direction)
     */
    bool isAlignedForWarp() const;
    
    /**
     * Get time remaining to align for warp
     */
    float getTimeToAlign() const;
    
    /**
     * Apply propulsion module effects (afterburner, microwarpdrive)
     */
    void applyPropulsionBonus(float velocityMultiplier);
    void removePropulsionBonus();
    
private:
    void updateAcceleration(float deltaTime);
    void updateOrbit(float deltaTime);
    void applySpaceFriction(float deltaTime);
    
    // Ship stats
    ShipStats m_stats;
    
    // Current state
    glm::vec3 m_position;
    glm::vec3 m_velocity;
    glm::vec3 m_desiredDirection;
    
    // Navigation state
    enum class NavigationMode {
        MANUAL,
        APPROACH,
        ORBIT,
        KEEP_AT_RANGE,
        WARPING,
        STOPPED
    };
    
    NavigationMode m_navMode;
    glm::vec3 m_navTarget;
    float m_navRange;
    
    // Propulsion bonus
    bool m_propulsionActive;
    float m_propulsionMultiplier;
    
    // Constants
    static constexpr float SPACE_FRICTION = 0.5f;  // Simulated space friction
    static constexpr float WARP_ALIGN_THRESHOLD = 0.75f;  // 75% of max velocity
    static constexpr float ACCELERATION_CONSTANT = 500000.0f;  // EVE's constant
};

} // namespace eve
