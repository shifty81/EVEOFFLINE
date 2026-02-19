#ifndef EVE_SYSTEMS_COMBAT_THREAT_SYSTEM_H
#define EVE_SYSTEMS_COMBAT_THREAT_SYSTEM_H

#include "ecs/system.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Adjusts star-system threat level in response to combat events
 *
 * When combat occurs in a system (damage dealt, ships destroyed) the
 * system's threat_level rises.  Ship destruction causes a larger
 * spike than ongoing combat.  This feeds into the background
 * simulation's event triggers (lockdown, security response, NPC
 * rerouting).
 */
class CombatThreatSystem : public ecs::System {
public:
    explicit CombatThreatSystem(ecs::World* world);
    ~CombatThreatSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "CombatThreatSystem"; }

    // --- API ---

    /** Record that damage was dealt in a star system */
    void recordCombatDamage(const std::string& system_id, float damage);

    /** Record that a ship was destroyed in a star system */
    void recordShipDestruction(const std::string& system_id);

    /** Get accumulated combat damage for a system this tick */
    float getPendingDamage(const std::string& system_id) const;

    /** Get pending destruction count for a system */
    int getPendingDestructions(const std::string& system_id) const;

    // --- Configuration ---
    float damage_threat_factor = 0.0001f;   // threat per point of damage
    float destruction_threat_spike = 0.05f; // threat per ship destroyed
    float max_threat = 1.0f;                // threat capped at this value

private:
    std::map<std::string, float> pending_damage_;
    std::map<std::string, int>   pending_destructions_;
};

} // namespace systems
} // namespace atlas

#endif // EVE_SYSTEMS_COMBAT_THREAT_SYSTEM_H
