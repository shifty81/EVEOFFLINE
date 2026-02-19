#include "systems/combat_threat_system.h"
#include "ecs/world.h"
#include <algorithm>

namespace atlas {
namespace systems {

CombatThreatSystem::CombatThreatSystem(ecs::World* world)
    : System(world) {
}

void CombatThreatSystem::update(float /*delta_time*/) {
    // Apply accumulated combat events to star-system threat levels
    for (auto& [system_id, damage] : pending_damage_) {
        auto* entity = world_->getEntity(system_id);
        if (!entity) continue;

        auto* state = entity->getComponent<components::SimStarSystemState>();
        if (!state) continue;

        state->threat_level += damage * damage_threat_factor;
    }

    for (auto& [system_id, count] : pending_destructions_) {
        auto* entity = world_->getEntity(system_id);
        if (!entity) continue;

        auto* state = entity->getComponent<components::SimStarSystemState>();
        if (!state) continue;

        state->threat_level += count * destruction_threat_spike;
    }

    // Clamp all system threat levels
    auto entities = world_->getEntities<components::SimStarSystemState>();
    for (auto* entity : entities) {
        auto* state = entity->getComponent<components::SimStarSystemState>();
        if (state) {
            state->threat_level = std::clamp(state->threat_level, 0.0f, max_threat);
        }
    }

    // Clear pending events for next tick
    pending_damage_.clear();
    pending_destructions_.clear();
}

// -----------------------------------------------------------------------
// Event recording API
// -----------------------------------------------------------------------

void CombatThreatSystem::recordCombatDamage(const std::string& system_id,
                                             float damage) {
    pending_damage_[system_id] += damage;
}

void CombatThreatSystem::recordShipDestruction(const std::string& system_id) {
    pending_destructions_[system_id]++;
}

float CombatThreatSystem::getPendingDamage(const std::string& system_id) const {
    auto it = pending_damage_.find(system_id);
    return (it != pending_damage_.end()) ? it->second : 0.0f;
}

int CombatThreatSystem::getPendingDestructions(const std::string& system_id) const {
    auto it = pending_destructions_.find(system_id);
    return (it != pending_destructions_.end()) ? it->second : 0;
}

} // namespace systems
} // namespace atlas
