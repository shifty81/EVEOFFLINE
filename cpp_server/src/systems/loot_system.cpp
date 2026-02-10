#include "systems/loot_system.h"
#include "ecs/world.h"
#include "components/game_components.h"
#include <random>
#include <algorithm>

namespace eve {
namespace systems {

LootSystem::LootSystem(ecs::World* world)
    : System(world) {
}

void LootSystem::update(float /*delta_time*/) {
    // Loot generation is event-driven; no per-tick work.
}

void LootSystem::setRandomSeed(uint32_t seed) {
    seed_ = seed;
}

float LootSystem::nextRandom() {
    // Use mt19937 seeded deterministically so tests are repeatable
    std::mt19937 gen(seed_);
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    float val = dist(gen);
    seed_ = gen();  // advance seed for next call
    return val;
}

std::string LootSystem::generateLoot(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return "";

    auto* loot_table = entity->getComponent<components::LootTable>();
    if (!loot_table) return "";

    // Create wreck entity
    std::string wreck_id = "wreck_" + entity_id + "_" + std::to_string(wreck_counter_++);
    auto* wreck = world_->createEntity(wreck_id);
    if (!wreck) return "";

    // Add Inventory to wreck
    auto inv = std::make_unique<components::Inventory>();
    inv->max_capacity = 10000.0f;  // wrecks have large capacity

    // Add LootTable to wreck so we can store ISK bounty
    auto wreck_lt = std::make_unique<components::LootTable>();
    wreck_lt->isk_drop = loot_table->isk_drop;

    // Roll for each entry
    for (const auto& entry : loot_table->entries) {
        float roll = nextRandom();
        if (roll < entry.drop_chance) {
            // Determine quantity
            int qty = entry.min_quantity;
            if (entry.max_quantity > entry.min_quantity) {
                float range_roll = nextRandom();
                qty += static_cast<int>(range_roll * (entry.max_quantity - entry.min_quantity + 1));
                qty = std::min(qty, entry.max_quantity);
            }

            // Add item to wreck inventory
            components::Inventory::Item item;
            item.item_id = entry.item_id;
            item.name    = entry.name;
            item.type    = entry.type;
            item.quantity = qty;
            item.volume   = entry.volume;
            inv->items.push_back(item);
        }
    }

    wreck->addComponent(std::move(inv));
    wreck->addComponent(std::move(wreck_lt));
    return wreck_id;
}

bool LootSystem::collectLoot(const std::string& wreck_id,
                             const std::string& player_id) {
    auto* wreck  = world_->getEntity(wreck_id);
    auto* player_entity = world_->getEntity(player_id);
    if (!wreck || !player_entity) return false;

    auto* wreck_inv = wreck->getComponent<components::Inventory>();
    auto* player_inv = player_entity->getComponent<components::Inventory>();
    auto* player_comp = player_entity->getComponent<components::Player>();
    if (!wreck_inv || !player_inv) return false;

    // Transfer items
    for (const auto& item : wreck_inv->items) {
        // Check capacity
        float needed = item.volume * item.quantity;
        if (player_inv->freeCapacity() < needed) continue;

        // Stack with existing or add new
        bool stacked = false;
        for (auto& pi : player_inv->items) {
            if (pi.item_id == item.item_id) {
                pi.quantity += item.quantity;
                stacked = true;
                break;
            }
        }
        if (!stacked) {
            player_inv->items.push_back(item);
        }
    }
    wreck_inv->items.clear();

    // Add ISK bounty
    auto* wreck_lt = wreck->getComponent<components::LootTable>();
    if (wreck_lt && player_comp) {
        player_comp->isk += wreck_lt->isk_drop;
    }

    return true;
}

} // namespace systems
} // namespace eve
