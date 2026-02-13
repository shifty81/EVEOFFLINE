#include "rendering/renderer.h"
#include "rendering/shader.h"
#include "rendering/camera.h"
#include "rendering/model.h"
#include "rendering/healthbar_renderer.h"
#include "rendering/warp_effect_renderer.h"
#include "core/entity.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_inverse.hpp>
#include <iostream>
#include <random>
#define _USE_MATH_DEFINES
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace atlas {

Renderer::Renderer()
    : m_nebulaVAO(0)
    , m_nebulaVBO(0)
    , m_starfieldVAO(0)
    , m_starfieldVBO(0)
    , m_starCount(4000)
    , m_sunVAO(0)
    , m_sunVBO(0)
    , m_sunEBO(0)
    , m_sunIndexCount(0)
    , m_sunEnabled(false)
    , m_sunPosition(0.0f)
    , m_sunColor(1.0f, 0.95f, 0.85f)
    , m_sunRadius(500000.0f)
    , m_initialized(false)
{
}

Renderer::~Renderer() {
    if (m_nebulaVAO != 0) {
        glDeleteVertexArrays(1, &m_nebulaVAO);
    }
    if (m_nebulaVBO != 0) {
        glDeleteBuffers(1, &m_nebulaVBO);
    }
    if (m_starfieldVAO != 0) {
        glDeleteVertexArrays(1, &m_starfieldVAO);
    }
    if (m_starfieldVBO != 0) {
        glDeleteBuffers(1, &m_starfieldVBO);
    }
    if (m_sunVAO != 0) {
        glDeleteVertexArrays(1, &m_sunVAO);
    }
    if (m_sunVBO != 0) {
        glDeleteBuffers(1, &m_sunVBO);
    }
    if (m_sunEBO != 0) {
        glDeleteBuffers(1, &m_sunEBO);
    }
}

bool Renderer::initialize() {
    std::cout << "Initializing renderer..." << std::endl;
    
    // Initialize GLEW
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW: " << glewGetErrorString(err) << std::endl;
        return false;
    }
    
    std::cout << "OpenGL " << glGetString(GL_VERSION) << std::endl;
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
    
    // Enable features
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Load shaders
    m_basicShader = std::make_unique<Shader>();
    if (!m_basicShader->loadFromFiles("shaders/basic.vert", "shaders/basic.frag")) {
        std::cerr << "Failed to load basic shader" << std::endl;
        return false;
    }
    
    m_starfieldShader = std::make_unique<Shader>();
    if (!m_starfieldShader->loadFromFiles("shaders/starfield.vert", "shaders/starfield.frag")) {
        std::cerr << "Failed to load starfield shader" << std::endl;
        return false;
    }
    
    m_nebulaShader = std::make_unique<Shader>();
    if (!m_nebulaShader->loadFromFiles("shaders/nebula.vert", "shaders/nebula.frag")) {
        std::cerr << "Warning: Failed to load nebula shader - nebula disabled" << std::endl;
        m_nebulaShader.reset();
    }
    
    m_entityShader = std::make_unique<Shader>();
    if (!m_entityShader->loadFromFiles("shaders/entity.vert", "shaders/entity.frag")) {
        std::cerr << "Failed to load entity shader" << std::endl;
        return false;
    }
    
    // Initialize health bar renderer
    m_healthBarRenderer = std::make_unique<HealthBarRenderer>();
    if (!m_healthBarRenderer->initialize()) {
        std::cerr << "Failed to initialize health bar renderer" << std::endl;
        return false;
    }
    
    // Initialize warp effect renderer
    m_warpEffectRenderer = std::make_unique<WarpEffectRenderer>();
    if (!m_warpEffectRenderer->initialize()) {
        std::cerr << "Warning: Warp effect renderer failed — warp tunnel disabled" << std::endl;
        // Non-fatal: game continues without warp tunnel visuals
    }
    
    // Setup nebula background
    setupNebula();
    
    // Setup starfield
    setupStarfield();
    
    // Setup sun sphere
    setupSunMesh();
    
    m_initialized = true;
    std::cout << "Renderer initialized successfully" << std::endl;
    return true;
}

