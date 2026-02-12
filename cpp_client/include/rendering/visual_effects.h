#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <vector>

namespace atlas {

class Shader;
class ParticleSystem;

/**
 * Effect types
 */
enum class EffectType {
    LASER_BEAM,
    PROJECTILE_BEAM,
    MISSILE_TRAIL,
    RAILGUN_BEAM,
    BLASTER_BURST,
    EXPLOSION_SMALL,
    EXPLOSION_MEDIUM,
    EXPLOSION_LARGE,
    SHIELD_IMPACT,
    WARP_EFFECT
};

/**
 * Beam effect structure
 */
struct BeamEffect {
    glm::vec3 start;
    glm::vec3 end;
    glm::vec4 color;
    float width;
    float life;
    float maxLife;
    
    BeamEffect() : width(0.1f), life(0.0f), maxLife(0.2f) {}
    
    bool isAlive() const { return life > 0.0f; }
    
    void update(float deltaTime) {
        life -= deltaTime;
    }
};

/**
 * Visual Effects Manager
 * Manages weapon effects, explosions, and other visual effects
 */
class VisualEffects {
public:
    VisualEffects();
    ~VisualEffects();

    /**
     * Initialize the effects system
     */
    bool initialize();

    /**
     * Update all effects
     */
    void update(float deltaTime);

    /**
     * Render all effects
     */
    void render(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);

    /**
     * Create a weapon effect
     */
    void createWeaponEffect(EffectType type, const glm::vec3& start, const glm::vec3& end);

    /**
     * Create an explosion effect
     */
    void createExplosion(const glm::vec3& position, EffectType size = EffectType::EXPLOSION_MEDIUM);

    /**
     * Create a shield impact effect
     */
    void createShieldImpact(const glm::vec3& position);

    /**
     * Create a warp effect
     */
    void createWarpEffect(const glm::vec3& position, const glm::vec3& direction);

    /**
     * Clear all effects
     */
    void clear();

    /**
     * Set particle system for particle-based effects
     */
    void setParticleSystem(ParticleSystem* particleSystem) { m_particleSystem = particleSystem; }

private:
    std::vector<BeamEffect> m_beams;
    ParticleSystem* m_particleSystem;
    
    // OpenGL resources for beam rendering
    unsigned int m_beamVAO;
    unsigned int m_beamVBO;
    std::unique_ptr<Shader> m_beamShader;
    
    // Animation time tracker
    float m_time;
    
    // Helper methods
    void renderBeams(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);
    void addBeam(const BeamEffect& beam);
    void removeDeadBeams();
    glm::vec4 getWeaponColor(EffectType type);
    void createBeamGeometry();
};

} // namespace atlas
