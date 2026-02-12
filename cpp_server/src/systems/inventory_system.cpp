#include "systems/inventory_system.h"
#include "ecs/world.h"
#include "components/game_components.h"
#include <algorithm>

namespace atlas {
namespace systems {

InventorySystem::InventorySystem(ecs::World* world)
    : System(world) {
}

void InventorySystem::update(float /*delta_time*/) {
    // Inventory is event-driven; no per-tick work needed.
}

bool InventorySystem::addItem(const std::string& entity_id,
                              const std::string& item_id,
                              const std::string& name,
                              const std::string& type,
                              int quantity,
                              float volume) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* inv = entity->getComponent<components::Inventory>();
    if (!inv) return false;

    float total_volume = volume * quantity;
    if (inv->freeCapacity() < total_volume) return false;

    // Stack with existing item of same id
    for (auto& item : inv->items) {
        if (item.item_id == item_id) {
            item.quantity += quantity;
            return true;
        }
    }

    // New stack
    components::Inventory::Item new_item;
    new_item.item_id  = item_id;
    new_item.name     = name;
    new_item.type     = type;
    new_item.quantity  = quantity;
    new_item.volume    = volume;
    inv->items.push_back(new_item);
    return true;
}

int InventorySystem::removeItem(const std::string& entity_id,
                                const std::string& item_id,
                                int quantity) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;

    auto* inv = entity->getComponent<components::Inventory>();
    if (!inv) return 0;

    for (auto it = inv->items.begin(); it != inv->items.end(); ++it) {
        if (it->item_id == item_id) {
            int removed = std::min(quantity, it->quantity);
            it->quantity -= removed;
            if (it->quantity <= 0) {
                inv->items.erase(it);
            }
            return removed;
        }
    }
    return 0;
}

bool InventorySystem::transferItem(const std::string& from_id,
                                   const std::string& to_id,
                                   const std::string& item_id,
                                   int quantity) {
    auto* from_entity = world_->getEntity(from_id);
    auto* to_entity   = world_->getEntity(to_id);
    if (!from_entity || !to_entity) return false;

    auto* from_inv = from_entity->getComponent<components::Inventory>();
    auto* to_inv   = to_entity->getComponent<components::Inventory>();
    if (!from_inv || !to_inv) return false;

    // Find item in source
    for (auto it = from_inv->items.begin(); it != from_inv->items.end(); ++it) {
        if (it->item_id == item_id) {
            if (it->quantity < quantity) return false;

            float total_volume = it->volume * quantity;
            if (to_inv->freeCapacity() < total_volume) return false;

            // Add to destination
            if (!addItem(to_id, item_id, it->name, it->type, quantity, it->volume))
                return false;

            // Remove from source
            it->quantity -= quantity;
            if (it->quantity <= 0) {
                from_inv->items.erase(it);
            }
            return true;
        }
    }
    return false;
}

int InventorySystem::getItemCount(const std::string& entity_id,
                                  const std::string& item_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;

    auto* inv = entity->getComponent<components::Inventory>();
    if (!inv) return 0;

    for (const auto& item : inv->items) {
        if (item.item_id == item_id) return item.quantity;
    }
    return 0;
}

bool InventorySystem::hasItem(const std::string& entity_id,
                              const std::string& item_id,
                              int quantity) {
    return getItemCount(entity_id, item_id) >= quantity;
}

} // namespace systems
} // namespace atlas
