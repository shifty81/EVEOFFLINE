#ifndef EVE_SYSTEMS_SNAPSHOT_REPLICATION_SYSTEM_H
#define EVE_SYSTEMS_SNAPSHOT_REPLICATION_SYSTEM_H

#include "ecs/system.h"
#include "ecs/entity.h"
#include "ecs/world.h"
#include <string>
#include <unordered_map>
#include <cstdint>

namespace atlas {
namespace systems {

/**
 * @brief Delta-compressed snapshot replication for network state updates
 *
 * Tracks the last-sent state of each entity per client and computes
 * per-field deltas so that only changed values are included in each
 * state update.  When a client has no previous state for an entity
 * (first time seen or after a full-resync) the full state is sent.
 *
 * Position/velocity changes are detected using a configurable
 * tolerance (epsilon) to avoid sending micro-jitter.
 *
 * Usage:
 *   1. Each server tick, call buildDeltaUpdate(client_id) for every
 *      connected client to get a JSON state-update string that
 *      includes only changed fields.
 *   2. Call clearClient(client_id) when a client disconnects to
 *      free tracked state.
 */
class SnapshotReplicationSystem : public ecs::System {
public:
    explicit SnapshotReplicationSystem(ecs::World* world);
    ~SnapshotReplicationSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "SnapshotReplicationSystem"; }

    // ------------------------------------------------------------------
    // Per-entity cached state (what was last sent to a given client)
    // ------------------------------------------------------------------
    struct EntitySnapshot {
        // Position
        float x = 0.0f, y = 0.0f, z = 0.0f, rotation = 0.0f;
        // Velocity
        float vx = 0.0f, vy = 0.0f, vz = 0.0f;
        // Health
        float shield_hp = 0.0f, armor_hp = 0.0f, hull_hp = 0.0f;
        float shield_max = 0.0f, armor_max = 0.0f, hull_max = 0.0f;
        // Capacitor
        float capacitor = 0.0f, capacitor_max = 0.0f;
        // Ship info
        std::string ship_type;
        std::string ship_name;
        // Faction
        std::string faction_name;
        // Dirty tracking
        bool has_data = false;  // false = first time, full state needed
    };

    // ------------------------------------------------------------------
    // Public API
    // ------------------------------------------------------------------

    /**
     * Build a JSON state-update containing only changed entity fields
     * for the given client.  Entities not previously sent are
     * included in full.
     *
     * @param client_id Unique identifier for the client
     * @param sequence  Monotonically increasing snapshot sequence number
     * @return JSON string with format:
     *   {"type":"state_update","data":{"sequence":N,"timestamp":T,
     *    "delta":true,"entities":[...]}}
     */
    std::string buildDeltaUpdate(int client_id, uint64_t sequence);

    /**
     * Build a full (non-delta) state update for a client.
     * Resets the client's tracked state so subsequent calls to
     * buildDeltaUpdate will compute deltas from this baseline.
     */
    std::string buildFullUpdate(int client_id, uint64_t sequence);

    /** Remove all tracked state for a disconnected client */
    void clearClient(int client_id);

    /** Get the number of clients being tracked */
    size_t getTrackedClientCount() const { return client_snapshots_.size(); }

    /** Get the number of entities tracked for a client */
    size_t getTrackedEntityCount(int client_id) const;

    // ------------------------------------------------------------------
    // Configuration
    // ------------------------------------------------------------------

    /** Set position change tolerance */
    void setPositionEpsilon(float eps) { position_epsilon_ = eps; }
    float getPositionEpsilon() const { return position_epsilon_; }

    /** Set health change tolerance */
    void setHealthEpsilon(float eps) { health_epsilon_ = eps; }
    float getHealthEpsilon() const { return health_epsilon_; }

private:
    // Per-client map of entity_id → last sent snapshot
    using EntitySnapshotMap = std::unordered_map<std::string, EntitySnapshot>;
    std::unordered_map<int, EntitySnapshotMap> client_snapshots_;

    float position_epsilon_ = 0.1f;   // minimum position delta to report
    float health_epsilon_   = 0.5f;   // minimum health delta to report

    // Internal helpers
    bool hasPositionChanged(const EntitySnapshot& prev,
                            float x, float y, float z, float rot) const;
    bool hasVelocityChanged(const EntitySnapshot& prev,
                            float vx, float vy, float vz) const;
    bool hasHealthChanged(const EntitySnapshot& prev,
                          float s, float a, float h,
                          float sm, float am, float hm) const;
    bool hasCapacitorChanged(const EntitySnapshot& prev,
                             float cap, float cap_max) const;
};

} // namespace systems
} // namespace atlas

#endif // EVE_SYSTEMS_SNAPSHOT_REPLICATION_SYSTEM_H
