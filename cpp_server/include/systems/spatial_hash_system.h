#ifndef EVE_SYSTEMS_SPATIAL_HASH_SYSTEM_H
#define EVE_SYSTEMS_SPATIAL_HASH_SYSTEM_H

#include "ecs/system.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

namespace atlas {
namespace systems {

/**
 * @brief Spatial hash grid for O(1) proximity queries
 *
 * Partitions 3-D space into a uniform grid of cells.  Each entity
 * with a Position component is assigned to one cell per tick.
 * Nearby-entity queries only need to inspect the target cell plus
 * its immediate neighbours (27-cell neighbourhood in 3-D).
 *
 * Cell size should be at least as large as the largest interaction
 * radius in the game (e.g. weapon range or sensor range).
 *
 * Usage:
 *   SpatialHashSystem spatialHash(&world);
 *   spatialHash.setCellSize(5000.0f);   // 5 km cells
 *   spatialHash.update(dt);              // rebuild grid
 *   auto nearby = spatialHash.queryNear(x, y, z, 10000.0f);
 */
class SpatialHashSystem : public ecs::System {
public:
    explicit SpatialHashSystem(ecs::World* world);
    ~SpatialHashSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "SpatialHashSystem"; }

    // ---------------------------------------------------------------
    // Configuration
    // ---------------------------------------------------------------

    /** Set the size of each grid cell in world units (metres) */
    void setCellSize(float size);
    float getCellSize() const { return cell_size_; }

    // ---------------------------------------------------------------
    // Queries (valid after update())
    // ---------------------------------------------------------------

    /**
     * @brief Return entity IDs within `radius` of the given point.
     *
     * Only inspects cells that overlap the query sphere, making it
     * much faster than brute-force for large entity counts.
     */
    std::vector<std::string> queryNear(float x, float y, float z,
                                       float radius) const;

    /**
     * @brief Return entity IDs in the same cell as the named entity,
     *        plus its 26 neighbours.
     */
    std::vector<std::string> queryNeighbours(const std::string& entity_id) const;

    /** Total number of occupied cells after last update */
    int getOccupiedCellCount() const { return static_cast<int>(grid_.size()); }

    /** Total entities indexed */
    int getIndexedEntityCount() const { return indexed_count_; }

private:
    // Packed cell key from integer coordinates
    struct CellKey {
        int32_t cx, cy, cz;
        bool operator==(const CellKey& o) const {
            return cx == o.cx && cy == o.cy && cz == o.cz;
        }
    };

    struct CellKeyHash {
        std::size_t operator()(const CellKey& k) const {
            // Simple hash combining
            std::size_t h = std::hash<int32_t>()(k.cx);
            h ^= std::hash<int32_t>()(k.cy) + 0x9e3779b9 + (h << 6) + (h >> 2);
            h ^= std::hash<int32_t>()(k.cz) + 0x9e3779b9 + (h << 6) + (h >> 2);
            return h;
        }
    };

    CellKey cellKeyFor(float x, float y, float z) const;

    float cell_size_ = 5000.0f;
    int indexed_count_ = 0;

    // cell → list of entity IDs
    std::unordered_map<CellKey, std::vector<std::string>, CellKeyHash> grid_;

    // entity → cell key (for fast neighbour lookup)
    std::unordered_map<std::string, CellKey> entity_cells_;
};

} // namespace systems
} // namespace atlas

#endif // EVE_SYSTEMS_SPATIAL_HASH_SYSTEM_H
