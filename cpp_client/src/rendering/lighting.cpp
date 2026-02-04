#include "rendering/lighting.h"
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <algorithm>

namespace Lighting {

LightManager::LightManager()
    : m_ambientColor(0.15f, 0.15f, 0.2f)
    , m_ambientIntensity(1.0f)
{
}

LightManager::~LightManager() {
    clearLights();
}

int LightManager::addLight(const Light& light) {
    m_lights.push_back(light);
    return m_lights.size() - 1;
}

void LightManager::removeLight(int index) {
    if (index >= 0 && index < static_cast<int>(m_lights.size())) {
        m_lights.erase(m_lights.begin() + index);
    }
}

Light* LightManager::getLight(int index) {
    if (index >= 0 && index < static_cast<int>(m_lights.size())) {
        return &m_lights[index];
    }
    return nullptr;
}

void LightManager::setLightEnabled(int index, bool enabled) {
    if (Light* light = getLight(index)) {
        light->enabled = enabled;
    }
}

void LightManager::updateLight(int index, const Light& light) {
    if (index >= 0 && index < static_cast<int>(m_lights.size())) {
        m_lights[index] = light;
    }
}

void LightManager::clearLights() {
    m_lights.clear();
}

void LightManager::uploadToShader(Shader* shader) const {
    if (!shader) return;
    
    shader->use();
    
    // Upload ambient light
    shader->setVec3("ambientLight", getAmbientColor());
    
    // Count enabled lights by type
    int numDirLights = 0;
    int numPointLights = 0;
    int numSpotLights = 0;
    
    for (const auto& light : m_lights) {
        if (!light.enabled) continue;
        
        switch (light.type) {
            case LightType::DIRECTIONAL:
                numDirLights++;
                break;
            case LightType::POINT:
                numPointLights++;
                break;
            case LightType::SPOT:
                numSpotLights++;
                break;
        }
    }
    
    // Upload counts
    shader->setInt("numDirLights", numDirLights);
    shader->setInt("numPointLights", numPointLights);
    shader->setInt("numSpotLights", numSpotLights);
    
    // Upload directional lights
    int dirIndex = 0;
    for (const auto& light : m_lights) {
        if (!light.enabled || light.type != LightType::DIRECTIONAL) continue;
        
        std::string prefix = "dirLights[" + std::to_string(dirIndex) + "]";
        shader->setVec3(prefix + ".direction", light.direction);
        shader->setVec3(prefix + ".color", light.color * light.intensity);
        
        dirIndex++;
    }
    
    // Upload point lights
    int pointIndex = 0;
    for (const auto& light : m_lights) {
        if (!light.enabled || light.type != LightType::POINT) continue;
        
        std::string prefix = "pointLights[" + std::to_string(pointIndex) + "]";
        shader->setVec3(prefix + ".position", light.position);
        shader->setVec3(prefix + ".color", light.color * light.intensity);
        shader->setFloat(prefix + ".constant", light.constant);
        shader->setFloat(prefix + ".linear", light.linear);
        shader->setFloat(prefix + ".quadratic", light.quadratic);
        
        pointIndex++;
    }
    
    // Upload spot lights
    int spotIndex = 0;
    for (const auto& light : m_lights) {
        if (!light.enabled || light.type != LightType::SPOT) continue;
        
        std::string prefix = "spotLights[" + std::to_string(spotIndex) + "]";
        shader->setVec3(prefix + ".position", light.position);
        shader->setVec3(prefix + ".direction", light.direction);
        shader->setVec3(prefix + ".color", light.color * light.intensity);
        shader->setFloat(prefix + ".cutoff", light.cutoff);
        shader->setFloat(prefix + ".outerCutoff", light.outerCutoff);
        shader->setFloat(prefix + ".constant", light.constant);
        shader->setFloat(prefix + ".linear", light.linear);
        shader->setFloat(prefix + ".quadratic", light.quadratic);
        
        spotIndex++;
    }
}

void LightManager::setAmbientLight(const glm::vec3& color, float intensity) {
    m_ambientColor = color;
    m_ambientIntensity = intensity;
}

Light LightManager::createDirectionalLight(
    const glm::vec3& direction,
    const glm::vec3& color,
    float intensity
) {
    Light light;
    light.type = LightType::DIRECTIONAL;
    light.direction = glm::normalize(direction);
    light.color = color;
    light.intensity = intensity;
    light.castsShadows = false;
    
    return light;
}

Light LightManager::createPointLight(
    const glm::vec3& position,
    const glm::vec3& color,
    float intensity,
    float range
) {
    Light light;
    light.type = LightType::POINT;
    light.position = position;
    light.color = color;
    light.intensity = intensity;
    light.castsShadows = false;
    
    // Calculate attenuation for desired range
    calculateAttenuation(range, light.linear, light.quadratic);
    
    return light;
}

Light LightManager::createSpotLight(
    const glm::vec3& position,
    const glm::vec3& direction,
    const glm::vec3& color,
    float intensity,
    float range,
    float innerCutoff,
    float outerCutoff
) {
    Light light;
    light.type = LightType::SPOT;
    light.position = position;
    light.direction = glm::normalize(direction);
    light.color = color;
    light.intensity = intensity;
    light.cutoff = glm::cos(glm::radians(innerCutoff));
    light.outerCutoff = glm::cos(glm::radians(outerCutoff));
    light.castsShadows = false;
    
    // Calculate attenuation for desired range
    calculateAttenuation(range, light.linear, light.quadratic);
    
    return light;
}

void LightManager::calculateAttenuation(float range, float& linear, float& quadratic) {
    // Use a formula to calculate attenuation coefficients
    // These values give approximately 5% intensity at the range distance
    
    // For range = 100:   linear = 0.09,  quadratic = 0.032
    // For range = 50:    linear = 0.18,  quadratic = 0.128
    // For range = 200:   linear = 0.045, quadratic = 0.008
    
    // Approximate formula (can be tuned)
    linear = 4.5f / range;
    quadratic = 75.0f / (range * range);
}

void LightManager::setupEVEStyleLighting() {
    std::cout << "[LightManager] Setting up EVE-style lighting..." << std::endl;
    
    // Clear existing lights
    clearLights();
    
    // Set ambient light (dark space)
    setAmbientLight(glm::vec3(0.15f, 0.15f, 0.2f), 1.0f);
    
    // Main directional light (sun/star) - warm white
    Light sun = createDirectionalLight(
        glm::vec3(0.5f, -0.3f, -0.2f),  // Angled from top-right-front
        glm::vec3(1.0f, 0.95f, 0.9f),   // Warm white
        1.2f                              // Slightly bright
    );
    addLight(sun);
    
    // Fill light from opposite side (cool blue) - softer
    Light fill = createDirectionalLight(
        glm::vec3(-0.3f, -0.2f, 0.5f),  // Opposite angle
        glm::vec3(0.3f, 0.35f, 0.4f),    // Cool blue
        0.6f                              // Dimmer
    );
    addLight(fill);
    
    // Rim light from behind (subtle)
    Light rim = createDirectionalLight(
        glm::vec3(0.0f, 0.3f, -0.8f),   // From behind/above
        glm::vec3(0.2f, 0.25f, 0.3f),    // Cool rim
        0.4f                              // Subtle
    );
    addLight(rim);
    
    std::cout << "[LightManager] EVE-style lighting setup complete: "
              << m_lights.size() << " lights" << std::endl;
}

} // namespace Lighting
