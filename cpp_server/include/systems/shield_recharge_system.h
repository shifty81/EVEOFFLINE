#ifndef EVE_SYSTEMS_SHIELD_RECHARGE_SYSTEM_H
#define EVE_SYSTEMS_SHIELD_RECHARGE_SYSTEM_H

#include "ecs/system.h"
#include <string>

namespace eve {
namespace systems {

/**
 * @brief Handles shield recharge for all entities
 * 
 * Implements EVE Online-style passive shield recharge.
 * Shields regenerate over time based on shield_recharge_rate.
 */
class ShieldRechargeSystem : public ecs::System {
public:
    explicit ShieldRechargeSystem(ecs::World* world);
    ~ShieldRechargeSystem() override = default;
    
    void update(float delta_time) override;
    std::string getName() const override { return "ShieldRechargeSystem"; }
    
    /**
     * @brief Get current shield percentage for an entity
     * @param entity_id Entity to query
     * @return Shield percentage (0.0 - 1.0), or -1.0 if entity not found
     */
    float getShieldPercentage(const std::string& entity_id) const;
};

} // namespace systems
} // namespace eve

#endif // EVE_SYSTEMS_SHIELD_RECHARGE_SYSTEM_H
