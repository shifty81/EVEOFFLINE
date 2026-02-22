#include "systems/snapshot_replication_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <sstream>
#include <chrono>
#include <cmath>

namespace atlas {
namespace systems {

SnapshotReplicationSystem::SnapshotReplicationSystem(ecs::World* world)
    : System(world) {
}

void SnapshotReplicationSystem::update(float /*delta_time*/) {
    // No per-tick work needed; deltas are computed on demand in
    // buildDeltaUpdate / buildFullUpdate.
}

// ------------------------------------------------------------------
// Delta helpers
// ------------------------------------------------------------------

bool SnapshotReplicationSystem::hasPositionChanged(
        const EntitySnapshot& prev,
        float x, float y, float z, float rot) const {
    return std::fabs(prev.x - x) > position_epsilon_
        || std::fabs(prev.y - y) > position_epsilon_
        || std::fabs(prev.z - z) > position_epsilon_
        || std::fabs(prev.rotation - rot) > position_epsilon_;
}

bool SnapshotReplicationSystem::hasVelocityChanged(
        const EntitySnapshot& prev,
        float vx, float vy, float vz) const {
    return std::fabs(prev.vx - vx) > position_epsilon_
        || std::fabs(prev.vy - vy) > position_epsilon_
        || std::fabs(prev.vz - vz) > position_epsilon_;
}

bool SnapshotReplicationSystem::hasHealthChanged(
        const EntitySnapshot& prev,
        float s, float a, float h,
        float sm, float am, float hm) const {
    return std::fabs(prev.shield_hp - s) > health_epsilon_
        || std::fabs(prev.armor_hp - a)  > health_epsilon_
        || std::fabs(prev.hull_hp - h)   > health_epsilon_
        || std::fabs(prev.shield_max - sm) > health_epsilon_
        || std::fabs(prev.armor_max - am)  > health_epsilon_
        || std::fabs(prev.hull_max - hm)   > health_epsilon_;
}

bool SnapshotReplicationSystem::hasCapacitorChanged(
        const EntitySnapshot& prev,
        float cap, float cap_max) const {
    return std::fabs(prev.capacitor - cap) > health_epsilon_
        || std::fabs(prev.capacitor_max - cap_max) > health_epsilon_;
}

// ------------------------------------------------------------------
// buildDeltaUpdate
// ------------------------------------------------------------------

std::string SnapshotReplicationSystem::buildDeltaUpdate(int client_id,
                                                         uint64_t sequence) {
    auto& snap_map = client_snapshots_[client_id];

    std::ostringstream json;
    json << "{\"type\":\"state_update\",\"data\":{"
         << "\"sequence\":" << sequence << ","
         << "\"timestamp\":"
         << std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now().time_since_epoch()).count()
         << ",\"delta\":true,\"entities\":[";

    auto entities = world_->getAllEntities();
    bool first = true;

