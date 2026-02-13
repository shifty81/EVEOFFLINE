#include "systems/ai_system.h"
#include "systems/combat_system.h"
#include "ecs/world.h"
#include "components/game_components.h"
#include <cmath>
#include <limits>
#include <algorithm>

namespace atlas {
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
    
    // Apply dynamic orbit distance from ship class if enabled
    if (ai->use_dynamic_orbit) {
        auto* ship = entity->getComponent<components::Ship>();
        if (ship) {
            ai->orbit_distance = orbitDistanceForClass(ship->ship_class);
        }
    }
    
    // Derive engagement range from weapon if not set
    if (ai->engagement_range <= 0.0f) {
        ai->engagement_range = engagementRangeFromWeapon(entity);
    }
    
    // Use target selection strategy
    ecs::Entity* target = selectTarget(entity);
    
    // If found a target, switch to approaching
    if (target) {
        ai->target_entity_id = target->getId();
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
    
    // Use engagement range to decide when to transition.
    // If engagement_range is set and we're within it, start orbiting.
    // Otherwise fall back to orbit_distance.
    float engage = (ai->engagement_range > 0.0f) ? ai->engagement_range : ai->orbit_distance;
    
    if (distance < std::min(engage, ai->orbit_distance)) {
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
    // Check health for flee decision
    auto* ai = entity->getComponent<components::AI>();
    auto* health = entity->getComponent<components::Health>();
    if (ai && health) {
        float total_max = health->shield_max + health->armor_max + health->hull_max;
        float total_current = health->shield_hp + health->armor_hp + health->hull_hp;
        if (total_max > 0.0f && (total_current / total_max) < ai->flee_threshold) {
            ai->state = components::AI::State::Fleeing;
            return;
        }
    }
    
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

ecs::Entity* AISystem::selectTarget(ecs::Entity* entity) {
    auto* ai = entity->getComponent<components::AI>();
    auto* pos = entity->getComponent<components::Position>();
    if (!ai || !pos) return nullptr;
    
    auto all_entities = world_->getEntities<components::Position, components::Player>();
    
    ecs::Entity* best_target = nullptr;
    float best_score = std::numeric_limits<float>::max();
    
    for (auto* candidate : all_entities) {
        if (candidate == entity) continue;
        
        auto* target_pos = candidate->getComponent<components::Position>();
        if (!target_pos) continue;
        
        float dx = target_pos->x - pos->x;
        float dy = target_pos->y - pos->y;
        float dz = target_pos->z - pos->z;
        float distance = std::sqrt(dx * dx + dy * dy + dz * dz);
        
        if (distance > ai->awareness_range) continue;
        
        float score = 0.0f;
        
        switch (ai->target_selection) {
            case components::AI::TargetSelection::Closest:
                score = distance;
                break;
                
            case components::AI::TargetSelection::LowestHP: {
                auto* health = candidate->getComponent<components::Health>();
                if (health) {
                    float total_max = health->shield_max + health->armor_max + health->hull_max;
                    float total_current = health->shield_hp + health->armor_hp + health->hull_hp;
                    score = (total_max > 0.0f) ? (total_current / total_max) : 1.0f;
                } else {
                    score = 1.0f;
                }
                break;
            }
            
            case components::AI::TargetSelection::HighestThreat: {
                auto* dmg = entity->getComponent<components::DamageEvent>();
                // Lower score = higher priority; invert damage to make most-damaging a lower score
                float threat = 0.0f;
                if (dmg) {
                    for (const auto& hit : dmg->recent_hits) {
                        threat += hit.damage_amount;
                    }
                }
                // Use negative threat as score so higher threat = lower score = higher priority
                // Add distance as tiebreaker
                score = -threat + distance * 0.001f;
                break;
            }
        }
        
        if (score < best_score) {
            best_score = score;
            best_target = candidate;
        }
    }
    
    return best_target;
}

float AISystem::orbitDistanceForClass(const std::string& ship_class) {
    if (ship_class == "Frigate" || ship_class == "Destroyer") {
        return 5000.0f;
    } else if (ship_class == "Cruiser" || ship_class == "Battlecruiser") {
        return 15000.0f;
    } else if (ship_class == "Battleship") {
        return 30000.0f;
    } else if (ship_class == "Capital" || ship_class == "Carrier" ||
               ship_class == "Dreadnought" || ship_class == "Titan") {
        return 50000.0f;
    }
    return 10000.0f;  // default for unknown classes
}

float AISystem::engagementRangeFromWeapon(ecs::Entity* entity) {
    auto* weapon = entity->getComponent<components::Weapon>();
    if (!weapon) return 0.0f;
    return weapon->optimal_range + weapon->falloff_range;
}

} // namespace systems
} // namespace atlas
