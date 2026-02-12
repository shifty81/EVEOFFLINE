#ifndef EVE_ECS_WORLD_H
#define EVE_ECS_WORLD_H

#include "entity.h"
#include "system.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <typeindex>
#include <algorithm>

namespace atlas {
namespace ecs {

/**
 * @brief World manages all entities and systems
 * 
 * The World represents the game state and coordinates
 * all entities and systems in the game.
 */
class World {
public:
    World() = default;
    ~World() = default;
    
    // Entity management
    Entity* createEntity(const std::string& id);
    void destroyEntity(const std::string& id);
    Entity* getEntity(const std::string& id);
    const Entity* getEntity(const std::string& id) const;
    
    // Get all entities
    std::vector<Entity*> getAllEntities();
    
    // Get entities with specific components
    template<typename... ComponentTypes>
    std::vector<Entity*> getEntities();
    
    // System management
    void addSystem(std::unique_ptr<System> system);
    
    // Update all systems
    void update(float delta_time);
    
    // Get entity count
    size_t getEntityCount() const { return entities_.size(); }
    
private:
    std::unordered_map<std::string, std::unique_ptr<Entity>> entities_;
    std::vector<std::unique_ptr<System>> systems_;
    
    // Helper to get type indices from component types
    template<typename... ComponentTypes>
    std::vector<std::type_index> getTypeIndices();
};

// Template implementation
template<typename... ComponentTypes>
std::vector<Entity*> World::getEntities() {
    std::vector<Entity*> result;
    
    if constexpr (sizeof...(ComponentTypes) == 0) {
        // Return all entities if no component types specified
        return getAllEntities();
    } else {
        // Get type indices for the requested components
        std::vector<std::type_index> types = {std::type_index(typeid(ComponentTypes))...};
        
        // Filter entities that have all requested components
        for (auto& pair : entities_) {
            Entity* entity = pair.second.get();
            if (entity->hasComponents(types)) {
                result.push_back(entity);
            }
        }
    }
    
    return result;
}

} // namespace ecs
} // namespace atlas

#endif // EVE_ECS_WORLD_H
