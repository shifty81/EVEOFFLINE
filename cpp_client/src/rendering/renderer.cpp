#include "rendering/renderer.h"
#include "rendering/shader.h"
#include "rendering/camera.h"
#include <glad/glad.h>
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
    
    // Load GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
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

} // namespace eve
