#include "systems/npc_intent_system.h"
#include "ecs/world.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

NPCIntentSystem::NPCIntentSystem(ecs::World* world)
    : System(world) {
}

void NPCIntentSystem::update(float delta_time) {
    auto entities = world_->getEntities<components::SimNPCIntent>();
    for (auto* entity : entities) {
        auto* intent = entity->getComponent<components::SimNPCIntent>();
        if (!intent) continue;

        intent->intent_duration += delta_time;

        // Tick cooldown
        if (intent->intent_cooldown > 0.0f) {
            intent->intent_cooldown -= delta_time;
            if (intent->intent_cooldown < 0.0f)
                intent->intent_cooldown = 0.0f;
        }

        evaluateIntent(entity, intent, delta_time);
    }
}

// -----------------------------------------------------------------------
// Per-entity intent evaluation
// -----------------------------------------------------------------------

void NPCIntentSystem::evaluateIntent(ecs::Entity* entity,
                                      components::SimNPCIntent* intent,
                                      float /*dt*/) {
    // Don't re-evaluate while on cooldown (unless intent is complete)
    if (intent->intent_cooldown > 0.0f && !intent->intent_complete)
        return;

    // Immediate flee check — health-based override
    auto* health = entity->getComponent<components::Health>();
    if (health && health->isAlive()) {
        float hull_pct = health->hull_hp / std::max(health->hull_max, 1.0f);
        if (hull_pct < 0.25f && intent->current_intent != components::SimNPCIntent::Intent::Flee) {
            intent->previous_intent = intent->current_intent;
            intent->current_intent = components::SimNPCIntent::Intent::Flee;
            intent->intent_duration = 0.0f;
            intent->intent_complete = false;
            intent->intent_cooldown = re_eval_interval;
            return;
        }
    }

    // Look up star system state (if entity has a SolarSystem component or
    // we find one by target_system_id)
    const components::SimStarSystemState* sys_state = nullptr;
    if (!intent->target_system_id.empty()) {
        auto* sys_entity = world_->getEntity(intent->target_system_id);
        if (sys_entity) {
            sys_state = sys_entity->getComponent<components::SimStarSystemState>();
        }
    }

    // Score every intent
    using Intent = components::SimNPCIntent::Intent;
    Intent best = Intent::Idle;
    float best_score = -1.0f;

    const Intent candidates[] = {
        Intent::Trade, Intent::Patrol, Intent::Hunt,
        Intent::Explore, Intent::Flee, Intent::Escort,
        Intent::Salvage, Intent::Mine, Intent::Haul, Intent::Dock
    };

    for (auto candidate : candidates) {
        float score = scoreForSystem(candidate, intent, sys_state, health);
        if (score > best_score) {
            best_score = score;
            best = candidate;
        }
    }

    // Only switch if the best score beats a minimum threshold
    if (best_score > 0.1f && best != intent->current_intent) {
        intent->previous_intent = intent->current_intent;
        intent->current_intent = best;
        intent->intent_duration = 0.0f;
        intent->intent_complete = false;
    }

    intent->intent_cooldown = re_eval_interval;
}

// -----------------------------------------------------------------------
// Intent scoring — combines personality weight, system state, and needs
// -----------------------------------------------------------------------

float NPCIntentSystem::scoreForSystem(
        components::SimNPCIntent::Intent intent,
        const components::SimNPCIntent* npc,
        const components::SimStarSystemState* sys,
        const components::Health* health) const {

    float base = 0.0f;
    using Intent = components::SimNPCIntent::Intent;

    switch (intent) {
    case Intent::Trade:
        base = npc->trade_weight;
        if (sys) base *= sys->economic_index;
        if (npc->cargo_fill > 0.5f) base *= 1.5f;   // want to sell
        break;

    case Intent::Patrol:
        base = npc->patrol_weight;
        if (sys) base *= (1.0f - sys->security_level); // patrol where insecure
        break;

    case Intent::Hunt:
        base = npc->hunt_weight;
        if (sys) base *= sys->pirate_activity;
        break;

    case Intent::Explore:
        base = npc->explore_weight;
        if (sys) base *= sys->resource_availability;
        break;

    case Intent::Flee:
        base = npc->flee_weight;
        if (health) {
            float hull_pct = health->hull_hp / std::max(health->hull_max, 1.0f);
            base *= (1.0f - hull_pct);
        }
        if (sys) base *= sys->threat_level;
        break;

    case Intent::Escort:
        base = npc->escort_weight;
        if (sys) base *= sys->threat_level * 0.5f;
        break;

    case Intent::Salvage:
        base = npc->salvage_weight;
        if (sys) base *= sys->threat_level * 0.3f; // wrecks after combat
        break;

    case Intent::Mine:
        base = npc->mine_weight;
        if (sys) base *= sys->resource_availability;
        break;

    case Intent::Haul:
        base = npc->haul_weight;
        if (npc->cargo_fill > 0.8f) base *= 2.0f;  // cargo full, go haul
        if (sys) base *= sys->trade_volume;
        break;

    case Intent::Dock:
        // Dock when cargo is full or wallet target met
        base = 0.0f;
        if (npc->cargo_fill > 0.9f) base = 0.8f;
        if (npc->wallet >= npc->profit_target && npc->profit_target > 0.0)
            base = std::max(base, 0.6f);
        break;

    default:
        break;
    }

    return std::max(base, 0.0f);
}

