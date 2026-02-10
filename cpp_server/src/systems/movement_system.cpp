#include "systems/movement_system.h"
#include "ecs/world.h"
#include "components/game_components.h"
#include <cmath>

namespace eve {
namespace systems {

MovementSystem::MovementSystem(ecs::World* world)
    : System(world) {
}

void MovementSystem::setCollisionZones(const std::vector<CollisionZone>& zones) {
    m_collisionZones = zones;
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
        
        // Enforce celestial collision zones
        for (const auto& zone : m_collisionZones) {
            float dx = pos->x - zone.x;
            float dy = pos->y - zone.y;
            float dz = pos->z - zone.z;
            float dist = std::sqrt(dx * dx + dy * dy + dz * dz);
            
            if (dist < zone.radius && dist > 0.001f) {
                // Push entity to edge of collision zone
                float pushFactor = (zone.radius + 100.0f) / dist;
                pos->x = zone.x + dx * pushFactor;
                pos->y = zone.y + dy * pushFactor;
                pos->z = zone.z + dz * pushFactor;
                
                // Kill velocity toward the celestial
                float invDist = 1.0f / dist;
                float nx = dx * invDist;
                float ny = dy * invDist;
                float nz = dz * invDist;
                float velToward = -(vel->vx * nx + vel->vy * ny + vel->vz * nz);
                if (velToward > 0.0f) {
                    vel->vx += nx * velToward;
                    vel->vy += ny * velToward;
                    vel->vz += nz * velToward;
                }
            }
        }
    }
}

} // namespace systems
} // namespace eve
