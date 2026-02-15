#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <typeindex>
#include <memory>
#include <functional>
#include <any>
#include <string>

namespace atlas::ecs {

using EntityID = uint32_t;
using ComponentTypeID = uint32_t;

struct ComponentData {
    std::vector<uint8_t> data;
    size_t elementSize = 0;
};

class World {
public:
    EntityID CreateEntity();
    void DestroyEntity(EntityID id);

    bool IsAlive(EntityID id) const;
    std::vector<EntityID> GetEntities() const;
    size_t EntityCount() const;

    void Update(float dt);

    void SetTickCallback(std::function<void(float)> cb);

    // Component management
    template<typename T>
    void AddComponent(EntityID id, const T& component) {
        auto key = std::type_index(typeid(T));
        m_components[id][key] = component;
    }

    template<typename T>
    T* GetComponent(EntityID id) {
        auto it = m_components.find(id);
        if (it == m_components.end()) return nullptr;
        auto key = std::type_index(typeid(T));
        auto cit = it->second.find(key);
        if (cit == it->second.end()) return nullptr;
        return std::any_cast<T>(&cit->second);
    }

    template<typename T>
    bool HasComponent(EntityID id) const {
        auto it = m_components.find(id);
        if (it == m_components.end()) return false;
        auto key = std::type_index(typeid(T));
        return it->second.find(key) != it->second.end();
    }

    template<typename T>
    void RemoveComponent(EntityID id) {
        auto it = m_components.find(id);
        if (it == m_components.end()) return;
        auto key = std::type_index(typeid(T));
        it->second.erase(key);
    }

    std::vector<std::type_index> GetComponentTypes(EntityID id) const;

private:
    EntityID m_nextID = 1;
    std::vector<EntityID> m_entities;
    std::function<void(float)> m_tickCallback;

    // Component storage: entity -> (type -> data)
    std::unordered_map<EntityID, std::unordered_map<std::type_index, std::any>> m_components;
};

}
