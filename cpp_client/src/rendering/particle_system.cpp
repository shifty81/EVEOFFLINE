#include "rendering/particle_system.h"
#include "rendering/shader.h"
#include <GL/glew.h>
#include <iostream>
#include <algorithm>
#include <random>

namespace eve {

// Random number generator
static std::random_device s_rd;
static std::mt19937 s_gen(s_rd());

// Helper function to get random float in range
static float randomFloat(float min, float max) {
    std::uniform_real_distribution<float> dist(min, max);
    return dist(s_gen);
}

// Helper function to get random vector in sphere
static glm::vec3 randomInSphere(float radius) {
    float theta = randomFloat(0.0f, 2.0f * 3.14159f);
    float phi = randomFloat(0.0f, 3.14159f);
    float r = randomFloat(0.0f, radius);
    
    return glm::vec3(
        r * sin(phi) * cos(theta),
        r * sin(phi) * sin(theta),
        r * cos(phi)
    );
}

ParticleSystem::ParticleSystem()
    : m_maxParticles(10000)
    , m_vao(0)
    , m_vbo(0)
{
    m_particles.reserve(m_maxParticles);
}

ParticleSystem::~ParticleSystem() {
    if (m_vao != 0) {
        glDeleteVertexArrays(1, &m_vao);
    }
    if (m_vbo != 0) {
        glDeleteBuffers(1, &m_vbo);
    }
}

bool ParticleSystem::initialize() {
    std::cout << "Initializing particle system..." << std::endl;
    
    // Create VAO and VBO for particles
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    
    // Allocate buffer for max particles
    glBufferData(GL_ARRAY_BUFFER, m_maxParticles * sizeof(Particle), nullptr, GL_DYNAMIC_DRAW);
    
    // Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)offsetof(Particle, position));
    
    // Color
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)offsetof(Particle, color));
    
    // Size
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)offsetof(Particle, size));
    
    glBindVertexArray(0);
    
    // Load particle shaders
    m_shader = std::make_unique<Shader>();
    if (!m_shader->loadFromFiles("shaders/particle.vert", "shaders/particle.frag")) {
        std::cerr << "Failed to load particle shaders" << std::endl;
        return false;
    }
    
    std::cout << "Particle system initialized (max: " << m_maxParticles << ")" << std::endl;
    return true;
}

void ParticleSystem::update(float deltaTime) {
    // Update all particles
    for (auto& particle : m_particles) {
        particle.update(deltaTime);
        
        // Apply gravity/forces if needed
        // particle.velocity.z -= 9.8f * deltaTime;
    }
    
    // Remove dead particles
    removeDeadParticles();
}

void ParticleSystem::render(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) {
    if (m_particles.empty() || !m_shader) {
        return;
    }
    
    // Update buffer with current particles
    updateBuffers();
    
    // Enable point sprites and blending
    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE); // Additive blending
    glDepthMask(GL_FALSE); // Don't write to depth buffer
    
    // Use shader and set uniforms
    m_shader->use();
    m_shader->setMat4("view", viewMatrix);
    m_shader->setMat4("projection", projectionMatrix);
    
    // Draw particles as points
    glBindVertexArray(m_vao);
    glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(m_particles.size()));
    glBindVertexArray(0);
    
    // Restore state
    glDepthMask(GL_TRUE);
    glDisable(GL_PROGRAM_POINT_SIZE);
}

void ParticleSystem::emit(EmitterType type, const glm::vec3& position, const glm::vec3& direction, int count) {
    switch (type) {
        case EmitterType::ENGINE_TRAIL:
            for (int i = 0; i < count; ++i) {
                createEngineTrail(position, direction);
            }
            break;
        case EmitterType::EXPLOSION:
            createExplosion(position, 1.0f);
            break;
        case EmitterType::SHIELD_HIT:
            for (int i = 0; i < count; ++i) {
                createShieldHit(position);
            }
            break;
        case EmitterType::WARP_TUNNEL:
            createWarpTunnel(position, direction);
            break;
        case EmitterType::DEBRIS:
            createDebris(position, count);
            break;
        default:
            break;
    }
}

void ParticleSystem::createEngineTrail(const glm::vec3& position, const glm::vec3& velocity) {
    Particle p;
    p.position = position + randomInSphere(0.2f);
    p.velocity = -velocity * 0.3f + randomInSphere(2.0f);
    p.color = glm::vec4(1.0f, 0.7f, 0.3f, 1.0f); // Orange glow
    p.life = randomFloat(0.3f, 0.8f);
    p.maxLife = p.life;
    p.size = randomFloat(0.5f, 1.5f);
    
    addParticle(p);
}

