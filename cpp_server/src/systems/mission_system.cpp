#include "systems/mission_system.h"
#include "ecs/world.h"
#include "components/game_components.h"
#include <algorithm>

namespace eve {
namespace systems {

MissionSystem::MissionSystem(ecs::World* world)
    : System(world) {
}

void MissionSystem::update(float delta_time) {
    auto entities = world_->getEntities<components::MissionTracker>();

    for (auto* entity : entities) {
        auto* tracker = entity->getComponent<components::MissionTracker>();
        if (!tracker) continue;

        for (auto& mission : tracker->active_missions) {
            if (mission.completed || mission.failed) continue;

            // Tick down timer on timed missions
            if (mission.time_remaining > 0.0f) {
                mission.time_remaining -= delta_time;
                if (mission.time_remaining <= 0.0f) {
                    mission.time_remaining = 0.0f;
                    mission.failed = true;
                    continue;
                }
            }

            // Check completion
            if (mission.allObjectivesDone()) {
                mission.completed = true;

                // Award ISK
                auto* player = entity->getComponent<components::Player>();
                if (player) {
                    player->isk += mission.isk_reward;
                }

                // Award standing
                auto* standings = entity->getComponent<components::Standings>();
                if (standings && !mission.agent_faction.empty()) {
                    components::Standings::modifyStanding(
                        standings->faction_standings,
                        mission.agent_faction,
                        mission.standing_reward);
                }

                // Record completion
                tracker->completed_mission_ids.push_back(mission.mission_id);
            }
        }

        // Remove completed/failed missions from active list
        tracker->active_missions.erase(
            std::remove_if(tracker->active_missions.begin(),
                           tracker->active_missions.end(),
                           [](const components::MissionTracker::ActiveMission& m) {
                               return m.completed || m.failed;
                           }),
            tracker->active_missions.end());
    }
}

bool MissionSystem::acceptMission(const std::string& entity_id,
                                   const std::string& mission_id,
                                   const std::string& name,
                                   int level,
                                   const std::string& type,
                                   const std::string& agent_faction,
                                   double isk_reward,
                                   float standing_reward,
                                   float time_limit) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* tracker = entity->getComponent<components::MissionTracker>();
    if (!tracker) return false;

    // Check not already active
    for (const auto& m : tracker->active_missions) {
        if (m.mission_id == mission_id) return false;
    }

    components::MissionTracker::ActiveMission mission;
    mission.mission_id = mission_id;
    mission.name = name;
    mission.level = level;
    mission.type = type;
    mission.agent_faction = agent_faction;
    mission.isk_reward = isk_reward;
    mission.standing_reward = standing_reward;
    mission.time_remaining = time_limit;

    tracker->active_missions.push_back(std::move(mission));
    return true;
}

void MissionSystem::recordProgress(const std::string& entity_id,
                                    const std::string& mission_id,
                                    const std::string& objective_type,
                                    const std::string& target,
                                    int count) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return;

    auto* tracker = entity->getComponent<components::MissionTracker>();
    if (!tracker) return;

    for (auto& mission : tracker->active_missions) {
        if (mission.mission_id != mission_id) continue;
        for (auto& obj : mission.objectives) {
            if (obj.type == objective_type && obj.target == target && !obj.done()) {
                obj.completed = std::min(obj.completed + count, obj.required);
            }
        }
    }
}

void MissionSystem::abandonMission(const std::string& entity_id,
                                    const std::string& mission_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return;

    auto* tracker = entity->getComponent<components::MissionTracker>();
    if (!tracker) return;

    tracker->active_missions.erase(
        std::remove_if(tracker->active_missions.begin(),
                       tracker->active_missions.end(),
                       [&](const components::MissionTracker::ActiveMission& m) {
                           return m.mission_id == mission_id;
                       }),
        tracker->active_missions.end());
}

} // namespace systems
} // namespace eve
