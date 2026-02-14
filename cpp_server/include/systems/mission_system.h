#ifndef EVE_SYSTEMS_MISSION_SYSTEM_H
#define EVE_SYSTEMS_MISSION_SYSTEM_H

#include "ecs/system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Processes active missions — checks objectives, applies time limits,
 *        and distributes rewards on completion
 *
 * Each tick:
 *  - Decrements time_remaining on timed missions
 *  - Checks if all objectives are satisfied
 *  - Marks missions completed/failed
 *  - Awards ISK + standing on completion
 */
class MissionSystem : public ecs::System {
public:
    explicit MissionSystem(ecs::World* world);
    ~MissionSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "MissionSystem"; }

    /**
     * @brief Accept a new mission for a player entity
     * @return true if mission was added successfully
     */
    bool acceptMission(const std::string& entity_id,
                       const std::string& mission_id,
                       const std::string& name,
                       int level,
                       const std::string& type,
                       const std::string& agent_faction,
                       double isk_reward,
                       float standing_reward,
                       float time_limit = -1.0f);

    /**
     * @brief Set the system entity for economy effects on mission completion
     * Combat missions increase security (reduce pirate spawns),
     * mining missions reduce local ore reserves.
     */
    void setEconomySystemId(const std::string& system_id);

    /**
     * @brief Get count of missions completed in a specific system
     */
    int getCompletedMissionCount() const;

    /**
     * @brief Record objective progress (e.g. NPC destroyed, ore mined)
     * @param objective_type "destroy", "mine", "deliver", "reach"
     * @param target Name of target type/item
     * @param count Number completed this call
     */
    void recordProgress(const std::string& entity_id,
                        const std::string& mission_id,
                        const std::string& objective_type,
                        const std::string& target,
                        int count = 1);

    /**
     * @brief Abandon an active mission
     */
    void abandonMission(const std::string& entity_id,
                        const std::string& mission_id);

private:
    std::string economy_system_id_;
    int completed_count_ = 0;

    /**
     * Apply economy effects when a mission completes.
     * - Combat missions reduce spawn rate multiplier
     * - Mining missions reduce ore reserves
     */
    void applyEconomyEffects(const components::MissionTracker::ActiveMission& mission);
};

} // namespace systems
} // namespace atlas

#endif // EVE_SYSTEMS_MISSION_SYSTEM_H
