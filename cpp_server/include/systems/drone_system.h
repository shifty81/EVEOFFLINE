#ifndef EVE_SYSTEMS_DRONE_SYSTEM_H
#define EVE_SYSTEMS_DRONE_SYSTEM_H

#include "ecs/system.h"
#include <string>

namespace eve {
namespace systems {

/**
 * @brief Manages drone deployment, recall, and autonomous combat
 *
 * Handles launching drones from a ship's drone bay, recalling them,
 * and processing their combat actions each tick.  Enforces bandwidth
 * limits and removes destroyed drones.
 */
class DroneSystem : public ecs::System {
public:
    explicit DroneSystem(ecs::World* world);
    ~DroneSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "DroneSystem"; }

    /**
     * @brief Launch a drone from the bay into space
     * @param entity_id  Owner entity (ship)
     * @param drone_id   ID of the drone type to launch
     * @return true if the drone was deployed successfully
     */
    bool launchDrone(const std::string& entity_id,
                     const std::string& drone_id);

    /**
     * @brief Recall a single deployed drone back to the bay
     * @return true if the drone was recalled
     */
    bool recallDrone(const std::string& entity_id,
                     const std::string& drone_id);

    /**
     * @brief Recall all deployed drones back to the bay
     * @return Number of drones recalled
     */
    int recallAll(const std::string& entity_id);

    /**
     * @brief Get the number of currently deployed drones for an entity
     */
    int getDeployedCount(const std::string& entity_id);
};

} // namespace systems
} // namespace eve

#endif // EVE_SYSTEMS_DRONE_SYSTEM_H
