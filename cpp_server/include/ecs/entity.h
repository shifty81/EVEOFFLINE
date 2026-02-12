#ifndef EVE_ECS_ENTITY_H
#define EVE_ECS_ENTITY_H

#include "component.h"
#include <string>
#include <unordered_map>
#include <typeindex>
#include <memory>
#include <vector>

namespace atlas {
namespace ecs {

/**
 * @brief Entity represents a game object
 * 
 * Entities are just IDs with attached components.
 * They represent ships, NPCs, projectiles, stations, etc.
 */
class Entity {
public:
    explicit Entity(const std::string& id);
    ~Entity() = default;
    
    // Get entity ID
    const std::string& getId() const { return id_; }
    
    // Component management
    template<typename T>
    Entity& addComponent(std::unique_ptr<T> component);
    
    template<typename T>
    void removeComponent();
    
    template<typename T>
    T* getComponent();
    
    template<typename T>
    const T* getComponent() const;
    
    template<typename T>
    bool hasComponent() const;
    
    // Check if has all specified component types
    bool hasComponents(const std::vector<std::type_index>& types) const;
    
private:
    std::string id_;
    std::unordered_map<std::type_index, std::unique_ptr<Component>> components_;
};

// Template implementation
template<typename T>
Entity& Entity::addComponent(std::unique_ptr<T> component) {
    static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");
    components_[std::type_index(typeid(T))] = std::move(component);
    return *this;
}

template<typename T>
void Entity::removeComponent() {
    static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");
    components_.erase(std::type_index(typeid(T)));
}

template<typename T>
T* Entity::getComponent() {
    static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");
    auto it = components_.find(std::type_index(typeid(T)));
    if (it != components_.end()) {
        return static_cast<T*>(it->second.get());
    }
    return nullptr;
}

template<typename T>
const T* Entity::getComponent() const {
    static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");
    auto it = components_.find(std::type_index(typeid(T)));
    if (it != components_.end()) {
        return static_cast<const T*>(it->second.get());
    }
    return nullptr;
}

template<typename T>
bool Entity::hasComponent() const {
    static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");
    return components_.find(std::type_index(typeid(T))) != components_.end();
}

} // namespace ecs
} // namespace atlas

#endif // EVE_ECS_ENTITY_H
