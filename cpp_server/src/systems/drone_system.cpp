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

        // Get mining deposit target
        ecs::Entity* mining_deposit = nullptr;
        if (!bay->mining_target_id.empty()) {
            mining_deposit = world_->getEntity(bay->mining_target_id);
        }

        // Get salvage wreck target
        ecs::Entity* salvage_wreck = nullptr;
        if (!bay->salvage_target_id.empty()) {
            salvage_wreck = world_->getEntity(bay->salvage_target_id);
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

            // --- Mining drone behavior ---
            if (drone.type == "mining_drone" && mining_deposit) {
                auto* deposit = mining_deposit->getComponent<components::MineralDeposit>();
                auto* owner_inv = entity->getComponent<components::Inventory>();
                if (deposit && owner_inv && !deposit->isDepleted()) {
                    float yield = drone.mining_yield * deposit->yield_rate;
                    yield = std::min(yield, deposit->quantity_remaining);
                    float vol_needed = yield * deposit->volume_per_unit;
                    if (vol_needed > owner_inv->freeCapacity() && deposit->volume_per_unit > 0.0f) {
                        yield = owner_inv->freeCapacity() / deposit->volume_per_unit;
                    }
                    if (yield > 0.0f) {
                        deposit->quantity_remaining -= yield;
                        if (deposit->quantity_remaining < 0.0f) deposit->quantity_remaining = 0.0f;
                        bool stacked = false;
                        for (auto& item : owner_inv->items) {
                            if (item.item_id == deposit->mineral_type) {
                                item.quantity += static_cast<int>(yield);
                                stacked = true;
                                break;
                            }
                        }
                        if (!stacked) {
                            components::Inventory::Item ore;
                            ore.item_id  = deposit->mineral_type;
                            ore.name     = deposit->mineral_type;
                            ore.type     = "ore";
                            ore.quantity = static_cast<int>(yield);
                            ore.volume   = deposit->volume_per_unit;
                            owner_inv->items.push_back(ore);
                        }
                    }
                    drone.cooldown = drone.rate_of_fire;
                }
                continue;
            }

            // --- Salvage drone behavior ---
            if (drone.type == "salvage_drone" && salvage_wreck) {
                auto* wreck = salvage_wreck->getComponent<components::Wreck>();
                auto* wreck_inv = salvage_wreck->getComponent<components::Inventory>();
                auto* owner_inv = entity->getComponent<components::Inventory>();
                if (wreck && !wreck->salvaged && wreck_inv && owner_inv) {
                    float roll = nextSalvageRandom();
                    if (roll < drone.salvage_chance) {
                        for (const auto& item : wreck_inv->items) {
                            owner_inv->items.push_back(item);
                        }
                        wreck_inv->items.clear();
                        wreck->salvaged = true;
                    }
                    drone.cooldown = drone.rate_of_fire;
                }
                continue;
            }

            // --- Combat drone behavior (existing) ---
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

bool DroneSystem::setMiningTarget(const std::string& entity_id,
                                   const std::string& deposit_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* bay = entity->getComponent<components::DroneBay>();
    if (!bay) return false;

    auto* deposit_entity = world_->getEntity(deposit_id);
    if (!deposit_entity) return false;

    auto* deposit = deposit_entity->getComponent<components::MineralDeposit>();
    if (!deposit || deposit->isDepleted()) return false;

    bay->mining_target_id = deposit_id;
    return true;
}

bool DroneSystem::setSalvageTarget(const std::string& entity_id,
                                    const std::string& wreck_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* bay = entity->getComponent<components::DroneBay>();
    if (!bay) return false;

    auto* wreck_entity = world_->getEntity(wreck_id);
    if (!wreck_entity) return false;

    auto* wreck = wreck_entity->getComponent<components::Wreck>();
    if (!wreck || wreck->salvaged) return false;

    bay->salvage_target_id = wreck_id;
    return true;
}

float DroneSystem::nextSalvageRandom() {
    salvage_seed_ = salvage_seed_ * 1103515245u + 12345u;
    return static_cast<float>((salvage_seed_ >> 16) & 0x7FFF) / 32767.0f;
}

} // namespace systems
} // namespace atlas
