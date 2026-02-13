#include "systems/mining_system.h"
#include "ecs/world.h"
#include "components/game_components.h"
#include <cmath>
#include <algorithm>
#include <sstream>

namespace atlas {
namespace systems {

MiningSystem::MiningSystem(ecs::World* world)
    : System(world) {
}

void MiningSystem::update(float delta_time) {
    for (auto* entity : world_->getAllEntities()) {
        auto* laser = entity->getComponent<components::MiningLaser>();
        if (!laser || !laser->active) continue;

        auto* deposit_entity = world_->getEntity(laser->target_deposit_id);
        if (!deposit_entity) {
            // Deposit gone — stop mining
            laser->active = false;
            laser->cycle_progress = 0.0f;
            continue;
        }

        auto* deposit = deposit_entity->getComponent<components::MineralDeposit>();
        if (!deposit || deposit->isDepleted()) {
            laser->active = false;
            laser->cycle_progress = 0.0f;
            continue;
        }

        laser->cycle_progress += delta_time;
        if (laser->cycle_progress >= laser->cycle_time) {
            completeCycle(entity, deposit_entity);
            laser->cycle_progress = 0.0f;
            // If deposit depleted, stop
            if (deposit->isDepleted()) {
                laser->active = false;
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Start / Stop mining
// ---------------------------------------------------------------------------

bool MiningSystem::startMining(const std::string& miner_id,
                                const std::string& deposit_id,
                                float mining_range) {
    auto* miner = world_->getEntity(miner_id);
    auto* deposit_entity = world_->getEntity(deposit_id);
    if (!miner || !deposit_entity) return false;

    auto* laser = miner->getComponent<components::MiningLaser>();
    if (!laser) return false;

    auto* deposit = deposit_entity->getComponent<components::MineralDeposit>();
    if (!deposit || deposit->isDepleted()) return false;

    // Range check
    auto* miner_pos = miner->getComponent<components::Position>();
    auto* dep_pos   = deposit_entity->getComponent<components::Position>();
    if (!miner_pos || !dep_pos) return false;

    float dx = miner_pos->x - dep_pos->x;
    float dy = miner_pos->y - dep_pos->y;
    float dz = miner_pos->z - dep_pos->z;
    float dist = std::sqrt(dx * dx + dy * dy + dz * dz);
    if (dist > mining_range) return false;

    laser->active = true;
    laser->cycle_progress = 0.0f;
    laser->target_deposit_id = deposit_id;
    return true;
}

bool MiningSystem::stopMining(const std::string& miner_id) {
    auto* miner = world_->getEntity(miner_id);
    if (!miner) return false;

    auto* laser = miner->getComponent<components::MiningLaser>();
    if (!laser || !laser->active) return false;

    laser->active = false;
    laser->cycle_progress = 0.0f;
    return true;
}

// ---------------------------------------------------------------------------
// Deposit creation
// ---------------------------------------------------------------------------

std::string MiningSystem::createDeposit(const std::string& mineral_type,
                                         float quantity, float x, float y, float z,
                                         float volume_per_unit) {
    std::string id = "deposit_" + std::to_string(deposit_counter_++);
    auto* entity = world_->createEntity(id);
    if (!entity) return "";

    auto pos = std::make_unique<components::Position>();
    pos->x = x;
    pos->y = y;
    pos->z = z;
    entity->addComponent(std::move(pos));

    auto dep = std::make_unique<components::MineralDeposit>();
    dep->mineral_type = mineral_type;
    dep->quantity_remaining = quantity;
    dep->max_quantity = quantity;
    dep->volume_per_unit = volume_per_unit;
    entity->addComponent(std::move(dep));

    return id;
}

// ---------------------------------------------------------------------------
// Query helpers
// ---------------------------------------------------------------------------

int MiningSystem::getActiveMinerCount() const {
    int count = 0;
    for (const auto* entity : world_->getAllEntities()) {
        auto* laser = entity->getComponent<components::MiningLaser>();
        if (laser && laser->active) count++;
    }
    return count;
}

// ---------------------------------------------------------------------------
// Cycle completion
// ---------------------------------------------------------------------------

void MiningSystem::completeCycle(ecs::Entity* miner, ecs::Entity* deposit_entity) {
    auto* laser   = miner->getComponent<components::MiningLaser>();
    auto* deposit = deposit_entity->getComponent<components::MineralDeposit>();
    auto* inv     = miner->getComponent<components::Inventory>();
    if (!laser || !deposit || !inv) return;

    float effective_yield = laser->yield_per_cycle * deposit->yield_rate;
    // Clamp to available ore
    effective_yield = std::min(effective_yield, deposit->quantity_remaining);

    // Clamp to free cargo space
    float volume_needed = effective_yield * deposit->volume_per_unit;
    float free = inv->freeCapacity();
    if (volume_needed > free && deposit->volume_per_unit > 0.0f) {
        effective_yield = free / deposit->volume_per_unit;
    }

    if (effective_yield <= 0.0f) {
        laser->active = false;
        return;
    }

    // Remove ore from deposit
    deposit->quantity_remaining -= effective_yield;
    if (deposit->quantity_remaining < 0.0f) deposit->quantity_remaining = 0.0f;

    // Add ore to miner inventory (stack if same type exists)
    bool stacked = false;
    for (auto& item : inv->items) {
        if (item.item_id == deposit->mineral_type) {
            item.quantity += static_cast<int>(effective_yield);
            stacked = true;
            break;
        }
    }
    if (!stacked) {
        components::Inventory::Item ore;
        ore.item_id  = deposit->mineral_type;
        ore.name     = deposit->mineral_type;
        ore.type     = "ore";
        ore.quantity = static_cast<int>(effective_yield);
        ore.volume   = deposit->volume_per_unit;
        inv->items.push_back(ore);
    }
}

} // namespace systems
} // namespace atlas
