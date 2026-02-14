#include "systems/fleet_system.h"
#include "ecs/world.h"
#include "components/game_components.h"
#include <algorithm>
#include <cmath>
#include <iostream>

namespace atlas {
namespace systems {

FleetSystem::FleetSystem(ecs::World* world)
    : System(world) {
}

void FleetSystem::update(float /*delta_time*/) {
    // Apply fleet bonuses each tick (handles member changes, booster changes)
    for (auto& [fleet_id, fleet] : fleets_) {
        applyFleetBonuses(fleet_id);
    }
}

// ---- Fleet lifecycle ----

std::string FleetSystem::createFleet(const std::string& commander_entity_id,
                                     const std::string& fleet_name) {
    auto* entity = world_->getEntity(commander_entity_id);
    if (!entity) return "";

    // Entity must not already be in a fleet
    if (entity_fleet_.find(commander_entity_id) != entity_fleet_.end()) return "";

    std::string fleet_id = "fleet_" + std::to_string(next_fleet_id_++);

    Fleet fleet;
    fleet.fleet_id = fleet_id;
    fleet.fleet_name = fleet_name;
    fleet.commander_entity_id = commander_entity_id;

    // Determine character name from Player component if available
    std::string char_name = fleet_name + " FC";
    auto* player = entity->getComponent<components::Player>();
    if (player) {
        char_name = player->character_name;
    }

    FleetMemberInfo info;
    info.entity_id = commander_entity_id;
    info.character_name = char_name;
    info.role = "FleetCommander";
    fleet.members[commander_entity_id] = info;

    fleets_[fleet_id] = std::move(fleet);
    entity_fleet_[commander_entity_id] = fleet_id;

    // Add FleetMembership component
    auto membership = std::make_unique<components::FleetMembership>();
    membership->fleet_id = fleet_id;
    membership->role = "FleetCommander";
    entity->addComponent(std::move(membership));

    return fleet_id;
}

bool FleetSystem::disbandFleet(const std::string& fleet_id,
                               const std::string& requester_entity_id) {
    auto it = fleets_.find(fleet_id);
    if (it == fleets_.end()) return false;

    // Only FC can disband
    if (it->second.commander_entity_id != requester_entity_id) return false;

    // Remove all members' FleetMembership components and bonuses
    for (auto& [eid, info] : it->second.members) {
        removeFleetBonuses(eid);
        auto* entity = world_->getEntity(eid);
        if (entity) {
            entity->removeComponent<components::FleetMembership>();
        }
        entity_fleet_.erase(eid);
    }

    fleets_.erase(it);
    return true;
}

// ---- Membership ----

bool FleetSystem::addMember(const std::string& fleet_id,
                            const std::string& entity_id,
                            const std::string& character_name) {
    auto it = fleets_.find(fleet_id);
    if (it == fleets_.end()) return false;

    // Check capacity
    if (it->second.members.size() >= it->second.max_members) return false;

    // Entity must exist
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    // Must not already be in a fleet
    if (entity_fleet_.find(entity_id) != entity_fleet_.end()) return false;

    FleetMemberInfo info;
    info.entity_id = entity_id;
    info.character_name = character_name;
    info.role = "Member";

    auto* player = entity->getComponent<components::Player>();
    if (player && character_name.empty()) {
        info.character_name = player->character_name;
    }

    it->second.members[entity_id] = info;
    entity_fleet_[entity_id] = fleet_id;

    // Add FleetMembership component
    auto membership = std::make_unique<components::FleetMembership>();
    membership->fleet_id = fleet_id;
    membership->role = "Member";
    entity->addComponent(std::move(membership));

    return true;
}

bool FleetSystem::removeMember(const std::string& fleet_id,
                               const std::string& entity_id) {
    auto it = fleets_.find(fleet_id);
    if (it == fleets_.end()) return false;

    auto member_it = it->second.members.find(entity_id);
    if (member_it == it->second.members.end()) return false;

    // Remove bonuses and component
    removeFleetBonuses(entity_id);
    auto* entity = world_->getEntity(entity_id);
    if (entity) {
        entity->removeComponent<components::FleetMembership>();
    }

    // Remove from squads
    for (auto& [squad_id, members] : it->second.squads) {
        members.erase(std::remove(members.begin(), members.end(), entity_id), members.end());
    }

    // Remove from booster slots
    for (auto booster_it = it->second.active_boosters.begin();
         booster_it != it->second.active_boosters.end(); ) {
        if (booster_it->second == entity_id) {
            booster_it = it->second.active_boosters.erase(booster_it);
        } else {
            ++booster_it;
        }
    }

    bool was_fc = (it->second.commander_entity_id == entity_id);
    it->second.members.erase(member_it);
    entity_fleet_.erase(entity_id);

    // If FC left, promote someone or disband
    if (was_fc) {
        if (it->second.members.empty()) {
            fleets_.erase(it);
        } else {
            // Promote first remaining member to FC
            auto& new_fc = it->second.members.begin()->second;
            new_fc.role = "FleetCommander";
            it->second.commander_entity_id = new_fc.entity_id;

            auto* new_fc_entity = world_->getEntity(new_fc.entity_id);
            if (new_fc_entity) {
                auto* fm = new_fc_entity->getComponent<components::FleetMembership>();
                if (fm) fm->role = "FleetCommander";
            }
        }
    }

    return true;
}

std::string FleetSystem::getFleetForEntity(const std::string& entity_id) const {
    auto it = entity_fleet_.find(entity_id);
    if (it != entity_fleet_.end()) return it->second;
    return "";
}

// ---- Roles ----

bool FleetSystem::promoteMember(const std::string& fleet_id,
                                const std::string& requester_entity_id,
                                const std::string& target_entity_id,
                                const std::string& new_role) {
    auto it = fleets_.find(fleet_id);
    if (it == fleets_.end()) return false;

    // Only FC can promote
    if (it->second.commander_entity_id != requester_entity_id) return false;

    auto member_it = it->second.members.find(target_entity_id);
    if (member_it == it->second.members.end()) return false;

    // Validate role
    if (new_role != "FleetCommander" && new_role != "WingCommander" &&
        new_role != "SquadCommander" && new_role != "Member") {
        return false;
    }

    // If promoting to FC, demote current FC
    if (new_role == "FleetCommander") {
        auto& old_fc = it->second.members[requester_entity_id];
        old_fc.role = "Member";
        auto* old_fc_entity = world_->getEntity(requester_entity_id);
        if (old_fc_entity) {
            auto* fm = old_fc_entity->getComponent<components::FleetMembership>();
            if (fm) fm->role = "Member";
        }
        it->second.commander_entity_id = target_entity_id;
    }

    member_it->second.role = new_role;
    auto* target_entity = world_->getEntity(target_entity_id);
    if (target_entity) {
        auto* fm = target_entity->getComponent<components::FleetMembership>();
        if (fm) fm->role = new_role;
    }

    return true;
}

// ---- Organization ----

bool FleetSystem::assignToSquad(const std::string& fleet_id,
                                const std::string& entity_id,
                                const std::string& squad_id) {
    auto it = fleets_.find(fleet_id);
    if (it == fleets_.end()) return false;

    // Entity must be in this fleet
    if (it->second.members.find(entity_id) == it->second.members.end()) return false;

    // Remove from any current squad
    for (auto& [sid, members] : it->second.squads) {
        members.erase(std::remove(members.begin(), members.end(), entity_id), members.end());
    }

    // Add to new squad (create if needed)
    it->second.squads[squad_id].push_back(entity_id);
    it->second.members[entity_id].squad_id = squad_id;

    auto* entity = world_->getEntity(entity_id);
    if (entity) {
        auto* fm = entity->getComponent<components::FleetMembership>();
        if (fm) fm->squad_id = squad_id;
    }

    return true;
}

bool FleetSystem::assignSquadToWing(const std::string& fleet_id,
                                    const std::string& squad_id,
                                    const std::string& wing_id) {
    auto it = fleets_.find(fleet_id);
    if (it == fleets_.end()) return false;

    // Squad must exist
    if (it->second.squads.find(squad_id) == it->second.squads.end()) return false;

    // Remove squad from any current wing
    for (auto& [wid, squads] : it->second.wings) {
        squads.erase(std::remove(squads.begin(), squads.end(), squad_id), squads.end());
    }

    // Add to new wing (create if needed)
    it->second.wings[wing_id].push_back(squad_id);

    // Update wing_id for all members in this squad
    for (const auto& eid : it->second.squads[squad_id]) {
        it->second.members[eid].wing_id = wing_id;
        auto* entity = world_->getEntity(eid);
        if (entity) {
            auto* fm = entity->getComponent<components::FleetMembership>();
            if (fm) fm->wing_id = wing_id;
        }
    }

    return true;
}

// ---- Bonuses ----

bool FleetSystem::setBooster(const std::string& fleet_id,
                             const std::string& booster_type,
                             const std::string& booster_entity_id) {
    auto it = fleets_.find(fleet_id);
    if (it == fleets_.end()) return false;

    // Booster must be a fleet member
    if (it->second.members.find(booster_entity_id) == it->second.members.end()) return false;

    // Validate booster type
    if (booster_type != "armor" && booster_type != "shield" &&
        booster_type != "skirmish" && booster_type != "information") {
        return false;
    }

    it->second.active_boosters[booster_type] = booster_entity_id;
    return true;
}

std::vector<FleetBonus> FleetSystem::getBonusesForType(const std::string& booster_type) const {
    std::vector<FleetBonus> bonuses;

    if (booster_type == "armor") {
        bonuses.push_back({"armor", "hp_bonus", 0.10f});
        bonuses.push_back({"armor", "resist_bonus", 0.05f});
    } else if (booster_type == "shield") {
        bonuses.push_back({"shield", "hp_bonus", 0.10f});
        bonuses.push_back({"shield", "resist_bonus", 0.05f});
    } else if (booster_type == "skirmish") {
        bonuses.push_back({"skirmish", "speed_bonus", 0.15f});
        bonuses.push_back({"skirmish", "agility_bonus", 0.10f});
    } else if (booster_type == "information") {
        bonuses.push_back({"information", "targeting_range_bonus", 0.20f});
        bonuses.push_back({"information", "scan_resolution_bonus", 0.15f});
    }

    return bonuses;
}

// ---- Coordination ----

int FleetSystem::broadcastTarget(const std::string& fleet_id,
                                 const std::string& /*broadcaster_entity_id*/,
                                 const std::string& target_entity_id) {
    auto it = fleets_.find(fleet_id);
    if (it == fleets_.end()) return 0;

    // Verify target exists
    auto* target = world_->getEntity(target_entity_id);
    if (!target) return 0;

    int count = 0;
    for (auto& [eid, info] : it->second.members) {
        auto* entity = world_->getEntity(eid);
        if (!entity) continue;

        auto* target_comp = entity->getComponent<components::Target>();
        if (!target_comp) continue;

        // Don't self-broadcast or broadcast already locked targets
        if (eid == target_entity_id) continue;
        auto locked_it = std::find(target_comp->locked_targets.begin(),
                                   target_comp->locked_targets.end(),
                                   target_entity_id);
        if (locked_it != target_comp->locked_targets.end()) continue;

        // Start locking if not already locking
        if (target_comp->locking_targets.find(target_entity_id) == target_comp->locking_targets.end()) {
            target_comp->locking_targets[target_entity_id] = 0.0f;
            count++;
        }
    }

    return count;
}

int FleetSystem::fleetWarp(const std::string& fleet_id,
                           const std::string& commander_entity_id,
                           float dest_x, float dest_y, float dest_z) {
    auto it = fleets_.find(fleet_id);
    if (it == fleets_.end()) return 0;

    // Only FC or Wing Commander can fleet warp
    auto member_it = it->second.members.find(commander_entity_id);
    if (member_it == it->second.members.end()) return 0;

    const std::string& role = member_it->second.role;
    if (role != "FleetCommander" && role != "WingCommander") return 0;

    int count = 0;
    for (auto& [eid, info] : it->second.members) {
        auto* entity = world_->getEntity(eid);
        if (!entity) continue;

        auto* vel = entity->getComponent<components::Velocity>();
        auto* pos = entity->getComponent<components::Position>();
        if (!vel || !pos) continue;

        // Set velocity toward destination (simplified warp initiation)
        float dx = dest_x - pos->x;
        float dy = dest_y - pos->y;
        float dz = dest_z - pos->z;
        float dist = std::sqrt(dx * dx + dy * dy + dz * dz);

        if (dist > 0.01f) {
            float speed = vel->max_speed;
            vel->vx = (dx / dist) * speed;
            vel->vy = (dy / dist) * speed;
            vel->vz = (dz / dist) * speed;
            count++;
        }
    }

    return count;
}

// ---- Queries ----

const Fleet* FleetSystem::getFleet(const std::string& fleet_id) const {
    auto it = fleets_.find(fleet_id);
    if (it != fleets_.end()) return &it->second;
    return nullptr;
}

size_t FleetSystem::getFleetCount() const {
    return fleets_.size();
}

size_t FleetSystem::getMemberCount(const std::string& fleet_id) const {
    auto it = fleets_.find(fleet_id);
    if (it != fleets_.end()) return it->second.members.size();
    return 0;
}

// ---- Private helpers ----

void FleetSystem::applyFleetBonuses(const std::string& fleet_id) {
    auto it = fleets_.find(fleet_id);
    if (it == fleets_.end()) return;

    // For each active booster, apply bonuses to all members
    for (const auto& [booster_type, booster_eid] : it->second.active_boosters) {
        auto bonuses = getBonusesForType(booster_type);
        for (auto& [eid, info] : it->second.members) {
            auto* entity = world_->getEntity(eid);
            if (!entity) continue;

            auto* fm = entity->getComponent<components::FleetMembership>();
            if (!fm) continue;

            for (const auto& bonus : bonuses) {
                std::string key = bonus.type + "_" + bonus.stat;
                fm->active_bonuses[key] = bonus.value;
            }
        }
    }
}

void FleetSystem::removeFleetBonuses(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return;

    auto* fm = entity->getComponent<components::FleetMembership>();
    if (!fm) return;

    fm->active_bonuses.clear();
}

// ---- Player Fleet (player + up to 4 captains) ----

std::string FleetSystem::createPlayerFleet(const std::string& player_entity_id,
                                            const std::string& fleet_name) {
    auto* entity = world_->getEntity(player_entity_id);
    if (!entity) return "";

    // Entity must not already be in a fleet
    if (entity_fleet_.find(player_entity_id) != entity_fleet_.end()) return "";

    std::string fleet_id = "fleet_" + std::to_string(next_fleet_id_++);

    Fleet fleet;
    fleet.fleet_id = fleet_id;
    fleet.fleet_name = fleet_name;
    fleet.commander_entity_id = player_entity_id;
    fleet.player_fleet = true;
    fleet.max_members = PLAYER_FLEET_MAX;

    std::string char_name = fleet_name + " FC";
    auto* player = entity->getComponent<components::Player>();
    if (player) {
        char_name = player->character_name;
    }

    FleetMemberInfo info;
    info.entity_id = player_entity_id;
    info.character_name = char_name;
    info.role = "FleetCommander";
    fleet.members[player_entity_id] = info;

    fleets_[fleet_id] = std::move(fleet);
    entity_fleet_[player_entity_id] = fleet_id;

    auto membership = std::make_unique<components::FleetMembership>();
    membership->fleet_id = fleet_id;
    membership->role = "FleetCommander";
    entity->addComponent(std::move(membership));

    return fleet_id;
}

bool FleetSystem::assignCaptain(const std::string& fleet_id,
                                 const std::string& captain_entity_id,
                                 const std::string& captain_name) {
    auto it = fleets_.find(fleet_id);
    if (it == fleets_.end()) return false;

    // Must be a player fleet
    if (!it->second.player_fleet) return false;

    // Check capacity (max 5 total = 1 player + 4 captains)
    if (it->second.members.size() >= PLAYER_FLEET_MAX) return false;

    // Entity must exist
    auto* entity = world_->getEntity(captain_entity_id);
    if (!entity) return false;

    // Must not already be in a fleet
    if (entity_fleet_.find(captain_entity_id) != entity_fleet_.end()) return false;

    FleetMemberInfo info;
    info.entity_id = captain_entity_id;
    info.character_name = captain_name;
    info.role = "Member";

    it->second.members[captain_entity_id] = info;
    entity_fleet_[captain_entity_id] = fleet_id;

    auto membership = std::make_unique<components::FleetMembership>();
    membership->fleet_id = fleet_id;
    membership->role = "Member";
    entity->addComponent(std::move(membership));

    return true;
}

bool FleetSystem::isPlayerFleet(const std::string& fleet_id) const {
    auto it = fleets_.find(fleet_id);
    if (it == fleets_.end()) return false;
    return it->second.player_fleet;
}

} // namespace systems
} // namespace atlas
