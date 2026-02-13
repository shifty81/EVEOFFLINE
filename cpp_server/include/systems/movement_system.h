#ifndef EVE_SYSTEMS_MOVEMENT_SYSTEM_H
#define EVE_SYSTEMS_MOVEMENT_SYSTEM_H

#include "ecs/system.h"
#include <string>
#include <vector>
#include <map>

namespace atlas {
namespace systems {

/**
 * @brief Handles entity movement and physics
 * 
 * Updates entity positions based on their velocity.
 * Applies speed limits and handles basic physics.
 * Prevents entities from entering celestial collision zones (e.g., the sun).
 */
class MovementSystem : public ecs::System {
public:
    explicit MovementSystem(ecs::World* world);
    ~MovementSystem() override = default;
    
    void update(float delta_time) override;
    std::string getName() const override { return "MovementSystem"; }

    /**
     * Celestial collision zone for server-side boundary enforcement.
     */
    struct CollisionZone {
        float x, y, z;     // Center position
        float radius;       // Collision radius
    };

    /**
     * Set celestial collision zones for the current system.
     * Entities will be pushed out of these zones during movement.
     */
    void setCollisionZones(const std::vector<CollisionZone>& zones);

    /**
     * @brief Command an entity to orbit another entity
     * @param entity_id The orbiting entity
     * @param target_id The entity to orbit around
     * @param distance Desired orbit distance in meters
     */
    void commandOrbit(const std::string& entity_id,
                      const std::string& target_id,
                      float distance);

    /**
     * @brief Command an entity to approach another entity
     * @param entity_id The approaching entity
     * @param target_id The entity to approach
     */
    void commandApproach(const std::string& entity_id,
                         const std::string& target_id);

    /**
     * @brief Command an entity to stop
     */
    void commandStop(const std::string& entity_id);

    /**
     * @brief Command an entity to warp to a position
     * @param entity_id The entity to warp
     * @param dest_x Destination X
     * @param dest_y Destination Y
     * @param dest_z Destination Z
     * @return true if warp initiated (not disrupted, meets minimum distance)
     */
    bool commandWarp(const std::string& entity_id,
                     float dest_x, float dest_y, float dest_z);

    /**
     * @brief Check if an entity is currently warp disrupted
     * @param entity_id The entity to check
     * @return true if warp disruption strength >= warp core strength
     */
    bool isWarpDisrupted(const std::string& entity_id) const;

private:
    struct MovementCommand {
        enum class Type { None, Orbit, Approach, Warp, Stop };
        Type type = Type::None;
        std::string target_id;
        float orbit_distance = 1000.0f;
        float warp_dest_x = 0.0f;
        float warp_dest_y = 0.0f;
        float warp_dest_z = 0.0f;
        float warp_progress = 0.0f;  // 0-1
        float warp_duration = 10.0f; // seconds (computed from distance / warp_speed)
        float align_time = 2.5f;     // seconds for align phase (from Ship component)
        bool warping = false;
    };
    std::map<std::string, MovementCommand> movement_commands_;

    std::vector<CollisionZone> m_collisionZones;

    static constexpr float COLLISION_PUSH_MARGIN = 100.0f;  // Extra meters beyond collision radius
};

} // namespace systems
} // namespace atlas

#endif // EVE_SYSTEMS_MOVEMENT_SYSTEM_H
