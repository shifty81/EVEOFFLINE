#include "systems/targeting_system.h"
#include "ecs/world.h"
#include "components/game_components.h"
#include <algorithm>

namespace atlas {
namespace systems {

TargetingSystem::TargetingSystem(ecs::World* world)
    : System(world) {
}

void TargetingSystem::update(float delta_time) {
    // Get all entities with targeting capability
    auto entities = world_->getEntities<components::Target, components::Ship>();
    
    for (auto* entity : entities) {
        auto* target_comp = entity->getComponent<components::Target>();
        auto* ship = entity->getComponent<components::Ship>();
        
        if (!target_comp || !ship) continue;
        
        // Process locking targets
        std::vector<std::string> completed_locks;
        
        for (auto& [target_id, progress] : target_comp->locking_targets) {
            // Lock time based on scan resolution (simplified)
            // Higher scan resolution = faster locking
            float lock_time = 1000.0f / ship->scan_resolution;  // seconds
            progress += delta_time / lock_time;
            
            // Check if lock is complete
            if (progress >= 1.0f) {
                completed_locks.push_back(target_id);
            }
        }
        
        // Complete locks
        for (const auto& target_id : completed_locks) {
            // Check if we have room for more locked targets
            if (target_comp->locked_targets.size() < static_cast<size_t>(ship->max_locked_targets)) {
                target_comp->locked_targets.push_back(target_id);
            }
            // Remove from locking map
            target_comp->locking_targets.erase(target_id);
        }
    }
}

bool TargetingSystem::startLock(const std::string& entity_id, const std::string& target_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    
    auto* target_comp = entity->getComponent<components::Target>();
    auto* ship = entity->getComponent<components::Ship>();
    
    if (!target_comp || !ship) return false;
    
    // Check if already locked
    auto it = std::find(target_comp->locked_targets.begin(), 
                       target_comp->locked_targets.end(), 
                       target_id);
    if (it != target_comp->locked_targets.end()) {
        return true;  // Already locked
    }
    
    // Check if already locking
    if (target_comp->locking_targets.find(target_id) != target_comp->locking_targets.end()) {
        return true;  // Already locking
    }
    
    // Check max targets
    size_t total_targets = target_comp->locked_targets.size() + target_comp->locking_targets.size();
    if (total_targets >= static_cast<size_t>(ship->max_locked_targets)) {
        return false;  // Too many targets
    }
    
    // Check if target exists
    auto* target = world_->getEntity(target_id);
    if (!target) return false;
    
    // Start locking
    target_comp->locking_targets[target_id] = 0.0f;
    return true;
}

void TargetingSystem::unlockTarget(const std::string& entity_id, const std::string& target_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return;
    
    auto* target_comp = entity->getComponent<components::Target>();
    if (!target_comp) return;
    
    // Remove from locked targets
    auto it = std::find(target_comp->locked_targets.begin(), 
                       target_comp->locked_targets.end(), 
                       target_id);
    if (it != target_comp->locked_targets.end()) {
        target_comp->locked_targets.erase(it);
    }
    
    // Remove from locking targets
    target_comp->locking_targets.erase(target_id);
}

bool TargetingSystem::isTargetLocked(const std::string& entity_id, const std::string& target_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    
    auto* target_comp = entity->getComponent<components::Target>();
    if (!target_comp) return false;
    
    auto it = std::find(target_comp->locked_targets.begin(), 
                       target_comp->locked_targets.end(), 
                       target_id);
    return it != target_comp->locked_targets.end();
}

} // namespace systems
} // namespace atlas
