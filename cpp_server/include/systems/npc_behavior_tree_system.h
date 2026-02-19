#ifndef EVE_SYSTEMS_NPC_BEHAVIOR_TREE_SYSTEM_H
#define EVE_SYSTEMS_NPC_BEHAVIOR_TREE_SYSTEM_H

#include "ecs/system.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <string>
#include <vector>

namespace atlas {
namespace systems {

/**
 * @brief Per-archetype behavior tree execution for NPCs
 *
 * Each NPC's SimNPCIntent drives a per-archetype behavior tree that
 * steps through ordered phases.  The tree maps high-level intents
 * (Trade, Mine, Patrol …) into concrete behavioral phases that the
 * AI system can act on.
 *
 * Phase progression examples:
 *   Trader  Trade intent → FindGoods → TravelToMarket → SellGoods → Idle
 *   Miner   Mine  intent → FindDeposit → MineOre → HaulToStation → Idle
 *   Pirate  Hunt  intent → SearchTargets → Approach → Engage → Loot → Idle
 *   Patrol  Patrol intent → PickWaypoint → TravelTo → ScanArea → PickWaypoint
 */
class NPCBehaviorTreeSystem : public ecs::System {
public:
    explicit NPCBehaviorTreeSystem(ecs::World* world);
    ~NPCBehaviorTreeSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "NPCBehaviorTreeSystem"; }

    // --- Query API ---

    /** Get current behavior phase for an NPC entity */
    std::string getCurrentPhase(const std::string& entity_id) const;

    /** Get elapsed time in the current phase */
    float getPhaseElapsed(const std::string& entity_id) const;

    /** Check if an NPC has completed its current behavior tree */
    bool isTreeComplete(const std::string& entity_id) const;

    /** Reset the behavior tree for an NPC (restarts from phase 0) */
    void resetTree(const std::string& entity_id);

    /** Get the full phase list for a given archetype + intent combo */
    static std::vector<std::string> getPhasesForIntent(
        components::SimNPCIntent::Archetype archetype,
        components::SimNPCIntent::Intent intent);

private:
    void tickBehavior(ecs::Entity* entity,
                      components::NPCBehaviorState* behavior,
                      components::SimNPCIntent* intent,
                      float dt);

    bool shouldAdvancePhase(const components::NPCBehaviorState* behavior,
                            const components::SimNPCIntent* intent) const;
};

} // namespace systems
} // namespace atlas

#endif // EVE_SYSTEMS_NPC_BEHAVIOR_TREE_SYSTEM_H
