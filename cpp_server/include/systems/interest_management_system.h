#ifndef EVE_SYSTEMS_INTEREST_MANAGEMENT_SYSTEM_H
#define EVE_SYSTEMS_INTEREST_MANAGEMENT_SYSTEM_H

#include "ecs/system.h"
#include "ecs/entity.h"
#include "ecs/world.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

namespace atlas {
namespace systems {

/**
 * @brief Per-client interest management for bandwidth optimisation
 *
 * Determines which entities are relevant for each connected client
 * based on distance from the client's player entity.  Entities beyond
 * the maximum range are excluded from state updates entirely, saving
 * bandwidth and CPU time.
 *
 * Priority tiers (matching LODSystem thresholds):
 *   near  (< nearRange)  → always included, full update rate
 *   mid   (< midRange)   → included, may use reduced update rate
 *   far   (< farRange)   → included at low rate
 *   beyond (>= farRange) → excluded unless force_visible
 *
 * force_visible entities (e.g. the client's own ship, fleet members,
 * locked targets) are always included regardless of distance.
 */
class InterestManagementSystem : public ecs::System {
public:
    explicit InterestManagementSystem(ecs::World* world);
    ~InterestManagementSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "InterestManagementSystem"; }

    // ------------------------------------------------------------------
    // Client registration
    // ------------------------------------------------------------------

    /** Register a client and their associated player entity */
    void registerClient(int client_id, const std::string& entity_id);

    /** Unregister a client (e.g. on disconnect) */
    void unregisterClient(int client_id);

    /** Mark an entity as force-visible for a specific client */
    void addForceVisible(int client_id, const std::string& entity_id);

    /** Remove force-visible flag for a specific client */
    void removeForceVisible(int client_id, const std::string& entity_id);

    // ------------------------------------------------------------------
    // Queries
    // ------------------------------------------------------------------

    /**
     * Get the set of entity IDs relevant for a given client.
     * Must be called after update() to reflect the latest positions.
     */
    const std::unordered_set<std::string>& getRelevantEntities(int client_id) const;

    /** Check if a specific entity is relevant for a client */
    bool isRelevant(int client_id, const std::string& entity_id) const;

    /** Get the number of registered clients */
    size_t getClientCount() const { return client_data_.size(); }

    /** Get count of relevant entities for a client */
    size_t getRelevantCount(int client_id) const;

    // ------------------------------------------------------------------
    // Configuration
    // ------------------------------------------------------------------

    void setNearRange(float d) { near_range_ = d; }
    void setMidRange(float d)  { mid_range_  = d; }
    void setFarRange(float d)  { far_range_  = d; }

    float getNearRange() const { return near_range_; }
    float getMidRange()  const { return mid_range_; }
    float getFarRange()  const { return far_range_; }

private:
    struct ClientData {
        std::string player_entity_id;
        std::unordered_set<std::string> relevant_entities;
        std::unordered_set<std::string> force_visible;
    };

    std::unordered_map<int, ClientData> client_data_;

    float near_range_ = 5000.0f;   // 5 km
    float mid_range_  = 20000.0f;  // 20 km
    float far_range_  = 80000.0f;  // 80 km

    // Empty set returned for unknown clients
    static const std::unordered_set<std::string> empty_set_;
};

} // namespace systems
} // namespace atlas

#endif // EVE_SYSTEMS_INTEREST_MANAGEMENT_SYSTEM_H
