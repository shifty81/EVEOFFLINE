#include "systems/wreck_salvage_system.h"
#include "ecs/world.h"
#include "components/game_components.h"
#include <cmath>
#include <sstream>

namespace eve {
namespace systems {

static uint32_t s_next_wreck_id = 1;

WreckSalvageSystem::WreckSalvageSystem(ecs::World* world)
    : System(world) {
}

void WreckSalvageSystem::update(float delta_time) {
    // Tick down wreck lifetimes and destroy expired ones
    std::vector<std::string> expired;

    for (auto* entity : world_->getAllEntities()) {
        auto* wreck = entity->getComponent<components::Wreck>();
        if (!wreck) continue;

        wreck->lifetime_remaining -= delta_time;
        if (wreck->lifetime_remaining <= 0.0f) {
            expired.push_back(entity->getId());
        }
    }

    for (const auto& id : expired) {
        world_->destroyEntity(id);
    }
}

// ---------------------------------------------------------------------------
// Wreck creation
// ---------------------------------------------------------------------------

std::string WreckSalvageSystem::createWreck(const std::string& destroyed_entity_id,
                                             float x, float y, float z,
                                             float wreck_lifetime) {
    std::string wreck_id = "wreck_" + std::to_string(s_next_wreck_id++);

    auto* entity = world_->createEntity(wreck_id);
    if (!entity) return "";

    // Position — same as destroyed ship
    auto pos = std::make_unique<components::Position>();
    pos->x = x;
    pos->y = y;
    pos->z = z;
    entity->addComponent(std::move(pos));

    // Wreck component
    auto wreck = std::make_unique<components::Wreck>();
    wreck->source_entity_id = destroyed_entity_id;
    wreck->lifetime_remaining = wreck_lifetime;
    wreck->salvaged = false;
    entity->addComponent(std::move(wreck));

    // Inventory — wrecks can hold loot items
    auto inv = std::make_unique<components::Inventory>();
    inv->max_capacity = 500.0f; // wreck cargo capacity
    entity->addComponent(std::move(inv));

    return wreck_id;
}

// ---------------------------------------------------------------------------
// Salvage
// ---------------------------------------------------------------------------

bool WreckSalvageSystem::salvageWreck(const std::string& player_entity_id,
                                       const std::string& wreck_entity_id,
                                       float salvage_range) {
    auto* player_entity = world_->getEntity(player_entity_id);
    auto* wreck_entity  = world_->getEntity(wreck_entity_id);
    if (!player_entity || !wreck_entity) return false;

    auto* wreck = wreck_entity->getComponent<components::Wreck>();
    if (!wreck || wreck->salvaged) return false;

    // Range check
    auto* player_pos = player_entity->getComponent<components::Position>();
    auto* wreck_pos  = wreck_entity->getComponent<components::Position>();
    if (!player_pos || !wreck_pos) return false;

    float dx = player_pos->x - wreck_pos->x;
    float dy = player_pos->y - wreck_pos->y;
    float dz = player_pos->z - wreck_pos->z;
    float dist = std::sqrt(dx * dx + dy * dy + dz * dz);

    if (dist > salvage_range) return false;

    // Transfer items from wreck inventory to player inventory
    auto* wreck_inv  = wreck_entity->getComponent<components::Inventory>();
    auto* player_inv = player_entity->getComponent<components::Inventory>();

    if (wreck_inv && player_inv) {
        for (const auto& item : wreck_inv->items) {
            components::Inventory::Item copy = item;
            player_inv->items.push_back(copy);
        }
        wreck_inv->items.clear();
    }

    wreck->salvaged = true;
    return true;
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

int WreckSalvageSystem::getActiveWreckCount() const {
    int count = 0;
    for (const auto* entity : world_->getAllEntities()) {
        auto* wreck = entity->getComponent<components::Wreck>();
        if (wreck && !wreck->salvaged) count++;
    }
    return count;
}

} // namespace systems
} // namespace eve
