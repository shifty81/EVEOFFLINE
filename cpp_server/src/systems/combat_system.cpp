#include "systems/combat_system.h"
#include "ecs/world.h"
#include "components/game_components.h"
#include <cmath>
#include <algorithm>

namespace eve {
namespace systems {

CombatSystem::CombatSystem(ecs::World* world)
    : System(world) {
}

void CombatSystem::update(float delta_time) {
    // Update weapon cooldowns
    auto entities = world_->getEntities<components::Weapon>();
    
    for (auto* entity : entities) {
        auto* weapon = entity->getComponent<components::Weapon>();
        if (weapon && weapon->cooldown > 0.0f) {
            weapon->cooldown -= delta_time;
            if (weapon->cooldown < 0.0f) {
                weapon->cooldown = 0.0f;
            }
        }
    }
    
    // Update shield recharge
    auto health_entities = world_->getEntities<components::Health>();
    for (auto* entity : health_entities) {
        auto* health = entity->getComponent<components::Health>();
        if (health && health->shield_hp < health->shield_max) {
            float recharge = health->shield_recharge_rate * delta_time;
            health->shield_hp = std::min(health->shield_hp + recharge, health->shield_max);
        }
    }
    
    // Update capacitor recharge
    auto cap_entities = world_->getEntities<components::Capacitor>();
    for (auto* entity : cap_entities) {
        auto* cap = entity->getComponent<components::Capacitor>();
        if (cap && cap->capacitor < cap->capacitor_max) {
            float recharge = cap->recharge_rate * delta_time;
            cap->capacitor = std::min(cap->capacitor + recharge, cap->capacitor_max);
        }
    }
}

bool CombatSystem::applyDamage(const std::string& target_id, float damage, const std::string& damage_type) {
    auto* target = world_->getEntity(target_id);
    if (!target) return false;
    
    auto* health = target->getComponent<components::Health>();
    if (!health) return false;
    
    // Apply damage to shields first
    if (health->shield_hp > 0.0f) {
        float resist = getResistance(
            health->shield_em_resist,
            health->shield_thermal_resist,
            health->shield_kinetic_resist,
            health->shield_explosive_resist,
            damage_type
        );
        float effective_damage = calculateDamage(damage, resist);
        health->shield_hp -= effective_damage;
        
        if (health->shield_hp < 0.0f) {
            // Overflow damage goes to armor
            float overflow_damage = -health->shield_hp;
            health->shield_hp = 0.0f;
            damage = overflow_damage;
        } else {
            return true;  // All damage absorbed by shields
        }
    }
    
    // Apply remaining damage to armor
    if (health->armor_hp > 0.0f) {
        float resist = getResistance(
            health->armor_em_resist,
            health->armor_thermal_resist,
            health->armor_kinetic_resist,
            health->armor_explosive_resist,
            damage_type
        );
        float effective_damage = calculateDamage(damage, resist);
        health->armor_hp -= effective_damage;
        
        if (health->armor_hp < 0.0f) {
            // Overflow damage goes to hull
            float overflow_damage = -health->armor_hp;
            health->armor_hp = 0.0f;
            damage = overflow_damage;
        } else {
            return true;  // All damage absorbed by armor
        }
    }
    
    // Apply remaining damage to hull
    if (health->hull_hp > 0.0f) {
        float resist = getResistance(
            health->hull_em_resist,
            health->hull_thermal_resist,
            health->hull_kinetic_resist,
            health->hull_explosive_resist,
            damage_type
        );
        float effective_damage = calculateDamage(damage, resist);
        health->hull_hp -= effective_damage;
        
        if (health->hull_hp < 0.0f) {
            health->hull_hp = 0.0f;
        }
    }
    
    return true;
}

bool CombatSystem::fireWeapon(const std::string& shooter_id, const std::string& target_id) {
    auto* shooter = world_->getEntity(shooter_id);
    auto* target = world_->getEntity(target_id);
    
    if (!shooter || !target) return false;
    
    auto* weapon = shooter->getComponent<components::Weapon>();
    auto* shooter_pos = shooter->getComponent<components::Position>();
    auto* target_pos = target->getComponent<components::Position>();
    
    if (!weapon || !shooter_pos || !target_pos) return false;
    
    // Check if weapon is ready
    if (weapon->cooldown > 0.0f || weapon->ammo_count <= 0) return false;
    
    // Calculate distance
    float dx = target_pos->x - shooter_pos->x;
    float dy = target_pos->y - shooter_pos->y;
    float dz = target_pos->z - shooter_pos->z;
    float distance = std::sqrt(dx * dx + dy * dy + dz * dz);
    
    // Check if target is in range (optimal + falloff)
    float max_range = weapon->optimal_range + weapon->falloff_range;
    if (distance > max_range) return false;
    
    // Calculate damage falloff
    float damage_multiplier = 1.0f;
    if (distance > weapon->optimal_range) {
        float falloff_distance = distance - weapon->optimal_range;
        damage_multiplier = 1.0f - (falloff_distance / weapon->falloff_range);
        damage_multiplier = std::max(0.0f, damage_multiplier);
    }
    
    // Apply damage
    float effective_damage = weapon->damage * damage_multiplier;
    applyDamage(target_id, effective_damage, weapon->damage_type);
    
    // Set weapon cooldown and consume ammo
    weapon->cooldown = weapon->rate_of_fire;
    weapon->ammo_count--;
    
    return true;
}

float CombatSystem::calculateDamage(float base_damage, float resistance) {
    // Resistance is 0.0 to 1.0 (0% to 100%)
    return base_damage * (1.0f - resistance);
}

float CombatSystem::getResistance(float em_resist, float thermal_resist,
                                  float kinetic_resist, float explosive_resist,
                                  const std::string& damage_type) {
    if (damage_type == "em") return em_resist;
    if (damage_type == "thermal") return thermal_resist;
    if (damage_type == "kinetic") return kinetic_resist;
    if (damage_type == "explosive") return explosive_resist;
    return 0.0f;  // Unknown damage type, no resistance
}

} // namespace systems
} // namespace eve
