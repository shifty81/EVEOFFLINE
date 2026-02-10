#pragma once

#include <glm/glm.hpp>
#include <vector>

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
    void alignTo(const glm::vec3& destination);
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

    /**
     * Warp phase enum for proper EVE-style warp (4 phases)
     */
    enum class WarpPhase {
        NONE,           // Not warping
        ALIGNING,       // Turning and accelerating to 75% max subwarp speed
        ACCELERATING,   // Accelerating from subwarp to max warp speed
        CRUISING,       // Traveling at max warp speed (warp tunnel)
        DECELERATING    // Slowing from warp speed back to subwarp
    };

    /**
     * Get current warp phase
     */
    WarpPhase getWarpPhase() const { return m_warpPhase; }

    /**
     * Get warp progress (0.0 = start, 1.0 = arrived)
     */
    float getWarpProgress() const { return m_warpProgress; }

    /**
     * Get current warp speed in AU/s (only meaningful during warp)
     */
    float getWarpSpeedAU() const { return m_currentWarpSpeedAU; }

    /**
     * Set base warp speed for ship class (AU/s)
     */
    void setWarpSpeed(float auPerSecond) { m_baseWarpSpeedAU = auPerSecond; }

    /**
     * Check if ship is in any warp phase (including aligning)
     */
    bool isWarping() const { return m_warpPhase != WarpPhase::NONE; }

    /**
     * Get the engine throttle level (0.0-1.0) for visual trail intensity
     */
    float getEngineThrottle() const;

    /**
     * Get heading direction (normalized direction the ship is facing)
     */
    glm::vec3 getHeading() const { return m_heading; }

    /**
     * Celestial collision zone info for warp path checking.
     * Represents a sphere that the ship cannot warp through or into.
     */
    struct CelestialCollisionZone {
        glm::vec3 position;
        float radius;           // Physical radius of celestial
        float collisionRadius;  // Collision zone radius (larger than physical)
    };

    /**
     * Check if a warp path intersects any celestial collision zone.
     * Returns true if the warp is blocked (ship would pass through a celestial).
     */
    bool isWarpPathBlocked(const glm::vec3& from, const glm::vec3& to,
                           const std::vector<CelestialCollisionZone>& zones) const;

    /**
     * Check if the ship is currently inside a celestial collision zone.
     * Ships inside a collision zone cannot initiate warp ("bouncing").
     */
    bool isInsideCollisionZone(const std::vector<CelestialCollisionZone>& zones) const;

    /**
     * Push ship out of a collision zone if it has drifted inside one.
     * Returns the corrected position, or current position if not inside any zone.
     */
    glm::vec3 resolveCollision(const std::vector<CelestialCollisionZone>& zones);

    /**
     * Set position directly (used for collision resolution)
     */
    void setPosition(const glm::vec3& pos) { m_position = pos; }

private:
    void updateAcceleration(float deltaTime);
    void updateOrbit(float deltaTime);
    void applySpaceFriction(float deltaTime);
    void updateWarp(float deltaTime);
    
    // Ship stats
    ShipStats m_stats;
    
    // Current state
    glm::vec3 m_position;
    glm::vec3 m_velocity;
    glm::vec3 m_desiredDirection;
    glm::vec3 m_heading;  // Ship facing direction (visual)
    
    // Navigation state
    enum class NavigationMode {
        MANUAL,
        APPROACH,
        ORBIT,
        KEEP_AT_RANGE,
        ALIGN_TO,
        WARPING,
        STOPPED
    };
    
    NavigationMode m_navMode;
    glm::vec3 m_navTarget;
    float m_navRange;
    
    // Warp state (EVE-style 4-phase warp)
    WarpPhase m_warpPhase;
    float m_warpProgress;           // 0.0 to 1.0
    float m_warpDistanceTotal;      // Total warp distance in meters
    float m_warpDistanceTraveled;   // Distance covered so far
    float m_currentWarpSpeedAU;     // Current warp speed in AU/s
    float m_baseWarpSpeedAU;        // Ship class warp speed (e.g. 5.0 for frigate)
    float m_warpPhaseTimer;         // Timer within current phase
    glm::vec3 m_warpStartPos;       // Position where warp began
    glm::vec3 m_warpDirection;      // Normalized warp direction
    
    // Propulsion bonus
    bool m_propulsionActive;
    float m_propulsionMultiplier;
    
    // Constants
    static constexpr float SPACE_FRICTION = 0.5f;  // Simulated space friction
    static constexpr float WARP_ALIGN_THRESHOLD = 0.75f;  // 75% of max velocity
    static constexpr float ACCELERATION_CONSTANT = 500000.0f;  // EVE's constant
    static constexpr float AU_IN_METERS = 149597870700.0f;  // 1 AU in meters
    static constexpr float MIN_WARP_DISTANCE = 150000.0f;  // Minimum 150km to warp
    static constexpr float WARP_EXIT_DISTANCE = 2500.0f;   // Land within 2500m of target
    static constexpr float WARP_EXIT_SPEED_FRACTION = 0.25f;  // Exit warp at 25% max subwarp speed
};

} // namespace eve
