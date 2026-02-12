#ifndef EVE_SYSTEMS_SKILL_SYSTEM_H
#define EVE_SYSTEMS_SKILL_SYSTEM_H

#include "ecs/system.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Processes skill training queues and applies completion
 *
 * Each tick:
 *  - Decrements time_remaining on the front of the training queue
 *  - On completion, levels up the skill and pops from queue
 *  - Accumulates total SP
 */
class SkillSystem : public ecs::System {
public:
    explicit SkillSystem(ecs::World* world);
    ~SkillSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "SkillSystem"; }

    /**
     * @brief Add a skill to the training queue
     * @return true if queued successfully
     */
    bool queueSkillTraining(const std::string& entity_id,
                            const std::string& skill_id,
                            const std::string& skill_name,
                            int target_level,
                            float training_time,
                            float multiplier = 1.0f);

    /**
     * @brief Instantly train a skill to a level (for testing/admin)
     */
    bool trainSkillInstant(const std::string& entity_id,
                           const std::string& skill_id,
                           const std::string& skill_name,
                           int level);

    /**
     * @brief Get the level of a specific skill for an entity
     * @return skill level (0 if not trained)
     */
    int getSkillLevel(const std::string& entity_id,
                      const std::string& skill_id);
};

} // namespace systems
} // namespace atlas

#endif // EVE_SYSTEMS_SKILL_SYSTEM_H
