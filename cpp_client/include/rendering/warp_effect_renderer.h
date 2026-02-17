#pragma once

#include <memory>
#include <functional>
#include <glm/glm.hpp>

namespace atlas {

class Shader;

/**
 * Warp audio event types for audio system integration.
 */
enum class WarpAudioEvent {
    ENTRY_START,    // Warp acceleration begins (play entry sound)
    CRUISE_START,   // Entered cruise phase (start looping drone)
    CRUISE_STOP,    // Exiting cruise phase (stop looping drone)
    EXIT_START,     // Deceleration begins (play exit sound)
    EXIT_COMPLETE   // Warp finished (cleanup)
};

/**
 * Callback signature for warp audio events.
 * @param event The audio event type
 * @param massNorm Normalized ship mass (0=frigate, 1=capital) for pitch adjustment
 */
using WarpAudioCallback = std::function<void(WarpAudioEvent event, float massNorm)>;

/**
 * WarpEffectRenderer — renders the full-screen warp tunnel overlay.
 *
 * During warp travel this draws a multi-layer cinematic tunnel effect:
 *   Layer 1: Radial distortion (barrel/pincushion around centre)
 *   Layer 2: Starfield velocity bloom (speed lines)
 *   Layer 3: Tunnel skin (procedural noise band)
 *   Layer 4: Vignette (edge darkening)
 *   Layer 5: Breathing effect (subtle pulsing during cruise for meditative feel)
 *
 * Layer intensities are driven by the server-computed WarpTunnelConfig
 * and modulated per-frame from ship mass, warp phase, and accessibility settings.
 *
 * Audio Integration:
 *   Set a WarpAudioCallback to receive phase transition events for audio cues.
 *   The callback is invoked when entering/exiting warp phases, allowing the
 *   audio system to play entry sounds, looping drones, and exit sounds.
 *
 * Usage:
 *   renderer.initialize();
 *   renderer.setAudioCallback([](WarpAudioEvent e, float m) { ... });
 *   // each frame, after scene rendering:
 *   renderer.update(deltaTime, phase, progress, intensity, direction);
 *   renderer.setMassNorm(mass);
 *   renderer.render();
 */
class WarpEffectRenderer {
public:
    WarpEffectRenderer();
    ~WarpEffectRenderer();

    /** Compile shaders and create the fullscreen quad. */
    bool initialize();

    /**
     * Feed per-frame warp state.
     * @param deltaTime  Frame time in seconds.
     * @param phase      Warp phase (0=none, 1=align, 2=accel, 3=cruise, 4=decel).
     * @param progress   Overall warp progress 0–1.
     * @param intensity  Effect intensity 0–1 (0 = hidden, 1 = full tunnel).
     * @param direction  Normalised warp heading (world space; only x/z used).
     */
    void update(float deltaTime, int phase, float progress,
                float intensity, const glm::vec3& direction);

    /**
     * Draw the warp tunnel overlay.
     * Must be called with blending enabled (additive).
     */
    void render();

    /** True when a warp effect is visually active. */
    bool isActive() const { return m_intensity > 0.001f; }

    /**
     * Set normalised ship mass for dynamic intensity (0 = frigate, 1 = capital).
     * Heavier ships produce more radial distortion and deeper audio.
     */
    void setMassNorm(float mass) { m_massNorm = mass; }
    float getMassNorm() const { return m_massNorm; }

    /**
     * Accessibility controls — scale motion, blur, and bass intensity.
     * Each value is 0.0–1.0 (1.0 = full effect, 0.0 = disabled).
     */
    void setAccessibility(float motion, float blur) {
        m_motionScale = motion;
        m_blurScale   = blur;
    }

    /**
     * Set callback for warp audio events.
     * Called when warp phases transition, allowing audio system integration.
     */
    void setAudioCallback(WarpAudioCallback callback) {
        m_audioCallback = std::move(callback);
    }

    /**
     * Get current warp phase (0=none, 1=align, 2=accel, 3=cruise, 4=decel).
     */
    int getCurrentPhase() const { return static_cast<int>(m_phase); }

    /**
     * Get breathing intensity for external audio modulation (0.0-1.0).
     * Synced with visual breathing during cruise phase.
     */
    float getBreathingIntensity() const;

private:
    void createFullscreenQuad();
    void fireAudioEvent(WarpAudioEvent event);

    std::unique_ptr<Shader> m_shader;
    unsigned int m_quadVAO = 0;
    unsigned int m_quadVBO = 0;

    float m_time      = 0.0f;
    float m_intensity = 0.0f;
    float m_phase     = 0.0f;
    float m_progress  = 0.0f;
    float m_massNorm  = 0.0f;
    float m_motionScale = 1.0f;
    float m_blurScale   = 1.0f;
    glm::vec2 m_direction{0.0f, 1.0f};

    // Phase tracking for audio event firing
    int m_lastPhase = 0;

    // Audio callback
    WarpAudioCallback m_audioCallback;
};

} // namespace atlas
