#ifndef EVE_SYSTEMS_BOUNTY_SYSTEM_H
#define EVE_SYSTEMS_BOUNTY_SYSTEM_H

#include "ecs/system.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Processes NPC kill bounty payouts
 *
 * When an NPC is killed, the system awards the bounty ISK to the
 * killer's wallet and records the kill in a BountyLedger component
 * for history tracking.
 */
class BountySystem : public ecs::System {
public:
    explicit BountySystem(ecs::World* world);
    ~BountySystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "BountySystem"; }

    /**
     * @brief Process a kill and award bounty to the killer
     * @return bounty amount awarded (0 if no bounty)
     */
    double processKill(const std::string& killer_id,
                       const std::string& target_id,
                       const std::string& target_name,
                       double bounty_amount,
                       const std::string& faction = "");

    /**
     * @brief Get total bounty earned by a player
     */
    double getTotalBounty(const std::string& entity_id);

    /**
     * @brief Get total kills by a player
     */
    int getTotalKills(const std::string& entity_id);
};

} // namespace systems
} // namespace atlas

#endif // EVE_SYSTEMS_BOUNTY_SYSTEM_H
