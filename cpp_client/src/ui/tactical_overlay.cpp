#include "ui/tactical_overlay.h"
#include "rendering/shader.h"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>
#include <iostream>

namespace eve {

TacticalOverlay::TacticalOverlay()
    : m_visible(false)
    , m_rangeIncrement(DEFAULT_RANGE_INCREMENT)
    , m_maxRange(DEFAULT_MAX_RANGE)
    , m_targetingRange(0.0f)
    , m_weaponOptimal(0.0f)
    , m_weaponFalloff(0.0f)
    , m_circleVAO(0)
    , m_circleVBO(0)
    , m_lineVAO(0)
    , m_lineVBO(0)
    , m_overlayShader(nullptr)
{
    // EVE-style colors
    m_gridColor = glm::vec4(0.2f, 0.2f, 0.3f, 0.3f);              // Subtle grid
    m_rangeColor = glm::vec4(0.4f, 0.6f, 0.8f, 0.4f);             // Blue range circles
    m_targetingRangeColor = glm::vec4(1.0f, 0.0f, 0.0f, 0.5f);    // Red targeting range
    m_weaponOptimalColor = glm::vec4(0.0f, 1.0f, 0.0f, 0.6f);     // Green optimal
    m_weaponFalloffColor = glm::vec4(1.0f, 0.8f, 0.0f, 0.5f);     // Yellow falloff
    m_velocityColor = glm::vec4(0.0f, 1.0f, 1.0f, 0.8f);          // Cyan velocity vector
    m_targetLineColor = glm::vec4(1.0f, 1.0f, 1.0f, 0.5f);        // White target lines
    m_hostileColor = glm::vec4(1.0f, 0.2f, 0.2f, 0.8f);           // Red hostile
    m_friendlyColor = glm::vec4(0.2f, 0.6f, 1.0f, 0.8f);          // Blue friendly
}

TacticalOverlay::~TacticalOverlay() {
    if (m_circleVAO) glDeleteVertexArrays(1, &m_circleVAO);
    if (m_circleVBO) glDeleteBuffers(1, &m_circleVBO);
    if (m_lineVAO) glDeleteVertexArrays(1, &m_lineVAO);
    if (m_lineVBO) glDeleteBuffers(1, &m_lineVBO);
    delete m_overlayShader;
}

void TacticalOverlay::initialize() {
    // Create VAO/VBO for circle rendering
    glGenVertexArrays(1, &m_circleVAO);
    glGenBuffers(1, &m_circleVBO);
    
    glBindVertexArray(m_circleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_circleVBO);
    
    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glBindVertexArray(0);
    
    // Create VAO/VBO for line rendering
    glGenVertexArrays(1, &m_lineVAO);
    glGenBuffers(1, &m_lineVBO);
    
    glBindVertexArray(m_lineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_lineVBO);
    
    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glBindVertexArray(0);
    
    // Load overlay shader
    m_overlayShader = new Shader();
    if (!m_overlayShader->loadFromFiles("shaders/overlay.vert", "shaders/overlay.frag")) {
        std::cerr << "[TacticalOverlay] Failed to load overlay shader" << std::endl;
    }
    
    std::cout << "[TacticalOverlay] Initialized" << std::endl;
}

void TacticalOverlay::update(float deltaTime) {
    // Update animation states if needed
    // For now, the overlay is static
}

void TacticalOverlay::render(const glm::vec3& playerPosition, 
                            const glm::vec3& playerVelocity,
                            const glm::mat4& view,
                            const glm::mat4& projection) {
    if (!m_visible) return;
    
    // Enable blending for transparent overlays
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Disable depth testing so overlay always visible
    glDisable(GL_DEPTH_TEST);
    
    // Render range circles
    renderRangeCircles(playerPosition, view, projection);
    
    // Render velocity vector
    if (glm::length(playerVelocity) > 0.1f) {
        renderVelocityVector(playerPosition, playerVelocity, view, projection);
    }
    
    // Render target indicators
    if (!m_targets.empty()) {
        renderTargetLines(playerPosition, view, projection);
    }
    
    // Re-enable depth testing
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}

void TacticalOverlay::toggle() {
    m_visible = !m_visible;
    std::cout << "[TacticalOverlay] " << (m_visible ? "Enabled" : "Disabled") << std::endl;
}

void TacticalOverlay::setWeaponRanges(float optimal, float falloff) {
    m_weaponOptimal = optimal;
    m_weaponFalloff = optimal + falloff;
}

void TacticalOverlay::clearWeaponRanges() {
    m_weaponOptimal = 0.0f;
    m_weaponFalloff = 0.0f;
}

void TacticalOverlay::addTargetIndicator(const glm::vec3& targetPos, bool hostile) {
    TargetIndicator indicator;
    indicator.position = targetPos;
    indicator.hostile = hostile;
    m_targets.push_back(indicator);
}

void TacticalOverlay::clearTargetIndicators() {
    m_targets.clear();
}

void TacticalOverlay::renderRangeCircles(const glm::vec3& center, 
                                        const glm::mat4& view,
                                        const glm::mat4& projection) {
    if (!m_overlayShader) return;
    
    m_overlayShader->use();
    m_overlayShader->setMat4("view", view);
    m_overlayShader->setMat4("projection", projection);
    
    glBindVertexArray(m_circleVAO);
    
    // Render range increment circles
    for (float range = m_rangeIncrement; range <= m_maxRange; range += m_rangeIncrement) {
        std::vector<float> vertices;
        generateCircleVertices(range, CIRCLE_SEGMENTS, vertices);
        
        // Translate vertices to player position
        for (size_t i = 0; i < vertices.size(); i += 3) {
            vertices[i] += center.x;
            vertices[i + 1] += center.y;
            vertices[i + 2] += center.z;
        }
        
        glBindBuffer(GL_ARRAY_BUFFER, m_circleVBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), 
                    vertices.data(), GL_DYNAMIC_DRAW);
        
        // Set color and draw
        m_overlayShader->setVec4("overlayColor", m_rangeColor);
        glLineWidth(1.0f);
        glDrawArrays(GL_LINE_LOOP, 0, CIRCLE_SEGMENTS);
    }
    
