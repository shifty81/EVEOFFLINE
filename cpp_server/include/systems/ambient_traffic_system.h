#ifndef EVE_SYSTEMS_AMBIENT_TRAFFIC_SYSTEM_H
#define EVE_SYSTEMS_AMBIENT_TRAFFIC_SYSTEM_H

#include "ecs/system.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <string>
#include <vector>

namespace atlas {
namespace systems {

/**
 * @brief Spawns ambient NPC traffic driven by star-system state
 *
 * Each system with an AmbientTrafficState tracks a spawn timer.
 * When the timer fires, the system evaluates the local
 * SimStarSystemState to decide how many and what types of NPCs
 * should be present:
 *   - High economic_index → more traders/haulers
 *   - High resource_availability → more miners
 *   - High pirate_activity → more pirate spawns
 *   - High security_level → more patrol ships
 *
 * The system does not directly create entities but records spawn
 * requests in the AmbientTrafficState so that other systems or the
 * game session can act on them.
 */
class AmbientTrafficSystem : public ecs::System {
public:
    explicit AmbientTrafficSystem(ecs::World* world);
    ~AmbientTrafficSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "AmbientTrafficSystem"; }

    // --- Query API ---

    /** Get pending spawn requests for a system (type strings) */
    std::vector<std::string> getPendingSpawns(const std::string& system_id) const;

    /** Get active traffic count for a system */
    int getActiveTrafficCount(const std::string& system_id) const;

    /** Clear pending spawn list for a system (after game session processes them) */
    void clearPendingSpawns(const std::string& system_id);

    // --- Configuration ---
    float spawn_interval = 60.0f;     // seconds between spawn evaluations
    int   max_traffic_per_system = 20; // hard cap on NPC traffic
    float trader_economy_threshold = 0.4f;   // min economic_index for traders
    float miner_resource_threshold = 0.3f;   // min resource_availability for miners
    float pirate_activity_threshold = 0.3f;  // min pirate_activity for pirates

private:
    void evaluateSpawns(ecs::Entity* entity,
                        components::AmbientTrafficState* traffic,
                        const components::SimStarSystemState* state);
};

} // namespace systems
} // namespace atlas

#endif // EVE_SYSTEMS_AMBIENT_TRAFFIC_SYSTEM_H
