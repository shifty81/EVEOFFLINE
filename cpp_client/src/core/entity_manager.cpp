#include "core/entity_manager.h"
#include <iostream>
#include <algorithm>

namespace eve {

EntityManager::EntityManager() {
}

void EntityManager::spawnEntity(const std::string& id, const glm::vec3& position,
                                const Health& health, const std::string& shipType,
                                const std::string& shipName, const std::string& faction) {
    // Check if entity already exists
    auto it = m_entities.find(id);
    if (it != m_entities.end()) {
        std::cerr << "Warning: Entity " << id << " already exists, updating instead" << std::endl;
        it->second->updateFromSpawn(position, health, shipType, shipName, faction);
        return;
    }

    // Create new entity
    auto entity = std::make_shared<Entity>(id);
    entity->updateFromSpawn(position, health, shipType, shipName, faction);
    
    // Store entity
    m_entities[id] = entity;
    
    std::cout << "Spawned entity: " << id;
    if (!shipType.empty()) {
        std::cout << " (" << shipType << ")";
    }
    std::cout << " at (" << position.x << ", " << position.y << ", " << position.z << ")" << std::endl;
    
    // Notify callback
    if (m_onEntitySpawned) {
        m_onEntitySpawned(entity);
    }
}

void EntityManager::destroyEntity(const std::string& id) {
    auto it = m_entities.find(id);
    if (it == m_entities.end()) {
        // Entity doesn't exist, silently ignore
        return;
    }

    std::cout << "Destroyed entity: " << id << std::endl;
    
    // Notify callback before removal
    if (m_onEntityDestroyed) {
        m_onEntityDestroyed(it->second);
    }
    
    // Remove entity
    m_entities.erase(it);
}

void EntityManager::updateEntityState(const std::string& id, const glm::vec3& position,
                                      const glm::vec3& velocity, float rotation, const Health& health) {
    auto it = m_entities.find(id);
    if (it == m_entities.end()) {
        // Entity doesn't exist yet, spawn it
        spawnEntity(id, position, health);
        return;
    }

    // Update existing entity
    it->second->updateFromState(position, velocity, rotation, health);
    
    // Notify callback
    if (m_onEntityUpdated) {
        m_onEntityUpdated(it->second);
    }
}

void EntityManager::processStateUpdate(const std::vector<std::string>& entityIds) {
    // Find entities that are no longer in the update
    std::vector<std::string> toRemove;
    
    for (const auto& [id, entity] : m_entities) {
        // Check if entity is in the update list
        bool found = std::find(entityIds.begin(), entityIds.end(), id) != entityIds.end();
        if (!found) {
            toRemove.push_back(id);
        }
    }
    
    // Remove entities not in update
    for (const auto& id : toRemove) {
        destroyEntity(id);
    }
}

void EntityManager::update(float deltaTime) {
    // Interpolate all entities
    for (auto& [id, entity] : m_entities) {
        entity->interpolate(deltaTime);
    }
}

std::shared_ptr<Entity> EntityManager::getEntity(const std::string& id) const {
    auto it = m_entities.find(id);
    if (it != m_entities.end()) {
        return it->second;
    }
    return nullptr;
}

void EntityManager::clear() {
    std::cout << "Clearing all entities (" << m_entities.size() << " total)" << std::endl;
    
    // Notify callbacks
    if (m_onEntityDestroyed) {
        for (const auto& [id, entity] : m_entities) {
            m_onEntityDestroyed(entity);
        }
    }
    
    m_entities.clear();
}

} // namespace eve
