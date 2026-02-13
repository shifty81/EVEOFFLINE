#include "systems/movement_system.h"
#include "ecs/world.h"
#include "components/game_components.h"
#include <cmath>

namespace atlas {
namespace systems {

// Minimum distance required to initiate warp (150 km)
static constexpr float MIN_WARP_DISTANCE = 150000.0f;
// 1 AU in meters
static constexpr float AU_IN_METERS = 149597870700.0f;
// Default warp duration if no Ship component (fallback: 10 seconds)
static constexpr float DEFAULT_WARP_DURATION = 10.0f;
// Default align time if no Ship component (fallback: 2.5 seconds)
static constexpr float DEFAULT_ALIGN_TIME = 2.5f;

MovementSystem::MovementSystem(ecs::World* world)
    : System(world) {
}

void MovementSystem::setCollisionZones(const std::vector<CollisionZone>& zones) {
    m_collisionZones = zones;
}

void MovementSystem::update(float delta_time) {
    // Process movement commands (orbit, approach, warp)
    for (auto it = movement_commands_.begin(); it != movement_commands_.end(); ) {
        auto* entity = world_->getEntity(it->first);
        if (!entity) {
            it = movement_commands_.erase(it);
            continue;
        }

        auto* pos = entity->getComponent<components::Position>();
        auto* vel = entity->getComponent<components::Velocity>();
        if (!pos || !vel) {
            ++it;
            continue;
        }

        auto& cmd = it->second;

        if (cmd.type == MovementCommand::Type::Approach) {
            auto* target = world_->getEntity(cmd.target_id);
            if (target) {
                auto* tpos = target->getComponent<components::Position>();
                if (tpos) {
                    float dx = tpos->x - pos->x;
                    float dy = tpos->y - pos->y;
                    float dz = tpos->z - pos->z;
                    float dist = std::sqrt(dx * dx + dy * dy + dz * dz);
                    if (dist > 0.001f) {
                        float invDist = 1.0f / dist;
                        vel->vx = dx * invDist * vel->max_speed;
                        vel->vy = dy * invDist * vel->max_speed;
                        vel->vz = dz * invDist * vel->max_speed;
                    }
                }
            }
        } else if (cmd.type == MovementCommand::Type::Orbit) {
            auto* target = world_->getEntity(cmd.target_id);
            if (target) {
                auto* tpos = target->getComponent<components::Position>();
                if (tpos) {
                    float dx = pos->x - tpos->x;
                    float dy = pos->y - tpos->y;
                    float dz = pos->z - tpos->z;
                    float dist = std::sqrt(dx * dx + dy * dy + dz * dz);
                    if (dist > 0.001f) {
                        float invDist = 1.0f / dist;
                        float nx = dx * invDist;
                        float ny = dy * invDist;
                        float nz = dz * invDist;
                        // Tangential velocity (perpendicular to radial in XZ plane)
                        float tx = -nz;
                        float ty = 0.0f;
                        float tz = nx;
                        float tLen = std::sqrt(tx * tx + ty * ty + tz * tz);
                        if (tLen < 0.001f) {
                            tx = 1.0f; ty = 0.0f; tz = 0.0f;
                            tLen = 1.0f;
                        }
                        tx /= tLen; ty /= tLen; tz /= tLen;

                        // Radial correction toward desired orbit distance
                        float radial_error = dist - cmd.orbit_distance;
                        float radial_factor = radial_error / (cmd.orbit_distance + 1.0f);
                        radial_factor = std::max(-1.0f, std::min(1.0f, radial_factor));

                        float tangent_weight = 1.0f - std::fabs(radial_factor);
                        vel->vx = (tx * tangent_weight - nx * radial_factor) * vel->max_speed;
                        vel->vy = (ty * tangent_weight - ny * radial_factor) * vel->max_speed;
                        vel->vz = (tz * tangent_weight - nz * radial_factor) * vel->max_speed;
                    }
                }
            }
        } else if (cmd.type == MovementCommand::Type::Warp) {
            // Progress rate derived from computed warp duration
            float progress_rate = (cmd.warp_duration > 0.0f) ? (1.0f / cmd.warp_duration) : 0.1f;
            cmd.warp_progress += delta_time * progress_rate;
            
            // Update WarpState component for phase tracking
            auto* warpState = entity->getComponent<components::WarpState>();
            if (warpState) {
                warpState->warp_time += delta_time;
                warpState->distance_remaining = (1.0f - cmd.warp_progress) * 
                    std::sqrt((cmd.warp_dest_x - pos->x) * (cmd.warp_dest_x - pos->x) +
                              (cmd.warp_dest_y - pos->y) * (cmd.warp_dest_y - pos->y) +
                              (cmd.warp_dest_z - pos->z) * (cmd.warp_dest_z - pos->z));
                warpState->intensity = std::min(1.0f, warpState->warp_time * 0.5f + warpState->mass_norm * 0.3f);
                
                // Phase transitions based on progress
                if (cmd.warp_progress < 0.1f) {
                    warpState->phase = components::WarpState::WarpPhase::Align;
                } else if (cmd.warp_progress < 0.2f) {
                    warpState->phase = components::WarpState::WarpPhase::Entry;
                } else if (cmd.warp_progress < 0.85f) {
                    warpState->phase = components::WarpState::WarpPhase::Cruise;
                } else {
                    warpState->phase = components::WarpState::WarpPhase::Exit;
                }
            }
            
            if (cmd.warp_progress >= 1.0f) {
                pos->x = cmd.warp_dest_x;
                pos->y = cmd.warp_dest_y;
                pos->z = cmd.warp_dest_z;
                vel->vx = 0.0f;
                vel->vy = 0.0f;
                vel->vz = 0.0f;
                // Reset WarpState on arrival
                if (warpState) {
                    warpState->phase = components::WarpState::WarpPhase::None;
                    warpState->warp_time = 0.0f;
                    warpState->distance_remaining = 0.0f;
                    warpState->intensity = 0.0f;
                }
                it = movement_commands_.erase(it);
                continue;
            }
        }

        ++it;
    }

    // Get all entities with Position and Velocity components
    auto entities = world_->getEntities<components::Position, components::Velocity>();
    
    for (auto* entity : entities) {
        auto* pos = entity->getComponent<components::Position>();
        auto* vel = entity->getComponent<components::Velocity>();
        
        if (!pos || !vel) continue;
        
        // Update position based on velocity
        pos->x += vel->vx * delta_time;
        pos->y += vel->vy * delta_time;
        pos->z += vel->vz * delta_time;
        pos->rotation += vel->angular_velocity * delta_time;
        
        // Calculate current speed
        float speed = std::sqrt(vel->vx * vel->vx + vel->vy * vel->vy + vel->vz * vel->vz);
        
        // Apply speed limit
        if (speed > vel->max_speed && speed > 0.0f) {
            float factor = vel->max_speed / speed;
            vel->vx *= factor;
            vel->vy *= factor;
            vel->vz *= factor;
        }
        
        // Enforce celestial collision zones
        for (const auto& zone : m_collisionZones) {
            float dx = pos->x - zone.x;
            float dy = pos->y - zone.y;
            float dz = pos->z - zone.z;
            float dist = std::sqrt(dx * dx + dy * dy + dz * dz);
            
            if (dist < zone.radius && dist > 0.001f) {
                // Push entity to edge of collision zone
                float pushFactor = (zone.radius + COLLISION_PUSH_MARGIN) / dist;
                pos->x = zone.x + dx * pushFactor;
                pos->y = zone.y + dy * pushFactor;
                pos->z = zone.z + dz * pushFactor;
                
                // Kill velocity toward the celestial
                float invDist = 1.0f / dist;
                float nx = dx * invDist;
                float ny = dy * invDist;
                float nz = dz * invDist;
                float velToward = -(vel->vx * nx + vel->vy * ny + vel->vz * nz);
                if (velToward > 0.0f) {
                    vel->vx += nx * velToward;
                    vel->vy += ny * velToward;
                    vel->vz += nz * velToward;
                }
            }
        }
    }
}

void MovementSystem::commandOrbit(const std::string& entity_id,
                                   const std::string& target_id,
                                   float distance) {
    MovementCommand cmd;
    cmd.type = MovementCommand::Type::Orbit;
    cmd.target_id = target_id;
    cmd.orbit_distance = distance;
    movement_commands_[entity_id] = cmd;
}

void MovementSystem::commandApproach(const std::string& entity_id,
                                      const std::string& target_id) {
    MovementCommand cmd;
    cmd.type = MovementCommand::Type::Approach;
    cmd.target_id = target_id;
    movement_commands_[entity_id] = cmd;
}

void MovementSystem::commandStop(const std::string& entity_id) {
    movement_commands_.erase(entity_id);
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return;
    auto* vel = entity->getComponent<components::Velocity>();
    if (vel) {
        vel->vx = 0.0f;
        vel->vy = 0.0f;
        vel->vz = 0.0f;
    }
}

bool MovementSystem::commandWarp(const std::string& entity_id,
                                  float dest_x, float dest_y, float dest_z) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* pos = entity->getComponent<components::Position>();
    if (!pos) return false;

