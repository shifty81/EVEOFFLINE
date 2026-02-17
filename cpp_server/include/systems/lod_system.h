#ifndef EVE_SYSTEMS_LOD_SYSTEM_H
#define EVE_SYSTEMS_LOD_SYSTEM_H

#include "ecs/system.h"
#include <string>
#include <vector>

namespace atlas {
namespace systems {

/**
 * @brief Server-side LOD priority manager for large-battle optimisation
 *
 * Each tick the system iterates all entities that carry a LODPriority
 * component and recomputes their priority based on distance from a
 * configurable reference point (typically the player / camera).
 *
 * The client uses the resulting priority values to decide rendering
 * detail level and update rate.
 *
 * LOD tiers:
 *   Distance < nearThreshold   → priority 2.0  (full detail, 30 Hz)
 *   Distance < midThreshold    → priority 1.0  (reduced, 15 Hz)
 *   Distance < farThreshold    → priority 0.5  (merged mesh, 5 Hz)
 *   Distance >= farThreshold   → priority 0.1  (impostor/billboard, 1 Hz)
 *
 * Entities with force_visible == true always keep priority >= 2.0.
 */
class LODSystem : public ecs::System {
public:
    explicit LODSystem(ecs::World* world);
    ~LODSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "LODSystem"; }

    // ---------------------------------------------------------------
    // Reference point (the position that LOD is measured from)
    // ---------------------------------------------------------------

    /** Set the reference position for distance calculations */
    void setReferencePoint(float x, float y, float z);
    void getReferencePoint(float& x, float& y, float& z) const;

    // ---------------------------------------------------------------
    // Distance thresholds
    // ---------------------------------------------------------------

    void setNearThreshold(float d);
    void setMidThreshold(float d);
    void setFarThreshold(float d);

    float getNearThreshold() const { return near_threshold_; }
    float getMidThreshold()  const { return mid_threshold_; }
    float getFarThreshold()  const { return far_threshold_; }

    // ---------------------------------------------------------------
    // Queries
    // ---------------------------------------------------------------

    /** Get the number of entities at each LOD tier after the last update */
    int getFullDetailCount() const { return full_detail_count_; }
    int getReducedCount()    const { return reduced_count_; }
    int getMergedCount()     const { return merged_count_; }
    int getImpostorCount()   const { return impostor_count_; }

    /** Compute distance from ref to an entity (squared, for speed) */
    float distanceSqToEntity(const std::string& entity_id) const;

private:
    float ref_x_ = 0.0f, ref_y_ = 0.0f, ref_z_ = 0.0f;

    float near_threshold_ = 5000.0f;   // 5 km
    float mid_threshold_  = 20000.0f;  // 20 km
    float far_threshold_  = 80000.0f;  // 80 km

    int full_detail_count_ = 0;
    int reduced_count_     = 0;
    int merged_count_      = 0;
    int impostor_count_    = 0;
};

} // namespace systems
} // namespace atlas

#endif // EVE_SYSTEMS_LOD_SYSTEM_H
