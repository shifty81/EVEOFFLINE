#pragma once

#include <memory>
#include <glm/glm.hpp>

namespace atlas {

class Shader;

/**
 * WarpEffectRenderer — renders the full-screen warp tunnel overlay.
 *
 * During warp travel this draws speed lines, tunnel vignette, and blue-shift
 * color effects as a screen-space post-processing pass.  The effect intensity
 * varies across the four warp phases (align → accelerate → cruise → decelerate).
 *
 * Usage:
 *   renderer.initialize();
 *   // each frame, after scene rendering:
 *   renderer.update(deltaTime, phase, progress, intensity, direction);
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

private:
    void createFullscreenQuad();

    std::unique_ptr<Shader> m_shader;
    unsigned int m_quadVAO = 0;
    unsigned int m_quadVBO = 0;

    float m_time      = 0.0f;
    float m_intensity = 0.0f;
    float m_phase     = 0.0f;
    float m_progress  = 0.0f;
    glm::vec2 m_direction{0.0f, 1.0f};
};

} // namespace atlas
