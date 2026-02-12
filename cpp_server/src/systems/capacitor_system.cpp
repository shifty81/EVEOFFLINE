#include "systems/capacitor_system.h"
#include "ecs/world.h"
#include "components/game_components.h"
#include <algorithm>

namespace atlas {
namespace systems {

CapacitorSystem::CapacitorSystem(ecs::World* world)
    : System(world) {
}

void CapacitorSystem::update(float delta_time) {
    auto entities = world_->getEntities<components::Capacitor>();
    
    for (auto* entity : entities) {
        auto* cap = entity->getComponent<components::Capacitor>();
        if (!cap) continue;
        
        // Recharge capacitor over time
        if (cap->capacitor < cap->capacitor_max) {
            float recharge = cap->recharge_rate * delta_time;
            cap->capacitor = std::min(cap->capacitor + recharge, cap->capacitor_max);
        }
    }
}

bool CapacitorSystem::consumeCapacitor(const std::string& entity_id, float amount) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    
    auto* cap = entity->getComponent<components::Capacitor>();
    if (!cap) return false;
    
    if (cap->capacitor >= amount) {
        cap->capacitor -= amount;
        return true;
    }
    
    return false;  // Not enough capacitor
}

float CapacitorSystem::getCapacitorPercentage(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return -1.0f;
    
    auto* cap = entity->getComponent<components::Capacitor>();
    if (!cap || cap->capacitor_max <= 0.0f) return -1.0f;
    
    return cap->capacitor / cap->capacitor_max;
}

} // namespace systems
} // namespace atlas
