#include "systems/spatial_hash_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <cmath>

namespace atlas {
namespace systems {

SpatialHashSystem::SpatialHashSystem(ecs::World* world)
    : System(world) {
}

void SpatialHashSystem::setCellSize(float size) {
    if (size > 0.0f) cell_size_ = size;
}

SpatialHashSystem::CellKey SpatialHashSystem::cellKeyFor(float x, float y, float z) const {
    return {
        static_cast<int32_t>(std::floor(x / cell_size_)),
        static_cast<int32_t>(std::floor(y / cell_size_)),
        static_cast<int32_t>(std::floor(z / cell_size_))
    };
}

void SpatialHashSystem::update(float /*delta_time*/) {
    grid_.clear();
    entity_cells_.clear();
    indexed_count_ = 0;

    auto entities = world_->getEntities();
    for (auto* entity : entities) {
        auto* pos = entity->getComponent<components::Position>();
        if (!pos) continue;

        CellKey key = cellKeyFor(pos->x, pos->y, pos->z);
        const std::string& id = entity->getId();
        grid_[key].push_back(id);
        entity_cells_[id] = key;
        ++indexed_count_;
    }
}

std::vector<std::string> SpatialHashSystem::queryNear(
    float x, float y, float z, float radius) const {

    std::vector<std::string> result;
    const float radiusSq = radius * radius;

    // How many cells does the radius span?
    int span = static_cast<int>(std::ceil(radius / cell_size_));

    CellKey centre = cellKeyFor(x, y, z);

    for (int dx = -span; dx <= span; ++dx) {
        for (int dy = -span; dy <= span; ++dy) {
            for (int dz = -span; dz <= span; ++dz) {
                CellKey probe{centre.cx + dx, centre.cy + dy, centre.cz + dz};
                auto it = grid_.find(probe);
                if (it == grid_.end()) continue;

                for (const auto& id : it->second) {
                    auto* entity = world_->getEntity(id);
                    if (!entity) continue;
                    auto* pos = entity->getComponent<components::Position>();
                    if (!pos) continue;

                    float ex = pos->x - x;
                    float ey = pos->y - y;
                    float ez = pos->z - z;
                    if (ex * ex + ey * ey + ez * ez <= radiusSq) {
                        result.push_back(id);
                    }
                }
            }
        }
    }

    return result;
}

std::vector<std::string> SpatialHashSystem::queryNeighbours(
    const std::string& entity_id) const {

    std::vector<std::string> result;
    auto it = entity_cells_.find(entity_id);
    if (it == entity_cells_.end()) return result;

    const CellKey& centre = it->second;
    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            for (int dz = -1; dz <= 1; ++dz) {
                CellKey probe{centre.cx + dx, centre.cy + dy, centre.cz + dz};
                auto git = grid_.find(probe);
                if (git == grid_.end()) continue;
                for (const auto& id : git->second) {
                    if (id != entity_id) {
                        result.push_back(id);
                    }
                }
            }
        }
    }

    return result;
}

} // namespace systems
} // namespace atlas
