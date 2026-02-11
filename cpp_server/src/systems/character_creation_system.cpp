#include "systems/character_creation_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <iostream>
#include <algorithm>

namespace eve {
namespace systems {

static constexpr int CLONE_JUMP_COOLDOWN_SECONDS = 86400;  // 24 hours

CharacterCreationSystem::CharacterCreationSystem(ecs::World* world) : System(world) {}

void CharacterCreationSystem::update(float delta_time) {
    for (auto* entity : world_->getAllEntities()) {
        auto* sheet = entity->getComponent<components::CharacterSheet>();
        if (!sheet) continue;

        if (sheet->clone_jump_cooldown > 0) {
            sheet->clone_jump_cooldown -= static_cast<int>(delta_time);
            if (sheet->clone_jump_cooldown < 0) {
                sheet->clone_jump_cooldown = 0;
            }
        }
    }
}

bool CharacterCreationSystem::createCharacter(const std::string& entity_id,
                                              const std::string& character_name,
                                              const std::string& race,
                                              const std::string& bloodline,
                                              const std::string& ancestry,
                                              const std::string& gender) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* sheet = entity->getComponent<components::CharacterSheet>();
    if (!sheet) return false;

    // Validate race
    if (race != "Caldari" && race != "Amarr" && race != "Gallente" && race != "Minmatar") {
        return false;
    }

    sheet->character_name = character_name;
    sheet->race = race;
    sheet->bloodline = bloodline;
    sheet->ancestry = ancestry;
    sheet->gender = gender;

    // Set race-specific starting attributes
    if (race == "Caldari") {
        sheet->intelligence = 23;
        sheet->perception = 20;
        sheet->charisma = 19;
        sheet->willpower = 20;
        sheet->memory = 21;
    } else if (race == "Amarr") {
        sheet->intelligence = 22;
        sheet->perception = 20;
        sheet->charisma = 20;
        sheet->willpower = 22;
        sheet->memory = 20;
    } else if (race == "Gallente") {
        sheet->intelligence = 21;
        sheet->perception = 20;
        sheet->charisma = 22;
        sheet->willpower = 19;
        sheet->memory = 21;
    } else if (race == "Minmatar") {
        sheet->intelligence = 20;
        sheet->perception = 22;
        sheet->charisma = 19;
        sheet->willpower = 21;
        sheet->memory = 21;
    }

    return true;
}

bool CharacterCreationSystem::installImplant(const std::string& entity_id,
                                             const std::string& implant_id,
                                             const std::string& implant_name,
                                             int slot,
                                             const std::string& attribute_bonus,
                                             int bonus_amount) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* sheet = entity->getComponent<components::CharacterSheet>();
    if (!sheet) return false;

    // Validate slot range
    if (slot < 1 || slot > 10) return false;

    // Check if slot already occupied
    for (const auto& imp : sheet->implants) {
        if (imp.slot == slot) return false;
    }

    components::CharacterSheet::Implant implant;
    implant.implant_id = implant_id;
    implant.implant_name = implant_name;
    implant.slot = slot;
    implant.attribute_bonus = attribute_bonus;
    implant.bonus_amount = bonus_amount;
    sheet->implants.push_back(implant);

    return true;
}

bool CharacterCreationSystem::removeImplant(const std::string& entity_id, int slot) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* sheet = entity->getComponent<components::CharacterSheet>();
    if (!sheet) return false;

    for (auto it = sheet->implants.begin(); it != sheet->implants.end(); ++it) {
        if (it->slot == slot) {
            sheet->implants.erase(it);
            return true;
        }
    }

    return false;
}

bool CharacterCreationSystem::setCloneGrade(const std::string& entity_id,
                                            const std::string& grade) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* sheet = entity->getComponent<components::CharacterSheet>();
    if (!sheet) return false;

    if (grade != "alpha" && grade != "omega") return false;

    sheet->clone_grade = grade;
    return true;
}

bool CharacterCreationSystem::setCloneLocation(const std::string& entity_id,
                                               const std::string& station_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* sheet = entity->getComponent<components::CharacterSheet>();
    if (!sheet) return false;

    sheet->clone_location = station_id;
    return true;
}

bool CharacterCreationSystem::jumpClone(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* sheet = entity->getComponent<components::CharacterSheet>();
    if (!sheet) return false;

    if (sheet->clone_jump_cooldown > 0) return false;

    sheet->clone_jump_cooldown = CLONE_JUMP_COOLDOWN_SECONDS;
    return true;
}

int CharacterCreationSystem::getEffectiveAttribute(const std::string& entity_id,
                                                   const std::string& attribute) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;

    auto* sheet = entity->getComponent<components::CharacterSheet>();
    if (!sheet) return 0;

    return sheet->getEffectiveAttribute(attribute);
}

bool CharacterCreationSystem::modifySecurityStatus(const std::string& entity_id, float delta) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* sheet = entity->getComponent<components::CharacterSheet>();
    if (!sheet) return false;

    sheet->security_status += delta;
    if (sheet->security_status > 10.0f) sheet->security_status = 10.0f;
    if (sheet->security_status < -10.0f) sheet->security_status = -10.0f;

    return true;
}

bool CharacterCreationSystem::addEmploymentRecord(const std::string& entity_id,
                                                  const std::string& corp_id,
                                                  const std::string& corp_name,
                                                  float join_date) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* sheet = entity->getComponent<components::CharacterSheet>();
    if (!sheet) return false;

    components::CharacterSheet::EmploymentRecord record;
    record.corp_id = corp_id;
    record.corp_name = corp_name;
    record.join_date = join_date;
    sheet->employment_history.push_back(record);

    return true;
}

} // namespace systems
} // namespace eve
