#include "systems/skill_system.h"
#include "ecs/world.h"
#include "components/game_components.h"
#include <algorithm>

namespace atlas {
namespace systems {

// Base skill points awarded per level of training
static constexpr double BASE_SP_PER_LEVEL = 1000.0;

SkillSystem::SkillSystem(ecs::World* world)
    : System(world) {
}

void SkillSystem::update(float delta_time) {
    auto entities = world_->getEntities<components::SkillSet>();

    for (auto* entity : entities) {
        auto* skillset = entity->getComponent<components::SkillSet>();
        if (!skillset || skillset->training_queue.empty()) continue;

        auto& front = skillset->training_queue.front();
        front.time_remaining -= delta_time;

        if (front.time_remaining <= 0.0f) {
            // Skill training complete
            auto it = skillset->skills.find(front.skill_id);
            if (it != skillset->skills.end()) {
                if (front.target_level <= it->second.max_level) {
                    it->second.level = front.target_level;
                }
            } else {
                // New skill
                components::SkillSet::TrainedSkill skill;
                skill.skill_id = front.skill_id;
                skill.level = front.target_level;
                skillset->skills[front.skill_id] = skill;
            }

            // Award SP (base: 1000 SP per level, scaled by multiplier)
            double sp_gain = BASE_SP_PER_LEVEL * front.target_level;
            auto skill_it = skillset->skills.find(front.skill_id);
            if (skill_it != skillset->skills.end()) {
                sp_gain *= skill_it->second.training_multiplier;
            }
            skillset->total_sp += sp_gain;

            skillset->training_queue.erase(skillset->training_queue.begin());
        }
    }
}

bool SkillSystem::queueSkillTraining(const std::string& entity_id,
                                      const std::string& skill_id,
                                      const std::string& skill_name,
                                      int target_level,
                                      float training_time,
                                      float multiplier) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* skillset = entity->getComponent<components::SkillSet>();
    if (!skillset) return false;

    if (target_level < 1 || target_level > 5) return false;

    // Ensure skill entry exists
    if (skillset->skills.find(skill_id) == skillset->skills.end()) {
        components::SkillSet::TrainedSkill skill;
        skill.skill_id = skill_id;
        skill.name = skill_name;
        skill.level = 0;
        skill.training_multiplier = multiplier;
        skillset->skills[skill_id] = skill;
    }

    // Check that target level is higher than current
    int current = skillset->getSkillLevel(skill_id);
    if (target_level <= current) return false;

    // Add to queue
    components::SkillSet::QueueEntry entry;
    entry.skill_id = skill_id;
    entry.target_level = target_level;
    entry.time_remaining = training_time;
    skillset->training_queue.push_back(entry);

    return true;
}

bool SkillSystem::trainSkillInstant(const std::string& entity_id,
                                     const std::string& skill_id,
                                     const std::string& skill_name,
                                     int level) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* skillset = entity->getComponent<components::SkillSet>();
    if (!skillset) return false;

    if (level < 1 || level > 5) return false;

    components::SkillSet::TrainedSkill skill;
    skill.skill_id = skill_id;
    skill.name = skill_name;
    skill.level = level;
    skillset->skills[skill_id] = skill;

    skillset->total_sp += BASE_SP_PER_LEVEL * level;
    return true;
}

int SkillSystem::getSkillLevel(const std::string& entity_id,
                                const std::string& skill_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;

    auto* skillset = entity->getComponent<components::SkillSet>();
    if (!skillset) return 0;

    return skillset->getSkillLevel(skill_id);
}

} // namespace systems
} // namespace atlas
