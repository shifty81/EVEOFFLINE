#include "systems/fleet_formation_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"

#include <cmath>

namespace atlas {
namespace systems {

FleetFormationSystem::FleetFormationSystem(ecs::World* world)
    : System(world) {
}

void FleetFormationSystem::update(float /*delta_time*/) {
    computeOffsets();
}

void FleetFormationSystem::setFormation(
    const std::string& entity_id,
    components::FleetFormation::FormationType type,
    int slot_index) {

    auto* entity = world_->getEntity(entity_id);
    if (!entity) return;

    auto* form = entity->getComponent<components::FleetFormation>();
    if (!form) {
        entity->addComponent(std::make_unique<components::FleetFormation>());
        form = entity->getComponent<components::FleetFormation>();
    }

    form->formation = type;
    form->slot_index = slot_index;
}

components::FleetFormation::FormationType
FleetFormationSystem::getFormation(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return components::FleetFormation::FormationType::None;

    auto* form = entity->getComponent<components::FleetFormation>();
    if (!form) return components::FleetFormation::FormationType::None;

    return form->formation;
}

void FleetFormationSystem::computeOffsets() {
    auto entities = world_->getEntities<components::FleetFormation>();
    for (auto* entity : entities) {
        auto* form = entity->getComponent<components::FleetFormation>();
        if (!form) continue;

        using FT = components::FleetFormation::FormationType;
        switch (form->formation) {
            case FT::Arrow:   computeArrow(form);   break;
            case FT::Line:    computeLine(form);     break;
            case FT::Wedge:   computeWedge(form);    break;
            case FT::Spread:  computeSpread(form);   break;
            case FT::Diamond: computeDiamond(form);  break;
            default:
                form->offset_x = 0.0f;
                form->offset_y = 0.0f;
                form->offset_z = 0.0f;
                break;
        }
    }
}

bool FleetFormationSystem::getOffset(const std::string& entity_id,
                                     float& ox, float& oy, float& oz) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* form = entity->getComponent<components::FleetFormation>();
    if (!form) return false;

    ox = form->offset_x;
    oy = form->offset_y;
    oz = form->offset_z;
    return true;
}

// ---- Formation patterns ----

// Arrow: leader at tip, members fan out behind in V shape
//   Slot 0: (0, 0, 0)
//   Slot 1: (-spacing, 0, -spacing)
//   Slot 2: (+spacing, 0, -spacing)
//   Slot 3: (-2*spacing, 0, -2*spacing)
//   Slot 4: (+2*spacing, 0, -2*spacing)
void FleetFormationSystem::computeArrow(components::FleetFormation* f) {
    if (f->slot_index == 0) {
        f->offset_x = 0.0f; f->offset_y = 0.0f; f->offset_z = 0.0f;
        return;
    }
    float spacing = kDefaultSpacing * f->spacing_modifier;
    int row = (f->slot_index + 1) / 2;             // 1,1,2,2,3,3...
    int side = (f->slot_index % 2 == 1) ? -1 : 1;  // odd=left, even=right
    f->offset_x = side * row * spacing;
    f->offset_y = 0.0f;
    f->offset_z = -row * spacing;
}

// Line: single file behind the leader
void FleetFormationSystem::computeLine(components::FleetFormation* f) {
    float spacing = kDefaultSpacing * f->spacing_modifier;
    f->offset_x = 0.0f;
    f->offset_y = 0.0f;
    f->offset_z = -f->slot_index * spacing;
}

// Wedge: like Arrow but shallower — mostly used for combat approach
void FleetFormationSystem::computeWedge(components::FleetFormation* f) {
    if (f->slot_index == 0) {
        f->offset_x = 0.0f; f->offset_y = 0.0f; f->offset_z = 0.0f;
        return;
    }
    float spacing = kDefaultSpacing * f->spacing_modifier;
    int row = (f->slot_index + 1) / 2;
    int side = (f->slot_index % 2 == 1) ? -1 : 1;
    f->offset_x = side * row * spacing;
    f->offset_y = 0.0f;
    f->offset_z = -row * spacing * 0.5f;   // half the depth of Arrow
}

// Spread: members fan out along the X axis
void FleetFormationSystem::computeSpread(components::FleetFormation* f) {
    float spacing = kDefaultSpacing * f->spacing_modifier;
    int centered = f->slot_index;
    // Alternate left/right: 0, -1, +1, -2, +2 ...
    int side = (centered % 2 == 1) ? -1 : 1;
    int half = (centered + 1) / 2;
    if (centered == 0) { half = 0; side = 1; }
    f->offset_x = side * half * spacing;
    f->offset_y = 0.0f;
    f->offset_z = 0.0f;
}

// Diamond: compact 4-member diamond with leader in front
//   Slot 0: front, 1: left, 2: right, 3: rear, 4+: extra row behind
void FleetFormationSystem::computeDiamond(components::FleetFormation* f) {
    float spacing = kDefaultSpacing * f->spacing_modifier;
    switch (f->slot_index) {
        case 0:
            f->offset_x = 0.0f; f->offset_y = 0.0f; f->offset_z = 0.0f;
            break;
        case 1:
            f->offset_x = -spacing; f->offset_y = 0.0f; f->offset_z = -spacing;
            break;
        case 2:
            f->offset_x =  spacing; f->offset_y = 0.0f; f->offset_z = -spacing;
            break;
        case 3:
            f->offset_x = 0.0f; f->offset_y = 0.0f; f->offset_z = -2.0f * spacing;
            break;
        default: {
            // Extra members trail behind in a line
            int extra = f->slot_index - 3;
            f->offset_x = 0.0f;
            f->offset_y = 0.0f;
            f->offset_z = -(2 + extra) * spacing;
            break;
        }
    }
}

// ---------------------------------------------------------------------------
// Phase 9: Relationship-based spacing modifier
// ---------------------------------------------------------------------------

void FleetFormationSystem::applyRelationshipSpacing(const std::string& entity_id,
                                                     const std::string& leader_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return;

    auto* form = entity->getComponent<components::FleetFormation>();
    if (!form) return;

    auto* rel = entity->getComponent<components::CaptainRelationship>();
    if (!rel) {
        form->spacing_modifier = 1.0f;
        return;
    }

    float affinity = rel->getAffinityWith(leader_id);

    if (affinity > 50.0f) {
        form->spacing_modifier = 0.7f;   // Friend: fly closer
    } else if (affinity > 20.0f) {
        form->spacing_modifier = 0.85f;  // Ally
    } else if (affinity < -50.0f) {
        form->spacing_modifier = 1.5f;   // Grudge: fly wider
    } else if (affinity < -20.0f) {
        form->spacing_modifier = 1.25f;  // Rival
    } else {
        form->spacing_modifier = 1.0f;   // Neutral
    }
}

} // namespace systems
} // namespace atlas
