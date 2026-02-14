#ifndef EVE_SYSTEMS_MISSION_GENERATOR_SYSTEM_H
#define EVE_SYSTEMS_MISSION_GENERATOR_SYSTEM_H

#include "ecs/system.h"
#include "components/game_components.h"
#include "systems/mission_template_system.h"
#include <string>
#include <vector>
#include <map>

namespace atlas {
namespace systems {

/**
 * @brief Generates available missions for solar systems based on world state
 *
 * Uses system properties (security status, resources, anomalies) to
 * determine which mission types are appropriate, then leverages
 * MissionTemplateSystem to produce concrete mission offers.
 * Missions are generated on demand, not per tick.
 */
class MissionGeneratorSystem : public ecs::System {
public:
    MissionGeneratorSystem(ecs::World* world, MissionTemplateSystem* templates);
    ~MissionGeneratorSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "MissionGeneratorSystem"; }

    /**
     * @brief Generate available missions for a solar system
     * @param system_id  Entity id of the SolarSystem
     * @param seed       Deterministic seed for generation
     * @return number of missions generated
     */
    int generateMissionsForSystem(const std::string& system_id, uint32_t seed);

    struct AvailableMission {
        std::string template_id;
        std::string system_id;
        components::MissionTracker::ActiveMission mission;
    };

    /**
     * @brief Get currently available missions for a system
     */
    std::vector<AvailableMission> getAvailableMissions(const std::string& system_id) const;

    /**
     * @brief Offer a mission from the available list to a player
     * @param player_id     Entity id of the player
     * @param system_id     Entity id of the solar system
     * @param mission_index Index into the available missions list
     * @return true if the mission was successfully offered
     */
    bool offerMissionToPlayer(const std::string& player_id,
                              const std::string& system_id,
                              int mission_index);

private:
    MissionTemplateSystem* templates_;
    std::map<std::string, std::vector<AvailableMission>> system_missions_;
};

} // namespace systems
} // namespace atlas

#endif // EVE_SYSTEMS_MISSION_GENERATOR_SYSTEM_H
