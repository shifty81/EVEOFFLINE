#include "systems/manufacturing_system.h"
#include "ecs/world.h"
#include "components/game_components.h"

namespace atlas {
namespace systems {

ManufacturingSystem::ManufacturingSystem(ecs::World* world)
    : System(world) {
}

void ManufacturingSystem::update(float delta_time) {
    for (auto* entity : world_->getAllEntities()) {
        auto* facility = entity->getComponent<components::ManufacturingFacility>();
        if (!facility) continue;

        for (auto& job : facility->jobs) {
            if (job.status != "active") continue;

            job.time_remaining -= delta_time;
            if (job.time_remaining <= 0.0f) {
                job.runs_completed++;
                if (job.runs_completed >= job.runs) {
                    job.time_remaining = 0.0f;
                    job.status = "completed";
                } else {
                    // Start next run
                    job.time_remaining = job.time_per_run;
                }
            }
        }
    }
}

std::string ManufacturingSystem::startJob(const std::string& facility_entity_id,
                                           const std::string& owner_id,
                                           const std::string& blueprint_id,
                                           const std::string& output_item_id,
                                           const std::string& output_item_name,
                                           int runs,
                                           float time_per_run,
                                           double install_cost) {
    auto* entity = world_->getEntity(facility_entity_id);
    if (!entity) return "";

    auto* facility = entity->getComponent<components::ManufacturingFacility>();
    if (!facility) return "";

    // Check job slot availability
    if (facility->activeJobCount() >= facility->max_jobs) return "";

    // Deduct install cost from owner
    auto* owner = world_->getEntity(owner_id);
    if (owner) {
        auto* player = owner->getComponent<components::Player>();
        if (player) {
            if (player->isk < install_cost) return "";
            player->isk -= install_cost;
        }
    }

    components::ManufacturingFacility::ManufacturingJob job;
    job.job_id = "mfg_" + std::to_string(++job_counter_);
    job.blueprint_id = blueprint_id;
    job.owner_id = owner_id;
    job.output_item_id = output_item_id;
    job.output_item_name = output_item_name;
    job.runs = runs;
    job.runs_completed = 0;
    job.time_per_run = time_per_run;
    job.time_remaining = time_per_run;
    job.install_cost = install_cost;
    job.status = "active";

    facility->jobs.push_back(job);
    return job.job_id;
}

bool ManufacturingSystem::cancelJob(const std::string& facility_entity_id,
                                     const std::string& job_id) {
    auto* entity = world_->getEntity(facility_entity_id);
    if (!entity) return false;

    auto* facility = entity->getComponent<components::ManufacturingFacility>();
    if (!facility) return false;

    for (auto& job : facility->jobs) {
        if (job.job_id == job_id && job.status == "active") {
            job.status = "cancelled";
            return true;
        }
    }
    return false;
}

int ManufacturingSystem::getActiveJobCount(const std::string& facility_entity_id) {
    auto* entity = world_->getEntity(facility_entity_id);
    if (!entity) return 0;

    auto* facility = entity->getComponent<components::ManufacturingFacility>();
    if (!facility) return 0;

    return facility->activeJobCount();
}

int ManufacturingSystem::getCompletedJobCount(const std::string& facility_entity_id) {
    auto* entity = world_->getEntity(facility_entity_id);
    if (!entity) return 0;

    auto* facility = entity->getComponent<components::ManufacturingFacility>();
    if (!facility) return 0;

    int count = 0;
    for (const auto& job : facility->jobs)
        if (job.status == "completed") ++count;
    return count;
}

int ManufacturingSystem::getTotalRunsCompleted(const std::string& facility_entity_id) {
    auto* entity = world_->getEntity(facility_entity_id);
    if (!entity) return 0;

    auto* facility = entity->getComponent<components::ManufacturingFacility>();
    if (!facility) return 0;

    int total = 0;
    for (const auto& job : facility->jobs)
        total += job.runs_completed;
    return total;
}

} // namespace systems
} // namespace atlas
