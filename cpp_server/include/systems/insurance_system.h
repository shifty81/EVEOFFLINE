#ifndef EVE_SYSTEMS_INSURANCE_SYSTEM_H
#define EVE_SYSTEMS_INSURANCE_SYSTEM_H

#include "ecs/system.h"
#include <string>

namespace eve {
namespace systems {

/**
 * @brief Manages ship insurance policies and payouts
 *
 * Players can purchase insurance on their ships. If a ship is
 * destroyed the player can claim the policy for an ISK payout
 * proportional to the ship value and the tier purchased.
 */
class InsuranceSystem : public ecs::System {
public:
    explicit InsuranceSystem(ecs::World* world);
    ~InsuranceSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "InsuranceSystem"; }

    /**
     * @brief Purchase an insurance policy for an entity
     * @param entity_id The entity (ship) to insure
     * @param tier "basic", "standard", or "platinum"
     * @param ship_value The appraised value of the ship in ISK
     * @return true if policy was purchased successfully
     */
    bool purchaseInsurance(const std::string& entity_id,
                           const std::string& tier,
                           double ship_value);

    /**
     * @brief Claim insurance payout on a lost ship
     * @param entity_id The entity whose policy to claim
     * @return ISK payout amount (0.0 if no valid policy)
     */
    double claimInsurance(const std::string& entity_id);

    /**
     * @brief Check if an entity has an active, unclaimed policy
     */
    bool hasActivePolicy(const std::string& entity_id);
};

} // namespace systems
} // namespace eve

#endif // EVE_SYSTEMS_INSURANCE_SYSTEM_H
