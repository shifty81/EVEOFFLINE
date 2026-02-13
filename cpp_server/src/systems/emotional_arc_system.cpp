#include "systems/emotional_arc_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <algorithm>

namespace atlas {
namespace systems {

EmotionalArcSystem::EmotionalArcSystem(ecs::World* world)
    : System(world) {
}

void EmotionalArcSystem::update(float delta_time) {
    auto entities = world_->getEntities<components::EmotionalState>();
    for (auto* entity : entities) {
        auto* state = entity->getComponent<components::EmotionalState>();
        if (!state) continue;

        // Fatigue increases by 0.01 per minute of active play
        state->fatigue += 0.01f * (delta_time / 60.0f);
        state->fatigue = std::clamp(state->fatigue, 0.0f, 100.0f);

        // Confidence and hope drift slowly toward neutral (50)
        float drift_rate = 0.05f * (delta_time / 60.0f);
        if (state->confidence > 50.0f) {
            state->confidence = std::max(50.0f, state->confidence - drift_rate);
        } else if (state->confidence < 50.0f) {
            state->confidence = std::min(50.0f, state->confidence + drift_rate);
        }

        if (state->hope > 50.0f) {
            state->hope = std::max(50.0f, state->hope - drift_rate);
        } else if (state->hope < 50.0f) {
            state->hope = std::min(50.0f, state->hope + drift_rate);
        }
    }
}

void EmotionalArcSystem::onCombatVictory(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return;

    auto* state = entity->getComponent<components::EmotionalState>();
    if (!state) {
        entity->addComponent(std::make_unique<components::EmotionalState>());
        state = entity->getComponent<components::EmotionalState>();
    }

    state->confidence = std::clamp(state->confidence + 5.0f, 0.0f, 100.0f);
    state->hope = std::clamp(state->hope + 3.0f, 0.0f, 100.0f);
    state->fatigue = std::clamp(state->fatigue + 2.0f, 0.0f, 100.0f);
}

void EmotionalArcSystem::onCombatDefeat(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return;

    auto* state = entity->getComponent<components::EmotionalState>();
    if (!state) {
        entity->addComponent(std::make_unique<components::EmotionalState>());
        state = entity->getComponent<components::EmotionalState>();
    }

    state->confidence = std::clamp(state->confidence - 8.0f, 0.0f, 100.0f);
    state->hope = std::clamp(state->hope - 5.0f, 0.0f, 100.0f);
    state->fatigue = std::clamp(state->fatigue + 5.0f, 0.0f, 100.0f);
}

void EmotionalArcSystem::onRest(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return;

    auto* state = entity->getComponent<components::EmotionalState>();
    if (!state) {
        entity->addComponent(std::make_unique<components::EmotionalState>());
        state = entity->getComponent<components::EmotionalState>();
    }

    state->fatigue = std::max(0.0f, state->fatigue - 10.0f);
}

void EmotionalArcSystem::onPlayerTrust(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return;

    auto* state = entity->getComponent<components::EmotionalState>();
    if (!state) {
        entity->addComponent(std::make_unique<components::EmotionalState>());
        state = entity->getComponent<components::EmotionalState>();
    }

    state->trust_in_player = std::clamp(state->trust_in_player + 5.0f, 0.0f, 100.0f);
}

void EmotionalArcSystem::onPlayerBetray(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return;

    auto* state = entity->getComponent<components::EmotionalState>();
    if (!state) {
        entity->addComponent(std::make_unique<components::EmotionalState>());
        state = entity->getComponent<components::EmotionalState>();
    }

    state->trust_in_player = std::clamp(state->trust_in_player - 15.0f, 0.0f, 100.0f);
}

float EmotionalArcSystem::getConfidence(const std::string& entity_id) const {
    const auto* entity = world_->getEntity(entity_id);
    if (!entity) return 50.0f;

    const auto* state = entity->getComponent<components::EmotionalState>();
    if (!state) return 50.0f;

    return state->confidence;
}

float EmotionalArcSystem::getTrust(const std::string& entity_id) const {
    const auto* entity = world_->getEntity(entity_id);
    if (!entity) return 50.0f;

    const auto* state = entity->getComponent<components::EmotionalState>();
    if (!state) return 50.0f;

    return state->trust_in_player;
}

float EmotionalArcSystem::getFatigue(const std::string& entity_id) const {
    const auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;

    const auto* state = entity->getComponent<components::EmotionalState>();
    if (!state) return 0.0f;

    return state->fatigue;
}

float EmotionalArcSystem::getHope(const std::string& entity_id) const {
    const auto* entity = world_->getEntity(entity_id);
    if (!entity) return 50.0f;

    const auto* state = entity->getComponent<components::EmotionalState>();
    if (!state) return 50.0f;

    return state->hope;
}

} // namespace systems
} // namespace atlas
