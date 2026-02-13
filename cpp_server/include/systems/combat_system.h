#ifndef EVE_SYSTEMS_COMBAT_SYSTEM_H
#define EVE_SYSTEMS_COMBAT_SYSTEM_H

#include "ecs/system.h"
#include "ecs/entity.h"
#include <string>
#include <functional>

namespace atlas {
namespace systems {

/**
 * @brief Handles combat mechanics
 * 
 * Manages weapon firing, damage calculation, and health management.
 * Implements EVE Online's damage and resistance system.
 */
class CombatSystem : public ecs::System {
public:
    /**
     * @brief Callback invoked when an entity's hull reaches zero.
     *
     * Parameters: entity_id, x, y, z of the destroyed entity.
     */
    using DeathCallback = std::function<void(const std::string&, float, float, float)>;

    explicit CombatSystem(ecs::World* world);
    ~CombatSystem() override = default;
    
    void update(float delta_time) override;
    std::string getName() const override { return "CombatSystem"; }
    
    /**
     * @brief Apply damage to an entity
     * @param target_id Target entity ID
     * @param damage Amount of damage
     * @param damage_type Damage type (em, thermal, kinetic, explosive)
     * @return true if damage was applied, false otherwise
     */
    bool applyDamage(const std::string& target_id, float damage, const std::string& damage_type);
    
    /**
     * @brief Fire weapon at target
     * @param shooter_id Shooter entity ID
     * @param target_id Target entity ID
     * @return true if weapon fired successfully, false otherwise
     */
    bool fireWeapon(const std::string& shooter_id, const std::string& target_id);

    /**
     * @brief Register a callback for entity death (hull reaches zero)
     */
    void setDeathCallback(DeathCallback cb) { death_callback_ = std::move(cb); }
    
private:
    DeathCallback death_callback_;
    /**
     * @brief Calculate effective damage after resistances
     */
    float calculateDamage(float base_damage, float resistance);
    
    /**
     * @brief Get resistance value for a damage type on a specific layer
     */
    float getResistance(float em_resist, float thermal_resist, 
                       float kinetic_resist, float explosive_resist,
                       const std::string& damage_type);
};

} // namespace systems
} // namespace atlas

#endif // EVE_SYSTEMS_COMBAT_SYSTEM_H
