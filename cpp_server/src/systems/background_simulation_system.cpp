#include "systems/background_simulation_system.h"
#include "ecs/world.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

BackgroundSimulationSystem::BackgroundSimulationSystem(ecs::World* world)
    : System(world) {
}

void BackgroundSimulationSystem::update(float delta_time) {
    auto entities = world_->getEntities<components::SimStarSystemState>();
    for (auto* entity : entities) {
        auto* state = entity->getComponent<components::SimStarSystemState>();
        if (!state) continue;

        updateSystemState(state, delta_time);
        evaluateEvents(entity->getId(), state);
        tickEventTimers(state, delta_time);
    }
}

// -----------------------------------------------------------------------
// State drift: values move toward equilibrium over time
// -----------------------------------------------------------------------

void BackgroundSimulationSystem::updateSystemState(
        components::SimStarSystemState* state, float dt) {
    // Threat naturally decays toward 0
    if (state->threat_level > 0.0f) {
        state->threat_level = std::max(0.0f,
            state->threat_level - threat_decay_rate * dt);
    }

    // Economy recovers toward 0.5 baseline
    if (state->economic_index < 0.5f) {
        state->economic_index = std::min(0.5f,
            state->economic_index + economic_recovery_rate * dt);
    }

    // Resources slowly regenerate toward 1.0
    if (state->resource_availability < 1.0f) {
        state->resource_availability = std::min(1.0f,
            state->resource_availability + resource_regen_rate * dt);
    }

    // Traffic drifts toward baseline (0.5)
    float traffic_diff = 0.5f - state->traffic_level;
    state->traffic_level += traffic_diff * traffic_fluctuation_rate * dt;

    // Pirate activity increases when security is low
    if (state->security_level < 0.3f) {
        state->pirate_activity = std::min(1.0f,
            state->pirate_activity + 0.01f * dt);
    } else {
        // Pirate activity decays in secure systems
        state->pirate_activity = std::max(0.0f,
            state->pirate_activity - 0.005f * dt);
    }

    // Price modifier responds to supply/demand
    state->price_modifier = 1.0f + (1.0f - state->resource_availability) * 0.5f
                                 - (state->trade_volume - 0.5f) * 0.2f;
    state->price_modifier = std::clamp(state->price_modifier, 0.5f, 2.0f);
}

// -----------------------------------------------------------------------
// Threshold-based event evaluation
// -----------------------------------------------------------------------

void BackgroundSimulationSystem::evaluateEvents(
        const std::string& /*system_id*/,
        components::SimStarSystemState* state) {
    // Pirate surge: high pirate activity triggers surge
    if (!state->pirate_surge &&
        state->pirate_activity >= pirate_surge_threshold) {
        state->pirate_surge = true;
        state->event_timer = std::max(state->event_timer, event_duration);
    }

    // Resource shortage: low availability triggers shortage
    if (!state->resource_shortage &&
        state->resource_availability <= shortage_threshold) {
        state->resource_shortage = true;
        state->event_timer = std::max(state->event_timer, event_duration);
    }

    // Lockdown: extreme threat triggers lockdown
    if (!state->lockdown &&
        state->threat_level >= lockdown_threat_threshold) {
        state->lockdown = true;
        state->event_timer = std::max(state->event_timer, event_duration);
    }
}

// -----------------------------------------------------------------------
// Event timer countdown — clears events when timer expires
// -----------------------------------------------------------------------

void BackgroundSimulationSystem::tickEventTimers(
        components::SimStarSystemState* state, float dt) {
    if (state->event_timer > 0.0f) {
        state->event_timer -= dt;
        if (state->event_timer <= 0.0f) {
            state->event_timer = 0.0f;
            // Clear events whose underlying conditions have subsided
            if (state->pirate_activity < pirate_surge_threshold) {
                state->pirate_surge = false;
            }
            if (state->resource_availability > shortage_threshold) {
                state->resource_shortage = false;
            }
            if (state->threat_level < lockdown_threat_threshold) {
                state->lockdown = false;
            }
        }
    }
}

// -----------------------------------------------------------------------
// Query API
// -----------------------------------------------------------------------

const components::SimStarSystemState*
BackgroundSimulationSystem::getSystemState(const std::string& system_id) const {
    auto* entity = world_->getEntity(system_id);
    if (!entity) return nullptr;
    return entity->getComponent<components::SimStarSystemState>();
}

bool BackgroundSimulationSystem::isEventActive(
        const std::string& system_id,
        const std::string& event_type) const {
    auto* state = getSystemState(system_id);
    if (!state) return false;

    if (event_type == "pirate_surge") return state->pirate_surge;
    if (event_type == "resource_shortage") return state->resource_shortage;
    if (event_type == "lockdown") return state->lockdown;
    return false;
}

std::vector<std::string>
BackgroundSimulationSystem::getSystemsWithEvent(
        const std::string& event_type) const {
    std::vector<std::string> result;
    auto entities = const_cast<ecs::World*>(world_)->getEntities<components::SimStarSystemState>();
    for (auto* entity : entities) {
        if (isEventActive(entity->getId(), event_type)) {
            result.push_back(entity->getId());
        }
    }
    return result;
}

} // namespace systems
} // namespace atlas
