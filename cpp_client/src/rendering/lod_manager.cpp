#include "rendering/lod_manager.h"
#include "rendering/frustum_culler.h"
#include <iostream>
#include <algorithm>

namespace atlas {

LODManager::LODManager() 
    : m_frustumCuller(std::make_unique<FrustumCuller>())
{
}

LODManager::~LODManager() {
}

void LODManager::registerEntity(unsigned int id, const glm::vec3& position, float boundingRadius) {
    LODEntity entity;
    entity.id = id;
    entity.position = position;
    entity.boundingRadius = boundingRadius;
    entity.currentLOD = LODLevel::HIGH;
    entity.lastUpdateTime = 0.0f;
    entity.isVisible = true;
    
    m_entities[id] = entity;
}

void LODManager::unregisterEntity(unsigned int id) {
    m_entities.erase(id);
}

void LODManager::updateEntityPosition(unsigned int id, const glm::vec3& position) {
    auto it = m_entities.find(id);
    if (it != m_entities.end()) {
        it->second.position = position;
    }
}

void LODManager::update(const glm::vec3& cameraPosition, float deltaTime, const glm::mat4* viewProjection) {
    // Update frustum culler if view-projection matrix provided
    if (viewProjection && m_frustumCuller) {
        m_frustumCuller->update(*viewProjection);
    }
    
    for (auto& pair : m_entities) {
        LODEntity& entity = pair.second;
        
        // Calculate distance from camera
        float distance = glm::length(entity.position - cameraPosition);
        
        // Determine LOD level based on distance
        LODLevel newLOD = calculateLOD(distance);
        
        // Apply frustum culling if enabled
        bool visibleInFrustum = true;
        if (viewProjection && m_frustumCuller && m_frustumCuller->isEnabled()) {
            visibleInFrustum = m_frustumCuller->isVisible(entity.position, entity.boundingRadius);
            
            // If not in frustum, force to CULLED
            if (!visibleInFrustum) {
                newLOD = LODLevel::CULLED;
            }
        }
        
        // Update LOD if changed
        if (newLOD != entity.currentLOD) {
            entity.currentLOD = newLOD;
        }
        
        // Update visibility
        entity.isVisible = (newLOD != LODLevel::CULLED);
    }
}

LODLevel LODManager::getEntityLOD(unsigned int id) const {
    auto it = m_entities.find(id);
    if (it != m_entities.end()) {
        return it->second.currentLOD;
    }
    return LODLevel::CULLED;
}

bool LODManager::shouldUpdateEntity(unsigned int id, float currentTime) const {
    auto it = m_entities.find(id);
    if (it == m_entities.end() || !it->second.isVisible) {
        return false;
    }
    
    const LODEntity& entity = it->second;
    float updateInterval = getUpdateInterval(entity.currentLOD);
    
    return (currentTime - entity.lastUpdateTime) >= (1.0f / updateInterval);
}

bool LODManager::isEntityVisible(unsigned int id) const {
    auto it = m_entities.find(id);
    if (it != m_entities.end()) {
        return it->second.isVisible;
    }
    return false;
}

std::vector<unsigned int> LODManager::getVisibleEntities() const {
    std::vector<unsigned int> visible;
    visible.reserve(m_entities.size());
    
    for (const auto& pair : m_entities) {
        if (pair.second.isVisible) {
            visible.push_back(pair.first);
        }
    }
    
    return visible;
}

std::vector<unsigned int> LODManager::getEntitiesByLOD(LODLevel lod) const {
    std::vector<unsigned int> entities;
    
    for (const auto& pair : m_entities) {
        if (pair.second.currentLOD == lod) {
            entities.push_back(pair.first);
        }
    }
    
    return entities;
}

LODManager::Stats LODManager::getStats() const {
    Stats stats = {};
    stats.totalEntities = static_cast<unsigned int>(m_entities.size());
    stats.frustumCulled = 0;
    
    for (const auto& pair : m_entities) {
        const LODEntity& entity = pair.second;
        
        switch (entity.currentLOD) {
            case LODLevel::HIGH:
                stats.highLOD++;
                break;
            case LODLevel::MEDIUM:
                stats.mediumLOD++;
                break;
            case LODLevel::LOW:
                stats.lowLOD++;
                break;
            case LODLevel::CULLED:
                stats.culled++;
                break;
        }
        
        if (entity.isVisible) {
            stats.visible++;
        }
    }
    
    // Get frustum culler stats if available
    if (m_frustumCuller && m_frustumCuller->isEnabled()) {
        stats.frustumCulled = m_frustumCuller->getStats().culledEntities;
    }
    
    return stats;
}

void LODManager::clear() {
    m_entities.clear();
}

LODLevel LODManager::calculateLOD(float distance) const {
    if (distance >= m_config.cullDistance) {
        return LODLevel::CULLED;
    } else if (distance >= m_config.lowDistance) {
        return LODLevel::LOW;
    } else if (distance >= m_config.mediumDistance) {
        return LODLevel::MEDIUM;
    } else if (distance >= m_config.highDistance) {
        return LODLevel::HIGH;
    } else {
        return LODLevel::HIGH;
    }
}

float LODManager::getUpdateInterval(LODLevel lod) const {
    switch (lod) {
        case LODLevel::HIGH:
            return m_config.highUpdateRate;
        case LODLevel::MEDIUM:
            return m_config.mediumUpdateRate;
        case LODLevel::LOW:
            return m_config.lowUpdateRate;
        case LODLevel::CULLED:
        default:
            return 0.0f;
    }
}

void LODManager::setFrustumCullingEnabled(bool enabled) {
    if (m_frustumCuller) {
        m_frustumCuller->setEnabled(enabled);
    }
}

bool LODManager::isFrustumCullingEnabled() const {
    return m_frustumCuller && m_frustumCuller->isEnabled();
}

const FrustumCuller* LODManager::getFrustumCuller() const {
    return m_frustumCuller.get();
}

} // namespace atlas
