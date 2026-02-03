#include "systems/movement_system.h"
#include "ecs/world.h"
#include "components/game_components.h"
#include <cmath>

namespace eve {
namespace systems {

MovementSystem::MovementSystem(ecs::World* world)
    : System(world) {
}

void MovementSystem::update(float delta_time) {
    // Get all entities with Position and Velocity components
    auto entities = world_->getEntities<components::Position, components::Velocity>();
    
    for (auto* entity : entities) {
        auto* pos = entity->getComponent<components::Position>();
        auto* vel = entity->getComponent<components::Velocity>();
        
        if (!pos || !vel) continue;
        
        // Update position based on velocity
        pos->x += vel->vx * delta_time;
        pos->y += vel->vy * delta_time;
        pos->z += vel->vz * delta_time;
        pos->rotation += vel->angular_velocity * delta_time;
        
        // Calculate current speed
        float speed = std::sqrt(vel->vx * vel->vx + vel->vy * vel->vy + vel->vz * vel->vz);
        
        // Apply speed limit
        if (speed > vel->max_speed && speed > 0.0f) {
            float factor = vel->max_speed / speed;
            vel->vx *= factor;
            vel->vy *= factor;
            vel->vz *= factor;
        }
    }
}

} // namespace systems
} // namespace eve
