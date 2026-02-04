#include "ui/hud.h"
#include <iostream>

namespace eve {

HUD::HUD() {
}

HUD::~HUD() {
}

bool HUD::initialize() {
    std::cout << "HUD initialized" << std::endl;
    return true;
}

void HUD::render() {
    // Render HUD elements (not yet implemented)
}

void HUD::update(float deltaTime) {
    // Update HUD state
}

void HUD::addLogMessage(const std::string& message) {
    m_combatLog.push_back(message);
    if (m_combatLog.size() > MAX_LOG_MESSAGES) {
        m_combatLog.erase(m_combatLog.begin());
    }
}

} // namespace eve
