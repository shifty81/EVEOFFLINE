#include "systems/drone_system.h"
#include "ecs/world.h"
#include "components/game_components.h"
#include <algorithm>

namespace atlas {
namespace systems {

DroneSystem::DroneSystem(ecs::World* world)
    : System(world) {
}

void DroneSystem::update(float delta_time) {
    // Process deployed drones: tick cooldowns and apply damage to owner's
    // locked target (if the owner has a Target component with a lock).
    auto entities = world_->getEntities<components::DroneBay>();
    for (auto* entity : entities) {
        auto* bay = entity->getComponent<components::DroneBay>();
        if (!bay) continue;

        // Get owner's primary target (first locked target)
        auto* target_comp = entity->getComponent<components::Target>();
        ecs::Entity* target_entity = nullptr;
        if (target_comp && !target_comp->locked_targets.empty()) {
            target_entity = world_->getEntity(target_comp->locked_targets[0]);
        }

        // Remove destroyed drones (hp <= 0)
        bay->deployed_drones.erase(
            std::remove_if(bay->deployed_drones.begin(),
                           bay->deployed_drones.end(),
                           [](const components::DroneBay::DroneInfo& d) {
                               return d.current_hp <= 0.0f;
                           }),
            bay->deployed_drones.end());

        // Tick each deployed drone
        for (auto& drone : bay->deployed_drones) {
            // Tick cooldown
            if (drone.cooldown > 0.0f) {
                drone.cooldown -= delta_time;
                if (drone.cooldown < 0.0f) drone.cooldown = 0.0f;
                continue;
            }

            // Fire at target if available and in range
            if (target_entity) {
                auto* target_hp = target_entity->getComponent<components::Health>();
                if (target_hp && target_hp->isAlive()) {
                    // Simple damage application to shields first, then armor, then hull
                    float dmg = drone.damage;

                    // Apply to shields first
                    if (target_hp->shield_hp > 0.0f) {
                        float resist = 0.0f;
                        if (drone.damage_type == "em")        resist = target_hp->shield_em_resist;
                        else if (drone.damage_type == "thermal")   resist = target_hp->shield_thermal_resist;
                        else if (drone.damage_type == "kinetic")   resist = target_hp->shield_kinetic_resist;
                        else if (drone.damage_type == "explosive") resist = target_hp->shield_explosive_resist;
                        float effective = dmg * (1.0f - resist);
                        if (effective > target_hp->shield_hp) {
                            float overflow = effective - target_hp->shield_hp;
                            target_hp->shield_hp = 0.0f;
                            // Overflow to armor
                            target_hp->armor_hp -= overflow;
                            if (target_hp->armor_hp < 0.0f) {
                                target_hp->hull_hp += target_hp->armor_hp;
                                target_hp->armor_hp = 0.0f;
                            }
                        } else {
                            target_hp->shield_hp -= effective;
                        }
                    } else if (target_hp->armor_hp > 0.0f) {
                        float resist = 0.0f;
                        if (drone.damage_type == "em")        resist = target_hp->armor_em_resist;
                        else if (drone.damage_type == "thermal")   resist = target_hp->armor_thermal_resist;
                        else if (drone.damage_type == "kinetic")   resist = target_hp->armor_kinetic_resist;
                        else if (drone.damage_type == "explosive") resist = target_hp->armor_explosive_resist;
                        float effective = dmg * (1.0f - resist);
                        target_hp->armor_hp -= effective;
                        if (target_hp->armor_hp < 0.0f) {
                            target_hp->hull_hp += target_hp->armor_hp;
                            target_hp->armor_hp = 0.0f;
                        }
                    } else {
                        float resist = 0.0f;
                        if (drone.damage_type == "em")        resist = target_hp->hull_em_resist;
                        else if (drone.damage_type == "thermal")   resist = target_hp->hull_thermal_resist;
                        else if (drone.damage_type == "kinetic")   resist = target_hp->hull_kinetic_resist;
                        else if (drone.damage_type == "explosive") resist = target_hp->hull_explosive_resist;
                        float effective = dmg * (1.0f - resist);
                        target_hp->hull_hp -= effective;
                    }

                    drone.cooldown = drone.rate_of_fire;
                }
            }
        }
    }
}

bool DroneSystem::launchDrone(const std::string& entity_id,
                              const std::string& drone_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* bay = entity->getComponent<components::DroneBay>();
    if (!bay) return false;

    // Find the drone in stored drones
    auto it = std::find_if(bay->stored_drones.begin(),
                           bay->stored_drones.end(),
                           [&](const components::DroneBay::DroneInfo& d) {
                               return d.drone_id == drone_id;
                           });
    if (it == bay->stored_drones.end()) return false;

    // Check bandwidth limit
    if (bay->usedBandwidth() + it->bandwidth_use > bay->max_bandwidth)
        return false;

    // Move from stored to deployed
    bay->deployed_drones.push_back(*it);
    bay->stored_drones.erase(it);
    return true;
}

bool DroneSystem::recallDrone(const std::string& entity_id,
                              const std::string& drone_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* bay = entity->getComponent<components::DroneBay>();
    if (!bay) return false;

    auto it = std::find_if(bay->deployed_drones.begin(),
                           bay->deployed_drones.end(),
                           [&](const components::DroneBay::DroneInfo& d) {
                               return d.drone_id == drone_id;
                           });
    if (it == bay->deployed_drones.end()) return false;

    // Move from deployed back to stored
    bay->stored_drones.push_back(*it);
    bay->deployed_drones.erase(it);
    return true;
}

int DroneSystem::recallAll(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;

    auto* bay = entity->getComponent<components::DroneBay>();
    if (!bay) return 0;

    int count = static_cast<int>(bay->deployed_drones.size());
    for (auto& drone : bay->deployed_drones) {
        bay->stored_drones.push_back(drone);
    }
    bay->deployed_drones.clear();
    return count;
}

int DroneSystem::getDeployedCount(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;

    auto* bay = entity->getComponent<components::DroneBay>();
    if (!bay) return 0;

    return static_cast<int>(bay->deployed_drones.size());
}

} // namespace systems
} // namespace atlas
