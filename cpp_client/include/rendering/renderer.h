#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include <glm/glm.hpp>

namespace eve {

// Forward declarations
class Shader;
class Camera;
class Mesh;
class Model;
class HealthBarRenderer;
class Entity;

/**
 * Visual representation of a game entity
 */
struct EntityVisual {
    std::shared_ptr<Model> model;
    glm::vec3 position;
    glm::vec3 rotation;  // Euler angles (pitch, yaw, roll)
    float scale;
    
    // Health data for health bars
    float currentShield;
    float maxShield;
    float currentArmor;
    float maxArmor;
    float currentHull;
    float maxHull;
    
    EntityVisual() 
        : position(0.0f)
        , rotation(0.0f)
        , scale(1.0f)
        , currentShield(0.0f)
        , maxShield(0.0f)
        , currentArmor(0.0f)
        , maxArmor(0.0f)
        , currentHull(0.0f)
        , maxHull(0.0f)
    {}
};

/**
 * Main renderer class
 * Handles all OpenGL rendering
 */
class Renderer {
public:
    Renderer();
    ~Renderer();

    /**
     * Initialize renderer
     */
    bool initialize();

    /**
     * Begin frame
     */
    void beginFrame();

    /**
     * End frame
     */
    void endFrame();

    /**
     * Render the scene
     */
    void renderScene(Camera& camera);

    /**
     * Clear screen with color
     */
    void clear(const glm::vec4& color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

    /**
     * Set viewport
     */
    void setViewport(int x, int y, int width, int height);

    // === Entity Visual Management ===
    
    /**
     * Create visual representation for an entity
     * @param entity The entity to create visuals for
     * @return true if successful
     */
    bool createEntityVisual(const std::shared_ptr<Entity>& entity);
    
    /**
     * Remove visual representation for an entity
     * @param entityId The entity ID to remove
     */
    void removeEntityVisual(const std::string& entityId);
    
    /**
     * Update entity visuals from entity manager
     * @param entities Map of entity ID to entity
     */
    void updateEntityVisuals(const std::unordered_map<std::string, std::shared_ptr<Entity>>& entities);

private:
    void setupStarfield();
    void renderStarfield(Camera& camera);
    void renderEntities(Camera& camera);
    void renderHealthBars(Camera& camera);

    std::unique_ptr<Shader> m_basicShader;
    std::unique_ptr<Shader> m_starfieldShader;
    std::unique_ptr<Shader> m_entityShader;
    std::unique_ptr<HealthBarRenderer> m_healthBarRenderer;
    
    // Starfield data
    unsigned int m_starfieldVAO;
    unsigned int m_starfieldVBO;
    int m_starCount;

    // Entity visuals
    std::unordered_map<std::string, EntityVisual> m_entityVisuals;

    bool m_initialized;
};

} // namespace eve
