#ifndef EVE_SYSTEMS_WORMHOLE_SYSTEM_H
#define EVE_SYSTEMS_WORMHOLE_SYSTEM_H

#include "ecs/system.h"
#include "data/wormhole_database.h"
#include <string>

namespace eve {
namespace systems {

/**
 * @brief Manages wormhole connections: lifetime decay, mass tracking, and collapse
 *
 * Each tick the system ages every WormholeConnection entity and collapses
 * any that exceed their lifetime or have their remaining mass depleted.
 */
class WormholeSystem : public ecs::System {
public:
    explicit WormholeSystem(ecs::World* world);
    ~WormholeSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "WormholeSystem"; }

    /**
     * @brief Attempt to jump a ship through a wormhole
     * @param wormhole_entity_id Entity with WormholeConnection component
     * @param ship_mass Mass of the ship attempting to jump (kg)
     * @return true if jump succeeds, false if ship too heavy or wormhole collapsed
     */
    bool jumpThroughWormhole(const std::string& wormhole_entity_id, double ship_mass);

    /**
     * @brief Check whether a wormhole is still open
     */
    bool isWormholeStable(const std::string& wormhole_entity_id) const;

    /**
     * @brief Get remaining mass fraction (0.0-1.0) for a wormhole
     * @return fraction, or -1.0 if entity not found
     */
    float getRemainingMassFraction(const std::string& wormhole_entity_id) const;

    /**
     * @brief Get remaining lifetime fraction (0.0-1.0) for a wormhole
     * @return fraction, or -1.0 if entity not found
     */
    float getRemainingLifetimeFraction(const std::string& wormhole_entity_id) const;
};

} // namespace systems
} // namespace eve

#endif // EVE_SYSTEMS_WORMHOLE_SYSTEM_H
