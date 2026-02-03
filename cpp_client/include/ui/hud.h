#pragma once

#include <string>
#include <vector>

namespace eve {

/**
 * HUD (Heads-Up Display) for showing game information
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
     * Render HUD
     */
    void render();

    /**
     * Update HUD with game state
     */
    void update(float deltaTime);

    /**
     * Add message to combat log
     */
    void addLogMessage(const std::string& message);

private:
    std::vector<std::string> m_combatLog;
    static constexpr int MAX_LOG_MESSAGES = 10;
};

} // namespace eve
