#include "systems/wormhole_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/game_components.h"

namespace atlas {
namespace systems {

WormholeSystem::WormholeSystem(ecs::World* world)
    : ecs::System(world) {}

void WormholeSystem::update(float delta_time) {
    float delta_hours = delta_time / 3600.0f;

    auto entities = world_->getEntities();
    for (auto* entity : entities) {
        auto* wh = entity->getComponent<components::WormholeConnection>();
        if (!wh || wh->collapsed) continue;

        wh->elapsed_hours += delta_hours;

        // Collapse if lifetime exceeded or mass depleted
        if (wh->elapsed_hours >= wh->max_lifetime_hours || wh->remaining_mass <= 0.0) {
            wh->collapsed = true;
        }
    }
}

bool WormholeSystem::jumpThroughWormhole(const std::string& wormhole_entity_id, double ship_mass) {
    auto* entity = world_->getEntity(wormhole_entity_id);
    if (!entity) return false;

    auto* wh = entity->getComponent<components::WormholeConnection>();
    if (!wh || wh->collapsed) return false;

    // Check single-ship mass limit
    if (ship_mass > wh->max_jump_mass) return false;

    // Check remaining mass
    if (ship_mass > wh->remaining_mass) return false;

    wh->remaining_mass -= ship_mass;

    // Collapse if mass fully depleted
    if (wh->remaining_mass <= 0.0) {
        wh->remaining_mass = 0.0;
        wh->collapsed = true;
    }

    return true;
}

bool WormholeSystem::isWormholeStable(const std::string& wormhole_entity_id) const {
    auto* entity = world_->getEntity(wormhole_entity_id);
    if (!entity) return false;

    auto* wh = entity->getComponent<components::WormholeConnection>();
    if (!wh) return false;

    return wh->isStable();
}

float WormholeSystem::getRemainingMassFraction(const std::string& wormhole_entity_id) const {
    auto* entity = world_->getEntity(wormhole_entity_id);
    if (!entity) return -1.0f;

    auto* wh = entity->getComponent<components::WormholeConnection>();
    if (!wh || wh->max_mass <= 0.0) return -1.0f;

    return static_cast<float>(wh->remaining_mass / wh->max_mass);
}

float WormholeSystem::getRemainingLifetimeFraction(const std::string& wormhole_entity_id) const {
    auto* entity = world_->getEntity(wormhole_entity_id);
    if (!entity) return -1.0f;

    auto* wh = entity->getComponent<components::WormholeConnection>();
    if (!wh || wh->max_lifetime_hours <= 0.0f) return -1.0f;

    float remaining = wh->max_lifetime_hours - wh->elapsed_hours;
    if (remaining < 0.0f) remaining = 0.0f;
    return remaining / wh->max_lifetime_hours;
}

} // namespace systems
} // namespace atlas
