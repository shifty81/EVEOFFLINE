#include "rendering/visual_effects.h"
#include "rendering/particle_system.h"
#include "rendering/shader.h"
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <algorithm>

namespace eve {

VisualEffects::VisualEffects()
    : m_particleSystem(nullptr)
    , m_beamVAO(0)
    , m_beamVBO(0)
{
}

VisualEffects::~VisualEffects() {
    if (m_beamVAO != 0) {
        glDeleteVertexArrays(1, &m_beamVAO);
    }
    if (m_beamVBO != 0) {
        glDeleteBuffers(1, &m_beamVBO);
    }
}

bool VisualEffects::initialize() {
    std::cout << "Initializing visual effects system..." << std::endl;
    
    // Create beam geometry
    createBeamGeometry();
    
    // Load beam shader
    m_beamShader = std::make_unique<Shader>();
    // TODO: Load actual beam shaders
    
    std::cout << "Visual effects system initialized" << std::endl;
    return true;
}

void VisualEffects::update(float deltaTime) {
    // Update all beam effects
    for (auto& beam : m_beams) {
        beam.update(deltaTime);
    }
    
    // Remove dead beams
    removeDeadBeams();
}

void VisualEffects::render(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) {
    // Render beam effects
    renderBeams(viewMatrix, projectionMatrix);
    
    // Particle effects are rendered by the particle system separately
}

void VisualEffects::createWeaponEffect(EffectType type, const glm::vec3& start, const glm::vec3& end) {
    glm::vec4 color = getWeaponColor(type);
    
    // Create beam effect
    BeamEffect beam;
    beam.start = start;
    beam.end = end;
    beam.color = color;
    
    switch (type) {
        case EffectType::LASER_BEAM:
            beam.width = 0.15f;
            beam.maxLife = 0.15f;
            beam.life = beam.maxLife;
            break;
            
        case EffectType::PROJECTILE_BEAM:
            beam.width = 0.08f;
            beam.maxLife = 0.1f;
            beam.life = beam.maxLife;
            break;
            
        case EffectType::RAILGUN_BEAM:
            beam.width = 0.2f;
            beam.maxLife = 0.2f;
            beam.life = beam.maxLife;
            break;
            
        case EffectType::BLASTER_BURST:
            beam.width = 0.25f;
            beam.maxLife = 0.12f;
            beam.life = beam.maxLife;
            // Also create particles
            if (m_particleSystem) {
                m_particleSystem->createWeaponBeam(start, end, color);
            }
            break;
            
        case EffectType::MISSILE_TRAIL:
            // Missiles use particle trails
            if (m_particleSystem) {
                glm::vec3 direction = glm::normalize(end - start);
                m_particleSystem->createEngineTrail(start, direction * 10.0f);
            }
            return; // No beam for missiles
            
        default:
            beam.width = 0.1f;
            beam.maxLife = 0.15f;
            beam.life = beam.maxLife;
            break;
    }
    
    addBeam(beam);
}

void VisualEffects::createExplosion(const glm::vec3& position, EffectType size) {
    if (!m_particleSystem) {
        return;
    }
    
    float explosionSize = 1.0f;
    switch (size) {
        case EffectType::EXPLOSION_SMALL:
            explosionSize = 0.5f;
            break;
        case EffectType::EXPLOSION_MEDIUM:
            explosionSize = 1.0f;
            break;
        case EffectType::EXPLOSION_LARGE:
            explosionSize = 2.0f;
            break;
        default:
            explosionSize = 1.0f;
            break;
    }
    
    m_particleSystem->createExplosion(position, explosionSize);
}

void VisualEffects::createShieldImpact(const glm::vec3& position) {
    if (m_particleSystem) {
        m_particleSystem->createShieldHit(position);
    }
}

void VisualEffects::createWarpEffect(const glm::vec3& position, const glm::vec3& direction) {
    if (m_particleSystem) {
        m_particleSystem->createWarpTunnel(position, direction);
    }
}

void VisualEffects::clear() {
    m_beams.clear();
}

void VisualEffects::renderBeams(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) {
    if (m_beams.empty() || !m_beamShader) {
        return;
    }
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE); // Additive blending for beams
    glDisable(GL_DEPTH_TEST); // Render on top
    
    m_beamShader->use();
    m_beamShader->setMat4("view", viewMatrix);
    m_beamShader->setMat4("projection", projectionMatrix);
    
    glBindVertexArray(m_beamVAO);
    
    for (const auto& beam : m_beams) {
        // Calculate alpha based on life
        float alpha = beam.life / beam.maxLife;
        glm::vec4 color = beam.color;
        color.a *= alpha;
        
        // Create model matrix for the beam
        glm::vec3 direction = beam.end - beam.start;
        float length = glm::length(direction);
        direction = glm::normalize(direction);
        
        // Calculate rotation to align with direction
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, beam.start);
        
        // Align with direction (billboard towards end point)
        // TODO: Proper beam orientation
        
        model = glm::scale(model, glm::vec3(beam.width, length, beam.width));
        
        m_beamShader->setMat4("model", model);
        m_beamShader->setVec4("beamColor", color);
        
        glDrawArrays(GL_LINES, 0, 2);
    }
    
    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);
}

void VisualEffects::addBeam(const BeamEffect& beam) {
    m_beams.push_back(beam);
}

void VisualEffects::removeDeadBeams() {
    m_beams.erase(
        std::remove_if(m_beams.begin(), m_beams.end(),
            [](const BeamEffect& beam) { return !beam.isAlive(); }),
        m_beams.end()
    );
}

glm::vec4 VisualEffects::getWeaponColor(EffectType type) {
    switch (type) {
        case EffectType::LASER_BEAM:
            return glm::vec4(1.0f, 0.2f, 0.2f, 0.9f); // Red laser
            
        case EffectType::PROJECTILE_BEAM:
            return glm::vec4(1.0f, 0.7f, 0.3f, 0.9f); // Orange projectile
            
        case EffectType::RAILGUN_BEAM:
            return glm::vec4(0.3f, 0.7f, 1.0f, 0.9f); // Blue railgun
            
        case EffectType::BLASTER_BURST:
            return glm::vec4(0.3f, 1.0f, 0.3f, 0.9f); // Green blaster
            
        case EffectType::MISSILE_TRAIL:
            return glm::vec4(1.0f, 0.5f, 0.2f, 0.8f); // Orange trail
            
        default:
            return glm::vec4(1.0f, 1.0f, 1.0f, 0.9f); // White default
    }
}

void VisualEffects::createBeamGeometry() {
    // Create a simple line for beam rendering
    float vertices[] = {
        0.0f, 0.0f, 0.0f,  // Start
        0.0f, 1.0f, 0.0f   // End (along Y axis, will be scaled)
    };
    
    glGenVertexArrays(1, &m_beamVAO);
    glGenBuffers(1, &m_beamVBO);
    
    glBindVertexArray(m_beamVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_beamVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    // Position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    
    glBindVertexArray(0);
}

} // namespace eve
