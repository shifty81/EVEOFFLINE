#include "systems/ai_system.h"
#include "systems/combat_system.h"
#include "ecs/world.h"
#include "components/game_components.h"
#include <cmath>
#include <limits>

namespace eve {
namespace systems {

AISystem::AISystem(ecs::World* world)
    : System(world) {
}

void AISystem::update(float delta_time) {
    // Get all entities with AI component
    auto entities = world_->getEntities<components::AI, components::Position, components::Velocity>();
    
    for (auto* entity : entities) {
        auto* ai = entity->getComponent<components::AI>();
        
        // Execute behavior based on current state
        switch (ai->state) {
            case components::AI::State::Idle:
                idleBehavior(entity);
                break;
            case components::AI::State::Approaching:
                approachBehavior(entity);
                break;
            case components::AI::State::Orbiting:
                orbitBehavior(entity);
                break;
            case components::AI::State::Attacking:
                attackBehavior(entity);
                break;
            case components::AI::State::Fleeing:
                fleeBehavior(entity);
                break;
        }
    }
}

void AISystem::idleBehavior(ecs::Entity* entity) {
    auto* ai = entity->getComponent<components::AI>();
    auto* pos = entity->getComponent<components::Position>();
    
    if (!ai || !pos) return;
    
    // Only aggressive NPCs actively seek targets
    if (ai->behavior != components::AI::Behavior::Aggressive) {
        return;
    }
    
    // Find nearest player within awareness range
    auto all_entities = world_->getEntities<components::Position, components::Player>();
    
    ecs::Entity* nearest_target = nullptr;
    float nearest_distance = ai->awareness_range;
    
    for (auto* potential_target : all_entities) {
        if (potential_target == entity) continue;
        
        auto* target_pos = potential_target->getComponent<components::Position>();
        if (!target_pos) continue;
        
        // Calculate distance
        float dx = target_pos->x - pos->x;
        float dy = target_pos->y - pos->y;
        float dz = target_pos->z - pos->z;
        float distance = std::sqrt(dx * dx + dy * dy + dz * dz);
        
        if (distance < nearest_distance) {
            nearest_distance = distance;
            nearest_target = potential_target;
        }
    }
    
    // If found a target, switch to approaching
    if (nearest_target) {
        ai->target_entity_id = nearest_target->getId();
        ai->state = components::AI::State::Approaching;
    }
}

void AISystem::approachBehavior(ecs::Entity* entity) {
    auto* ai = entity->getComponent<components::AI>();
    auto* pos = entity->getComponent<components::Position>();
    auto* vel = entity->getComponent<components::Velocity>();
    
    if (!ai || !pos || !vel) return;
    
    // Check if target still exists
    if (ai->target_entity_id.empty()) {
        ai->state = components::AI::State::Idle;
        return;
    }
    
    auto* target = world_->getEntity(ai->target_entity_id);
    if (!target) {
        ai->state = components::AI::State::Idle;
        ai->target_entity_id.clear();
        return;
    }
    
    auto* target_pos = target->getComponent<components::Position>();
    if (!target_pos) {
        ai->state = components::AI::State::Idle;
        ai->target_entity_id.clear();
        return;
    }
    
    // Calculate distance to target
    float dx = target_pos->x - pos->x;
    float dy = target_pos->y - pos->y;
    float dz = target_pos->z - pos->z;
    float distance = std::sqrt(dx * dx + dy * dy + dz * dz);
    
    // If close enough, switch to orbiting
    if (distance < ai->orbit_distance) {
        ai->state = components::AI::State::Orbiting;
        return;
    }
    
    // Move towards target
    if (distance > 0.0f) {
        vel->vx = (dx / distance) * vel->max_speed;
        vel->vy = (dy / distance) * vel->max_speed;
        vel->vz = (dz / distance) * vel->max_speed;
    }
}

void AISystem::orbitBehavior(ecs::Entity* entity) {
    auto* ai = entity->getComponent<components::AI>();
    auto* pos = entity->getComponent<components::Position>();
    auto* vel = entity->getComponent<components::Velocity>();
    
    if (!ai || !pos || !vel) return;
    
    // Check if target still exists
    if (ai->target_entity_id.empty()) {
        ai->state = components::AI::State::Idle;
        return;
    }
    
    auto* target = world_->getEntity(ai->target_entity_id);
    if (!target) {
        ai->state = components::AI::State::Idle;
        ai->target_entity_id.clear();
        return;
    }
    
    auto* target_pos = target->getComponent<components::Position>();
    if (!target_pos) {
        ai->state = components::AI::State::Idle;
        ai->target_entity_id.clear();
        return;
    }
    
    // Calculate perpendicular velocity for orbiting
    float dx = target_pos->x - pos->x;
    float dy = target_pos->y - pos->y;
    float distance = std::sqrt(dx * dx + dy * dy);
    
    if (distance > 0.0f) {
        // Perpendicular velocity for orbiting (2D orbit in xy plane)
        vel->vx = -(dy / distance) * vel->max_speed * 0.5f;
        vel->vy = (dx / distance) * vel->max_speed * 0.5f;
        vel->vz = 0.0f;
    }
    
    // Switch to attacking if we have weapons
    if (entity->hasComponent<components::Weapon>()) {
        ai->state = components::AI::State::Attacking;
    }
}

void AISystem::attackBehavior(ecs::Entity* entity) {
    // Continue orbiting while attacking
    orbitBehavior(entity);
    
    // Weapon firing is handled by CombatSystem
    // The AI just maintains the target and orbit
}

void AISystem::fleeBehavior(ecs::Entity* entity) {
    auto* ai = entity->getComponent<components::AI>();
    auto* pos = entity->getComponent<components::Position>();
    auto* vel = entity->getComponent<components::Velocity>();
    
    if (!ai || !pos || !vel) return;
    
    // Check if target still exists
    if (ai->target_entity_id.empty()) {
        ai->state = components::AI::State::Idle;
        return;
    }
    
    auto* target = world_->getEntity(ai->target_entity_id);
    if (!target) {
        ai->state = components::AI::State::Idle;
        ai->target_entity_id.clear();
        return;
    }
    
    auto* target_pos = target->getComponent<components::Position>();
    if (!target_pos) {
        ai->state = components::AI::State::Idle;
        ai->target_entity_id.clear();
        return;
    }
    
    // Move away from target
    float dx = pos->x - target_pos->x;
    float dy = pos->y - target_pos->y;
    float dz = pos->z - target_pos->z;
    float distance = std::sqrt(dx * dx + dy * dy + dz * dz);
    
    if (distance > 0.0f) {
        vel->vx = (dx / distance) * vel->max_speed;
        vel->vy = (dy / distance) * vel->max_speed;
        vel->vz = (dz / distance) * vel->max_speed;
    }
    
    // If far enough away, switch back to idle
    if (distance > ai->awareness_range) {
        ai->state = components::AI::State::Idle;
        ai->target_entity_id.clear();
    }
}

} // namespace systems
} // namespace eve
