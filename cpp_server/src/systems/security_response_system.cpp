#include "systems/security_response_system.h"
#include "ecs/world.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

SecurityResponseSystem::SecurityResponseSystem(ecs::World* world)
    : System(world) {
}

void SecurityResponseSystem::update(float delta_time) {
    auto entities = world_->getEntities<components::SecurityResponseState>();
    for (auto* entity : entities) {
        auto* resp  = entity->getComponent<components::SecurityResponseState>();
        auto* state = entity->getComponent<components::SimStarSystemState>();
        if (!resp || !state) continue;

        evaluateSystem(entity, resp, state, delta_time);
    }
}

// -----------------------------------------------------------------------
// Per-system security response evaluation
// -----------------------------------------------------------------------

void SecurityResponseSystem::evaluateSystem(
        ecs::Entity* /*entity*/,
        components::SecurityResponseState* resp,
        const components::SimStarSystemState* state,
        float dt) {

    // Already responding — count down duration
    if (resp->responding) {
        resp->response_timer -= dt;
        if (resp->response_timer <= 0.0f) {
            resp->responding = false;
            resp->response_timer = 0.0f;
        }
        return;
    }

    // No response in low-sec / null-sec
    if (state->security_level < security_min_level) {
        resp->response_timer = 0.0f;
        return;
    }

    // Threat below trigger threshold — reset timer
    if (state->threat_level < threat_threshold) {
        resp->response_timer = 0.0f;
        return;
    }

    // Threat above threshold — start or tick the countdown
    if (resp->response_timer <= 0.0f) {
        // Calculate delay based on security level
        float delay = base_delay * (1.0f - state->security_level * speed_factor);
        delay = std::clamp(delay, min_delay, base_delay);
        resp->response_timer = delay;
    }

    resp->response_timer -= dt;
    if (resp->response_timer <= 0.0f) {
        resp->responding = true;
        resp->response_timer = response_duration;
        resp->response_strength = state->security_level;  // stronger in high-sec
    }
}

// -----------------------------------------------------------------------
// Query API
// -----------------------------------------------------------------------

bool SecurityResponseSystem::isResponding(const std::string& system_id) const {
    auto* entity = world_->getEntity(system_id);
    if (!entity) return false;

    auto* resp = entity->getComponent<components::SecurityResponseState>();
    if (!resp) return false;

    return resp->responding;
}

float SecurityResponseSystem::getResponseTimer(const std::string& system_id) const {
    auto* entity = world_->getEntity(system_id);
    if (!entity) return 0.0f;

    auto* resp = entity->getComponent<components::SecurityResponseState>();
    if (!resp) return 0.0f;

    return resp->response_timer;
}

std::vector<std::string>
SecurityResponseSystem::getRespondingSystems() const {
    std::vector<std::string> result;
    auto entities = const_cast<ecs::World*>(world_)->getEntities<components::SecurityResponseState>();
    for (auto* entity : entities) {
        auto* resp = entity->getComponent<components::SecurityResponseState>();
        if (resp && resp->responding) {
            result.push_back(entity->getId());
        }
    }
    return result;
}

} // namespace systems
} // namespace atlas
