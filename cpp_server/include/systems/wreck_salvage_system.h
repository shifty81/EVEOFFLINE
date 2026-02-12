#ifndef EVE_SYSTEMS_WRECK_SALVAGE_SYSTEM_H
#define EVE_SYSTEMS_WRECK_SALVAGE_SYSTEM_H

#include "ecs/system.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Manages wreck creation from destroyed ships and salvage collection
 *
 * When a ship's hull HP reaches zero the system creates a wreck entity
 * at the same position.  The wreck inherits loot from the destroyed
 * ship's LootTable (if any) and has a configurable lifetime before
 * despawning.
 *
 * Players can salvage wrecks to collect items and ISK.  Salvage
 * requires proximity (within salvage range) and takes one cycle.
 */
class WreckSalvageSystem : public ecs::System {
public:
    explicit WreckSalvageSystem(ecs::World* world);
    ~WreckSalvageSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "WreckSalvageSystem"; }

    /**
     * @brief Create a wreck entity from a destroyed ship
     * @param destroyed_entity_id  Entity that was destroyed
     * @param x,y,z                Position where the wreck spawns
     * @param wreck_lifetime       Seconds before the wreck despawns
     * @return the wreck entity id, or empty string on failure
     */
    std::string createWreck(const std::string& destroyed_entity_id,
                            float x, float y, float z,
                            float wreck_lifetime = 1800.0f);

    /**
     * @brief Attempt to salvage a wreck
     * @param player_entity_id  The entity doing the salvaging
     * @param wreck_entity_id   The wreck to salvage
     * @param salvage_range     Maximum distance for salvaging (metres)
     * @return true if salvage was successful
     */
    bool salvageWreck(const std::string& player_entity_id,
                      const std::string& wreck_entity_id,
                      float salvage_range = 2500.0f);

    /**
     * @brief Get the number of active wrecks in the world
     */
    int getActiveWreckCount() const;
};

} // namespace systems
} // namespace atlas

#endif // EVE_SYSTEMS_WRECK_SALVAGE_SYSTEM_H
