#ifndef EVE_SYSTEMS_AI_SYSTEM_H
#define EVE_SYSTEMS_AI_SYSTEM_H

#include "ecs/system.h"
#include "ecs/entity.h"
#include <string>

namespace eve {
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
    void idleBehavior(ecs::Entity* entity);
    void approachBehavior(ecs::Entity* entity);
    void orbitBehavior(ecs::Entity* entity);
    void attackBehavior(ecs::Entity* entity);
    void fleeBehavior(ecs::Entity* entity);
};

} // namespace systems
} // namespace eve

#endif // EVE_SYSTEMS_AI_SYSTEM_H
