#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <string>
#include "mesh.h"
#include "instanced_renderer.h"
#include "shader.h"
#include "camera.h"

namespace eve {

/**
 * Asteroid Field Renderer
 * Renders procedural asteroid fields using instanced rendering
 * Based on the EVE OFFLINE asteroid system mechanics
 */
class AsteroidFieldRenderer {
public:
    /**
     * Asteroid size categories matching Python implementation
     */
    enum class AsteroidSize {
        SMALL,
        MEDIUM,
        LARGE,
        HUGE
    };
    
    /**
     * Belt layout types matching Python implementation
     */
    enum class BeltLayout {
        SEMICIRCLE,
        SPHERICAL
    };
    
    /**
     * Individual asteroid instance data
     */
    struct AsteroidInstance {
        int instanceId;
        glm::vec3 position;
        AsteroidSize size;
        std::string meshType;
        float scale;
        float rotation;
    };
    
    AsteroidFieldRenderer();
    ~AsteroidFieldRenderer();
    
    /**
     * Initialize asteroid field renderer
     * Creates procedural asteroid meshes
     */
    bool initialize();
    
    /**
     * Generate an asteroid field
     * @param center Center point of the field
     * @param radius Radius of the field in meters
     * @param asteroidCounts Count of each size (SMALL, MEDIUM, LARGE, HUGE)
     * @param layout Layout pattern (SEMICIRCLE or SPHERICAL)
     * @param seed Random seed for deterministic generation
     */
    void generateField(
        const glm::vec3& center,
        float radius,
        const std::vector<int>& asteroidCounts,
        BeltLayout layout = BeltLayout::SEMICIRCLE,
        int seed = 42
    );
    
    /**
     * Clear all asteroids from the field
     */
    void clearField();
    
    /**
     * Render all asteroids in the field
     */
    void render(Shader* shader, const Camera& camera);
    
    /**
     * Get number of asteroids in field
     */
    size_t getAsteroidCount() const { return m_asteroids.size(); }

private:
    // Instanced renderer for efficient batch rendering
    std::unique_ptr<InstancedRenderer> m_renderer;
    
    // Asteroid meshes by type
    std::vector<std::shared_ptr<Mesh>> m_asteroidMeshes;
    
    // Active asteroid instances
    std::vector<AsteroidInstance> m_asteroids;
    
    // Field center for positioning
    glm::vec3 m_fieldCenter;
    
    /**
     * Create procedural asteroid meshes
     * Creates 3 different asteroid mesh types with varying detail
     */
    void createAsteroidMeshes();
    
    /**
     * Create a single asteroid mesh
     * @param subdivisions Number of icosphere subdivisions
     * @param displacement Random displacement amount
     * @param seed Random seed for deterministic generation
     */
    std::shared_ptr<Mesh> createAsteroidMesh(
        int subdivisions,
        float displacement,
        int seed
    );
    
    /**
     * Generate position based on layout
     */
    glm::vec3 generatePosition(
        const glm::vec3& center,
        float radius,
        BeltLayout layout,
        int seed,
        int index
    );
    
    /**
     * Get scale factor for asteroid size
     */
    float getSizeScale(AsteroidSize size) const;
    
    /**
     * Get color for asteroid type
     */
    glm::vec4 getAsteroidColor(int meshType, int seed) const;
    
    /**
     * Create icosphere mesh for base asteroid shape
     */
    void createIcosphere(
        std::vector<Vertex>& vertices,
        std::vector<unsigned int>& indices,
        int subdivisions
    );
    
    /**
     * Apply random displacement to vertices for rocky appearance
     */
    void displaceVertices(
        std::vector<Vertex>& vertices,
        float amount,
        int seed
    );
    
    /**
     * Simple hash function for deterministic randomness
     */
    float hash(int seed, int index) const;
};

} // namespace eve
