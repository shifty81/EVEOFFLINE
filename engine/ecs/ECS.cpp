#include "ECS.h"
#include <algorithm>

namespace atlas::ecs {

EntityID World::CreateEntity() {
    EntityID id = m_nextID++;
    m_entities.push_back(id);
    return id;
}

void World::DestroyEntity(EntityID id) {
    m_entities.erase(
        std::remove(m_entities.begin(), m_entities.end(), id),
        m_entities.end()
    );
    m_components.erase(id);
}

bool World::IsAlive(EntityID id) const {
    return std::find(m_entities.begin(), m_entities.end(), id) != m_entities.end();
}

std::vector<EntityID> World::GetEntities() const {
    return m_entities;
}

size_t World::EntityCount() const {
    return m_entities.size();
}

void World::Update(float dt) {
    if (m_tickCallback) {
        m_tickCallback(dt);
    }
}

void World::SetTickCallback(std::function<void(float)> cb) {
    m_tickCallback = std::move(cb);
}

std::vector<std::type_index> World::GetComponentTypes(EntityID id) const {
    std::vector<std::type_index> types;
    auto it = m_components.find(id);
    if (it != m_components.end()) {
        for (const auto& pair : it->second) {
            types.push_back(pair.first);
        }
    }
    return types;
}

}