    // Check minimum warp distance (150km in EVE)
    float dx = dest_x - pos->x;
    float dy = dest_y - pos->y;
    float dz = dest_z - pos->z;
    float dist = std::sqrt(dx * dx + dy * dy + dz * dz);
    if (dist < MIN_WARP_DISTANCE) return false;  // too close to warp

    // Check warp disruption: if disrupted, cannot warp
    if (isWarpDisrupted(entity_id)) return false;

    // Read ship-class warp parameters if Ship component present
    float warp_speed_au = 3.0f;
    float align_time = DEFAULT_ALIGN_TIME;
    auto* ship = entity->getComponent<components::Ship>();
    if (ship) {
        warp_speed_au = ship->warp_speed_au;
        align_time = ship->align_time;
    }

    // Compute warp duration: align_time + (distance_AU / warp_speed_AU)
    float distance_au = dist / AU_IN_METERS;
    float travel_time = (warp_speed_au > 0.0f) ? (distance_au / warp_speed_au) : 0.0f;
    float warp_duration = align_time + travel_time;
    if (warp_duration < 1.0f) warp_duration = 1.0f;  // minimum 1 second

    // Initialize WarpState if present
    auto* warpState = entity->getComponent<components::WarpState>();
    if (warpState) {
        warpState->phase = components::WarpState::WarpPhase::Align;
        warpState->warp_time = 0.0f;
        warpState->distance_remaining = dist;
        warpState->warp_speed = warp_speed_au;
        warpState->intensity = 0.0f;
    }

    MovementCommand cmd;
    cmd.type = MovementCommand::Type::Warp;
    cmd.warp_dest_x = dest_x;
    cmd.warp_dest_y = dest_y;
    cmd.warp_dest_z = dest_z;
    cmd.warp_progress = 0.0f;
    cmd.warp_duration = warp_duration;
    cmd.align_time = align_time;
    cmd.warping = true;
    movement_commands_[entity_id] = cmd;
    return true;
}

bool MovementSystem::isWarpDisrupted(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* warpState = entity->getComponent<components::WarpState>();
    if (!warpState) return false;

    int warp_strength = 1;  // default warp core strength
    auto* ship = entity->getComponent<components::Ship>();
    if (ship) {
        warp_strength = ship->warp_strength;
    }

    return warpState->warp_disrupt_strength >= warp_strength;
}

} // namespace systems
} // namespace atlas
