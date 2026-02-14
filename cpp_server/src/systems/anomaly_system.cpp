#include "systems/anomaly_system.h"
#include "ecs/world.h"
#include <cmath>
#include <algorithm>
#include <sstream>

namespace atlas {
namespace systems {

AnomalySystem::AnomalySystem(ecs::World* world)
    : System(world) {
}

void AnomalySystem::update(float delta_time) {
    // Tick despawn timers on all anomalies
    for (auto* entity : world_->getAllEntities()) {
        auto* anom = entity->getComponent<components::Anomaly>();
        if (!anom) continue;
        if (anom->completed) continue;

        anom->despawn_timer -= delta_time;
        if (anom->despawn_timer <= 0.0f) {
            anom->completed = true;  // mark for cleanup
        }
    }
}

// -----------------------------------------------------------------------
// Anomaly generation
// -----------------------------------------------------------------------

int AnomalySystem::generateAnomalies(const std::string& system_id,
                                      uint32_t seed,
                                      float security) {
    // Number of anomalies inversely proportional to security
    // Highsec (1.0): 2-3 anomalies
    // Lowsec  (0.4): 4-6 anomalies
    // Nullsec (0.0): 6-8 anomalies
    float sec_clamped = std::clamp(security, 0.0f, 1.0f);
    int base_count = static_cast<int>(3.0f + (1.0f - sec_clamped) * 5.0f);

    // Use seed to add variance of ±1
    int variance = static_cast<int>(seed % 3) - 1;  // -1, 0, or +1
    int count = std::max(1, base_count + variance);

    auto base_difficulty = difficultyFromSecurity(security);

    for (int i = 0; i < count; ++i) {
        // Deterministic per-anomaly sub-seed
        uint32_t sub_seed = seed ^ (static_cast<uint32_t>(i) * 2654435761u);

        auto type = typeFromSeed(sub_seed);
        auto diff = base_difficulty;

        // Some anomalies within a system can be ±1 difficulty level
        int diff_shift = static_cast<int>(sub_seed % 5);
        if (diff_shift == 0 && static_cast<int>(diff) > 0) {
            diff = static_cast<components::Anomaly::Difficulty>(static_cast<int>(diff) - 1);
        } else if (diff_shift == 4 && static_cast<int>(diff) < 4) {
            diff = static_cast<components::Anomaly::Difficulty>(static_cast<int>(diff) + 1);
        }

        // Deterministic position from seed
        float px = static_cast<float>((sub_seed >> 0) & 0xFFFF) / 65535.0f * 200000.0f - 100000.0f;
        float py = static_cast<float>((sub_seed >> 16) & 0xFFFF) / 65535.0f * 40000.0f - 20000.0f;
        float pz = static_cast<float>((sub_seed >> 8) & 0xFFFF) / 65535.0f * 200000.0f - 100000.0f;

        // Signature strength: higher difficulty = weaker signal (harder to scan)
        float sig = 1.0f - static_cast<float>(static_cast<int>(diff)) * 0.15f;
        sig = std::clamp(sig, 0.1f, 1.0f);

        // Create entity
        std::string anom_id = "anomaly_" + system_id + "_" + std::to_string(anomaly_counter_++);
        auto* entity = world_->createEntity(anom_id);
        if (!entity) continue;

        auto anom_comp = std::make_unique<components::Anomaly>();
        auto* anom = anom_comp.get();
        entity->addComponent(std::move(anom_comp));
        anom->anomaly_id = anom_id;
        anom->anomaly_name = generateName(type, i);
        anom->system_id = system_id;
        anom->type = type;
        anom->difficulty = diff;
        anom->signature_strength = sig;
        anom->x = px;
        anom->y = py;
        anom->z = pz;
        anom->npc_count = npcCountFromDifficulty(diff);
        anom->loot_multiplier = lootMultiplierFromDifficulty(diff);
        anom->discovered = false;
        anom->completed = false;
        anom->despawn_timer = 3600.0f + static_cast<float>(sub_seed % 3600);  // ~1-2 hours

        // Attach visual cue for client-side rendering
        auto cue_comp = std::make_unique<components::AnomalyVisualCue>();
        cue_comp->anomaly_id = anom_id;
        cue_comp->cue_type = cueTypeFromAnomalyType(type);
        cue_comp->intensity = sig;
        cue_comp->radius = 500.0f + anom->loot_multiplier * 200.0f;
        entity->addComponent(std::move(cue_comp));
    }

    return count;
}

std::vector<std::string> AnomalySystem::getAnomaliesInSystem(
        const std::string& system_id) const {
    std::vector<std::string> result;
    for (auto* entity : world_->getAllEntities()) {
        auto* anom = entity->getComponent<components::Anomaly>();
        if (anom && anom->system_id == system_id && !anom->completed) {
            result.push_back(entity->getId());
        }
    }
    return result;
}

int AnomalySystem::getActiveAnomalyCount(const std::string& system_id) const {
    int count = 0;
    for (auto* entity : world_->getAllEntities()) {
        auto* anom = entity->getComponent<components::Anomaly>();
        if (anom && anom->system_id == system_id && !anom->completed) {
            ++count;
        }
    }
    return count;
}

bool AnomalySystem::completeAnomaly(const std::string& anomaly_id) {
    auto* entity = world_->getEntity(anomaly_id);
    if (!entity) return false;
    auto* anom = entity->getComponent<components::Anomaly>();
    if (!anom) return false;
    anom->completed = true;
    return true;
}

// -----------------------------------------------------------------------
// Static helpers
// -----------------------------------------------------------------------

components::Anomaly::Difficulty AnomalySystem::difficultyFromSecurity(float security) {
    float sec = std::clamp(security, 0.0f, 1.0f);
    if (sec >= 0.8f) return components::Anomaly::Difficulty::Trivial;
    if (sec >= 0.6f) return components::Anomaly::Difficulty::Easy;
    if (sec >= 0.4f) return components::Anomaly::Difficulty::Medium;
    if (sec >= 0.2f) return components::Anomaly::Difficulty::Hard;
    return components::Anomaly::Difficulty::Deadly;
}

int AnomalySystem::npcCountFromDifficulty(components::Anomaly::Difficulty diff) {
    switch (diff) {
        case components::Anomaly::Difficulty::Trivial: return 3;
        case components::Anomaly::Difficulty::Easy:    return 5;
        case components::Anomaly::Difficulty::Medium:  return 8;
        case components::Anomaly::Difficulty::Hard:    return 12;
        case components::Anomaly::Difficulty::Deadly:  return 18;
    }
    return 5;
}

float AnomalySystem::lootMultiplierFromDifficulty(components::Anomaly::Difficulty diff) {
    switch (diff) {
        case components::Anomaly::Difficulty::Trivial: return 0.5f;
        case components::Anomaly::Difficulty::Easy:    return 0.8f;
        case components::Anomaly::Difficulty::Medium:  return 1.0f;
        case components::Anomaly::Difficulty::Hard:    return 1.5f;
        case components::Anomaly::Difficulty::Deadly:  return 2.5f;
    }
    return 1.0f;
}

components::Anomaly::Type AnomalySystem::typeFromSeed(uint32_t val) {
    int bucket = static_cast<int>(val % 100);
    // Distribution: Combat 40%, Mining 25%, Data 15%, Relic 10%, Gas 7%, Wormhole 3%
    if (bucket < 40) return components::Anomaly::Type::Combat;
    if (bucket < 65) return components::Anomaly::Type::Mining;
    if (bucket < 80) return components::Anomaly::Type::Data;
    if (bucket < 90) return components::Anomaly::Type::Relic;
    if (bucket < 97) return components::Anomaly::Type::Gas;
    return components::Anomaly::Type::Wormhole;
}

std::string AnomalySystem::generateName(components::Anomaly::Type type, int index) {
    std::string prefix;
    switch (type) {
        case components::Anomaly::Type::Combat:   prefix = "Pirate Hideout";  break;
        case components::Anomaly::Type::Mining:    prefix = "Rich Asteroid Cluster"; break;
        case components::Anomaly::Type::Data:      prefix = "Abandoned Research Post"; break;
        case components::Anomaly::Type::Relic:     prefix = "Ancient Ruins"; break;
        case components::Anomaly::Type::Gas:       prefix = "Nebula Pocket"; break;
        case components::Anomaly::Type::Wormhole:  prefix = "Unstable Wormhole"; break;
    }
    // Vary name by index using roman-numeral-style suffix
    static const char* suffixes[] = {"I", "II", "III", "IV", "V", "VI", "VII", "VIII"};
    int si = index % 8;
    return prefix + " " + suffixes[si];
}

components::AnomalyVisualCue::CueType AnomalySystem::cueTypeFromAnomalyType(
    components::Anomaly::Type type) {
    switch (type) {
        case components::Anomaly::Type::Wormhole:
            return components::AnomalyVisualCue::CueType::GravityLens;
        case components::Anomaly::Type::Gas:
            return components::AnomalyVisualCue::CueType::ParticleCloud;
        case components::Anomaly::Type::Combat:
            return components::AnomalyVisualCue::CueType::EnergyPulse;
        case components::Anomaly::Type::Mining:
            return components::AnomalyVisualCue::CueType::Shimmer;
        case components::Anomaly::Type::Data:
            return components::AnomalyVisualCue::CueType::ElectricArc;
        case components::Anomaly::Type::Relic:
            return components::AnomalyVisualCue::CueType::Shimmer;
        default:
            return components::AnomalyVisualCue::CueType::None;
    }
}

} // namespace systems
} // namespace atlas
