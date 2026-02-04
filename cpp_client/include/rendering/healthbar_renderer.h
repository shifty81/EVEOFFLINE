#pragma once

#include <glm/glm.hpp>
#include <memory>

namespace eve {

class Shader;

/**
 * Health bar configuration
 */
struct HealthBarConfig {
    float width = 2.0f;
    float height = 0.3f;
    float yOffset = 1.5f;  // Height above entity
    bool showShield = true;
    bool showArmor = true;
    bool showHull = true;
};

/**
 * Health bar renderer
 * Renders shield/armor/hull bars above ships
 */
class HealthBarRenderer {
public:
    HealthBarRenderer();
    ~HealthBarRenderer();

    /**
     * Initialize the health bar renderer
     */
    bool initialize();

    /**
     * Begin rendering health bars
     */
    void begin(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);

    /**
     * Draw a health bar for an entity
     * @param position Entity position
     * @param shield Shield percentage (0-1)
     * @param armor Armor percentage (0-1)
     * @param hull Hull percentage (0-1)
     * @param maxShield Maximum shield HP
     * @param maxArmor Maximum armor HP
     * @param maxHull Maximum hull HP
     */
    void drawHealthBar(const glm::vec3& position, 
                       float shield, float armor, float hull,
                       float maxShield, float maxArmor, float maxHull);

    /**
     * End rendering health bars
     */
    void end();

    /**
     * Set health bar configuration
     */
    void setConfig(const HealthBarConfig& config) { m_config = config; }

    /**
     * Get current configuration
     */
    const HealthBarConfig& getConfig() const { return m_config; }

private:
    HealthBarConfig m_config;
    std::unique_ptr<Shader> m_shader;
    
    // OpenGL resources
    unsigned int m_vao;
    unsigned int m_vbo;
    unsigned int m_ebo;
    
    // Current matrices
    glm::mat4 m_viewMatrix;
    glm::mat4 m_projectionMatrix;
    
    // Helper methods
    void createQuad();
    void drawBar(const glm::vec3& position, float value, const glm::vec4& color, float yOffset);
};

} // namespace eve
