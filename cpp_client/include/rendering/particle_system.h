#pragma once

#include <vector>
#include <memory>
#include <glm/glm.hpp>

namespace atlas {

class Shader;

/**
 * Particle structure
 */
struct Particle {
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec4 color;
    float life;       // Remaining life time
    float maxLife;    // Maximum life time
    float size;
    
    Particle() : life(0.0f), maxLife(1.0f), size(1.0f) {}
    
    bool isAlive() const { return life > 0.0f; }
    
    void update(float deltaTime) {
        life -= deltaTime;
        position += velocity * deltaTime;
    }
};

/**
 * Particle emitter types
 */
enum class EmitterType {
    ENGINE_TRAIL,
    EXPLOSION,
    SHIELD_HIT,
    WEAPON_BEAM,
    WARP_TUNNEL,
    DEBRIS
};

/**
 * Particle System
 * Manages particle emission, update, and rendering
 */
class ParticleSystem {
public:
    ParticleSystem();
    ~ParticleSystem();

    /**
     * Initialize the particle system
     */
    bool initialize();

    /**
     * Update all particles
     */
    void update(float deltaTime);

    /**
     * Render all particles
     */
    void render(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);

    /**
     * Emit particles from a specific emitter
     */
    void emit(EmitterType type, const glm::vec3& position, const glm::vec3& direction, int count = 1);

    /**
     * Create an engine trail effect
     */
    void createEngineTrail(const glm::vec3& position, const glm::vec3& velocity);

    /**
     * Create an explosion effect
     */
    void createExplosion(const glm::vec3& position, float size = 1.0f);

    /**
     * Create a shield hit effect
     */
    void createShieldHit(const glm::vec3& position);

    /**
     * Create a weapon beam effect
     */
    void createWeaponBeam(const glm::vec3& start, const glm::vec3& end, const glm::vec4& color);

    /**
     * Create a warp tunnel effect
     */
    void createWarpTunnel(const glm::vec3& position, const glm::vec3& direction);

    /**
     * Create debris particles
     */
    void createDebris(const glm::vec3& position, int count = 10);

    /**
     * Clear all particles
     */
    void clear();

    /**
     * Get active particle count
     */
    size_t getParticleCount() const { return m_particles.size(); }

    /**
     * Set maximum particle count
     */
    void setMaxParticles(size_t maxCount) { m_maxParticles = maxCount; }

private:
    std::vector<Particle> m_particles;
    size_t m_maxParticles;
    
    // OpenGL resources
    unsigned int m_vao;
    unsigned int m_vbo;
    std::unique_ptr<Shader> m_shader;
    
    // Helper methods
    void addParticle(const Particle& particle);
    void removeDeadParticles();
    void updateBuffers();
};

} // namespace atlas
