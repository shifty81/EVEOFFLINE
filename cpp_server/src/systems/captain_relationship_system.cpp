#include "systems/captain_relationship_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/game_components.h"

namespace atlas {
namespace systems {

CaptainRelationshipSystem::CaptainRelationshipSystem(ecs::World* world)
    : System(world) {
}

void CaptainRelationshipSystem::update(float delta_time) {
    // Passive affinity increase for captains in same fleet (+0.1 per minute)
    auto entities = world_->getEntities<components::CaptainRelationship, components::FleetMembership>();
    for (auto* entity : entities) {
        auto* rel = entity->getComponent<components::CaptainRelationship>();
        auto* membership = entity->getComponent<components::FleetMembership>();
        if (!rel || !membership) continue;

        // Find other fleet members and increase affinity
        auto all_fleet = world_->getEntities<components::CaptainRelationship, components::FleetMembership>();
        for (auto* other : all_fleet) {
            if (other->getId() == entity->getId()) continue;
            auto* other_membership = other->getComponent<components::FleetMembership>();
            if (!other_membership) continue;
            if (other_membership->fleet_id != membership->fleet_id) continue;

            float increase = 0.1f * (delta_time / 60.0f);
            rel->modifyAffinity(other->getId(), increase);
        }
    }
}

void CaptainRelationshipSystem::recordEvent(const std::string& entity_id, const std::string& other_id, const std::string& event_type) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return;

    auto* rel = entity->getComponent<components::CaptainRelationship>();
    if (!rel) {
        entity->addComponent(std::make_unique<components::CaptainRelationship>());
        rel = entity->getComponent<components::CaptainRelationship>();
    }

    float change = 0.0f;
    if (event_type == "saved_in_combat") {
        change = 10.0f;
    } else if (event_type == "abandoned") {
        change = -20.0f;
    } else if (event_type == "shared_loss") {
        change = -5.0f;
    } else if (event_type == "kill_credit_stolen") {
        change = -8.0f;
    } else if (event_type == "praised") {
        change = 3.0f;
    } else if (event_type == "shared_victory") {
        change = 5.0f;
    }

    rel->modifyAffinity(other_id, change);
}

float CaptainRelationshipSystem::getAffinity(const std::string& entity_id, const std::string& other_id) const {
    const auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;

    const auto* rel = entity->getComponent<components::CaptainRelationship>();
    if (!rel) return 0.0f;

    return rel->getAffinityWith(other_id);
}

std::string CaptainRelationshipSystem::getRelationshipStatus(const std::string& entity_id, const std::string& other_id) const {
    float affinity = getAffinity(entity_id, other_id);

    if (affinity > 50.0f) return "Friend";
    if (affinity > 20.0f) return "Ally";
    if (affinity < -50.0f) return "Grudge";
    if (affinity < -20.0f) return "Rival";
    return "Neutral";
}

} // namespace systems
} // namespace atlas
