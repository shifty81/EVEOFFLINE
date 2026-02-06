#include "rendering/renderer.h"
#include "rendering/shader.h"
#include "rendering/camera.h"
#include "rendering/model.h"
#include "rendering/healthbar_renderer.h"
#include "core/entity.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <random>

namespace eve {

Renderer::Renderer()
    : m_starfieldVAO(0)
    , m_starfieldVBO(0)
    , m_starCount(2000)
    , m_initialized(false)
{
}

Renderer::~Renderer() {
    if (m_starfieldVAO != 0) {
        glDeleteVertexArrays(1, &m_starfieldVAO);
    }
    if (m_starfieldVBO != 0) {
        glDeleteBuffers(1, &m_starfieldVBO);
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
    
    // Setup starfield
    setupStarfield();
    
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
    
    // Render starfield
    renderStarfield(camera);
    
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

} // namespace eve
