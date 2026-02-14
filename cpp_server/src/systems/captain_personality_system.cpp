#include "systems/captain_personality_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/game_components.h"

#include <algorithm>
#include <functional>

namespace atlas {
namespace systems {

CaptainPersonalitySystem::CaptainPersonalitySystem(ecs::World* world)
    : System(world) {
}

void CaptainPersonalitySystem::update(float /*delta_time*/) {
    // Personality traits are static — nothing to tick.
}

// ---------------------------------------------------------------------------
// Deterministic pseudo-random helper: maps a string to a float in [-1, 1]
// by hashing it and normalising the result.
// ---------------------------------------------------------------------------
static float deterministicVariation(const std::string& seed, const std::string& salt) {
    std::size_t h = std::hash<std::string>{}(seed + salt);
    // Map hash to [0, 1] then shift to [-1, 1]
    float normalised = static_cast<float>(h % 1000000u) / 1000000.0f;
    return (normalised * 2.0f) - 1.0f;  // range [-1, 1]
}

static float clamp01(float v) {
    return std::max(0.0f, std::min(1.0f, v));
}

void CaptainPersonalitySystem::assignPersonality(const std::string& entity_id,
                                                  const std::string& captain_name,
                                                  const std::string& faction) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return;

    auto* personality = entity->getComponent<components::CaptainPersonality>();
    if (!personality) {
        auto comp = std::make_unique<components::CaptainPersonality>();
        entity->addComponent(std::move(comp));
        personality = entity->getComponent<components::CaptainPersonality>();
    }

    personality->captain_name = captain_name;
    personality->faction = faction;

    // Faction baseline values
    float base_aggression      = 0.5f;
    float base_sociability     = 0.5f;
    float base_optimism        = 0.5f;
    float base_professionalism = 0.5f;

    if (faction == "Solari") {
        base_aggression      = 0.45f;
        base_sociability     = 0.4f;
        base_optimism        = 0.5f;
        base_professionalism = 0.75f;
    } else if (faction == "Veyren") {
        base_aggression      = 0.55f;
        base_sociability     = 0.7f;
        base_optimism        = 0.55f;
        base_professionalism = 0.5f;
    } else if (faction == "Aurelian") {
        base_aggression      = 0.3f;
        base_sociability     = 0.65f;
        base_optimism        = 0.75f;
        base_professionalism = 0.35f;
    } else if (faction == "Keldari") {
        base_aggression      = 0.8f;
        base_sociability     = 0.45f;
        base_optimism        = 0.4f;
        base_professionalism = 0.7f;
    }

    // Apply deterministic random variation of +/- 0.15
    constexpr float kVariation = 0.15f;
    personality->aggression      = clamp01(base_aggression      + kVariation * deterministicVariation(entity_id, "aggression"));
    personality->sociability     = clamp01(base_sociability     + kVariation * deterministicVariation(entity_id, "sociability"));
    personality->optimism        = clamp01(base_optimism        + kVariation * deterministicVariation(entity_id, "optimism"));
    personality->professionalism = clamp01(base_professionalism + kVariation * deterministicVariation(entity_id, "professionalism"));

    // Psychological axes — faction baselines with same variation
    float base_loyalty      = 0.5f;
    float base_paranoia     = 0.5f;
    float base_ambition     = 0.5f;
    float base_adaptability = 0.5f;

    if (faction == "Solari") {
        base_loyalty      = 0.7f;
        base_paranoia     = 0.35f;
        base_ambition     = 0.6f;
        base_adaptability = 0.4f;
    } else if (faction == "Veyren") {
        base_loyalty      = 0.55f;
        base_paranoia     = 0.5f;
        base_ambition     = 0.45f;
        base_adaptability = 0.65f;
    } else if (faction == "Aurelian") {
        base_loyalty      = 0.45f;
        base_paranoia     = 0.3f;
        base_ambition     = 0.55f;
        base_adaptability = 0.75f;
    } else if (faction == "Keldari") {
        base_loyalty      = 0.6f;
        base_paranoia     = 0.7f;
        base_ambition     = 0.7f;
        base_adaptability = 0.35f;
    }

    personality->loyalty      = clamp01(base_loyalty      + kVariation * deterministicVariation(entity_id, "loyalty"));
    personality->paranoia     = clamp01(base_paranoia     + kVariation * deterministicVariation(entity_id, "paranoia"));
    personality->ambition     = clamp01(base_ambition     + kVariation * deterministicVariation(entity_id, "ambition"));
    personality->adaptability = clamp01(base_adaptability + kVariation * deterministicVariation(entity_id, "adaptability"));
}

void CaptainPersonalitySystem::setPersonalityTrait(const std::string& entity_id,
                                                    const std::string& trait,
                                                    float value) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return;

    auto* personality = entity->getComponent<components::CaptainPersonality>();
    if (!personality) return;

    float clamped = clamp01(value);

    if (trait == "aggression") {
        personality->aggression = clamped;
    } else if (trait == "sociability") {
        personality->sociability = clamped;
    } else if (trait == "optimism") {
        personality->optimism = clamped;
    } else if (trait == "professionalism") {
        personality->professionalism = clamped;
    } else if (trait == "loyalty") {
        personality->loyalty = clamped;
    } else if (trait == "paranoia") {
        personality->paranoia = clamped;
    } else if (trait == "ambition") {
        personality->ambition = clamped;
    } else if (trait == "adaptability") {
        personality->adaptability = clamped;
    }
}

float CaptainPersonalitySystem::getPersonalityTrait(const std::string& entity_id,
                                                     const std::string& trait) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;

    auto* personality = entity->getComponent<components::CaptainPersonality>();
    if (!personality) return 0.0f;

    if (trait == "aggression")      return personality->aggression;
    if (trait == "sociability")     return personality->sociability;
    if (trait == "optimism")        return personality->optimism;
    if (trait == "professionalism") return personality->professionalism;
    if (trait == "loyalty")         return personality->loyalty;
    if (trait == "paranoia")        return personality->paranoia;
    if (trait == "ambition")        return personality->ambition;
    if (trait == "adaptability")    return personality->adaptability;

    return 0.0f;
}

std::string CaptainPersonalitySystem::getCaptainFaction(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return "";

    auto* personality = entity->getComponent<components::CaptainPersonality>();
    if (!personality) return "";

    return personality->faction;
}

} // namespace systems
} // namespace atlas
