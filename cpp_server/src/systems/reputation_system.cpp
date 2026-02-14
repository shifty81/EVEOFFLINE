#include "systems/reputation_system.h"
#include "ecs/world.h"
#include <algorithm>

namespace atlas {
namespace systems {

ReputationSystem::ReputationSystem(ecs::World* world)
    : System(world) {
}

void ReputationSystem::update(float /*delta_time*/) {
    // No-op: reputation changes are event-driven
}

// -----------------------------------------------------------------------
// Faction standing modification with derived effects
// -----------------------------------------------------------------------

void ReputationSystem::modifyFactionStanding(const std::string& entity_id,
                                             const std::string& faction,
                                             float change) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return;

    auto* standings = entity->getComponent<components::Standings>();
    if (!standings) return;

    // Apply the direct standing change
    components::Standings::modifyStanding(standings->faction_standings, faction, change);

    // Apply derived effects for every related faction
    for (const auto& [key, disposition] : faction_relationships_) {
        // Find entries that involve the changed faction
        std::string other_faction;
        auto sep = key.find(':');
        if (sep == std::string::npos) continue;

        std::string fa = key.substr(0, sep);
        std::string fb = key.substr(sep + 1);

        if (fa == faction) {
            other_faction = fb;
        } else if (fb == faction) {
            other_faction = fa;
        } else {
            continue;
        }

        // derived_change = change * disposition * 0.5
        float derived_change = change * disposition * 0.5f;
        if (derived_change == 0.0f) continue;

        components::Standings::modifyStanding(standings->faction_standings,
                                              other_faction, derived_change);
    }
}

// -----------------------------------------------------------------------
// Agent access check
// -----------------------------------------------------------------------

bool ReputationSystem::hasAgentAccess(const std::string& entity_id,
                                      const std::string& faction,
                                      float required_standing) const {
    return getEffectiveStanding(entity_id, faction) >= required_standing;
}

// -----------------------------------------------------------------------
// Effective standing (derived effects already baked in)
// -----------------------------------------------------------------------

float ReputationSystem::getEffectiveStanding(const std::string& entity_id,
                                             const std::string& faction) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;

    auto* standings = entity->getComponent<components::Standings>();
    if (!standings) return 0.0f;

    auto it = standings->faction_standings.find(faction);
    if (it != standings->faction_standings.end()) {
        return it->second;
    }
    return 0.0f;
}

// -----------------------------------------------------------------------
// Faction relationship setup
// -----------------------------------------------------------------------

void ReputationSystem::installFactionRelationships() {
    faction_relationships_.clear();

    // Player faction relationships
    faction_relationships_[pairKey("Solari", "Veyren")]   = -0.5f;  // rivals
    faction_relationships_[pairKey("Solari", "Aurelian")] =  0.3f;  // friendly
    faction_relationships_[pairKey("Solari", "Keldari")]  =  0.0f;  // neutral
    faction_relationships_[pairKey("Veyren", "Aurelian")] =  0.0f;  // neutral
    faction_relationships_[pairKey("Veyren", "Keldari")]  =  0.3f;  // friendly
    faction_relationships_[pairKey("Aurelian", "Keldari")]= -0.3f;  // cool

    // Pirate factions
    const std::string pirates[] = {
        "Serpentis", "Guristas", "Blood Raiders", "Sansha", "Angel Cartel"
    };
    const std::string player_factions[] = {
        "Solari", "Veyren", "Aurelian", "Keldari"
    };

    // All player factions vs pirate factions: hostile
    for (const auto& pf : player_factions) {
        for (const auto& pi : pirates) {
            faction_relationships_[pairKey(pf, pi)] = -1.0f;
        }
    }

    // Pirate factions vs each other: neutral
    for (size_t i = 0; i < 5; ++i) {
        for (size_t j = i + 1; j < 5; ++j) {
            faction_relationships_[pairKey(pirates[i], pirates[j])] = 0.0f;
        }
    }
}

float ReputationSystem::getFactionDisposition(const std::string& faction_a,
                                              const std::string& faction_b) const {
    auto it = faction_relationships_.find(pairKey(faction_a, faction_b));
    if (it != faction_relationships_.end()) {
        return it->second;
    }
    return 0.0f;  // default: neutral
}

// -----------------------------------------------------------------------
// Helper: sorted pair key for consistent bidirectional lookup
// -----------------------------------------------------------------------

std::string ReputationSystem::pairKey(const std::string& a,
                                      const std::string& b) const {
    if (a < b) return a + ":" + b;
    return b + ":" + a;
}

} // namespace systems
} // namespace atlas