void Renderer::beginFrame() {
    // Nothing needed here for now
}

void Renderer::endFrame() {
    // Nothing needed here for now
}

void Renderer::renderScene(Camera& camera) {
    if (!m_initialized) return;
    
    // Render nebula background
    renderNebula(camera);
    
    // Render starfield
    renderStarfield(camera);
    
    // Render solar system sun (before entities, after background)
    if (m_sunEnabled) {
        renderSun(camera, m_sunPosition, m_sunColor, m_sunRadius);
    }
    
    // Render entities
    renderEntities(camera);
    
    // Note: Health bars are NOT rendered in 3D space in EVE Online
    // They are displayed in the target list UI panel instead
}

void Renderer::clear(const glm::vec4& color) {
    glClearColor(color.r, color.g, color.b, color.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::setViewport(int x, int y, int width, int height) {
    glViewport(x, y, width, height);
}

void Renderer::setupNebula() {
    // Full-screen quad (two triangles in NDC)
    float quadVertices[] = {
        -1.0f, -1.0f,
         1.0f, -1.0f,
         1.0f,  1.0f,
        -1.0f, -1.0f,
         1.0f,  1.0f,
        -1.0f,  1.0f
    };

    glGenVertexArrays(1, &m_nebulaVAO);
    glGenBuffers(1, &m_nebulaVBO);

    glBindVertexArray(m_nebulaVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_nebulaVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    std::cout << "Nebula background setup complete" << std::endl;
}

void Renderer::renderNebula(Camera& camera) {
    if (!m_nebulaShader || m_nebulaVAO == 0) return;

    glDepthMask(GL_FALSE);
    glDisable(GL_DEPTH_TEST);

    m_nebulaShader->use();
    glm::mat4 invViewProj = glm::inverse(camera.getProjectionMatrix() * camera.getViewMatrix());
    m_nebulaShader->setMat4("invViewProj", invViewProj);

    glBindVertexArray(m_nebulaVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
}

void Renderer::setupStarfield() {
    std::cout << "Setting up starfield with " << m_starCount << " stars..." << std::endl;
    
    // Generate random star positions
    std::vector<float> vertices;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> posDist(-5000.0f, 5000.0f);
    std::uniform_real_distribution<float> sizeDist(1.0f, 3.0f);
    std::uniform_real_distribution<float> brightDist(0.3f, 1.0f);
    
    for (int i = 0; i < m_starCount; ++i) {
        // Position (x, y, z)
        vertices.push_back(posDist(gen));
        vertices.push_back(posDist(gen));
        vertices.push_back(posDist(gen));
        
        // Size
        vertices.push_back(sizeDist(gen));
        
        // Brightness (color will be white with varying alpha)
        vertices.push_back(brightDist(gen));
    }
    
    // Create VAO and VBO
    glGenVertexArrays(1, &m_starfieldVAO);
    glGenBuffers(1, &m_starfieldVBO);
    
    glBindVertexArray(m_starfieldVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_starfieldVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    
    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Size attribute
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // Brightness attribute
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(4 * sizeof(float)));
    glEnableVertexAttribArray(2);
    
    glBindVertexArray(0);
    
    std::cout << "Starfield setup complete" << std::endl;
}

void Renderer::renderStarfield(Camera& camera) {
    if (m_starfieldVAO == 0) return;
    
    // Disable depth writing for starfield
    glDepthMask(GL_FALSE);
    
    m_starfieldShader->use();
    m_starfieldShader->setMat4("view", camera.getViewMatrix());
    m_starfieldShader->setMat4("projection", camera.getProjectionMatrix());
    
    // Pass time for twinkling animation (uses wall clock for frame-rate independent animation)
    float starfieldTime = static_cast<float>(glfwGetTime());
    m_starfieldShader->setFloat("time", starfieldTime);
    
    glBindVertexArray(m_starfieldVAO);
    glDrawArrays(GL_POINTS, 0, m_starCount);
    glBindVertexArray(0);
    
    // Re-enable depth writing
    glDepthMask(GL_TRUE);
}

// === Entity Visual Management ===

bool Renderer::createEntityVisual(const std::shared_ptr<Entity>& entity) {
    if (!entity) {
        std::cerr << "Cannot create visual for null entity" << std::endl;
        return false;
    }
    
    const std::string& entityId = entity->getId();
    
    // Check if visual already exists
    if (m_entityVisuals.find(entityId) != m_entityVisuals.end()) {
        std::cout << "Visual already exists for entity " << entityId << std::endl;
        return true;
    }
    
    std::cout << "Creating visual for entity: " << entityId << " (" << entity->getShipType() << ")" << std::endl;
    
    // Create entity visual
    EntityVisual visual;
    
    // Create ship model
    visual.model = Model::createShipModel(entity->getShipType(), entity->getFaction());
    if (!visual.model) {
        std::cerr << "Failed to create ship model for " << entity->getShipType() << std::endl;
        return false;
    }
    
    // Set initial state
    visual.position = entity->getPosition();
    visual.rotation = glm::vec3(0.0f, entity->getRotation(), 0.0f);
    visual.scale = 1.0f;
    
    // Set health data
    const auto& health = entity->getHealth();
    visual.currentShield = health.currentShield;
    visual.maxShield = health.maxShield;
    visual.currentArmor = health.currentArmor;
    visual.maxArmor = health.maxArmor;
    visual.currentHull = health.currentHull;
    visual.maxHull = health.maxHull;
    
    // Store visual
    m_entityVisuals[entityId] = std::move(visual);
    
    std::cout << "Visual created for entity " << entityId << std::endl;
    return true;
}

void Renderer::removeEntityVisual(const std::string& entityId) {
    auto it = m_entityVisuals.find(entityId);
    if (it != m_entityVisuals.end()) {
        std::cout << "Removing visual for entity: " << entityId << std::endl;
        m_entityVisuals.erase(it);
    }
}

void Renderer::updateEntityVisuals(const std::unordered_map<std::string, std::shared_ptr<Entity>>& entities) {
    // Update existing entity visuals
    for (auto& [entityId, visual] : m_entityVisuals) {
        auto entityIt = entities.find(entityId);
        if (entityIt != entities.end()) {
            const auto& entity = entityIt->second;
            
            // Update position and rotation
            visual.position = entity->getPosition();
            visual.rotation = glm::vec3(0.0f, entity->getRotation(), 0.0f);
            
            // Update health data
            const auto& health = entity->getHealth();
            visual.currentShield = health.currentShield;
            visual.maxShield = health.maxShield;
            visual.currentArmor = health.currentArmor;
            visual.maxArmor = health.maxArmor;
            visual.currentHull = health.currentHull;
            visual.maxHull = health.maxHull;
        }
    }
}

void Renderer::renderEntities(Camera& camera) {
    if (!m_entityShader) return;
    
    m_entityShader->use();
    m_entityShader->setMat4("view", camera.getViewMatrix());
    m_entityShader->setMat4("projection", camera.getProjectionMatrix());
    
    // Simple directional light
    m_entityShader->setVec3("lightDir", glm::normalize(glm::vec3(-0.5f, -1.0f, -0.3f)));
    m_entityShader->setVec3("lightColor", glm::vec3(1.0f, 0.95f, 0.9f));
    m_entityShader->setVec3("viewPos", camera.getPosition());
    
    // Render each entity
    for (const auto& [entityId, visual] : m_entityVisuals) {
        if (!visual.model) continue;
        
        // Create model matrix
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, visual.position);
        model = glm::rotate(model, visual.rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, visual.rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, visual.rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, glm::vec3(visual.scale));
        
        m_entityShader->setMat4("model", model);
        
        // Draw model
        visual.model->draw();
    }
}

void Renderer::renderHealthBars(Camera& camera) {
    if (!m_healthBarRenderer) return;
    
    m_healthBarRenderer->begin(camera.getViewMatrix(), camera.getProjectionMatrix());
    
    // Render health bar for each entity
    for (const auto& [entityId, visual] : m_entityVisuals) {
        // Calculate shield/armor/hull percentages
        float shieldPct = (visual.maxShield > 0.0f) ? (visual.currentShield / visual.maxShield) : 0.0f;
        float armorPct = (visual.maxArmor > 0.0f) ? (visual.currentArmor / visual.maxArmor) : 0.0f;
        float hullPct = (visual.maxHull > 0.0f) ? (visual.currentHull / visual.maxHull) : 1.0f;
        
        m_healthBarRenderer->drawHealthBar(
            visual.position,
            shieldPct, armorPct, hullPct,
            visual.maxShield, visual.maxArmor, visual.maxHull
        );
    }
    
    m_healthBarRenderer->end();
}

void Renderer::setSunState(const glm::vec3& position, const glm::vec3& color, float radius) {
    m_sunEnabled = true;
    m_sunPosition = position;
    m_sunColor = color;
    m_sunRadius = radius;
}

void Renderer::disableSun() {
    m_sunEnabled = false;
}

void Renderer::setupSunMesh() {
    // Generate a UV sphere for the sun
    const int stacks = 16;
    const int slices = 24;
    
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    
    // Generate vertices
    for (int i = 0; i <= stacks; ++i) {
        float phi = static_cast<float>(M_PI) * static_cast<float>(i) / stacks;
        for (int j = 0; j <= slices; ++j) {
            float theta = 2.0f * static_cast<float>(M_PI) * static_cast<float>(j) / slices;
            
            float x = std::sin(phi) * std::cos(theta);
            float y = std::cos(phi);
            float z = std::sin(phi) * std::sin(theta);
            
            // Position (unit sphere — scaled in render)
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
            // Normal (same as position for unit sphere)
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
        }
    }
    
    // Generate indices
    for (int i = 0; i < stacks; ++i) {
        for (int j = 0; j < slices; ++j) {
            unsigned int first = static_cast<unsigned int>(i * (slices + 1) + j);
            unsigned int second = first + static_cast<unsigned int>(slices + 1);
            
            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);
            
            indices.push_back(second);
            indices.push_back(second + 1);
            indices.push_back(first + 1);
        }
    }
    
    m_sunIndexCount = static_cast<int>(indices.size());
    
    glGenVertexArrays(1, &m_sunVAO);
    glGenBuffers(1, &m_sunVBO);
    glGenBuffers(1, &m_sunEBO);
    
    glBindVertexArray(m_sunVAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, m_sunVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_sunEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    
    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glBindVertexArray(0);
    
    std::cout << "Sun mesh setup complete (" << m_sunIndexCount / 3 << " triangles)" << std::endl;
}

void Renderer::renderSun(Camera& camera, const glm::vec3& sunPosition,
                          const glm::vec3& sunColor, float sunRadius) {
    if (m_sunVAO == 0 || !m_entityShader) return;
    
    m_entityShader->use();
    m_entityShader->setMat4("view", camera.getViewMatrix());
    m_entityShader->setMat4("projection", camera.getProjectionMatrix());
    
    // Sun is self-illuminating — set light to point from the camera so the
    // entire visible surface is bright (emissive look)
    glm::vec3 toCam = glm::normalize(camera.getPosition() - sunPosition);
    m_entityShader->setVec3("lightDir", toCam);
    m_entityShader->setVec3("lightColor", sunColor * 1.5f);
    m_entityShader->setVec3("viewPos", camera.getPosition());
    
    // Create model matrix: translate to sun position, scale by radius
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, sunPosition);
    model = glm::scale(model, glm::vec3(sunRadius));
    m_entityShader->setMat4("model", model);
    
    // Draw the sun sphere with additive blending for glow
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    
    glBindVertexArray(m_sunVAO);
    glDrawElements(GL_TRIANGLES, m_sunIndexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    
    // Restore normal blending
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void Renderer::updateWarpEffect(int phase, float progress, float intensity,
                                 const glm::vec3& direction, float deltaTime) {
    if (m_warpEffectRenderer) {
        m_warpEffectRenderer->update(deltaTime, phase, progress, intensity, direction);
    }
}

void Renderer::renderWarpEffect() {
    if (m_warpEffectRenderer) {
        m_warpEffectRenderer->render();
    }
}

} // namespace atlas
