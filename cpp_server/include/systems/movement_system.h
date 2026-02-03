#ifndef EVE_SYSTEMS_MOVEMENT_SYSTEM_H
#define EVE_SYSTEMS_MOVEMENT_SYSTEM_H

#include "ecs/system.h"
#include <string>

namespace eve {
namespace systems {

/**
 * @brief Handles entity movement and physics
 * 
 * Updates entity positions based on their velocity.
 * Applies speed limits and handles basic physics.
 */
class MovementSystem : public ecs::System {
public:
    explicit MovementSystem(ecs::World* world);
    ~MovementSystem() override = default;
    
    void update(float delta_time) override;
    std::string getName() const override { return "MovementSystem"; }
};

} // namespace systems
} // namespace eve

#endif // EVE_SYSTEMS_MOVEMENT_SYSTEM_H
