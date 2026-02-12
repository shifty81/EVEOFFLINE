#ifndef EVE_SYSTEMS_LOOT_SYSTEM_H
#define EVE_SYSTEMS_LOOT_SYSTEM_H

#include "ecs/system.h"
#include <string>
#include <cstdint>

namespace atlas {
namespace systems {

/**
 * @brief Generates loot from destroyed NPCs and handles collection
 *
 * Reads an entity's LootTable component, creates a wreck entity
 * with an Inventory containing randomised drops, and allows
 * players to loot the wreck.
 */
class LootSystem : public ecs::System {
public:
    explicit LootSystem(ecs::World* world);
    ~LootSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "LootSystem"; }

    /**
     * @brief Generate loot from an entity with a LootTable
     * @return Wreck entity id, or empty string on failure
     */
    std::string generateLoot(const std::string& entity_id);

    /**
     * @brief Transfer all items from wreck to player inventory and add ISK
     * @return true if collection succeeded
     */
    bool collectLoot(const std::string& wreck_id, const std::string& player_id);

    /**
     * @brief Set random seed for deterministic testing
     */
    void setRandomSeed(uint32_t seed);

private:
    uint32_t seed_ = 12345;
    int wreck_counter_ = 0;

    // Simple deterministic random [0.0, 1.0)
    float nextRandom();
};

} // namespace systems
} // namespace atlas

#endif // EVE_SYSTEMS_LOOT_SYSTEM_H
