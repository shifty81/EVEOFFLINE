#include "systems/fleet_morale_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/game_components.h"

namespace atlas {
namespace systems {

FleetMoraleSystem::FleetMoraleSystem(ecs::World* world)
    : System(world) {
}

void FleetMoraleSystem::update(float /*delta_time*/) {
    // Update morale scores for all entities with a FleetMorale component
    auto entities = world_->getEntities<components::FleetMorale>();
    for (auto* entity : entities) {
        auto* morale = entity->getComponent<components::FleetMorale>();
        if (morale) {
            morale->updateMoraleScore();
        }
    }
}

// ---- Record combat outcomes ----

void FleetMoraleSystem::recordWin(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return;

    auto* morale = entity->getComponent<components::FleetMorale>();
    if (!morale) {
        entity->addComponent(std::make_unique<components::FleetMorale>());
        morale = entity->getComponent<components::FleetMorale>();
    }

    morale->wins++;
    morale->updateMoraleScore();
}

void FleetMoraleSystem::recordLoss(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return;

    auto* morale = entity->getComponent<components::FleetMorale>();
    if (!morale) {
        entity->addComponent(std::make_unique<components::FleetMorale>());
        morale = entity->getComponent<components::FleetMorale>();
    }

    morale->losses++;
    morale->updateMoraleScore();
}

void FleetMoraleSystem::recordShipLost(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return;

    auto* morale = entity->getComponent<components::FleetMorale>();
    if (!morale) {
        entity->addComponent(std::make_unique<components::FleetMorale>());
        morale = entity->getComponent<components::FleetMorale>();
    }

    morale->ships_lost++;
    morale->updateMoraleScore();
}

void FleetMoraleSystem::recordSavedByPlayer(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return;

    auto* morale = entity->getComponent<components::FleetMorale>();
    if (!morale) {
        entity->addComponent(std::make_unique<components::FleetMorale>());
        morale = entity->getComponent<components::FleetMorale>();
    }

    morale->times_saved_by_player++;
    morale->updateMoraleScore();
}

void FleetMoraleSystem::recordPlayerSaved(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return;

    auto* morale = entity->getComponent<components::FleetMorale>();
    if (!morale) {
        entity->addComponent(std::make_unique<components::FleetMorale>());
        morale = entity->getComponent<components::FleetMorale>();
    }

    morale->times_player_saved++;
    morale->updateMoraleScore();
}

void FleetMoraleSystem::recordMissionTogether(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return;

    auto* morale = entity->getComponent<components::FleetMorale>();
    if (!morale) {
        entity->addComponent(std::make_unique<components::FleetMorale>());
        morale = entity->getComponent<components::FleetMorale>();
    }

    morale->missions_together++;
    morale->updateMoraleScore();
}

// ---- Queries ----

float FleetMoraleSystem::getMoraleScore(const std::string& entity_id) const {
    const auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;

    const auto* morale = entity->getComponent<components::FleetMorale>();
    if (!morale) return 0.0f;

    return morale->morale_score;
}

std::string FleetMoraleSystem::getMoraleState(const std::string& entity_id) const {
    const auto* entity = world_->getEntity(entity_id);
    if (!entity) return "Steady";

    const auto* morale = entity->getComponent<components::FleetMorale>();
    if (!morale) return "Steady";

    return morale->morale_state;
}

} // namespace systems
} // namespace atlas
