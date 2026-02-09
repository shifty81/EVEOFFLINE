#pragma once

#include <string>
#include <vector>

namespace UI {
    struct ShipStatus;
}

namespace eve {

/**
 * HUD (Heads-Up Display) for showing game information.
 *
 * Wraps the EVE-style circular ship status gauge (shield/armor/hull/capacitor
 * rings, speed readout, module rack) provided by EVEPanels, and maintains
 * a scrollable combat log.
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
     * Render HUD elements using ImGui.
     * Renders the circular ship status display (via EVEPanels::RenderShipStatusCircular)
     * and the combat log.
     */
    void render();

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
};

} // namespace eve