    // Render targeting range circle (if set)
    if (m_targetingRange > 0.0f) {
        std::vector<float> vertices;
        generateCircleVertices(m_targetingRange, CIRCLE_SEGMENTS, vertices);
        
        for (size_t i = 0; i < vertices.size(); i += 3) {
            vertices[i] += center.x;
            vertices[i + 1] += center.y;
            vertices[i + 2] += center.z;
        }
        
        glBindBuffer(GL_ARRAY_BUFFER, m_circleVBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), 
                    vertices.data(), GL_DYNAMIC_DRAW);
        
        m_overlayShader->setVec4("overlayColor", m_targetingRangeColor);
        glLineWidth(2.0f);
        glDrawArrays(GL_LINE_LOOP, 0, CIRCLE_SEGMENTS);
    }
    
    // Render weapon optimal range (if set)
    if (m_weaponOptimal > 0.0f) {
        std::vector<float> vertices;
        generateCircleVertices(m_weaponOptimal, CIRCLE_SEGMENTS, vertices);
        
        for (size_t i = 0; i < vertices.size(); i += 3) {
            vertices[i] += center.x;
            vertices[i + 1] += center.y;
            vertices[i + 2] += center.z;
        }
        
        glBindBuffer(GL_ARRAY_BUFFER, m_circleVBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), 
                    vertices.data(), GL_DYNAMIC_DRAW);
        
        m_overlayShader->setVec4("overlayColor", m_weaponOptimalColor);
        glLineWidth(2.0f);
        glDrawArrays(GL_LINE_LOOP, 0, CIRCLE_SEGMENTS);
    }
    
    // Render weapon falloff range (if set)
    if (m_weaponFalloff > 0.0f) {
        std::vector<float> vertices;
        generateCircleVertices(m_weaponFalloff, CIRCLE_SEGMENTS, vertices);
        
        for (size_t i = 0; i < vertices.size(); i += 3) {
            vertices[i] += center.x;
            vertices[i + 1] += center.y;
            vertices[i + 2] += center.z;
        }
        
        glBindBuffer(GL_ARRAY_BUFFER, m_circleVBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), 
                    vertices.data(), GL_DYNAMIC_DRAW);
        
        m_overlayShader->setVec4("overlayColor", m_weaponFalloffColor);
        glLineWidth(1.5f);
        glDrawArrays(GL_LINE_LOOP, 0, CIRCLE_SEGMENTS);
    }
    
    glBindVertexArray(0);
}

