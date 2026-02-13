#include "rendering/warp_effect_renderer.h"
#include "rendering/shader.h"
#include <GL/glew.h>
#include <iostream>

namespace atlas {

WarpEffectRenderer::WarpEffectRenderer() = default;

WarpEffectRenderer::~WarpEffectRenderer() {
    if (m_quadVAO != 0) {
        glDeleteVertexArrays(1, &m_quadVAO);
    }
    if (m_quadVBO != 0) {
        glDeleteBuffers(1, &m_quadVBO);
    }
}

bool WarpEffectRenderer::initialize() {
    std::cout << "Initializing warp effect renderer..." << std::endl;

    createFullscreenQuad();

    m_shader = std::make_unique<Shader>();
    if (!m_shader->loadFromFiles("shaders/warp_tunnel.vert",
                                  "shaders/warp_tunnel.frag")) {
        std::cerr << "Warning: Failed to load warp tunnel shaders — warp visuals disabled" << std::endl;
        m_shader.reset();
        // Clean up GPU resources allocated by createFullscreenQuad()
        if (m_quadVAO != 0) { glDeleteVertexArrays(1, &m_quadVAO); m_quadVAO = 0; }
        if (m_quadVBO != 0) { glDeleteBuffers(1, &m_quadVBO);      m_quadVBO = 0; }
        return false;
    }

    std::cout << "Warp effect renderer initialized" << std::endl;
    return true;
}

void WarpEffectRenderer::update(float deltaTime, int phase, float progress,
                                 float intensity, const glm::vec3& direction) {
    m_time += deltaTime;
    m_phase     = static_cast<float>(phase);
    m_progress  = progress;
    m_intensity = intensity;
    // Project 3D direction to screen-space 2D (x/z plane)
    if (glm::length(glm::vec2(direction.x, direction.z)) > 0.001f) {
        m_direction = glm::normalize(glm::vec2(direction.x, direction.z));
    }
}

void WarpEffectRenderer::render() {
    if (!m_shader || m_intensity < 0.001f) return;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);  // Additive blending
    glDisable(GL_DEPTH_TEST);

    m_shader->use();
    m_shader->setFloat("uTime",      m_time);
    m_shader->setFloat("uIntensity", m_intensity);
    m_shader->setFloat("uPhase",     m_phase);
    m_shader->setFloat("uProgress",  m_progress);
    m_shader->setVec2("uDirection",  m_direction);

    glBindVertexArray(m_quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void WarpEffectRenderer::createFullscreenQuad() {
    float quadVertices[] = {
        -1.0f, -1.0f,
         1.0f, -1.0f,
         1.0f,  1.0f,
        -1.0f, -1.0f,
         1.0f,  1.0f,
        -1.0f,  1.0f
    };

    glGenVertexArrays(1, &m_quadVAO);
    glGenBuffers(1, &m_quadVBO);

    glBindVertexArray(m_quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

    glBindVertexArray(0);
}

} // namespace atlas
