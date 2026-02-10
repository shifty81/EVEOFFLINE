#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <functional>

namespace eve {

class ShipPhysics;

/**
 * Represents a celestial object in a solar system that can be warped to.
 * 
 * In EVE Online, each system has a sun at the origin and static celestials
 * (planets, moons, stations, stargates, asteroid belts) at fixed positions.
 * All are valid warp destinations. The sun casts directional light on the
 * entire system.
 */
struct Celestial {
    enum class Type {
        SUN,
        PLANET,
        MOON,
        STATION,
        STARGATE,
        ASTEROID_BELT,
        WORMHOLE
    };

    std::string id;
    std::string name;
    Type type;
    glm::vec3 position;          // World position in meters
    float radius;                // Object radius in meters
    float distanceFromSun_AU;    // Distance from sun in AU (for display)
    glm::vec3 lightColor;        // For sun: emitted light color
    float lightIntensity;        // For sun: light intensity
    std::string linkedSystem;    // For stargates: destination system ID
    std::vector<std::string> services;  // For stations: available services

    Celestial()
        : type(Type::PLANET)
        , position(0.0f)
        , radius(1000.0f)
        , distanceFromSun_AU(0.0f)
        , lightColor(1.0f)
        , lightIntensity(0.0f)
        , linkedSystem("")
    {}
};

/**
 * SolarSystemScene — Manages the layout and state of a single solar system.
 *
 * Responsibilities:
 *  - Holds all celestial objects (sun, planets, stations, gates, belts)
 *  - Sun position determines directional light for the system
 *  - Provides warp destination list for the overview/right-click menus
 *  - Tracks player ship position and warp state within the system
 *  - Manages engine trail particle emission based on ship throttle
 *
 * Scale: 1 AU = 149,597,870,700 meters. Positions use game-scale meters
 * where 1 game unit = 1 meter for sub-warp distances, and AU for warp.
 */
class SolarSystemScene {
public:
    SolarSystemScene();
    ~SolarSystemScene() = default;

    /**
     * Initialize a system from data (system ID, name, security, etc.)
     */
    void initialize(const std::string& systemId, const std::string& systemName,
                    float securityLevel);

    /**
     * Load a predefined test system with full celestial layout.
     * Creates: Sun, 3 planets, 2 asteroid belts, 1 station, 1 stargate.
     */
    void loadTestSystem();

    /**
     * Add a celestial object to the system.
     */
    void addCelestial(const Celestial& celestial);

    /**
     * Get all celestials in the system.
     */
    const std::vector<Celestial>& getCelestials() const { return m_celestials; }

    /**
     * Find a celestial by ID.
     */
    const Celestial* findCelestial(const std::string& id) const;

    /**
     * Get the sun (first SUN-type celestial, or null if none).
     */
    const Celestial* getSun() const;

    /**
     * Get the directional light direction from the sun toward a world position.
     * Used to set up per-system lighting (all objects lit by their system's star).
     */
    glm::vec3 getSunLightDirection(const glm::vec3& objectPosition) const;

    /**
     * Get the sun's light color and intensity.
     */
    glm::vec3 getSunLightColor() const;
    float getSunLightIntensity() const;

    /**
     * Get list of warpable destinations (all celestials except the one you're near).
     */
    std::vector<const Celestial*> getWarpDestinations(const glm::vec3& shipPosition,
                                                       float minWarpDistance = 150000.0f) const;

    /**
     * Update the scene (called each frame).
     * - Updates engine trail emission based on ship physics
     * - Updates warp tunnel effect state
     */
    void update(float deltaTime, ShipPhysics* shipPhysics = nullptr);

    /**
     * Get system info.
     */
    const std::string& getSystemId() const { return m_systemId; }
    const std::string& getSystemName() const { return m_systemName; }
    float getSecurityLevel() const { return m_securityLevel; }

    /**
     * Get the nearest celestial to a position.
     */
    const Celestial* getNearestCelestial(const glm::vec3& position) const;

    /**
     * Check if position is within docking range of a station.
     */
    bool isInDockingRange(const glm::vec3& position, const std::string& stationId,
                          float dockingRadius = 2500.0f) const;

    /**
     * Get collision zones for all celestials in the system.
     * Collision radius = physical radius * COLLISION_MULTIPLIER.
     * Ships cannot warp through or enter these zones.
     */
    std::vector<ShipPhysics::CelestialCollisionZone> getCollisionZones() const;

    /**
     * Check if a ship position is inside any celestial collision zone.
     */
    bool isInsideCelestialCollisionZone(const glm::vec3& position) const;

    /**
     * Collision zone multiplier: collision radius is this factor times the
     * celestial's physical radius. Provides a safety margin.
     */
    static constexpr float COLLISION_MULTIPLIER = 1.5f;

    /**
     * Extra distance (meters) beyond the collision zone edge where ships
     * land when warping to a celestial without a specific warp distance.
     */
    static constexpr float WARP_LANDING_MARGIN = 2500.0f;

    // Engine trail state for rendering
    struct EngineTrailState {
        bool emitting;
        float intensity;     // 0.0 - 1.0 (based on throttle)
        glm::vec3 position;  // Ship rear position
        glm::vec3 velocity;  // Ship velocity (trail goes opposite)
    };

    const EngineTrailState& getEngineTrailState() const { return m_engineTrail; }

    // Warp visual state
    struct WarpVisualState {
        bool active;
        float progress;      // 0.0 - 1.0
        int phase;            // 0=none, 1=align, 2=accel, 3=cruise, 4=decel
        glm::vec3 direction;  // Warp direction
        float speedAU;        // Current warp speed
    };

    const WarpVisualState& getWarpVisualState() const { return m_warpVisual; }

    // Callback for warp initiation (to trigger UI updates)
    using WarpCallback = std::function<void(const std::string& celestialId)>;
    void setWarpCallback(WarpCallback cb) { m_onWarp = std::move(cb); }

    /**
     * Initiate warp to a celestial.
     */
    void warpTo(const std::string& celestialId, ShipPhysics* shipPhysics,
                float warpDistance = 0.0f);

private:
    std::string m_systemId;
    std::string m_systemName;
    float m_securityLevel;

    std::vector<Celestial> m_celestials;

    EngineTrailState m_engineTrail;
    WarpVisualState m_warpVisual;

    WarpCallback m_onWarp;

    static constexpr float AU_IN_METERS = 149597870700.0f;
};

} // namespace eve
