#pragma once

#include <glm/glm.hpp>
#include <string>

namespace atlas {

class VisualEffects;
class ParticleSystem;

/**
 * @brief Damage visual effect helper
 * 
 * Reads damage event data from the server and triggers appropriate
 * visual effects on the client side:
 * - Shield hits: blue ripple / shield impact particles
 * - Armor hits: yellow/orange flash + sparks
 * - Hull hits: red pulse + structural debris
 * - Shield depleted: full-ship blue flash
 * - Armor depleted: fire/smoke particles begin
 * - Hull critical: screen shake + alarm overlay
 */
class DamageEffectHelper {
public:
    DamageEffectHelper();
    ~DamageEffectHelper() = default;

    /**
     * @brief Set rendering subsystem references
     */
    void setVisualEffects(VisualEffects* vfx) { m_vfx = vfx; }
    void setParticleSystem(ParticleSystem* ps) { m_particles = ps; }

    /**
     * @brief Process a damage event from the server and trigger visual effects
     * 
     * @param targetPosition  World position of the entity that was hit
     * @param damage          Amount of damage dealt
     * @param damageType      Damage type: "em", "thermal", "kinetic", "explosive"
     * @param layerHit        Layer that absorbed: "shield", "armor", "hull"
     * @param shieldDepleted  Shield reached 0 on this hit
     * @param armorDepleted   Armor reached 0 on this hit
     * @param hullCritical    Hull below 25% after this hit
     */
    void processDamageEvent(const glm::vec3& targetPosition,
                            float damage,
                            const std::string& damageType,
                            const std::string& layerHit,
                            bool shieldDepleted,
                            bool armorDepleted,
                            bool hullCritical);

    /**
     * @brief Update per-frame state (decay screen shake, etc.)
     */
    void update(float deltaTime);

    /**
     * @brief Current screen shake intensity (0.0 = none, 1.0 = maximum)
     */
    float getScreenShake() const { return m_screenShake; }

    /**
     * @brief Whether hull critical alarm is active
     */
    bool isHullCriticalAlarm() const { return m_hullCriticalAlarm; }

    /**
     * @brief Get the color for a damage type overlay flash
     */
    static glm::vec4 layerColor(const std::string& layerHit);

private:
    VisualEffects* m_vfx = nullptr;
    ParticleSystem* m_particles = nullptr;
    float m_screenShake = 0.0f;
    bool m_hullCriticalAlarm = false;
    float m_hullCriticalTimer = 0.0f;
};

} // namespace atlas
