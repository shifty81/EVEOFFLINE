#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <memory>
#include <map>
#include "shader.h"

namespace Rendering {

/**
 * Shadow Map
 * Handles shadow map generation for directional lights
 */
class ShadowMap {
public:
    /**
     * Constructor
     * @param width Shadow map width resolution
     * @param height Shadow map height resolution
     */
    ShadowMap(unsigned int width = 2048, unsigned int height = 2048);
    ~ShadowMap();

    // Disable copy
    ShadowMap(const ShadowMap&) = delete;
    ShadowMap& operator=(const ShadowMap&) = delete;

    /**
     * Begin shadow map rendering pass
     * Call this before rendering scene for shadow map
     */
    void beginShadowPass();

    /**
     * End shadow map rendering pass
     */
    void endShadowPass();

    /**
     * Bind shadow map texture for reading
     * @param textureUnit Texture unit to bind to (e.g., GL_TEXTURE0)
     */
    void bindShadowTexture(unsigned int textureUnit = 0) const;

    /**
     * Get light space matrix for shadow mapping
     * @param lightDir Directional light direction
     * @param sceneCenter Center of the scene
     * @param sceneRadius Radius of the scene bounding sphere
     * @return Light space projection * view matrix
     */
    glm::mat4 getLightSpaceMatrix(
        const glm::vec3& lightDir,
        const glm::vec3& sceneCenter = glm::vec3(0.0f),
        float sceneRadius = 500.0f
    ) const;

    /**
     * Get shadow map dimensions
     */
    unsigned int getWidth() const { return m_width; }
    unsigned int getHeight() const { return m_height; }

    /**
     * Get depth texture ID
     */
    GLuint getDepthTexture() const { return m_depthTexture; }

private:
    unsigned int m_width;
    unsigned int m_height;
    GLuint m_fbo;           // Framebuffer object
    GLuint m_depthTexture;  // Depth texture for shadow map
    
    void createFramebuffer();
};

/**
 * Shadow Manager
 * Manages multiple shadow maps for different lights
 */
class ShadowManager {
public:
    ShadowManager();
    ~ShadowManager();

    /**
     * Create shadow map for a light
     * @param lightIndex Index of the light
     * @param width Shadow map width
     * @param height Shadow map height
     */
    void createShadowMap(int lightIndex, unsigned int width = 2048, unsigned int height = 2048);

    /**
     * Get shadow map for a light
     * @param lightIndex Index of the light
     * @return Pointer to shadow map or nullptr if not found
     */
    ShadowMap* getShadowMap(int lightIndex);

    /**
     * Remove shadow map for a light
     * @param lightIndex Index of the light
     */
    void removeShadowMap(int lightIndex);

    /**
     * Clear all shadow maps
     */
    void clearShadowMaps();

    /**
     * Enable/disable shadows globally
     */
    void setShadowsEnabled(bool enabled) { m_shadowsEnabled = enabled; }
    bool areShadowsEnabled() const { return m_shadowsEnabled; }

    /**
     * Set shadow bias (to prevent shadow acne)
     */
    void setShadowBias(float bias) { m_shadowBias = bias; }
    float getShadowBias() const { return m_shadowBias; }

private:
    std::map<int, std::unique_ptr<ShadowMap>> m_shadowMaps;
    bool m_shadowsEnabled;
    float m_shadowBias;
};

} // namespace Rendering
