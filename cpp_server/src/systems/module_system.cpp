#include "systems/module_system.h"
#include "ecs/world.h"
#include "components/game_components.h"
#include <algorithm>

namespace eve {
namespace systems {

ModuleSystem::ModuleSystem(ecs::World* world)
    : System(world) {
}

void ModuleSystem::update(float delta_time) {
    auto entities = world_->getEntities<components::ModuleRack>();

    for (auto* entity : entities) {
        auto* rack = entity->getComponent<components::ModuleRack>();
        auto* cap = entity->getComponent<components::Capacitor>();
        if (!rack) continue;

        auto processSlots = [&](std::vector<components::ModuleRack::FittedModule>& slots) {
            for (auto& mod : slots) {
                if (!mod.active) continue;

                mod.cycle_progress += delta_time / mod.cycle_time;

                // Check cycle completion
                if (mod.cycle_progress >= 1.0f) {
                    mod.cycle_progress -= 1.0f;

                    // Consume capacitor
                    if (cap && mod.capacitor_cost > 0.0f) {
                        if (cap->capacitor >= mod.capacitor_cost) {
                            cap->capacitor -= mod.capacitor_cost;
                        } else {
                            // Not enough capacitor — deactivate
                            mod.active = false;
                            mod.cycle_progress = 0.0f;
                        }
                    }
                }
            }
        };

        processSlots(rack->high_slots);
        processSlots(rack->mid_slots);
        processSlots(rack->low_slots);
    }
}

static std::vector<components::ModuleRack::FittedModule>*
getSlots(components::ModuleRack* rack, const std::string& slot_type) {
    if (slot_type == "high") return &rack->high_slots;
    if (slot_type == "mid")  return &rack->mid_slots;
    if (slot_type == "low")  return &rack->low_slots;
    return nullptr;
}

bool ModuleSystem::toggleModule(const std::string& entity_id,
                                 const std::string& slot_type,
                                 int slot_index) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* rack = entity->getComponent<components::ModuleRack>();
    if (!rack) return false;

    auto* slots = getSlots(rack, slot_type);
    if (!slots || slot_index < 0 || slot_index >= static_cast<int>(slots->size()))
        return false;

    auto& mod = (*slots)[slot_index];
    if (mod.active) {
        mod.active = false;
        mod.cycle_progress = 0.0f;
    } else {
        // Check capacitor before activating
        auto* cap = entity->getComponent<components::Capacitor>();
        if (cap && cap->capacitor < mod.capacitor_cost) return false;
        mod.active = true;
        mod.cycle_progress = 0.0f;
    }
    return true;
}

bool ModuleSystem::activateModule(const std::string& entity_id,
                                   const std::string& slot_type,
                                   int slot_index) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* rack = entity->getComponent<components::ModuleRack>();
    if (!rack) return false;

    auto* slots = getSlots(rack, slot_type);
    if (!slots || slot_index < 0 || slot_index >= static_cast<int>(slots->size()))
        return false;

    auto& mod = (*slots)[slot_index];
    if (mod.active) return false;  // already active

    auto* cap = entity->getComponent<components::Capacitor>();
    if (cap && cap->capacitor < mod.capacitor_cost) return false;

    mod.active = true;
    mod.cycle_progress = 0.0f;
    return true;
}

bool ModuleSystem::deactivateModule(const std::string& entity_id,
                                     const std::string& slot_type,
                                     int slot_index) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* rack = entity->getComponent<components::ModuleRack>();
    if (!rack) return false;

    auto* slots = getSlots(rack, slot_type);
    if (!slots || slot_index < 0 || slot_index >= static_cast<int>(slots->size()))
        return false;

    auto& mod = (*slots)[slot_index];
    if (!mod.active) return false;  // already inactive

    mod.active = false;
    mod.cycle_progress = 0.0f;
    return true;
}

bool ModuleSystem::validateFitting(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* rack = entity->getComponent<components::ModuleRack>();
    auto* ship = entity->getComponent<components::Ship>();
    if (!rack || !ship) return false;

    float total_cpu = 0.0f;
    float total_pg = 0.0f;

    auto sumSlots = [&](const std::vector<components::ModuleRack::FittedModule>& slots) {
        for (const auto& mod : slots) {
            total_cpu += mod.cpu_usage;
            total_pg += mod.powergrid_usage;
        }
    };

    sumSlots(rack->high_slots);
    sumSlots(rack->mid_slots);
    sumSlots(rack->low_slots);

    return total_cpu <= ship->cpu_max && total_pg <= ship->powergrid_max;
}

} // namespace systems
} // namespace eve