    for (const auto* entity : entities) {
        const std::string& eid = entity->getId();
        auto& prev = snap_map[eid];  // inserts default if missing

        auto* pos  = entity->getComponent<components::Position>();
        auto* vel  = entity->getComponent<components::Velocity>();
        auto* hp   = entity->getComponent<components::Health>();
        auto* cap  = entity->getComponent<components::Capacitor>();
        auto* ship = entity->getComponent<components::Ship>();
        auto* fac  = entity->getComponent<components::Faction>();

        // First time this entity is seen by this client → full state
        bool full = !prev.has_data;

        // Detect field-level changes
        bool pos_changed = false;
        bool vel_changed = false;
        bool hp_changed  = false;
        bool cap_changed = false;
        bool ship_changed = false;
        bool fac_changed  = false;

        if (pos) {
            pos_changed = full || hasPositionChanged(prev, pos->x, pos->y,
                                                     pos->z, pos->rotation);
        }
        if (vel) {
            vel_changed = full || hasVelocityChanged(prev, vel->vx, vel->vy,
                                                     vel->vz);
        }
        if (hp) {
            hp_changed = full || hasHealthChanged(prev, hp->shield_hp,
                                                  hp->armor_hp, hp->hull_hp,
                                                  hp->shield_max,
                                                  hp->armor_max,
                                                  hp->hull_max);
        }
        if (cap) {
            cap_changed = full || hasCapacitorChanged(prev, cap->capacitor,
                                                      cap->capacitor_max);
        }
        if (ship) {
            ship_changed = full || (prev.ship_type != ship->ship_type)
                                || (prev.ship_name != ship->ship_name);
        }
        if (fac) {
            fac_changed = full || (prev.faction_name != fac->faction_name);
        }

        // Skip entity entirely if nothing changed
        if (!pos_changed && !vel_changed && !hp_changed &&
            !cap_changed && !ship_changed && !fac_changed) {
            continue;
        }

        if (!first) json << ",";
        first = false;

        json << "{\"id\":\"" << eid << "\"";

        if (pos_changed && pos) {
            json << ",\"pos\":{\"x\":" << pos->x
                 << ",\"y\":" << pos->y
                 << ",\"z\":" << pos->z
                 << ",\"rot\":" << pos->rotation << "}";
            prev.x = pos->x;
            prev.y = pos->y;
            prev.z = pos->z;
            prev.rotation = pos->rotation;
        }
        if (vel_changed && vel) {
            json << ",\"vel\":{\"vx\":" << vel->vx
                 << ",\"vy\":" << vel->vy
                 << ",\"vz\":" << vel->vz << "}";
            prev.vx = vel->vx;
            prev.vy = vel->vy;
            prev.vz = vel->vz;
        }
        if (hp_changed && hp) {
            json << ",\"health\":{"
                 << "\"shield\":" << hp->shield_hp
                 << ",\"armor\":" << hp->armor_hp
                 << ",\"hull\":" << hp->hull_hp
                 << ",\"max_shield\":" << hp->shield_max
                 << ",\"max_armor\":" << hp->armor_max
                 << ",\"max_hull\":" << hp->hull_max << "}";
            prev.shield_hp  = hp->shield_hp;
            prev.armor_hp   = hp->armor_hp;
            prev.hull_hp    = hp->hull_hp;
            prev.shield_max = hp->shield_max;
            prev.armor_max  = hp->armor_max;
            prev.hull_max   = hp->hull_max;
        }
        if (cap_changed && cap) {
            json << ",\"capacitor\":{"
                 << "\"current\":" << cap->capacitor
                 << ",\"max\":" << cap->capacitor_max << "}";
            prev.capacitor     = cap->capacitor;
            prev.capacitor_max = cap->capacitor_max;
        }
        if (ship_changed && ship) {
            json << ",\"ship_type\":\"" << ship->ship_type << "\"";
            json << ",\"ship_name\":\"" << ship->ship_name << "\"";
            prev.ship_type = ship->ship_type;
            prev.ship_name = ship->ship_name;
        }
        if (fac_changed && fac) {
            json << ",\"faction\":\"" << fac->faction_name << "\"";
            prev.faction_name = fac->faction_name;
        }

        json << "}";
        prev.has_data = true;
    }

    json << "]}}";
    return json.str();
}

// ------------------------------------------------------------------
// buildFullUpdate
// ------------------------------------------------------------------

std::string SnapshotReplicationSystem::buildFullUpdate(int client_id,
                                                        uint64_t sequence) {
    // Clear tracked state so everything is treated as new
    client_snapshots_[client_id].clear();
    return buildDeltaUpdate(client_id, sequence);
}

// ------------------------------------------------------------------
// Client lifecycle
// ------------------------------------------------------------------

void SnapshotReplicationSystem::clearClient(int client_id) {
    client_snapshots_.erase(client_id);
}

size_t SnapshotReplicationSystem::getTrackedEntityCount(int client_id) const {
    auto it = client_snapshots_.find(client_id);
    if (it == client_snapshots_.end()) return 0;
    return it->second.size();
}

} // namespace systems
} // namespace atlas
