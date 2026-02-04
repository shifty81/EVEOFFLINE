#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <functional>

namespace eve {

/**
 * Level of Detail enum
 */
enum class LODLevel {
    HIGH = 0,      // Full detail
    MEDIUM = 1,    // Reduced polygons
    LOW = 2,       // Minimal detail
    CULLED = 3     // Not rendered
};

/**
 * LOD configuration for distance thresholds
 */
struct LODConfig {
    float highDistance = 50.0f;    // Distance for HIGH detail
    float mediumDistance = 200.0f; // Distance for MEDIUM detail
    float lowDistance = 500.0f;    // Distance for LOW detail
    float cullDistance = 1000.0f;  // Distance beyond which to cull
    
    // Update frequency thresholds (updates per second)
    float highUpdateRate = 30.0f;
    float mediumUpdateRate = 15.0f;
    float lowUpdateRate = 5.0f;
};

/**
 * LOD entity information
 */
struct LODEntity {
    unsigned int id;
    glm::vec3 position;
    float boundingRadius;
    LODLevel currentLOD;
    float lastUpdateTime;
    bool isVisible;
    
    LODEntity()
        : id(0)
        , position(0.0f)
        , boundingRadius(1.0f)
        , currentLOD(LODLevel::HIGH)
        , lastUpdateTime(0.0f)
        , isVisible(true)
    {}
};

/**
 * LOD Manager
 * Manages level-of-detail for entities based on distance from camera
 */
class LODManager {
public:
    LODManager();
    ~LODManager();

    /**
     * Set LOD configuration
     */
    void setConfig(const LODConfig& config) { m_config = config; }

    /**
     * Get current configuration
     */
    const LODConfig& getConfig() const { return m_config; }

    /**
     * Register an entity for LOD management
     */
    void registerEntity(unsigned int id, const glm::vec3& position, float boundingRadius);

    /**
     * Unregister an entity
     */
    void unregisterEntity(unsigned int id);

    /**
     * Update entity position
     */
    void updateEntityPosition(unsigned int id, const glm::vec3& position);

    /**
     * Update LOD levels based on camera position
     */
    void update(const glm::vec3& cameraPosition, float deltaTime);

    /**
     * Get LOD level for an entity
     */
    LODLevel getEntityLOD(unsigned int id) const;

    /**
     * Check if entity should be updated this frame
     */
    bool shouldUpdateEntity(unsigned int id, float currentTime) const;

    /**
     * Check if entity is visible (not culled)
     */
    bool isEntityVisible(unsigned int id) const;

    /**
     * Get all visible entities
     */
    std::vector<unsigned int> getVisibleEntities() const;

    /**
     * Get entities by LOD level
     */
    std::vector<unsigned int> getEntitiesByLOD(LODLevel lod) const;

    /**
     * Get statistics
     */
    struct Stats {
        unsigned int totalEntities;
        unsigned int highLOD;
        unsigned int mediumLOD;
        unsigned int lowLOD;
        unsigned int culled;
        unsigned int visible;
    };
    
    Stats getStats() const;

    /**
     * Clear all entities
     */
    void clear();

private:
    LODConfig m_config;
    std::map<unsigned int, LODEntity> m_entities;
    
    // Helper methods
    LODLevel calculateLOD(float distance) const;
    float getUpdateInterval(LODLevel lod) const;
};

} // namespace eve
