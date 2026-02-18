#ifndef EVE_SYSTEMS_NPC_INTENT_SYSTEM_H
#define EVE_SYSTEMS_NPC_INTENT_SYSTEM_H

#include "ecs/system.h"
#include "components/game_components.h"
#include <string>
#include <vector>
#include <utility>

namespace atlas {
namespace systems {

/**
 * @brief Evaluates and assigns intents to NPC entities
 *
 * Each tick, NPCs with a SimNPCIntent component re-evaluate their
 * current intent using scoring functions that consider:
 *   - Archetype baseline weights
 *   - Star system state (economy, threat, resources)
 *   - Personal state (wallet, cargo, health)
 *
 * Once an intent is chosen it persists until completed, interrupted
 * by danger, or the cooldown expires.
 */
class NPCIntentSystem : public ecs::System {
public:
    explicit NPCIntentSystem(ecs::World* world);
    ~NPCIntentSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "NPCIntentSystem"; }

    // --- Configuration ---
    float re_eval_interval = 30.0f;  // seconds between intent re-evaluation

    // --- Query API ---

    /** Get current intent for an NPC entity */
    components::SimNPCIntent::Intent getIntent(const std::string& entity_id) const;

    /** Get all NPCs with a specific intent */
    std::vector<std::string> getNPCsWithIntent(
        components::SimNPCIntent::Intent intent) const;

    /** Get all NPCs of a specific archetype */
    std::vector<std::string> getNPCsByArchetype(
        components::SimNPCIntent::Archetype archetype) const;

    /** Score all intents for an NPC (returns sorted intent/score pairs) */
    std::vector<std::pair<components::SimNPCIntent::Intent, float>>
    scoreIntents(const std::string& entity_id) const;

    /** Apply default weights for an archetype */
    static void applyArchetypeWeights(components::SimNPCIntent* intent);

    /** Force an intent change on an NPC */
    void forceIntent(const std::string& entity_id,
                     components::SimNPCIntent::Intent intent);

private:
    void evaluateIntent(ecs::Entity* entity,
                        components::SimNPCIntent* intent,
                        float dt);

    float scoreForSystem(components::SimNPCIntent::Intent intent,
                         const components::SimNPCIntent* npc,
                         const components::SimStarSystemState* sys_state,
                         const components::Health* health) const;
};

} // namespace systems
} // namespace atlas

#endif // EVE_SYSTEMS_NPC_INTENT_SYSTEM_H
