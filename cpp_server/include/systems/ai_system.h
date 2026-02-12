#ifndef EVE_SYSTEMS_AI_SYSTEM_H
#define EVE_SYSTEMS_AI_SYSTEM_H

#include "ecs/system.h"
#include "ecs/entity.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Handles AI behavior for NPCs
 * 
 * Implements NPC AI states: idle, approaching, orbiting, attacking, fleeing.
 * NPCs can detect players, approach them, orbit at preferred distance, and attack.
 */
class AISystem : public ecs::System {
public:
    explicit AISystem(ecs::World* world);
    ~AISystem() override = default;
    
    void update(float delta_time) override;
    std::string getName() const override { return "AISystem"; }
    
private:
    /**
     * Idle behavior state
     * 
     * NPC waits and scans for targets. If a player is detected within detection
     * range, transitions to approach state. NPCs in this state have no velocity
     * and remain stationary.
     * 
     * Detection ranges are configured per-NPC in the AIComponent.
     * 
     * @param entity The NPC entity to update
     */
    void idleBehavior(ecs::Entity* entity);
    
    /**
     * Approach behavior state
     * 
     * NPC moves toward target at maximum velocity. Once within preferred orbit
     * range, transitions to orbit state. Uses simple direct-line movement
     * without collision avoidance.
     * 
     * Preferred orbit ranges vary by NPC configuration and ship class.
     * 
     * @param entity The NPC entity to update
     */
    void approachBehavior(ecs::Entity* entity);
    
    /**
     * Orbit behavior state
     * 
     * NPC maintains circular orbit around target at preferred distance. Uses
     * angular velocity to create circular motion. If target moves out of optimal
     * range, may transition back to approach. This is the primary combat state
     * where NPCs will continuously fire weapons.
     * 
     * Typical orbit distances by ship class:
     * - Frigates: ~10km (close-range brawlers)
     * - Cruisers: ~20km (medium-range combatants)
     * - Battleships: ~30km (long-range artillery)
     * 
     * Actual distances are configured per-NPC in the AIComponent.
     * 
     * @param entity The NPC entity to update
     */
    void orbitBehavior(ecs::Entity* entity);
    
    /**
     * Attack behavior state
     * 
     * NPC actively engages target with weapons while maintaining orbit. Triggers
     * weapon activation when in optimal range and manages target locking. If
     * NPC health drops below flee threshold (configured in AIComponent),
     * transitions to flee state.
     * 
     * @param entity The NPC entity to update
     */
    void attackBehavior(ecs::Entity* entity);
    
    /**
     * Flee behavior state
     * 
     * NPC attempts to escape when critically damaged. Moves away from target
     * at maximum velocity. Currently a terminal state - NPCs don't re-engage
     * after fleeing. May warp away if warp drive capabilities are implemented.
     * 
     * @param entity The NPC entity to update
     */
    void fleeBehavior(ecs::Entity* entity);
};

} // namespace systems
} // namespace atlas

#endif // EVE_SYSTEMS_AI_SYSTEM_H
