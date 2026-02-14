#ifndef EVE_SYSTEMS_DIFFICULTY_SCALING_SYSTEM_H
#define EVE_SYSTEMS_DIFFICULTY_SCALING_SYSTEM_H

#include "ecs/system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Computes and applies difficulty modifiers from security status
 *
 * When a DifficultyZone's security_status changes (or on initial setup),
 * this system recalculates the multipliers so that lower-security
 * systems have tougher NPCs but better rewards.
 */
class DifficultyScalingSystem : public ecs::System {
public:
    explicit DifficultyScalingSystem(ecs::World* world);
    ~DifficultyScalingSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "DifficultyScalingSystem"; }

    /**
     * @brief Initialize a DifficultyZone from a security status value
     * @param system_id  Entity with a DifficultyZone component
     * @param security   Security level (1.0 = highsec, 0.0 = nullsec)
     * @return true if the zone was updated
     */
    bool initializeZone(const std::string& system_id, float security);

    /**
     * @brief Apply difficulty modifiers to an NPC entity in a given system
     * @param npc_id     Entity with Health + Weapon components
     * @param system_id  Entity with a DifficultyZone component
     * @return true if modifiers were applied
     */
    bool applyToNPC(const std::string& npc_id, const std::string& system_id);

    /**
     * @brief Get the NPC HP multiplier for a security level
     */
    static float hpMultiplierFromSecurity(float security);

    /**
     * @brief Get the NPC damage multiplier for a security level
     */
    static float damageMultiplierFromSecurity(float security);

    /**
     * @brief Get the loot quality multiplier for a security level
     */
    static float lootMultiplierFromSecurity(float security);

    /**
     * @brief Get the ore richness multiplier for a security level
     */
    static float oreMultiplierFromSecurity(float security);

    /**
     * @brief Get the spawn rate multiplier for a security level
     */
    static float spawnRateFromSecurity(float security);

    /**
     * @brief Get the max NPC tier for a security level
     */
    static int maxTierFromSecurity(float security);
};

} // namespace systems
} // namespace atlas

#endif // EVE_SYSTEMS_DIFFICULTY_SCALING_SYSTEM_H