void ParticleSystem::createExplosion(const glm::vec3& position, float size) {
    int particleCount = static_cast<int>(50 * size);
    
    for (int i = 0; i < particleCount; ++i) {
        Particle p;
        p.position = position;
        p.velocity = randomInSphere(10.0f * size);
        
        // Color variation (orange to yellow)
        float colorVariation = randomFloat(0.0f, 1.0f);
        p.color = glm::vec4(1.0f, 0.5f + colorVariation * 0.5f, colorVariation * 0.3f, 1.0f);
        
        p.life = randomFloat(0.5f, 1.5f);
        p.maxLife = p.life;
        p.size = randomFloat(1.0f, 3.0f) * size;
        
        addParticle(p);
    }
}

void ParticleSystem::createShieldHit(const glm::vec3& position) {
    int particleCount = 20;
    
    for (int i = 0; i < particleCount; ++i) {
        Particle p;
        p.position = position + randomInSphere(1.0f);
        p.velocity = randomInSphere(5.0f);
        p.color = glm::vec4(0.3f, 0.7f, 1.0f, 1.0f); // Cyan shield color
        p.life = randomFloat(0.2f, 0.5f);
        p.maxLife = p.life;
        p.size = randomFloat(0.5f, 2.0f);
        
        addParticle(p);
    }
}

void ParticleSystem::createWeaponBeam(const glm::vec3& start, const glm::vec3& end, const glm::vec4& color) {
    int particleCount = 10;
    glm::vec3 direction = end - start;
    float distance = glm::length(direction);
    direction = glm::normalize(direction);
    
    for (int i = 0; i < particleCount; ++i) {
        Particle p;
        float t = static_cast<float>(i) / particleCount;
        p.position = start + direction * (distance * t) + randomInSphere(0.3f);
        p.velocity = randomInSphere(1.0f);
        p.color = color;
        p.life = 0.1f;
        p.maxLife = p.life;
        p.size = randomFloat(0.3f, 0.8f);
        
        addParticle(p);
    }
}

void ParticleSystem::createWarpTunnel(const glm::vec3& position, const glm::vec3& direction) {
    int particleCount = 100;
    glm::vec3 normalized = glm::normalize(direction);
    
    for (int i = 0; i < particleCount; ++i) {
        Particle p;
        // Create particles in a tunnel shape
        float angle = randomFloat(0.0f, 2.0f * 3.14159f);
        float radius = randomFloat(1.0f, 3.0f);
        float distance = randomFloat(0.0f, 20.0f);
        
        glm::vec3 perpendicular1 = glm::cross(normalized, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::vec3 perpendicular2 = glm::cross(normalized, perpendicular1);
        
        p.position = position + normalized * distance + 
                     perpendicular1 * (radius * cos(angle)) +
                     perpendicular2 * (radius * sin(angle));
        p.velocity = normalized * randomFloat(5.0f, 15.0f);
        p.color = glm::vec4(0.5f, 0.5f, 1.0f, 0.8f); // Blue warp color
        p.life = randomFloat(1.0f, 2.0f);
        p.maxLife = p.life;
        p.size = randomFloat(0.5f, 2.0f);
        
        addParticle(p);
    }
}

void ParticleSystem::createDebris(const glm::vec3& position, int count) {
    for (int i = 0; i < count; ++i) {
        Particle p;
        p.position = position + randomInSphere(0.5f);
        p.velocity = randomInSphere(8.0f);
        p.color = glm::vec4(0.3f, 0.3f, 0.3f, 1.0f); // Gray debris
        p.life = randomFloat(1.0f, 3.0f);
        p.maxLife = p.life;
        p.size = randomFloat(0.3f, 1.0f);
        
        addParticle(p);
    }
}

void ParticleSystem::clear() {
    m_particles.clear();
}

void ParticleSystem::addParticle(const Particle& particle) {
    if (m_particles.size() < m_maxParticles) {
        m_particles.push_back(particle);
    }
}

void ParticleSystem::removeDeadParticles() {
    m_particles.erase(
        std::remove_if(m_particles.begin(), m_particles.end(),
            [](const Particle& p) { return !p.isAlive(); }),
        m_particles.end()
    );
}

void ParticleSystem::updateBuffers() {
    if (m_particles.empty()) {
        return;
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, m_particles.size() * sizeof(Particle), m_particles.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

} // namespace eve
