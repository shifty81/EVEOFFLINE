#ifndef EVE_SYSTEMS_MOVEMENT_SYSTEM_H
#define EVE_SYSTEMS_MOVEMENT_SYSTEM_H

#include "ecs/system.h"
#include <string>
#include <vector>

namespace eve {
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

private:
    std::vector<CollisionZone> m_collisionZones;
};

} // namespace systems
} // namespace eve

#endif // EVE_SYSTEMS_MOVEMENT_SYSTEM_H
