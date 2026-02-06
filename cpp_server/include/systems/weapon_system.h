#ifndef EVE_SYSTEMS_WEAPON_SYSTEM_H
#define EVE_SYSTEMS_WEAPON_SYSTEM_H

#include "ecs/system.h"
#include <string>

namespace eve {
namespace systems {

/**
 * @brief Handles weapon cooldowns and auto-fire for NPC entities
 * 
 * Manages weapon cycle times and triggers auto-fire for AI-controlled
 * entities that are in the Attacking state. Consumes capacitor when
 * firing weapons. Delegates damage application to CombatSystem.
 */
class WeaponSystem : public ecs::System {
public:
    explicit WeaponSystem(ecs::World* world);
    ~WeaponSystem() override = default;
    
    void update(float delta_time) override;
    std::string getName() const override { return "WeaponSystem"; }
    
    /**
     * @brief Attempt to fire a weapon, consuming capacitor
     * @param shooter_id Entity firing the weapon
     * @param target_id Target entity
     * @return true if weapon fired successfully
     */
    bool fireWeapon(const std::string& shooter_id, const std::string& target_id);
    
private:
    /**
     * @brief Calculate damage falloff based on distance
     * @return Damage multiplier (0.0 - 1.0)
     */
    float calculateFalloff(float distance, float optimal_range, float falloff_range);
};

} // namespace systems
} // namespace eve

#endif // EVE_SYSTEMS_WEAPON_SYSTEM_H
