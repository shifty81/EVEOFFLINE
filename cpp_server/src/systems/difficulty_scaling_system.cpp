#include "systems/difficulty_scaling_system.h"
#include "ecs/world.h"
#include <cmath>
#include <algorithm>

namespace atlas {
namespace systems {

DifficultyScalingSystem::DifficultyScalingSystem(ecs::World* world)
    : System(world) {
}

void DifficultyScalingSystem::update(float /* delta_time */) {
    // Difficulty zones are static once initialized — no per-tick work needed.
    // Future: could dynamically adjust if systems change sovereignty.
}

bool DifficultyScalingSystem::initializeZone(const std::string& system_id,
                                              float security) {
    auto* entity = world_->getEntity(system_id);
    if (!entity) return false;

    auto* zone = entity->getComponent<components::DifficultyZone>();
    if (!zone) return false;

    float sec = std::clamp(security, 0.0f, 1.0f);
    zone->security_status = sec;
    zone->npc_hp_multiplier = hpMultiplierFromSecurity(sec);
    zone->npc_damage_multiplier = damageMultiplierFromSecurity(sec);
    zone->loot_quality_multiplier = lootMultiplierFromSecurity(sec);
    zone->ore_richness_multiplier = oreMultiplierFromSecurity(sec);
    zone->spawn_rate_multiplier = spawnRateFromSecurity(sec);
    zone->max_npc_tier = maxTierFromSecurity(sec);

    return true;
}

bool DifficultyScalingSystem::applyToNPC(const std::string& npc_id,
                                          const std::string& system_id) {
    auto* npc = world_->getEntity(npc_id);
    auto* sys_entity = world_->getEntity(system_id);
    if (!npc || !sys_entity) return false;

    auto* zone = sys_entity->getComponent<components::DifficultyZone>();
    if (!zone) return false;

    // Scale health
    auto* health = npc->getComponent<components::Health>();
    if (health) {
        health->shield_hp *= zone->npc_hp_multiplier;
        health->shield_max *= zone->npc_hp_multiplier;
        health->armor_hp *= zone->npc_hp_multiplier;
        health->armor_max *= zone->npc_hp_multiplier;
        health->hull_hp *= zone->npc_hp_multiplier;
        health->hull_max *= zone->npc_hp_multiplier;
    }

    // Scale weapon damage
    auto* weapon = npc->getComponent<components::Weapon>();
    if (weapon) {
        weapon->damage *= zone->npc_damage_multiplier;
    }

    return true;
}

// -----------------------------------------------------------------------
// Static multiplier calculations
// -----------------------------------------------------------------------

float DifficultyScalingSystem::hpMultiplierFromSecurity(float security) {
    // Highsec (1.0): 1.0x HP
    // Lowsec  (0.4): 1.8x HP
    // Nullsec (0.0): 2.5x HP
    float sec = std::clamp(security, 0.0f, 1.0f);
    return 1.0f + (1.0f - sec) * 1.5f;
}

float DifficultyScalingSystem::damageMultiplierFromSecurity(float security) {
    // Highsec (1.0): 1.0x damage
    // Lowsec  (0.4): 1.6x damage
    // Nullsec (0.0): 2.0x damage
    float sec = std::clamp(security, 0.0f, 1.0f);
    return 1.0f + (1.0f - sec) * 1.0f;
}

float DifficultyScalingSystem::lootMultiplierFromSecurity(float security) {
    // Better loot in more dangerous space
    // Highsec: 0.8x, Lowsec: 1.4x, Nullsec: 2.0x
    float sec = std::clamp(security, 0.0f, 1.0f);
    return 0.8f + (1.0f - sec) * 1.2f;
}

float DifficultyScalingSystem::oreMultiplierFromSecurity(float security) {
    // Richer ore in lower security
    // Highsec: 1.0x, Lowsec: 1.3x, Nullsec: 1.5x
    float sec = std::clamp(security, 0.0f, 1.0f);
    return 1.0f + (1.0f - sec) * 0.5f;
}

float DifficultyScalingSystem::spawnRateFromSecurity(float security) {
    // More NPCs in lower security
    // Highsec: 1.0x, Lowsec: 1.5x, Nullsec: 2.0x
    float sec = std::clamp(security, 0.0f, 1.0f);
    return 1.0f + (1.0f - sec) * 1.0f;
}

int DifficultyScalingSystem::maxTierFromSecurity(float security) {
    float sec = std::clamp(security, 0.0f, 1.0f);
    if (sec >= 0.8f) return 1;   // Highsec: only T1 NPCs
    if (sec >= 0.6f) return 2;   // High-lowsec border
    if (sec >= 0.4f) return 3;   // Lowsec
    if (sec >= 0.2f) return 4;   // Deep lowsec
    return 5;                     // Nullsec: all tiers
}

} // namespace systems
} // namespace atlas
