#include "systems/ship_fitting_system.h"
#include "ecs/world.h"
#include "components/game_components.h"
#include <algorithm>

namespace atlas {
namespace systems {

ShipFittingSystem::ShipFittingSystem(ecs::World* world)
    : System(world) {
}

void ShipFittingSystem::update(float /*delta_time*/) {
    // Ship fitting is event-driven; nothing to tick
}

int ShipFittingSystem::getSlotCapacity(const std::string& ship_class,
                                       const std::string& slot_type) {
    // Default slot layout per ship class
    struct SlotLayout { int high; int mid; int low; };

    static const std::map<std::string, SlotLayout> layouts = {
        {"Frigate",       {3, 3, 2}},
        {"Destroyer",     {4, 3, 3}},
        {"Cruiser",       {5, 4, 4}},
        {"Battlecruiser", {6, 4, 5}},
        {"Battleship",    {7, 5, 5}},
        {"Capital",       {8, 6, 6}},
        {"Carrier",       {8, 6, 6}},
        {"Titan",         {8, 6, 6}},
        {"Industrial",    {2, 4, 3}},
        {"Mining Barge",  {2, 4, 2}},
    };

    auto it = layouts.find(ship_class);
    if (it == layouts.end()) {
        // Unknown class → default frigate layout
        if (slot_type == "high") return 3;
        if (slot_type == "mid")  return 3;
        if (slot_type == "low")  return 2;
        return 0;
    }

    const auto& l = it->second;
    if (slot_type == "high") return l.high;
    if (slot_type == "mid")  return l.mid;
    if (slot_type == "low")  return l.low;
    return 0;
}

static std::vector<components::ModuleRack::FittedModule>*
getSlots(components::ModuleRack* rack, const std::string& slot_type) {
    if (slot_type == "high") return &rack->high_slots;
    if (slot_type == "mid")  return &rack->mid_slots;
    if (slot_type == "low")  return &rack->low_slots;
    return nullptr;
}

bool ShipFittingSystem::fitModule(const std::string& entity_id,
                                   const std::string& module_id,
                                   const std::string& module_name,
                                   const std::string& slot_type,
                                   float cpu_usage,
                                   float powergrid_usage,
                                   float capacitor_cost,
                                   float cycle_time) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* rack = entity->getComponent<components::ModuleRack>();
    auto* ship = entity->getComponent<components::Ship>();
    if (!rack || !ship) return false;

    auto* slots = getSlots(rack, slot_type);
    if (!slots) return false;

    // Check slot capacity
    int max_slots = getSlotCapacity(ship->ship_class, slot_type);
    if (static_cast<int>(slots->size()) >= max_slots) return false;

    // Check CPU/PG budget (current usage + new module <= max)
    float total_cpu = cpu_usage;
    float total_pg  = powergrid_usage;
    auto sumSlots = [&](const std::vector<components::ModuleRack::FittedModule>& s) {
        for (const auto& m : s) {
            total_cpu += m.cpu_usage;
            total_pg  += m.powergrid_usage;
        }
    };
    sumSlots(rack->high_slots);
    sumSlots(rack->mid_slots);
    sumSlots(rack->low_slots);

    if (total_cpu > ship->cpu_max || total_pg > ship->powergrid_max)
        return false;

    // Fit the module
    components::ModuleRack::FittedModule mod;
    mod.module_id       = module_id;
    mod.name            = module_name;
    mod.slot_type       = slot_type;
    mod.slot_index      = static_cast<int>(slots->size());
    mod.cpu_usage       = cpu_usage;
    mod.powergrid_usage = powergrid_usage;
    mod.capacitor_cost  = capacitor_cost;
    mod.cycle_time      = cycle_time;
    mod.active          = false;
    mod.cycle_progress  = 0.0f;

    slots->push_back(mod);
    return true;
}

bool ShipFittingSystem::removeModule(const std::string& entity_id,
                                      const std::string& slot_type,
                                      int slot_index) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* rack = entity->getComponent<components::ModuleRack>();
    if (!rack) return false;

    auto* slots = getSlots(rack, slot_type);
    if (!slots || slot_index < 0 || slot_index >= static_cast<int>(slots->size()))
        return false;

    // Deactivate before removing
    (*slots)[slot_index].active = false;

    slots->erase(slots->begin() + slot_index);

    // Re-index remaining modules
    for (int i = slot_index; i < static_cast<int>(slots->size()); ++i) {
        (*slots)[i].slot_index = i;
    }

    return true;
}

int ShipFittingSystem::getFittedCount(const std::string& entity_id,
                                       const std::string& slot_type) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return -1;

    auto* rack = entity->getComponent<components::ModuleRack>();
    if (!rack) return -1;

    if (slot_type == "high") return static_cast<int>(rack->high_slots.size());
    if (slot_type == "mid")  return static_cast<int>(rack->mid_slots.size());
    if (slot_type == "low")  return static_cast<int>(rack->low_slots.size());
    return -1;
}

bool ShipFittingSystem::validateFitting(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* rack = entity->getComponent<components::ModuleRack>();
    auto* ship = entity->getComponent<components::Ship>();
    if (!rack || !ship) return false;

    float total_cpu = 0.0f;
    float total_pg  = 0.0f;
    auto sum = [&](const std::vector<components::ModuleRack::FittedModule>& s) {
        for (const auto& m : s) {
            total_cpu += m.cpu_usage;
            total_pg  += m.powergrid_usage;
        }
    };
    sum(rack->high_slots);
    sum(rack->mid_slots);
    sum(rack->low_slots);

    // Check resource budget
    if (total_cpu > ship->cpu_max || total_pg > ship->powergrid_max)
        return false;

    // Check slot counts
    int maxH = getSlotCapacity(ship->ship_class, "high");
    int maxM = getSlotCapacity(ship->ship_class, "mid");
    int maxL = getSlotCapacity(ship->ship_class, "low");

    if (static_cast<int>(rack->high_slots.size()) > maxH) return false;
    if (static_cast<int>(rack->mid_slots.size()) > maxM)  return false;
    if (static_cast<int>(rack->low_slots.size()) > maxL)  return false;

    return true;
}

} // namespace systems
} // namespace atlas
