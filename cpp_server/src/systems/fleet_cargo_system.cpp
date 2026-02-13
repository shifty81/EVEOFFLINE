#include "systems/fleet_cargo_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <algorithm>

namespace atlas {
namespace systems {

FleetCargoSystem::FleetCargoSystem(ecs::World* world)
    : System(world) {
}

void FleetCargoSystem::update(float /*delta_time*/) {
    auto entities = world_->getEntities<components::FleetCargoPool>();
    for (auto* entity : entities) {
        recalculate(entity->getId());
    }
}

void FleetCargoSystem::addContributor(const std::string& pool_entity_id, const std::string& ship_entity_id) {
    auto* entity = world_->getEntity(pool_entity_id);
    if (!entity) return;

    auto* pool = entity->getComponent<components::FleetCargoPool>();
    if (!pool) {
        entity->addComponent(std::make_unique<components::FleetCargoPool>());
        pool = entity->getComponent<components::FleetCargoPool>();
    }

    auto& contributors = pool->contributor_ship_ids;
    if (std::find(contributors.begin(), contributors.end(), ship_entity_id) == contributors.end()) {
        contributors.push_back(ship_entity_id);
    }

    recalculate(pool_entity_id);
}

void FleetCargoSystem::removeContributor(const std::string& pool_entity_id, const std::string& ship_entity_id) {
    auto* entity = world_->getEntity(pool_entity_id);
    if (!entity) return;

    auto* pool = entity->getComponent<components::FleetCargoPool>();
    if (!pool) return;

    auto& contributors = pool->contributor_ship_ids;
    contributors.erase(
        std::remove(contributors.begin(), contributors.end(), ship_entity_id),
        contributors.end()
    );

    recalculate(pool_entity_id);
}

uint64_t FleetCargoSystem::getTotalCapacity(const std::string& pool_entity_id) const {
    const auto* entity = world_->getEntity(pool_entity_id);
    if (!entity) return 0;

    const auto* pool = entity->getComponent<components::FleetCargoPool>();
    if (!pool) return 0;

    return pool->total_capacity;
}

uint64_t FleetCargoSystem::getUsedCapacity(const std::string& pool_entity_id) const {
    const auto* entity = world_->getEntity(pool_entity_id);
    if (!entity) return 0;

    const auto* pool = entity->getComponent<components::FleetCargoPool>();
    if (!pool) return 0;

    return pool->used_capacity;
}

void FleetCargoSystem::recalculate(const std::string& pool_entity_id) {
    auto* entity = world_->getEntity(pool_entity_id);
    if (!entity) return;

    auto* pool = entity->getComponent<components::FleetCargoPool>();
    if (!pool) return;

    uint64_t total_cap = 0;
    uint64_t used_cap = 0;

    for (const auto& ship_id : pool->contributor_ship_ids) {
        auto* ship_entity = world_->getEntity(ship_id);
        if (!ship_entity) continue;

        auto* inventory = ship_entity->getComponent<components::Inventory>();
        if (!inventory) continue;

        total_cap += static_cast<uint64_t>(inventory->max_capacity);
        used_cap += static_cast<uint64_t>(inventory->usedCapacity());
    }

    pool->total_capacity = total_cap;
    pool->used_capacity = used_cap;
}

} // namespace systems
} // namespace atlas
