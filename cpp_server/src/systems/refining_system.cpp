#include "systems/refining_system.h"
#include "ecs/world.h"
#include "components/game_components.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

RefiningSystem::RefiningSystem(ecs::World* world)
    : System(world) {
}

void RefiningSystem::update(float /*delta_time*/) {
    // Refining is an on-demand action, not a per-tick process.
}

int RefiningSystem::refineOre(const std::string& player_id,
                               const std::string& station_id,
                               const std::string& ore_type,
                               int batches) {
    if (batches <= 0) return 0;

    auto* player = world_->getEntity(player_id);
    auto* station = world_->getEntity(station_id);
    if (!player || !station) return 0;

    auto* inv = player->getComponent<components::Inventory>();
    auto* facility = station->getComponent<components::RefiningFacility>();
    if (!inv || !facility) return 0;

    // Find recipe for this ore type
    const components::RefiningFacility::RefineRecipe* recipe = nullptr;
    for (const auto& r : facility->recipes) {
        if (r.ore_type == ore_type) {
            recipe = &r;
            break;
        }
    }
    if (!recipe) return 0;

    // Count available ore in inventory
    int available_ore = 0;
    for (const auto& item : inv->items) {
        if (item.item_id == ore_type) {
            available_ore = item.quantity;
            break;
        }
    }

    // Clamp batches to what the player actually has
    int max_batches = available_ore / recipe->ore_units_required;
    int actual_batches = std::min(batches, max_batches);
    if (actual_batches <= 0) return 0;

    // Consume ore
    int ore_consumed = actual_batches * recipe->ore_units_required;
    for (auto& item : inv->items) {
        if (item.item_id == ore_type) {
            item.quantity -= ore_consumed;
            break;
        }
    }

    // Remove zeroed-out items
    inv->items.erase(
        std::remove_if(inv->items.begin(), inv->items.end(),
            [](const components::Inventory::Item& i) { return i.quantity <= 0; }),
        inv->items.end());

    // Produce minerals
    float yield_mult = facility->efficiency * (1.0f - facility->tax_rate);

    for (const auto& output : recipe->outputs) {
        int produced = static_cast<int>(
            std::floor(output.base_quantity * actual_batches * yield_mult));
        if (produced <= 0) continue;

        // Stack into existing inventory or add new entry
        bool stacked = false;
        for (auto& item : inv->items) {
            if (item.item_id == output.mineral_type) {
                item.quantity += produced;
                stacked = true;
                break;
            }
        }
        if (!stacked) {
            components::Inventory::Item mineral;
            mineral.item_id = output.mineral_type;
            mineral.name = output.mineral_type;
            mineral.type = "mineral";
            mineral.quantity = produced;
            mineral.volume = 0.01f;  // minerals are compact
            inv->items.push_back(mineral);
        }
    }

    return actual_batches;
}

bool RefiningSystem::installDefaultRecipes(const std::string& station_id) {
    auto* station = world_->getEntity(station_id);
    if (!station) return false;

    auto* facility = station->getComponent<components::RefiningFacility>();
    if (!facility) return false;

    facility->recipes.clear();

    // Veldspar → Tritanium
    {
        components::RefiningFacility::RefineRecipe r;
        r.ore_type = "Veldspar";
        r.ore_units_required = 100;
        r.outputs.push_back({"Tritanium", 415});
        facility->recipes.push_back(r);
    }

    // Scordite → Tritanium + Pyerite
    {
        components::RefiningFacility::RefineRecipe r;
        r.ore_type = "Scordite";
        r.ore_units_required = 100;
        r.outputs.push_back({"Tritanium", 346});
        r.outputs.push_back({"Pyerite", 173});
        facility->recipes.push_back(r);
    }

    // Pyroxeres → Pyerite + Nocxidium
    {
        components::RefiningFacility::RefineRecipe r;
        r.ore_type = "Pyroxeres";
        r.ore_units_required = 100;
        r.outputs.push_back({"Pyerite", 315});
        r.outputs.push_back({"Nocxidium", 11});
        facility->recipes.push_back(r);
    }

    // Plagioclase → Tritanium + Pyerite + Mexallon
    {
        components::RefiningFacility::RefineRecipe r;
        r.ore_type = "Plagioclase";
        r.ore_units_required = 100;
        r.outputs.push_back({"Tritanium", 256});
        r.outputs.push_back({"Pyerite", 512});
        r.outputs.push_back({"Mexallon", 256});
        facility->recipes.push_back(r);
    }

    return true;
}

} // namespace systems
} // namespace atlas
