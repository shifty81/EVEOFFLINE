#include "rendering/healthbar_renderer.h"
#include "rendering/shader.h"
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

namespace eve {

// Configuration constants
constexpr float HEALTH_BAR_BORDER_WIDTH = 2.0f;

HealthBarRenderer::HealthBarRenderer()
    : m_vao(0)
    , m_vbo(0)
    , m_ebo(0)
{
}

HealthBarRenderer::~HealthBarRenderer() {
    if (m_vao != 0) {
        glDeleteVertexArrays(1, &m_vao);
    }
    if (m_vbo != 0) {
        glDeleteBuffers(1, &m_vbo);
    }
    if (m_ebo != 0) {
        glDeleteBuffers(1, &m_ebo);
    }
}

bool HealthBarRenderer::initialize() {
    std::cout << "Initializing health bar renderer..." << std::endl;
    
    // Create quad for rendering bars
    createQuad();
    
    // Load health bar shaders
    m_shader = std::make_unique<Shader>();
    if (!m_shader->loadFromFiles("shaders/healthbar.vert", "shaders/healthbar.frag")) {
        std::cerr << "Failed to load health bar shaders" << std::endl;
        return false;
    }
    
    std::cout << "Health bar renderer initialized" << std::endl;
    return true;
}

void HealthBarRenderer::begin(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) {
    m_viewMatrix = viewMatrix;
    m_projectionMatrix = projectionMatrix;
    
    if (m_shader) {
        m_shader->use();
        m_shader->setMat4("view", viewMatrix);
        m_shader->setMat4("projection", projectionMatrix);
    }
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST); // Draw on top
}

void HealthBarRenderer::end() {
    glEnable(GL_DEPTH_TEST);
}

void HealthBarRenderer::drawHealthBar(const glm::vec3& position,
                                     float shield, float armor, float hull,
                                     float maxShield, float maxArmor, float maxHull) {
    float barHeight = m_config.height;
    float barSpacing = barHeight * 0.2f;
    float currentY = m_config.yOffset;
    
    // Draw shield bar (blue)
    if (m_config.showShield && maxShield > 0.0f) {
        glm::vec4 shieldColor(0.3f, 0.7f, 1.0f, 0.8f); // Cyan blue
        drawBar(position, shield, shieldColor, currentY);
        currentY += barHeight + barSpacing;
    }
    
    // Draw armor bar (yellow-orange)
    if (m_config.showArmor && maxArmor > 0.0f) {
        glm::vec4 armorColor(1.0f, 0.8f, 0.2f, 0.8f); // Yellow-orange
        drawBar(position, armor, armorColor, currentY);
        currentY += barHeight + barSpacing;
    }
    
    // Draw hull bar (red)
    if (m_config.showHull && maxHull > 0.0f) {
        glm::vec4 hullColor(1.0f, 0.2f, 0.2f, 0.8f); // Red
        drawBar(position, hull, hullColor, currentY);
    }
}

void HealthBarRenderer::createQuad() {
    // Create a simple quad
    float vertices[] = {
        // positions      // texcoords
        0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f, 1.0f, 1.0f
    };
    
    unsigned int indices[] = {
        0, 1, 2,
        0, 2, 3
    };
    
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);
    
    glBindVertexArray(m_vao);
    
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    // Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    
    // Texcoords
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    
    glBindVertexArray(0);
}

void HealthBarRenderer::drawBar(const glm::vec3& position, float value, const glm::vec4& color, float yOffset) {
    if (!m_shader) {
        return;
    }
    
    // Clamp value
    value = glm::clamp(value, 0.0f, 1.0f);
    
    // Create model matrix for the bar
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position + glm::vec3(-m_config.width * 0.5f, yOffset, 0.0f));
    model = glm::scale(model, glm::vec3(m_config.width, m_config.height, 1.0f));
    
    // Draw background (dark gray)
    m_shader->setMat4("model", model);
    m_shader->setVec4("barColor", glm::vec4(0.2f, 0.2f, 0.2f, 0.6f));
    m_shader->setFloat("fillAmount", 1.0f);
    
    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    
    // Draw foreground (health)
    if (value > 0.0f) {
        m_shader->setVec4("barColor", color);
        m_shader->setFloat("fillAmount", value);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }
    
    // Draw border (white outline) using line rendering
    // Note: This approach causes some overdraw and state changes per health bar.
    // For optimization with many health bars, consider:
    // 1. Batching all bar fills in one pass, then all borders in another pass
    // 2. Using separate line strip geometry to avoid polygon mode changes
    // Current implementation is simple and works well for typical entity counts.
    m_shader->setVec4("barColor", glm::vec4(1.0f, 1.0f, 1.0f, 0.8f));
    m_shader->setFloat("fillAmount", 1.0f);
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glLineWidth(HEALTH_BAR_BORDER_WIDTH);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    
    glBindVertexArray(0);
}

} // namespace eve
