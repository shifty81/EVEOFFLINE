#include "systems/tactical_overlay_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/game_components.h"

namespace atlas {
namespace systems {

TacticalOverlaySystem::TacticalOverlaySystem(ecs::World* world)
    : System(world) {
}

void TacticalOverlaySystem::update(float /*delta_time*/) {
    // No-op: overlay is client-driven
}

void TacticalOverlaySystem::toggleOverlay(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return;

    auto* overlay = entity->getComponent<components::TacticalOverlayState>();
    if (!overlay) {
        entity->addComponent(std::make_unique<components::TacticalOverlayState>());
        overlay = entity->getComponent<components::TacticalOverlayState>();
    }

    overlay->enabled = !overlay->enabled;
}

bool TacticalOverlaySystem::isEnabled(const std::string& entity_id) const {
    const auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    const auto* overlay = entity->getComponent<components::TacticalOverlayState>();
    if (!overlay) return false;

    return overlay->enabled;
}

void TacticalOverlaySystem::setToolRange(const std::string& entity_id, float range, const std::string& tool_type) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return;

    auto* overlay = entity->getComponent<components::TacticalOverlayState>();
    if (!overlay) {
        entity->addComponent(std::make_unique<components::TacticalOverlayState>());
        overlay = entity->getComponent<components::TacticalOverlayState>();
    }

    overlay->tool_range = range;
    overlay->tool_type = tool_type;
}

std::vector<float> TacticalOverlaySystem::getRingDistances(const std::string& entity_id) const {
    const auto* entity = world_->getEntity(entity_id);
    if (!entity) return {};

    const auto* overlay = entity->getComponent<components::TacticalOverlayState>();
    if (!overlay) return {};

    return overlay->ring_distances;
}

void TacticalOverlaySystem::setRingDistances(const std::string& entity_id, const std::vector<float>& distances) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return;

    auto* overlay = entity->getComponent<components::TacticalOverlayState>();
    if (!overlay) {
        entity->addComponent(std::make_unique<components::TacticalOverlayState>());
        overlay = entity->getComponent<components::TacticalOverlayState>();
    }

    overlay->ring_distances = distances;
}

} // namespace systems
} // namespace atlas
