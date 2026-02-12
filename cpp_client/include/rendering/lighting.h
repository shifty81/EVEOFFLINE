#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <string>
#include "shader.h"

/**
 * Dynamic Lighting System
 * Supports multiple light types and shadows
 */

namespace Lighting {

/**
 * Light types
 */
enum class LightType {
    DIRECTIONAL,  // Sun/distant light (infinite distance)
    POINT,        // Omnidirectional point light (limited range)
    SPOT          // Cone-shaped spotlight (limited range + direction)
};

/**
 * Base light structure
 */
struct Light {
    LightType type;
    
    // Common properties
    glm::vec3 color;
    float intensity;
    bool castsShadows;
    bool enabled;
    
    // Position (for point/spot lights)
    glm::vec3 position;
    
    // Direction (for directional/spot lights)
    glm::vec3 direction;
    
    // Attenuation (for point/spot lights)
    float constant;    // Usually 1.0
    float linear;      // Distance falloff linear term
    float quadratic;   // Distance falloff quadratic term
    
    // Spot light properties
    float cutoff;      // Inner cone angle (cos)
    float outerCutoff; // Outer cone angle (cos)
    
    Light()
        : type(LightType::DIRECTIONAL)
        , color(1.0f, 1.0f, 1.0f)
        , intensity(1.0f)
        , castsShadows(false)
        , enabled(true)
        , position(0.0f, 0.0f, 0.0f)
        , direction(0.0f, -1.0f, 0.0f)
        , constant(1.0f)
        , linear(0.09f)
        , quadratic(0.032f)
        , cutoff(glm::cos(glm::radians(12.5f)))
        , outerCutoff(glm::cos(glm::radians(17.5f)))
    {}
};

/**
 * Light Manager
 * Manages all lights in the scene and uploads them to shaders
 */
class LightManager {
public:
    LightManager();
    ~LightManager();
    
    /**
     * Add a light to the scene
     * @return Index of the added light
     */
    int addLight(const Light& light);
    
    /**
     * Remove a light by index
     */
    void removeLight(int index);
    
    /**
     * Get light by index
     */
    Light* getLight(int index);
    
    /**
     * Get number of lights
     */
    size_t getLightCount() const { return m_lights.size(); }
    
    /**
     * Enable/disable a light
     */
    void setLightEnabled(int index, bool enabled);
    
    /**
     * Update light properties
     */
    void updateLight(int index, const Light& light);
    
    /**
     * Clear all lights
     */
    void clearLights();
    
    /**
     * Upload all light data to shader
     */
    void uploadToShader(atlas::Shader* shader) const;
    
    /**
     * Set ambient lighting
     */
    void setAmbientLight(const glm::vec3& color, float intensity);
    
    /**
     * Get ambient light color
     */
    glm::vec3 getAmbientColor() const { return m_ambientColor * m_ambientIntensity; }
    
    /**
     * Create a directional light (like the sun)
     */
    static Light createDirectionalLight(
        const glm::vec3& direction,
        const glm::vec3& color = glm::vec3(1.0f),
        float intensity = 1.0f
    );
    
    /**
     * Create a point light
     */
    static Light createPointLight(
        const glm::vec3& position,
        const glm::vec3& color = glm::vec3(1.0f),
        float intensity = 1.0f,
        float range = 100.0f
    );
    
    /**
     * Create a spot light
     */
    static Light createSpotLight(
        const glm::vec3& position,
        const glm::vec3& direction,
        const glm::vec3& color = glm::vec3(1.0f),
        float intensity = 1.0f,
        float range = 100.0f,
        float innerCutoff = 12.5f,
        float outerCutoff = 17.5f
    );
    
    /**
     * Setup default EVE-style lighting (sun + fill lights)
     */
    void setupEVEStyleLighting();

private:
    std::vector<Light> m_lights;
    glm::vec3 m_ambientColor;
    float m_ambientIntensity;
    
    /**
     * Calculate attenuation coefficients for a given range
     */
    static void calculateAttenuation(float range, float& linear, float& quadratic);
};

} // namespace Lighting