// -----------------------------------------------------------------------
// Archetype default weights
// -----------------------------------------------------------------------

void NPCIntentSystem::applyArchetypeWeights(components::SimNPCIntent* intent) {
    if (!intent) return;

    using Archetype = components::SimNPCIntent::Archetype;

    // Reset all weights
    intent->trade_weight   = 0.1f;
    intent->patrol_weight  = 0.1f;
    intent->hunt_weight    = 0.1f;
    intent->explore_weight = 0.1f;
    intent->flee_weight    = 0.3f;  // everyone wants to survive
    intent->escort_weight  = 0.1f;
    intent->salvage_weight = 0.1f;
    intent->mine_weight    = 0.1f;
    intent->haul_weight    = 0.1f;

    switch (intent->archetype) {
    case Archetype::Trader:
        intent->trade_weight = 0.9f;
        intent->haul_weight  = 0.7f;
        intent->flee_weight  = 0.6f;
        break;
    case Archetype::Pirate:
        intent->hunt_weight   = 0.9f;
        intent->patrol_weight = 0.6f;
        intent->salvage_weight= 0.5f;
        intent->flee_weight   = 0.4f;
        break;
    case Archetype::Patrol:
        intent->patrol_weight = 0.9f;
        intent->escort_weight = 0.7f;
        intent->hunt_weight   = 0.5f;
        break;
    case Archetype::Miner:
        intent->mine_weight   = 0.9f;
        intent->haul_weight   = 0.6f;
        intent->flee_weight   = 0.7f;
        break;
    case Archetype::Hauler:
        intent->haul_weight   = 0.9f;
        intent->trade_weight  = 0.6f;
        intent->flee_weight   = 0.7f;
        break;
    case Archetype::Industrialist:
        intent->trade_weight   = 0.7f;
        intent->mine_weight    = 0.6f;
        intent->haul_weight    = 0.5f;
        intent->explore_weight = 0.4f;
        break;
    }
}

// -----------------------------------------------------------------------
// Force intent
// -----------------------------------------------------------------------

void NPCIntentSystem::forceIntent(const std::string& entity_id,
                                   components::SimNPCIntent::Intent intent) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return;

    auto* npc = entity->getComponent<components::SimNPCIntent>();
    if (!npc) return;

    npc->previous_intent = npc->current_intent;
    npc->current_intent = intent;
    npc->intent_duration = 0.0f;
    npc->intent_complete = false;
    npc->intent_cooldown = re_eval_interval;
}

// -----------------------------------------------------------------------
// Query API
// -----------------------------------------------------------------------

components::SimNPCIntent::Intent
NPCIntentSystem::getIntent(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return components::SimNPCIntent::Intent::Idle;

    auto* npc = entity->getComponent<components::SimNPCIntent>();
    if (!npc) return components::SimNPCIntent::Intent::Idle;

    return npc->current_intent;
}

std::vector<std::string>
NPCIntentSystem::getNPCsWithIntent(
        components::SimNPCIntent::Intent intent) const {
    std::vector<std::string> result;
    auto entities = const_cast<ecs::World*>(world_)->getEntities<components::SimNPCIntent>();
    for (auto* entity : entities) {
        auto* npc = entity->getComponent<components::SimNPCIntent>();
        if (npc && npc->current_intent == intent) {
            result.push_back(entity->getId());
        }
    }
    return result;
}

std::vector<std::string>
NPCIntentSystem::getNPCsByArchetype(
        components::SimNPCIntent::Archetype archetype) const {
    std::vector<std::string> result;
    auto entities = const_cast<ecs::World*>(world_)->getEntities<components::SimNPCIntent>();
    for (auto* entity : entities) {
        auto* npc = entity->getComponent<components::SimNPCIntent>();
        if (npc && npc->archetype == archetype) {
            result.push_back(entity->getId());
        }
    }
    return result;
}

std::vector<std::pair<components::SimNPCIntent::Intent, float>>
NPCIntentSystem::scoreIntents(const std::string& entity_id) const {
    using Intent = components::SimNPCIntent::Intent;
    std::vector<std::pair<Intent, float>> scores;

    auto* entity = world_->getEntity(entity_id);
    if (!entity) return scores;

    auto* npc = entity->getComponent<components::SimNPCIntent>();
    if (!npc) return scores;

    auto* health = entity->getComponent<components::Health>();

    const components::SimStarSystemState* sys_state = nullptr;
    if (!npc->target_system_id.empty()) {
        auto* sys_entity = world_->getEntity(npc->target_system_id);
        if (sys_entity) {
            sys_state = sys_entity->getComponent<components::SimStarSystemState>();
        }
    }

    Intent candidates[] = {
        Intent::Trade, Intent::Patrol, Intent::Hunt,
        Intent::Explore, Intent::Flee, Intent::Escort,
        Intent::Salvage, Intent::Mine, Intent::Haul, Intent::Dock
    };

    for (auto candidate : candidates) {
        float s = scoreForSystem(candidate, npc, sys_state, health);
        scores.push_back({candidate, s});
    }

    std::sort(scores.begin(), scores.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });

    return scores;
}

} // namespace systems
} // namespace atlas
