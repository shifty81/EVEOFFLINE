#ifndef EVE_SYSTEMS_MISSION_TEMPLATE_SYSTEM_H
#define EVE_SYSTEMS_MISSION_TEMPLATE_SYSTEM_H

#include "ecs/system.h"
#include "components/game_components.h"
#include <string>
#include <vector>

namespace atlas {
namespace systems {

/**
 * @brief Manages a library of mission templates and generates concrete missions
 *
 * Stores template entities with MissionTemplate components.  Provides
 * methods to install default templates, query available templates by
 * faction/standing/level, and generate ActiveMission structs from a
 * chosen template using deterministic seeding.
 */
class MissionTemplateSystem : public ecs::System {
public:
    explicit MissionTemplateSystem(ecs::World* world);
    ~MissionTemplateSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "MissionTemplateSystem"; }

    /** Create 16 default templates across 5 types and multiple levels. */
    void installDefaultTemplates();

    /** Return template_ids matching faction, minimum standing, and level. */
    std::vector<std::string> getTemplatesForFaction(
        const std::string& faction,
        float standing,
        int level) const;

    /**
     * Build a concrete ActiveMission from a template.
     * Uses a deterministic seed derived from system_id + template_id.
     * Does NOT attach the mission to any entity.
     */
    components::MissionTracker::ActiveMission generateMissionFromTemplate(
        const std::string& template_id,
        const std::string& system_id,
        const std::string& player_entity_id) const;

private:
    int template_counter_ = 0;
    void addTemplate(const std::string& template_id,
                     const std::string& name_pattern,
                     const std::string& type,
                     int level,
                     const std::string& required_faction,
                     float min_standing,
                     const std::vector<components::MissionTemplate::ObjectiveTemplate>& objectives,
                     double base_isk,
                     double isk_per_level,
                     float base_standing_reward,
                     float standing_per_level,
                     float base_time_limit);
};

} // namespace systems
} // namespace atlas

#endif // EVE_SYSTEMS_MISSION_TEMPLATE_SYSTEM_H
