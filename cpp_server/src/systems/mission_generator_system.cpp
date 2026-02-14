#include "systems/mission_generator_system.h"
#include "ecs/world.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

MissionGeneratorSystem::MissionGeneratorSystem(ecs::World* world,
                                               MissionTemplateSystem* templates)
    : System(world), templates_(templates) {
}

void MissionGeneratorSystem::update(float /*delta_time*/) {
    // Missions are generated on demand, not per tick.
}

// ---------------------------------------------------------------------------
// generateMissionsForSystem
// ---------------------------------------------------------------------------

int MissionGeneratorSystem::generateMissionsForSystem(
        const std::string& system_id, uint32_t seed) {

    // Clear any previously generated missions for this system
    system_missions_[system_id].clear();

    // Look up system entity for DifficultyZone and SystemResources
    auto* sys_entity = world_->getEntity(system_id);
    float security = 0.5f;
    bool has_minerals = false;
    bool has_anomalies = false;

    if (sys_entity) {
        auto* dz = sys_entity->getComponent<components::DifficultyZone>();
        if (dz) {
            security = dz->security_status;
        }

        auto* res = sys_entity->getComponent<components::SystemResources>();
        if (res && !res->resources.empty()) {
            has_minerals = (res->totalRemaining() > 0.0f);
        }
    }

    // Check for anomalies in this system
    for (auto* entity : world_->getAllEntities()) {
        auto* anom = entity->getComponent<components::Anomaly>();
        if (anom && anom->system_id == system_id && !anom->completed) {
            has_anomalies = true;
            break;
        }
    }

    // Determine which mission types and levels to generate.
    // Lower security → higher level missions available.
    int max_level = 1;
    if (security < 0.8f) max_level = 2;
    if (security < 0.6f) max_level = 3;
    if (security < 0.4f) max_level = 4;
    if (security < 0.2f) max_level = 5;

    // Collect candidate template ids using a permissive query
    // (empty faction, 0 standing to get all templates that have no restriction)
    std::vector<std::string> all_candidates;
    for (int lvl = 1; lvl <= max_level; ++lvl) {
        auto ids = templates_->getTemplatesForFaction("", 0.0f, lvl);
        for (auto& id : ids) {
            all_candidates.push_back(std::move(id));
        }
    }

    // Filter by system conditions
    std::vector<std::string> suitable;
    for (const auto& tid : all_candidates) {
        // Find the template to check its type
        const components::MissionTemplate* tpl = nullptr;
        for (auto* entity : world_->getEntities<components::MissionTemplate>()) {
            auto* candidate = entity->getComponent<components::MissionTemplate>();
            if (candidate && candidate->template_id == tid) {
                tpl = candidate;
                break;
            }
        }
        if (!tpl) continue;

        // Always include combat and courier
        if (tpl->type == "combat" || tpl->type == "courier") {
            suitable.push_back(tid);
            continue;
        }

        // Mining missions only if system has mineral deposits
        if (tpl->type == "mining" && has_minerals) {
            suitable.push_back(tid);
            continue;
        }

        // Exploration missions only if system has anomalies
        if (tpl->type == "exploration" && has_anomalies) {
            suitable.push_back(tid);
            continue;
        }

        // Trade missions always available
        if (tpl->type == "trade") {
            suitable.push_back(tid);
            continue;
        }
    }

    // Generate concrete missions from suitable templates
    // Use seed to deterministically select a subset if there are many
    uint32_t rng = seed;
    int count = 0;

    for (const auto& tid : suitable) {
        auto mission = templates_->generateMissionFromTemplate(tid, system_id, "");

        AvailableMission am;
        am.template_id = tid;
        am.system_id = system_id;
        am.mission = std::move(mission);

        system_missions_[system_id].push_back(std::move(am));
        ++count;

        // Advance RNG (for potential future use in subset selection)
        rng = rng * 1664525u + 1013904223u;
    }

    return count;
}

// ---------------------------------------------------------------------------
// getAvailableMissions
// ---------------------------------------------------------------------------

std::vector<MissionGeneratorSystem::AvailableMission>
MissionGeneratorSystem::getAvailableMissions(const std::string& system_id) const {
    auto it = system_missions_.find(system_id);
    if (it != system_missions_.end()) {
        return it->second;
    }
    return {};
}

// ---------------------------------------------------------------------------
// offerMissionToPlayer
// ---------------------------------------------------------------------------

bool MissionGeneratorSystem::offerMissionToPlayer(
        const std::string& player_id,
        const std::string& system_id,
        int mission_index) {

    // Validate mission index
    auto it = system_missions_.find(system_id);
    if (it == system_missions_.end()) return false;

    auto& missions = it->second;
    if (mission_index < 0 || mission_index >= static_cast<int>(missions.size()))
        return false;

    // Get the player entity and their MissionTracker
    auto* player_entity = world_->getEntity(player_id);
    if (!player_entity) return false;

    auto* tracker = player_entity->getComponent<components::MissionTracker>();
    if (!tracker) return false;

    // Copy the mission into the player's active missions
    tracker->active_missions.push_back(missions[static_cast<size_t>(mission_index)].mission);

    // Remove the offered mission from the available list
    missions.erase(missions.begin() + mission_index);

    return true;
}

} // namespace systems
} // namespace atlas
