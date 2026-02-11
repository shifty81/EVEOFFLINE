#include "systems/pi_system.h"
#include "ecs/world.h"
#include "components/game_components.h"

namespace eve {
namespace systems {

PISystem::PISystem(ecs::World* world)
    : System(world) {
}

void PISystem::update(float delta_time) {
    for (auto* entity : world_->getAllEntities()) {
        auto* colony = entity->getComponent<components::PlanetaryColony>();
        if (!colony) continue;

        // Tick extractors
        for (auto& ext : colony->extractors) {
            if (!ext.active) continue;
            ext.cycle_progress += delta_time;
            while (ext.cycle_progress >= ext.cycle_time) {
                ext.cycle_progress -= ext.cycle_time;

                // Check storage capacity
                if (colony->totalStored() + ext.quantity_per_cycle > static_cast<int>(colony->storage_capacity))
                    continue;

                // Add extracted resource to storage
                bool found = false;
                for (auto& s : colony->storage) {
                    if (s.resource_type == ext.resource_type) {
                        s.quantity += ext.quantity_per_cycle;
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    components::PlanetaryColony::StoredResource sr;
                    sr.resource_type = ext.resource_type;
                    sr.quantity = ext.quantity_per_cycle;
                    colony->storage.push_back(sr);
                }
            }
        }

        // Tick processors
        for (auto& proc : colony->processors) {
            if (!proc.active) continue;
            proc.cycle_progress += delta_time;
            while (proc.cycle_progress >= proc.cycle_time) {
                proc.cycle_progress -= proc.cycle_time;

                // Check input availability
                int available_input = 0;
                for (const auto& s : colony->storage) {
                    if (s.resource_type == proc.input_type) {
                        available_input = s.quantity;
                        break;
                    }
                }
                if (available_input < proc.input_quantity) continue;

                // Check output storage capacity
                if (colony->totalStored() - proc.input_quantity + proc.output_quantity
                    > static_cast<int>(colony->storage_capacity))
                    continue;

                // Consume input
                for (auto& s : colony->storage) {
                    if (s.resource_type == proc.input_type) {
                        s.quantity -= proc.input_quantity;
                        break;
                    }
                }

                // Produce output
                bool found = false;
                for (auto& s : colony->storage) {
                    if (s.resource_type == proc.output_type) {
                        s.quantity += proc.output_quantity;
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    components::PlanetaryColony::StoredResource sr;
                    sr.resource_type = proc.output_type;
                    sr.quantity = proc.output_quantity;
                    colony->storage.push_back(sr);
                }
            }
        }
    }
}

bool PISystem::installExtractor(const std::string& colony_entity_id,
                                const std::string& resource_type,
                                int quantity_per_cycle) {
    auto* entity = world_->getEntity(colony_entity_id);
    if (!entity) return false;

    auto* colony = entity->getComponent<components::PlanetaryColony>();
    if (!colony) return false;

    components::PlanetaryColony::Extractor ext;
    ext.extractor_id = "ext_" + std::to_string(++extractor_counter_);
    ext.resource_type = resource_type;
    ext.quantity_per_cycle = quantity_per_cycle;

    // Check CPU/PG
    if (colony->usedCpu() + ext.cpu_usage > colony->cpu_max) return false;
    if (colony->usedPowergrid() + ext.powergrid_usage > colony->powergrid_max) return false;

    colony->extractors.push_back(ext);
    return true;
}

bool PISystem::installProcessor(const std::string& colony_entity_id,
                                const std::string& input_type,
                                const std::string& output_type,
                                int input_qty,
                                int output_qty) {
    auto* entity = world_->getEntity(colony_entity_id);
    if (!entity) return false;

    auto* colony = entity->getComponent<components::PlanetaryColony>();
    if (!colony) return false;

    components::PlanetaryColony::Processor proc;
    proc.processor_id = "proc_" + std::to_string(++processor_counter_);
    proc.input_type = input_type;
    proc.output_type = output_type;
    proc.input_quantity = input_qty;
    proc.output_quantity = output_qty;

    // Check CPU/PG
    if (colony->usedCpu() + proc.cpu_usage > colony->cpu_max) return false;
    if (colony->usedPowergrid() + proc.powergrid_usage > colony->powergrid_max) return false;

    colony->processors.push_back(proc);
    return true;
}

int PISystem::getStoredResource(const std::string& colony_entity_id,
                                const std::string& resource_type) {
    auto* entity = world_->getEntity(colony_entity_id);
    if (!entity) return 0;

    auto* colony = entity->getComponent<components::PlanetaryColony>();
    if (!colony) return 0;

    for (const auto& s : colony->storage) {
        if (s.resource_type == resource_type)
            return s.quantity;
    }
    return 0;
}

int PISystem::getTotalStored(const std::string& colony_entity_id) {
    auto* entity = world_->getEntity(colony_entity_id);
    if (!entity) return 0;

    auto* colony = entity->getComponent<components::PlanetaryColony>();
    if (!colony) return 0;

    return colony->totalStored();
}

int PISystem::getExtractorCount(const std::string& colony_entity_id) {
    auto* entity = world_->getEntity(colony_entity_id);
    if (!entity) return 0;

    auto* colony = entity->getComponent<components::PlanetaryColony>();
    if (!colony) return 0;

    return static_cast<int>(colony->extractors.size());
}

int PISystem::getProcessorCount(const std::string& colony_entity_id) {
    auto* entity = world_->getEntity(colony_entity_id);
    if (!entity) return 0;

    auto* colony = entity->getComponent<components::PlanetaryColony>();
    if (!colony) return 0;

    return static_cast<int>(colony->processors.size());
}

} // namespace systems
} // namespace eve
