#include "systems/ambient_traffic_system.h"
#include "ecs/world.h"
#include <algorithm>

namespace atlas {
namespace systems {

AmbientTrafficSystem::AmbientTrafficSystem(ecs::World* world)
    : System(world) {
}

void AmbientTrafficSystem::update(float delta_time) {
    auto entities = world_->getEntities<components::AmbientTrafficState>();
    for (auto* entity : entities) {
        auto* traffic = entity->getComponent<components::AmbientTrafficState>();
        auto* state   = entity->getComponent<components::SimStarSystemState>();
        if (!traffic || !state) continue;

        traffic->spawn_timer -= delta_time;
        if (traffic->spawn_timer <= 0.0f) {
            traffic->spawn_timer = spawn_interval;
            evaluateSpawns(entity, traffic, state);
        }
    }
}

// -----------------------------------------------------------------------
// Evaluate what NPC traffic to spawn based on system state
// -----------------------------------------------------------------------

void AmbientTrafficSystem::evaluateSpawns(
        ecs::Entity* /*entity*/,
        components::AmbientTrafficState* traffic,
        const components::SimStarSystemState* state) {

    // Don't exceed traffic cap
    if (traffic->active_traffic_count >= max_traffic_per_system) return;

    int slots = max_traffic_per_system - traffic->active_traffic_count;

    // Economy drives trader/hauler spawns
    if (state->economic_index >= trader_economy_threshold && slots > 0) {
        traffic->pending_spawns.push_back("trader");
        traffic->active_traffic_count++;
        slots--;
    }
    if (state->trade_volume >= 0.5f && slots > 0) {
        traffic->pending_spawns.push_back("hauler");
        traffic->active_traffic_count++;
        slots--;
    }

    // Resources drive miner spawns
    if (state->resource_availability >= miner_resource_threshold && slots > 0) {
        traffic->pending_spawns.push_back("miner");
        traffic->active_traffic_count++;
        slots--;
    }

    // Pirate activity drives pirate spawns
    if (state->pirate_activity >= pirate_activity_threshold && slots > 0) {
        traffic->pending_spawns.push_back("pirate");
        traffic->active_traffic_count++;
        slots--;
    }

    // Security drives patrol spawns
    if (state->security_level >= 0.5f && slots > 0) {
        traffic->pending_spawns.push_back("patrol");
        traffic->active_traffic_count++;
        slots--;
    }
}

// -----------------------------------------------------------------------
// Query API
// -----------------------------------------------------------------------

std::vector<std::string>
AmbientTrafficSystem::getPendingSpawns(const std::string& system_id) const {
    auto* entity = world_->getEntity(system_id);
    if (!entity) return {};

    auto* traffic = entity->getComponent<components::AmbientTrafficState>();
    if (!traffic) return {};

    return traffic->pending_spawns;
}

int AmbientTrafficSystem::getActiveTrafficCount(const std::string& system_id) const {
    auto* entity = world_->getEntity(system_id);
    if (!entity) return 0;

    auto* traffic = entity->getComponent<components::AmbientTrafficState>();
    if (!traffic) return 0;

    return traffic->active_traffic_count;
}

void AmbientTrafficSystem::clearPendingSpawns(const std::string& system_id) {
    auto* entity = world_->getEntity(system_id);
    if (!entity) return;

    auto* traffic = entity->getComponent<components::AmbientTrafficState>();
    if (!traffic) return;

    traffic->pending_spawns.clear();
}

} // namespace systems
} // namespace atlas
