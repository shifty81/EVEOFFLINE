#include "systems/interest_management_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <cmath>

namespace atlas {
namespace systems {

// Static empty set for unknown clients
const std::unordered_set<std::string> InterestManagementSystem::empty_set_;

InterestManagementSystem::InterestManagementSystem(ecs::World* world)
    : System(world) {
}

// ------------------------------------------------------------------
// Client registration
// ------------------------------------------------------------------

void InterestManagementSystem::registerClient(int client_id,
                                               const std::string& entity_id) {
    auto& cd = client_data_[client_id];
    cd.player_entity_id = entity_id;
    cd.relevant_entities.clear();
    cd.force_visible.clear();
    // The player's own entity is always force-visible
    cd.force_visible.insert(entity_id);
}

void InterestManagementSystem::unregisterClient(int client_id) {
    client_data_.erase(client_id);
}

void InterestManagementSystem::addForceVisible(int client_id,
                                                const std::string& entity_id) {
    auto it = client_data_.find(client_id);
    if (it != client_data_.end()) {
        it->second.force_visible.insert(entity_id);
    }
}

void InterestManagementSystem::removeForceVisible(int client_id,
                                                   const std::string& entity_id) {
    auto it = client_data_.find(client_id);
    if (it != client_data_.end()) {
        it->second.force_visible.erase(entity_id);
    }
}

// ------------------------------------------------------------------
// Per-tick update
// ------------------------------------------------------------------

void InterestManagementSystem::update(float /*delta_time*/) {
    const float far_sq = far_range_ * far_range_;

    auto all_entities = world_->getAllEntities();

    for (auto& kv : client_data_) {
        ClientData& cd = kv.second;
        cd.relevant_entities.clear();

        // Look up the player entity position
        const auto* player = world_->getEntity(cd.player_entity_id);
        if (!player) continue;

        const auto* player_pos = player->getComponent<components::Position>();
        if (!player_pos) continue;

        float px = player_pos->x;
        float py = player_pos->y;
        float pz = player_pos->z;

        for (const auto* entity : all_entities) {
            const std::string& eid = entity->getId();

            // Force-visible entities always included
            if (cd.force_visible.count(eid)) {
                cd.relevant_entities.insert(eid);
                continue;
            }

            // Check distance
            const auto* pos = entity->getComponent<components::Position>();
            if (!pos) {
                // Entities without position are always included (e.g. system-level entities)
                cd.relevant_entities.insert(eid);
                continue;
            }

            float dx = pos->x - px;
            float dy = pos->y - py;
            float dz = pos->z - pz;
            float dist_sq = dx * dx + dy * dy + dz * dz;

            if (dist_sq < far_sq) {
                cd.relevant_entities.insert(eid);
            }
        }
    }
}

// ------------------------------------------------------------------
// Queries
// ------------------------------------------------------------------

const std::unordered_set<std::string>&
InterestManagementSystem::getRelevantEntities(int client_id) const {
    auto it = client_data_.find(client_id);
    if (it == client_data_.end()) return empty_set_;
    return it->second.relevant_entities;
}

bool InterestManagementSystem::isRelevant(int client_id,
                                           const std::string& entity_id) const {
    auto it = client_data_.find(client_id);
    if (it == client_data_.end()) return false;
    return it->second.relevant_entities.count(entity_id) > 0;
}

size_t InterestManagementSystem::getRelevantCount(int client_id) const {
    auto it = client_data_.find(client_id);
    if (it == client_data_.end()) return 0;
    return it->second.relevant_entities.size();
}

} // namespace systems
} // namespace atlas
