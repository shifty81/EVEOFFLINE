#include "systems/research_system.h"
#include "ecs/world.h"
#include "components/game_components.h"

namespace eve {
namespace systems {

ResearchSystem::ResearchSystem(ecs::World* world)
    : System(world) {
}

float ResearchSystem::nextRandom() {
    // Simple LCG for deterministic results
    rng_state_ = rng_state_ * 1103515245u + 12345u;
    return static_cast<float>((rng_state_ >> 16) & 0x7FFF) / 32767.0f;
}

void ResearchSystem::update(float delta_time) {
    for (auto* entity : world_->getAllEntities()) {
        auto* lab = entity->getComponent<components::ResearchLab>();
        if (!lab) continue;

        for (auto& job : lab->jobs) {
            if (job.status != "active") continue;

            job.time_remaining -= delta_time;
            if (job.time_remaining <= 0.0f) {
                job.time_remaining = 0.0f;

                if (job.research_type == "invention") {
                    // Roll for success
                    float roll = nextRandom();
                    if (roll <= job.success_chance) {
                        job.status = "completed";
                    } else {
                        job.status = "failed";
                    }
                } else {
                    // ME/TE research always succeeds
                    job.status = "completed";
                }
            }
        }
    }
}

std::string ResearchSystem::startMEResearch(const std::string& lab_entity_id,
                                             const std::string& owner_id,
                                             const std::string& blueprint_id,
                                             int target_level,
                                             float total_time,
                                             double install_cost) {
    auto* entity = world_->getEntity(lab_entity_id);
    if (!entity) return "";

    auto* lab = entity->getComponent<components::ResearchLab>();
    if (!lab) return "";

    if (lab->activeJobCount() >= lab->max_jobs) return "";

    // Deduct install cost
    auto* owner = world_->getEntity(owner_id);
    if (owner) {
        auto* player = owner->getComponent<components::Player>();
        if (player) {
            if (player->isk < install_cost) return "";
            player->isk -= install_cost;
        }
    }

    components::ResearchLab::ResearchJob job;
    job.job_id = "res_" + std::to_string(++job_counter_);
    job.blueprint_id = blueprint_id;
    job.owner_id = owner_id;
    job.research_type = "material_efficiency";
    job.target_level = target_level;
    job.total_time = total_time;
    job.time_remaining = total_time;
    job.install_cost = install_cost;
    job.status = "active";

    lab->jobs.push_back(job);
    return job.job_id;
}

std::string ResearchSystem::startTEResearch(const std::string& lab_entity_id,
                                             const std::string& owner_id,
                                             const std::string& blueprint_id,
                                             int target_level,
                                             float total_time,
                                             double install_cost) {
    auto* entity = world_->getEntity(lab_entity_id);
    if (!entity) return "";

    auto* lab = entity->getComponent<components::ResearchLab>();
    if (!lab) return "";

    if (lab->activeJobCount() >= lab->max_jobs) return "";

    auto* owner = world_->getEntity(owner_id);
    if (owner) {
        auto* player = owner->getComponent<components::Player>();
        if (player) {
            if (player->isk < install_cost) return "";
            player->isk -= install_cost;
        }
    }

    components::ResearchLab::ResearchJob job;
    job.job_id = "res_" + std::to_string(++job_counter_);
    job.blueprint_id = blueprint_id;
    job.owner_id = owner_id;
    job.research_type = "time_efficiency";
    job.target_level = target_level;
    job.total_time = total_time;
    job.time_remaining = total_time;
    job.install_cost = install_cost;
    job.status = "active";

    lab->jobs.push_back(job);
    return job.job_id;
}

std::string ResearchSystem::startInvention(const std::string& lab_entity_id,
                                            const std::string& owner_id,
                                            const std::string& blueprint_id,
                                            const std::string& output_blueprint_id,
                                            const std::string& datacore_1,
                                            const std::string& datacore_2,
                                            float success_chance,
                                            float total_time,
                                            double install_cost) {
    auto* entity = world_->getEntity(lab_entity_id);
    if (!entity) return "";

    auto* lab = entity->getComponent<components::ResearchLab>();
    if (!lab) return "";

    if (lab->activeJobCount() >= lab->max_jobs) return "";

    auto* owner = world_->getEntity(owner_id);
    if (owner) {
        auto* player = owner->getComponent<components::Player>();
        if (player) {
            if (player->isk < install_cost) return "";
            player->isk -= install_cost;
        }
    }

    components::ResearchLab::ResearchJob job;
    job.job_id = "res_" + std::to_string(++job_counter_);
    job.blueprint_id = blueprint_id;
    job.owner_id = owner_id;
    job.research_type = "invention";
    job.output_blueprint_id = output_blueprint_id;
    job.datacore_1 = datacore_1;
    job.datacore_2 = datacore_2;
    job.success_chance = success_chance;
    job.total_time = total_time;
    job.time_remaining = total_time;
    job.install_cost = install_cost;
    job.status = "active";

    lab->jobs.push_back(job);
    return job.job_id;
}

int ResearchSystem::getActiveJobCount(const std::string& lab_entity_id) {
    auto* entity = world_->getEntity(lab_entity_id);
    if (!entity) return 0;

    auto* lab = entity->getComponent<components::ResearchLab>();
    if (!lab) return 0;

    return lab->activeJobCount();
}

int ResearchSystem::getCompletedJobCount(const std::string& lab_entity_id) {
    auto* entity = world_->getEntity(lab_entity_id);
    if (!entity) return 0;

    auto* lab = entity->getComponent<components::ResearchLab>();
    if (!lab) return 0;

    int count = 0;
    for (const auto& job : lab->jobs)
        if (job.status == "completed") ++count;
    return count;
}

int ResearchSystem::getFailedJobCount(const std::string& lab_entity_id) {
    auto* entity = world_->getEntity(lab_entity_id);
    if (!entity) return 0;

    auto* lab = entity->getComponent<components::ResearchLab>();
    if (!lab) return 0;

    int count = 0;
    for (const auto& job : lab->jobs)
        if (job.status == "failed") ++count;
    return count;
}

} // namespace systems
} // namespace eve
