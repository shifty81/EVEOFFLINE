#pragma once

#include <glm/glm.hpp>
#include <vector>

namespace atlas {

class Shader;

/**
 * Tactical Overlay
 * 
 * In-space visualization system showing:
 * - Range circles around player ship
 * - Targeting range indicator
 * - Velocity vectors
 * - Direction indicators
 * - Optimal/falloff weapon ranges
 */
class TacticalOverlay {
public:
    struct RangeCircle {
        float radius;
        glm::vec4 color;
        bool filled;
        float lineWidth;
    };

    TacticalOverlay();
    ~TacticalOverlay();

    /**
     * Initialize overlay rendering
     */
    void initialize();

    /**
     * Update overlay state
     */
    void update(float deltaTime);

    /**
     * Render overlay
     */
    void render(const glm::vec3& playerPosition, 
                const glm::vec3& playerVelocity,
                const glm::mat4& view,
                const glm::mat4& projection);

    /**
     * Toggle overlay visibility
     */
    void toggle();
    bool isVisible() const { return m_visible; }
    void setVisible(bool visible) { m_visible = visible; }

    /**
     * Configure range circles
     */
    void setRangeIncrements(float increment) { m_rangeIncrement = increment; }
    void setMaxRange(float maxRange) { m_maxRange = maxRange; }
    void setTargetingRange(float range) { m_targetingRange = range; }

    /**
     * Weapon range visualization
     */
    void setWeaponRanges(float optimal, float falloff);
    void clearWeaponRanges();

    /**
     * Target indicators
     */
    void addTargetIndicator(const glm::vec3& targetPos, bool hostile = true);
    void clearTargetIndicators();

    /**
     * Visual settings
     */
    void setGridColor(const glm::vec4& color) { m_gridColor = color; }
    void setRangeColor(const glm::vec4& color) { m_rangeColor = color; }
    void setVelocityColor(const glm::vec4& color) { m_velocityColor = color; }

private:
    void renderRangeCircles(const glm::vec3& center, 
                           const glm::mat4& view,
                           const glm::mat4& projection);
    void renderVelocityVector(const glm::vec3& position,
                             const glm::vec3& velocity,
                             const glm::mat4& view,
                             const glm::mat4& projection);
    void renderTargetLines(const glm::vec3& playerPos,
                          const glm::mat4& view,
                          const glm::mat4& projection);
    void generateCircleVertices(float radius, int segments, std::vector<float>& vertices);

    // State
    bool m_visible;
    float m_rangeIncrement;      // Distance between range circles
    float m_maxRange;             // Maximum range to display
    float m_targetingRange;       // Targeting range circle (red dotted)
    float m_weaponOptimal;        // Weapon optimal range
    float m_weaponFalloff;        // Weapon optimal + falloff

    // Target tracking
    struct TargetIndicator {
        glm::vec3 position;
        bool hostile;
    };
    std::vector<TargetIndicator> m_targets;

    // Visual settings
    glm::vec4 m_gridColor;
    glm::vec4 m_rangeColor;
    glm::vec4 m_targetingRangeColor;
    glm::vec4 m_weaponOptimalColor;
    glm::vec4 m_weaponFalloffColor;
    glm::vec4 m_velocityColor;
    glm::vec4 m_targetLineColor;
    glm::vec4 m_hostileColor;
    glm::vec4 m_friendlyColor;

    // Rendering
    unsigned int m_circleVAO;
    unsigned int m_circleVBO;
    unsigned int m_lineVAO;
    unsigned int m_lineVBO;
    Shader* m_overlayShader;

    // Constants
    static constexpr int CIRCLE_SEGMENTS = 64;
    static constexpr float DEFAULT_RANGE_INCREMENT = 10000.0f;  // 10km
    static constexpr float DEFAULT_MAX_RANGE = 100000.0f;       // 100km
};

} // namespace atlas
