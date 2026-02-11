#pragma once

#include <string>
#include <vector>

namespace atlas { class AtlasContext; }

namespace UI {
    struct ShipStatus;
}

namespace eve {

/**
 * Damage flash type for visual feedback when taking damage.
 */
enum class DamageLayer {
    SHIELD,  // Blue ripple
    ARMOR,   // Yellow/gold flash
    HULL     // Red pulse
};

/**
 * Active damage flash effect for HUD visual feedback.
 */
struct DamageFlash {
    DamageLayer layer = DamageLayer::SHIELD;
    float intensity = 1.0f;    // 0-1, fades over time
    float elapsed = 0.0f;      // Time since flash started
    float duration = 0.5f;     // Total flash duration

    DamageFlash() = default;
    DamageFlash(DamageLayer l, float dur = 0.5f)
        : layer(l), intensity(1.0f), elapsed(0.0f), duration(dur) {}
};

/**
 * HUD (Heads-Up Display) for showing game information.
 *
 * Wraps the EVE-style circular ship status gauge (shield/armor/hull/capacitor
 * rings, speed readout, module rack) provided by HUDPanels, and maintains
 * a scrollable combat log. Supports damage feedback visual effects.
 */
class HUD {
public:
    HUD();
    ~HUD();

    /**
     * Initialize HUD
     */
    bool initialize();

    /**
     * Render HUD elements using Atlas UI.
     * Renders the circular ship status display (via HUDPanels::RenderShipStatusCircular)
     * and the combat log.
     */
    void render(atlas::AtlasContext& ctx);

    /**
     * Update HUD with game state
     */
    void update(float deltaTime);

    /**
     * Set ship status data used for the circular gauge display.
     */
    void setShipStatus(const UI::ShipStatus& status);

    /**
     * Add message to combat log
     */
    void addLogMessage(const std::string& message);

    /**
     * Get combat log messages (read-only).
     */
    const std::vector<std::string>& getCombatLog() const { return m_combatLog; }

    /**
     * Trigger a damage feedback flash on the specified layer.
     * Produces a visual pulse (blue for shield, gold for armor, red for hull).
     */
    void triggerDamageFlash(DamageLayer layer);

    /**
     * Check if any damage flash is currently active.
     */
    bool hasDamageFlash() const { return !m_damageFlashes.empty(); }

private:
    std::vector<std::string> m_combatLog;
    static constexpr int MAX_LOG_MESSAGES = 50;

    // Cached ship status for rendering
    float m_shields = 100.0f;
    float m_shieldsMax = 100.0f;
    float m_armor = 100.0f;
    float m_armorMax = 100.0f;
    float m_hull = 100.0f;
    float m_hullMax = 100.0f;
    float m_capacitor = 100.0f;
    float m_capacitorMax = 100.0f;
    float m_velocity = 0.0f;
    float m_maxVelocity = 100.0f;

    // Damage feedback flashes
    std::vector<DamageFlash> m_damageFlashes;
    static constexpr int MAX_FLASHES = 3;
};

} // namespace eve