void TacticalOverlay::renderVelocityVector(const glm::vec3& position,
                                          const glm::vec3& velocity,
                                          const glm::mat4& view,
                                          const glm::mat4& projection) {
    if (!m_overlayShader) return;
    
    m_overlayShader->use();
    m_overlayShader->setMat4("view", view);
    m_overlayShader->setMat4("projection", projection);
    
    // Render a line showing velocity direction and magnitude
    glBindVertexArray(m_lineVAO);
    
    // Scale velocity vector for visibility
    float vectorScale = 10.0f;  // Scale factor for display
    glm::vec3 endPoint = position + (velocity * vectorScale);
    
    float vertices[] = {
        position.x, position.y, position.z,
        endPoint.x, endPoint.y, endPoint.z
    };
    
    glBindBuffer(GL_ARRAY_BUFFER, m_lineVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
    
    // Set color
    m_overlayShader->setVec4("overlayColor", m_velocityColor);
    glLineWidth(3.0f);
    glDrawArrays(GL_LINES, 0, 2);
    
    // Draw arrow head (simple triangle pointing in direction of velocity)
    glm::vec3 velocityDir = glm::normalize(velocity);
    glm::vec3 perpendicular = glm::normalize(glm::cross(velocityDir, glm::vec3(0.0f, 1.0f, 0.0f)));
    
    // If velocity is perfectly vertical, use a different perpendicular
    if (glm::length(perpendicular) < 0.01f) {
        perpendicular = glm::normalize(glm::cross(velocityDir, glm::vec3(1.0f, 0.0f, 0.0f)));
    }
    
    float arrowSize = 50.0f;  // Size of arrow head
    glm::vec3 arrowLeft = endPoint - velocityDir * arrowSize + perpendicular * (arrowSize * 0.5f);
    glm::vec3 arrowRight = endPoint - velocityDir * arrowSize - perpendicular * (arrowSize * 0.5f);
    
    float arrowVertices[] = {
        endPoint.x, endPoint.y, endPoint.z,
        arrowLeft.x, arrowLeft.y, arrowLeft.z,
        
        endPoint.x, endPoint.y, endPoint.z,
        arrowRight.x, arrowRight.y, arrowRight.z
    };
    
    glBufferData(GL_ARRAY_BUFFER, sizeof(arrowVertices), arrowVertices, GL_DYNAMIC_DRAW);
    glDrawArrays(GL_LINES, 0, 4);
    
    glBindVertexArray(0);
}

void TacticalOverlay::renderTargetLines(const glm::vec3& playerPos,
                                       const glm::mat4& view,
                                       const glm::mat4& projection) {
    if (!m_overlayShader) return;
    
    m_overlayShader->use();
    m_overlayShader->setMat4("view", view);
    m_overlayShader->setMat4("projection", projection);
    
    glBindVertexArray(m_lineVAO);
    
    for (const auto& target : m_targets) {
        float vertices[] = {
            playerPos.x, playerPos.y, playerPos.z,
            target.position.x, target.position.y, target.position.z
        };
        
        glBindBuffer(GL_ARRAY_BUFFER, m_lineVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
        
        // Set color based on hostile/friendly
        glm::vec4 color = target.hostile ? m_hostileColor : m_friendlyColor;
        m_overlayShader->setVec4("overlayColor", color);
        
        glLineWidth(2.0f);
        glDrawArrays(GL_LINES, 0, 2);
    }
    
    glBindVertexArray(0);
}

void TacticalOverlay::generateCircleVertices(float radius, int segments, std::vector<float>& vertices) {
    vertices.clear();
    vertices.reserve(segments * 3);
    
    for (int i = 0; i < segments; i++) {
        float angle = (float)i / (float)segments * 2.0f * M_PI;
        float x = radius * cos(angle);
        float z = radius * sin(angle);
        
        vertices.push_back(x);
        vertices.push_back(0.0f);  // Y=0 for horizontal circles
        vertices.push_back(z);
    }
}

} // namespace eve
