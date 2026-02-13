#include "rendering/damage_effect_helper.h"
#include "rendering/visual_effects.h"
#include "rendering/particle_system.h"

namespace atlas {

DamageEffectHelper::DamageEffectHelper() = default;

void DamageEffectHelper::processDamageEvent(const glm::vec3& targetPosition,
                                             float damage,
                                             const std::string& damageType,
                                             const std::string& layerHit,
                                             bool shieldDepleted,
                                             bool armorDepleted,
                                             bool hullCritical) {
    // Shield hit: blue ripple effect
    if (layerHit == "shield") {
        if (m_vfx) {
            m_vfx->createShieldImpact(targetPosition);
        }
        if (m_particles) {
            m_particles->createShieldHit(targetPosition);
        }
        // Shield depleted: burst of particles marking shield collapse
        if (shieldDepleted && m_particles) {
            m_particles->emit(EmitterType::SHIELD_HIT, targetPosition, glm::vec3(0, 1, 0), 20);
        }
    }
    // Armor hit: orange sparks
    else if (layerHit == "armor") {
        if (m_particles) {
            // Sparks flying off armor plating
            m_particles->emit(EmitterType::DEBRIS, targetPosition, glm::vec3(0, 1, 0), 5);
        }
        if (armorDepleted && m_particles) {
            // Fire/smoke effect when armor is stripped
            m_particles->createExplosion(targetPosition, 0.5f);
        }
    }
    // Hull hit: red debris + structural damage
    else if (layerHit == "hull") {
        if (m_particles) {
            m_particles->createDebris(targetPosition, 8);
        }
        if (m_vfx) {
            m_vfx->createExplosion(targetPosition, EffectType::EXPLOSION_SMALL);
        }
    }

    // Hull critical: screen shake + alarm
    if (hullCritical) {
        m_screenShake = 1.0f;
        m_hullCriticalAlarm = true;
        m_hullCriticalTimer = 3.0f;  // alarm lasts 3 seconds
    }

    // Proportional screen shake for large hits
    if (damage > 100.0f) {
        float shake = std::min(damage / 500.0f, 0.5f);
        if (shake > m_screenShake) {
            m_screenShake = shake;
        }
    }
}

void DamageEffectHelper::update(float deltaTime) {
    // Decay screen shake
    if (m_screenShake > 0.0f) {
        m_screenShake -= deltaTime * 3.0f;  // decay over ~0.33s
        if (m_screenShake < 0.0f) {
            m_screenShake = 0.0f;
        }
    }

    // Decay hull critical alarm
    if (m_hullCriticalAlarm) {
        m_hullCriticalTimer -= deltaTime;
        if (m_hullCriticalTimer <= 0.0f) {
            m_hullCriticalAlarm = false;
            m_hullCriticalTimer = 0.0f;
        }
    }
}

glm::vec4 DamageEffectHelper::layerColor(const std::string& layerHit) {
    if (layerHit == "shield") {
        return glm::vec4(0.2f, 0.4f, 1.0f, 0.6f);  // blue
    } else if (layerHit == "armor") {
        return glm::vec4(1.0f, 0.7f, 0.1f, 0.6f);  // orange/yellow
    } else if (layerHit == "hull") {
        return glm::vec4(1.0f, 0.15f, 0.1f, 0.7f); // red
    }
    return glm::vec4(1.0f, 1.0f, 1.0f, 0.3f);  // white default
}

} // namespace atlas
