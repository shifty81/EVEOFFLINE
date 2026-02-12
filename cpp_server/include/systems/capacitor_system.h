#ifndef EVE_SYSTEMS_CAPACITOR_SYSTEM_H
#define EVE_SYSTEMS_CAPACITOR_SYSTEM_H

#include "ecs/system.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Handles capacitor recharge for all entities
 * 
 * Implements EVE Online-style capacitor recharge.
 * Capacitor is the energy resource used for module activation (weapons, EWAR, etc.).
 */
class CapacitorSystem : public ecs::System {
public:
    explicit CapacitorSystem(ecs::World* world);
    ~CapacitorSystem() override = default;
    
    void update(float delta_time) override;
    std::string getName() const override { return "CapacitorSystem"; }
    
    /**
     * @brief Consume capacitor from an entity
     * @param entity_id Entity to drain capacitor from
     * @param amount Amount of capacitor to consume (GJ)
     * @return true if enough capacitor was available and consumed
     */
    bool consumeCapacitor(const std::string& entity_id, float amount);
    
    /**
     * @brief Get current capacitor percentage for an entity
     * @param entity_id Entity to query
     * @return Capacitor percentage (0.0 - 1.0), or -1.0 if entity not found
     */
    float getCapacitorPercentage(const std::string& entity_id) const;
};

} // namespace systems
} // namespace atlas

#endif // EVE_SYSTEMS_CAPACITOR_SYSTEM_H
