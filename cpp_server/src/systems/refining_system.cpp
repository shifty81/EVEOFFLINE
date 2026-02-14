#include "systems/refining_system.h"
#include "ecs/world.h"
#include "components/game_components.h"
#include <algorithm>
#include <sstream>

namespace atlas {
namespace systems {

RefiningSystem::RefiningSystem(ecs::World* world)
    : System(world) {
}

void RefiningSystem::update(float delta_time) {
    for (auto* entity : world_->getAllEntities()) {
        auto* facility = entity->getComponent<components::RefiningFacility>();
        if (!facility) continue;

        for (auto& job : facility->jobs) {
            if (job.completed) continue;
            job.progress += delta_time / job.time_per_batch;
            if (job.progress >= 1.0f) {
                job.progress = 1.0f;
                job.completed = true;

                // Deliver refined minerals to owner inventory
                auto* owner = world_->getEntity(job.owner_id);
                if (!owner) continue;
                auto* inv = owner->getComponent<components::Inventory>();
                if (!inv) continue;

                std::string output = facility->getOutputMineral(job.ore_type);
                if (output.empty()) continue;

                float yield = facility->getYieldForOre(job.ore_type);
                int output_qty = static_cast<int>(job.ore_quantity * yield);
                if (output_qty <= 0) continue;

                // Stack into existing inventory item or create new
                bool stacked = false;
                for (auto& item : inv->items) {
                    if (item.item_id == output) {
                        item.quantity += output_qty;
                        stacked = true;
                        break;
                    }
                }
                if (!stacked) {
                    components::Inventory::Item mineral;
                    mineral.item_id  = output;
                    mineral.name     = output;
                    mineral.type     = "mineral";
                    mineral.quantity = output_qty;
                    mineral.volume   = 0.01f;
                    inv->items.push_back(mineral);
                }
            }
        }

        // Remove completed jobs
        facility->jobs.erase(
            std::remove_if(facility->jobs.begin(), facility->jobs.end(),
                           [](const components::RefiningFacility::RefiningJob& j) {
                               return j.completed;
                           }),
            facility->jobs.end());
    }
}

std::string RefiningSystem::startRefining(const std::string& station_id,
                                           const std::string& owner_id,
                                           const std::string& ore_type,
                                           int ore_quantity) {
    auto* station = world_->getEntity(station_id);
    if (!station) return "";

    auto* facility = station->getComponent<components::RefiningFacility>();
    if (!facility) return "";

    // Check that the facility has a recipe for this ore
    if (facility->getOutputMineral(ore_type).empty()) return "";

    // Check owner has ore in inventory
    auto* owner = world_->getEntity(owner_id);
    if (!owner) return "";
    auto* inv = owner->getComponent<components::Inventory>();
    if (!inv) return "";

    bool found = false;
    for (auto& item : inv->items) {
        if (item.item_id == ore_type && item.quantity >= ore_quantity) {
            item.quantity -= ore_quantity;
            found = true;
            break;
        }
    }
    if (!found) return "";

    // Create job
    std::string job_id = "refine_" + std::to_string(job_counter_++);
    components::RefiningFacility::RefiningJob job;
    job.job_id = job_id;
    job.owner_id = owner_id;
    job.ore_type = ore_type;
    job.ore_quantity = ore_quantity;
    job.progress = 0.0f;
    job.time_per_batch = 30.0f;
    job.completed = false;
    facility->jobs.push_back(job);

    return job_id;
}

int RefiningSystem::getActiveJobCount(const std::string& station_id) {
    auto* station = world_->getEntity(station_id);
    if (!station) return 0;
    auto* facility = station->getComponent<components::RefiningFacility>();
    if (!facility) return 0;
    int count = 0;
    for (const auto& j : facility->jobs) {
        if (!j.completed) count++;
    }
    return count;
}

int RefiningSystem::getCompletedJobCount(const std::string& station_id) {
    // Note: completed jobs are removed each tick, so this returns 0
    // unless called between update ticks. Provided for completeness.
    auto* station = world_->getEntity(station_id);
    if (!station) return 0;
    auto* facility = station->getComponent<components::RefiningFacility>();
    if (!facility) return 0;
    int count = 0;
    for (const auto& j : facility->jobs) {
        if (j.completed) count++;
    }
    return count;
}

void RefiningSystem::seedStandardRecipes(const std::string& station_id) {
    auto* station = world_->getEntity(station_id);
    if (!station) return;
    auto* facility = station->getComponent<components::RefiningFacility>();
    if (!facility) return;

    facility->recipes.clear();
    facility->recipes.push_back({"Dustite",    "Ferrium",  415.0f});
    facility->recipes.push_back({"Ferrite",    "Ignium",    346.0f});
    facility->recipes.push_back({"Ignaite",   "Allonium",   333.0f});
    facility->recipes.push_back({"Crystite", "Isodium",     256.0f});
    facility->recipes.push_back({"Shadite",       "Noctium",     85.0f});
    facility->recipes.push_back({"Corite",     "Zyrium",     65.0f});
    facility->recipes.push_back({"Cosmite",     "Megrium",    40.0f});
}

} // namespace systems
} // namespace atlas
