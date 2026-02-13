#ifndef EVE_SYSTEMS_MINING_SYSTEM_H
#define EVE_SYSTEMS_MINING_SYSTEM_H

#include "ecs/system.h"
#include "ecs/entity.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Handles ore mining from mineral deposits
 *
 * Players or AI miners activate a MiningLaser targeting a MineralDeposit
 * entity.  Each tick advances the cycle timer; when a cycle completes
 * ore is transferred from the deposit into the miner's Inventory
 * (respecting cargo capacity).
 */
class MiningSystem : public ecs::System {
public:
    explicit MiningSystem(ecs::World* world);
    ~MiningSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "MiningSystem"; }

    /**
     * @brief Start mining a deposit
     * @param miner_id    Entity with a MiningLaser component
     * @param deposit_id  Entity with a MineralDeposit component
     * @param mining_range Maximum distance allowed (metres)
     * @return true if mining started successfully
     */
    bool startMining(const std::string& miner_id,
                     const std::string& deposit_id,
                     float mining_range = 10000.0f);

    /**
     * @brief Stop an active mining cycle
     * @return true if mining was active and is now stopped
     */
    bool stopMining(const std::string& miner_id);

    /**
     * @brief Create a mineral deposit entity at a location
     * @return deposit entity id, or empty string on failure
     */
    std::string createDeposit(const std::string& mineral_type,
                              float quantity, float x, float y, float z,
                              float volume_per_unit = 0.1f);

    /**
     * @brief Get the number of active mining operations
     */
    int getActiveMinerCount() const;

private:
    int deposit_counter_ = 0;

    /**
     * @brief Complete one mining cycle: transfer ore to miner inventory
     */
    void completeCycle(ecs::Entity* miner, ecs::Entity* deposit);
};

} // namespace systems
} // namespace atlas

#endif // EVE_SYSTEMS_MINING_SYSTEM_H
