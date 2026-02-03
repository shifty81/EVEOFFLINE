#include "ecs/world.h"
#include <iostream>

namespace eve {
namespace ecs {

Entity* World::createEntity(const std::string& id) {
    auto entity = std::make_unique<Entity>(id);
    Entity* ptr = entity.get();
    entities_[id] = std::move(entity);
    return ptr;
}

void World::destroyEntity(const std::string& id) {
    entities_.erase(id);
}

Entity* World::getEntity(const std::string& id) {
    auto it = entities_.find(id);
    if (it != entities_.end()) {
        return it->second.get();
    }
    return nullptr;
}

const Entity* World::getEntity(const std::string& id) const {
    auto it = entities_.find(id);
    if (it != entities_.end()) {
        return it->second.get();
    }
    return nullptr;
}

std::vector<Entity*> World::getAllEntities() {
    std::vector<Entity*> result;
    result.reserve(entities_.size());
    
    for (auto& pair : entities_) {
        result.push_back(pair.second.get());
    }
    
    return result;
}

void World::addSystem(std::unique_ptr<System> system) {
    systems_.push_back(std::move(system));
}

void World::update(float delta_time) {
    for (auto& system : systems_) {
        system->update(delta_time);
    }
}

} // namespace ecs
} // namespace eve
