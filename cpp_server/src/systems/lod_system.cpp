#include "systems/lod_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <cmath>

namespace atlas {
namespace systems {

LODSystem::LODSystem(ecs::World* world)
    : System(world) {
}

void LODSystem::setReferencePoint(float x, float y, float z) {
    ref_x_ = x;
    ref_y_ = y;
    ref_z_ = z;
}

void LODSystem::getReferencePoint(float& x, float& y, float& z) const {
    x = ref_x_;
    y = ref_y_;
    z = ref_z_;
}

void LODSystem::setNearThreshold(float d) { near_threshold_ = d; }
void LODSystem::setMidThreshold(float d)  { mid_threshold_  = d; }
void LODSystem::setFarThreshold(float d)  { far_threshold_  = d; }

void LODSystem::update(float /*delta_time*/) {
    full_detail_count_ = 0;
    reduced_count_     = 0;
    merged_count_      = 0;
    impostor_count_    = 0;

    const float near_sq = near_threshold_ * near_threshold_;
    const float mid_sq  = mid_threshold_  * mid_threshold_;
    const float far_sq  = far_threshold_  * far_threshold_;

    auto entities = world_->getEntities();
    for (auto* entity : entities) {
        auto* lod = entity->getComponent<components::LODPriority>();
        if (!lod) continue;

        auto* pos = entity->getComponent<components::Position>();
        if (!pos) continue;

        float dx = pos->x - ref_x_;
        float dy = pos->y - ref_y_;
        float dz = pos->z - ref_z_;
        float distSq = dx * dx + dy * dy + dz * dz;

        if (lod->force_visible) {
            lod->priority = 2.0f;
            ++full_detail_count_;
        } else if (distSq < near_sq) {
            lod->priority = 2.0f;
            ++full_detail_count_;
        } else if (distSq < mid_sq) {
            lod->priority = 1.0f;
            ++reduced_count_;
        } else if (distSq < far_sq) {
            lod->priority = 0.5f;
            ++merged_count_;
        } else {
            lod->priority = 0.1f;
            ++impostor_count_;
        }
    }
}

float LODSystem::distanceSqToEntity(const std::string& entity_id) const {
    const auto* entity = world_->getEntity(entity_id);
    if (!entity) return -1.0f;

    const auto* pos = entity->getComponent<components::Position>();
    if (!pos) return -1.0f;

    float dx = pos->x - ref_x_;
    float dy = pos->y - ref_y_;
    float dz = pos->z - ref_z_;
    return dx * dx + dy * dy + dz * dz;
}

} // namespace systems
} // namespace atlas
