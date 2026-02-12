#include "systems/station_system.h"
#include "ecs/world.h"
#include "components/game_components.h"
#include <cmath>

namespace atlas {
namespace systems {

static constexpr float DEFAULT_DOCKING_RANGE = 2500.0f;

StationSystem::StationSystem(ecs::World* world)
    : System(world) {
}

void StationSystem::update(float /*delta_time*/) {
    // Stations are static — nothing to tick.
    // Docking/undocking and repair happen on demand via public methods.
}

// ---------------------------------------------------------------------------
// Station creation
// ---------------------------------------------------------------------------

bool StationSystem::createStation(const std::string& station_id,
                                  const std::string& name,
                                  float x, float y, float z,
                                  float docking_range,
                                  float repair_cost_per_hp) {
    if (world_->getEntity(station_id)) return false; // already exists

    auto* entity = world_->createEntity(station_id);
    if (!entity) return false;

    // Position
    auto pos = std::make_unique<components::Position>();
    pos->x = x;
    pos->y = y;
    pos->z = z;
    entity->addComponent(std::move(pos));

    // Station component
    auto station = std::make_unique<components::Station>();
    station->station_name = name;
    station->docking_range = docking_range;
    station->repair_cost_per_hp = repair_cost_per_hp;
    entity->addComponent(std::move(station));

    // Health — stations are indestructible (very high HP)
    auto hp = std::make_unique<components::Health>();
    hp->shield_hp = hp->shield_max = 1000000.0f;
    hp->armor_hp  = hp->armor_max  = 1000000.0f;
    hp->hull_hp   = hp->hull_max   = 1000000.0f;
    entity->addComponent(std::move(hp));

    return true;
}

// ---------------------------------------------------------------------------
// Docking
// ---------------------------------------------------------------------------

bool StationSystem::dockAtStation(const std::string& entity_id,
                                  const std::string& station_id) {
    auto* entity  = world_->getEntity(entity_id);
    auto* station_entity = world_->getEntity(station_id);
    if (!entity || !station_entity) return false;

    // Must not already be docked
    if (entity->getComponent<components::Docked>()) return false;

    // Station must have Station component
    auto* station = station_entity->getComponent<components::Station>();
    if (!station) return false;

    // Range check
    auto* entity_pos  = entity->getComponent<components::Position>();
    auto* station_pos = station_entity->getComponent<components::Position>();
    if (!entity_pos || !station_pos) return false;

    float dx = entity_pos->x - station_pos->x;
    float dy = entity_pos->y - station_pos->y;
    float dz = entity_pos->z - station_pos->z;
    float dist = std::sqrt(dx * dx + dy * dy + dz * dz);

    if (dist > station->docking_range) return false;

    // Dock: stop movement and add Docked component
    auto* vel = entity->getComponent<components::Velocity>();
    if (vel) {
        vel->vx = 0.0f;
        vel->vy = 0.0f;
        vel->vz = 0.0f;
    }

    auto docked = std::make_unique<components::Docked>();
    docked->station_id = station_id;
    entity->addComponent(std::move(docked));

    station->docked_count++;
    return true;
}

// ---------------------------------------------------------------------------
// Undocking
// ---------------------------------------------------------------------------

bool StationSystem::undockFromStation(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* docked = entity->getComponent<components::Docked>();
    if (!docked) return false;

    // Decrement station count
    auto* station_entity = world_->getEntity(docked->station_id);
    if (station_entity) {
        auto* station = station_entity->getComponent<components::Station>();
        if (station && station->docked_count > 0) {
            station->docked_count--;
        }
    }

    entity->removeComponent<components::Docked>();
    return true;
}

// ---------------------------------------------------------------------------
// Repair
// ---------------------------------------------------------------------------

double StationSystem::repairShip(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0;

    auto* docked = entity->getComponent<components::Docked>();
    if (!docked) return 0.0; // must be docked

    auto* hp = entity->getComponent<components::Health>();
    if (!hp) return 0.0;

    // Find station repair cost
    float cost_per_hp = 1.0f;
    auto* station_entity = world_->getEntity(docked->station_id);
    if (station_entity) {
        auto* station = station_entity->getComponent<components::Station>();
        if (station) cost_per_hp = station->repair_cost_per_hp;
    }

    // Calculate damage
    float shield_damage = hp->shield_max - hp->shield_hp;
    float armor_damage  = hp->armor_max  - hp->armor_hp;
    float hull_damage   = hp->hull_max   - hp->hull_hp;
    float total_damage  = shield_damage + armor_damage + hull_damage;

    if (total_damage <= 0.0f) return 0.0; // nothing to repair

    double cost = static_cast<double>(total_damage) * cost_per_hp;

    // Check if player can afford
    auto* player = entity->getComponent<components::Player>();
    if (player && player->isk < cost) return 0.0; // can't afford

    // Deduct ISK
    if (player) player->isk -= cost;

    // Repair to full
    hp->shield_hp = hp->shield_max;
    hp->armor_hp  = hp->armor_max;
    hp->hull_hp   = hp->hull_max;

    return cost;
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

bool StationSystem::isDocked(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    return entity->getComponent<components::Docked>() != nullptr;
}

std::string StationSystem::getDockedStation(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return "";
    auto* docked = entity->getComponent<components::Docked>();
    if (!docked) return "";
    return docked->station_id;
}

} // namespace systems
} // namespace atlas
