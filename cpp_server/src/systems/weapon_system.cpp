#include "systems/weapon_system.h"
#include "systems/combat_system.h"
#include "ecs/world.h"
#include "components/game_components.h"
#include <cmath>
#include <algorithm>

namespace eve {
namespace systems {

WeaponSystem::WeaponSystem(ecs::World* world)
    : System(world) {
}

void WeaponSystem::update(float delta_time) {
    auto entities = world_->getEntities<components::Weapon>();
    
    for (auto* entity : entities) {
        auto* weapon = entity->getComponent<components::Weapon>();
        if (!weapon) continue;
        
        // Update weapon cooldowns
        if (weapon->cooldown > 0.0f) {
            weapon->cooldown -= delta_time;
            if (weapon->cooldown < 0.0f) {
                weapon->cooldown = 0.0f;
            }
        }
        
        // Auto-fire for AI entities in Attacking state
        auto* ai = entity->getComponent<components::AI>();
        if (ai && ai->state == components::AI::State::Attacking 
            && !ai->target_entity_id.empty()) {
            if (weapon->cooldown <= 0.0f) {
                fireWeapon(entity->getId(), ai->target_entity_id);
            }
        }
    }
}

bool WeaponSystem::fireWeapon(const std::string& shooter_id, const std::string& target_id) {
    auto* shooter = world_->getEntity(shooter_id);
    auto* target = world_->getEntity(target_id);
    
    if (!shooter || !target) return false;
    
    auto* weapon = shooter->getComponent<components::Weapon>();
    auto* shooter_pos = shooter->getComponent<components::Position>();
    auto* target_pos = target->getComponent<components::Position>();
    
    if (!weapon || !shooter_pos || !target_pos) return false;
    
    // Check if weapon is ready
    if (weapon->cooldown > 0.0f || weapon->ammo_count <= 0) return false;
    
    // Check capacitor cost
    auto* cap = shooter->getComponent<components::Capacitor>();
    if (cap && weapon->capacitor_cost > 0.0f) {
        if (cap->capacitor < weapon->capacitor_cost) return false;
        cap->capacitor -= weapon->capacitor_cost;
    }
    
    // Calculate distance
    float dx = target_pos->x - shooter_pos->x;
    float dy = target_pos->y - shooter_pos->y;
    float dz = target_pos->z - shooter_pos->z;
    float distance = std::sqrt(dx * dx + dy * dy + dz * dz);
    
    // Check if target is in range (optimal + falloff)
    float max_range = weapon->optimal_range + weapon->falloff_range;
    if (distance > max_range) return false;
    
    // Calculate damage with falloff
    float damage_multiplier = calculateFalloff(distance, weapon->optimal_range, weapon->falloff_range);
    float effective_damage = weapon->damage * damage_multiplier;
    
    // Apply damage through CombatSystem
    auto* target_health = target->getComponent<components::Health>();
    if (!target_health) return false;
    
    // Apply damage to shields first, then armor, then hull (EVE damage cascade)
    float remaining = effective_damage;
    
    // Shield layer
    if (target_health->shield_hp > 0.0f && remaining > 0.0f) {
        float resist = 0.0f;
        if (weapon->damage_type == "em") resist = target_health->shield_em_resist;
        else if (weapon->damage_type == "thermal") resist = target_health->shield_thermal_resist;
        else if (weapon->damage_type == "kinetic") resist = target_health->shield_kinetic_resist;
        else if (weapon->damage_type == "explosive") resist = target_health->shield_explosive_resist;
        
        float applied = remaining * (1.0f - resist);
        target_health->shield_hp -= applied;
        if (target_health->shield_hp < 0.0f) {
            remaining = -target_health->shield_hp / (1.0f - resist);
            target_health->shield_hp = 0.0f;
        } else {
            remaining = 0.0f;
        }
    }
    
    // Armor layer
    if (target_health->armor_hp > 0.0f && remaining > 0.0f) {
        float resist = 0.0f;
        if (weapon->damage_type == "em") resist = target_health->armor_em_resist;
        else if (weapon->damage_type == "thermal") resist = target_health->armor_thermal_resist;
        else if (weapon->damage_type == "kinetic") resist = target_health->armor_kinetic_resist;
        else if (weapon->damage_type == "explosive") resist = target_health->armor_explosive_resist;
        
        float applied = remaining * (1.0f - resist);
        target_health->armor_hp -= applied;
        if (target_health->armor_hp < 0.0f) {
            remaining = -target_health->armor_hp / (1.0f - resist);
            target_health->armor_hp = 0.0f;
        } else {
            remaining = 0.0f;
        }
    }
    
    // Hull layer
    if (target_health->hull_hp > 0.0f && remaining > 0.0f) {
        float resist = 0.0f;
        if (weapon->damage_type == "em") resist = target_health->hull_em_resist;
        else if (weapon->damage_type == "thermal") resist = target_health->hull_thermal_resist;
        else if (weapon->damage_type == "kinetic") resist = target_health->hull_kinetic_resist;
        else if (weapon->damage_type == "explosive") resist = target_health->hull_explosive_resist;
        
        float applied = remaining * (1.0f - resist);
        target_health->hull_hp -= applied;
        if (target_health->hull_hp < 0.0f) {
            target_health->hull_hp = 0.0f;
        }
    }
    
    // Set weapon cooldown and consume ammo
    weapon->cooldown = weapon->rate_of_fire;
    weapon->ammo_count--;
    
    return true;
}

float WeaponSystem::calculateFalloff(float distance, float optimal_range, float falloff_range) {
    if (distance <= optimal_range) return 1.0f;
    if (falloff_range <= 0.0f) return 0.0f;
    
    float falloff_distance = distance - optimal_range;
    float multiplier = 1.0f - (falloff_distance / falloff_range);
    return std::max(0.0f, multiplier);
}

} // namespace systems
} // namespace eve
