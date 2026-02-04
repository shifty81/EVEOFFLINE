#include "rendering/shadow_map.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

namespace Rendering {

ShadowMap::ShadowMap(unsigned int width, unsigned int height)
    : m_width(width)
    , m_height(height)
    , m_fbo(0)
    , m_depthTexture(0)
{
    createFramebuffer();
}

ShadowMap::~ShadowMap() {
    if (m_depthTexture) {
        glDeleteTextures(1, &m_depthTexture);
    }
    if (m_fbo) {
        glDeleteFramebuffers(1, &m_fbo);
    }
}

void ShadowMap::createFramebuffer() {
    // Create framebuffer
    glGenFramebuffers(1, &m_fbo);
    
    // Create depth texture
    glGenTextures(1, &m_depthTexture);
    glBindTexture(GL_TEXTURE_2D, m_depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 
                 m_width, m_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    
    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    
    // Set border color to white (no shadow outside frustum)
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    
    // Attach depth texture to framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthTexture, 0);
    
    // We don't need color buffer for shadow map
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    
    // Check framebuffer completeness
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "[ShadowMap] ERROR: Framebuffer is not complete!" << std::endl;
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    std::cout << "[ShadowMap] Created shadow map: " << m_width << "x" << m_height << std::endl;
}

void ShadowMap::beginShadowPass() {
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glViewport(0, 0, m_width, m_height);
    glClear(GL_DEPTH_BUFFER_BIT);
    
    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    
    // Prevent shadow acne with polygon offset
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(2.0f, 4.0f);
}

void ShadowMap::endShadowPass() {
    glDisable(GL_POLYGON_OFFSET_FILL);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ShadowMap::bindShadowTexture(unsigned int textureUnit) const {
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_2D, m_depthTexture);
}

glm::mat4 ShadowMap::getLightSpaceMatrix(
    const glm::vec3& lightDir,
    const glm::vec3& sceneCenter,
    float sceneRadius
) const {
    // Calculate light position (far from scene)
    glm::vec3 lightPos = sceneCenter - glm::normalize(lightDir) * sceneRadius * 2.0f;
    
    // Create light view matrix (looking at scene center)
    glm::mat4 lightView = glm::lookAt(
        lightPos,
        sceneCenter,
        glm::vec3(0.0f, 1.0f, 0.0f)
    );
    
    // Create orthographic projection
    // Size should encompass the entire scene
    float orthoSize = sceneRadius * 1.5f;
    glm::mat4 lightProjection = glm::ortho(
        -orthoSize, orthoSize,
        -orthoSize, orthoSize,
        0.1f, sceneRadius * 4.0f
    );
    
    return lightProjection * lightView;
}

// Shadow Manager Implementation

ShadowManager::ShadowManager()
    : m_shadowsEnabled(true)
    , m_shadowBias(0.005f)
{
}

ShadowManager::~ShadowManager() {
    clearShadowMaps();
}

void ShadowManager::createShadowMap(int lightIndex, unsigned int width, unsigned int height) {
    m_shadowMaps[lightIndex] = std::make_unique<ShadowMap>(width, height);
}

ShadowMap* ShadowManager::getShadowMap(int lightIndex) {
    auto it = m_shadowMaps.find(lightIndex);
    if (it != m_shadowMaps.end()) {
        return it->second.get();
    }
    return nullptr;
}

void ShadowManager::removeShadowMap(int lightIndex) {
    m_shadowMaps.erase(lightIndex);
}

void ShadowManager::clearShadowMaps() {
    m_shadowMaps.clear();
}

} // namespace Rendering
