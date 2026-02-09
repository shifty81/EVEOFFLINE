#include "ui/hud.h"
#include "ui/ui_manager.h"
#include "ui/eve_panels.h"
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
    // Build a ShipStatus from cached values and delegate to the circular gauge
    UI::ShipStatus status;
    status.shields = m_shields;
    status.shields_max = m_shieldsMax;
    status.armor = m_armor;
    status.armor_max = m_armorMax;
    status.hull = m_hull;
    status.hull_max = m_hullMax;
    status.capacitor = m_capacitor;
    status.capacitor_max = m_capacitorMax;
    status.velocity = m_velocity;
    status.max_velocity = m_maxVelocity;

    UI::EVEPanels::RenderShipStatusCircular(status);
}

void HUD::update(float deltaTime) {
    // Update HUD state (animations, timers, etc.)
    (void)deltaTime;
}

void HUD::setShipStatus(const UI::ShipStatus& status) {
    m_shields = status.shields;
    m_shieldsMax = status.shields_max;
    m_armor = status.armor;
    m_armorMax = status.armor_max;
    m_hull = status.hull;
    m_hullMax = status.hull_max;
    m_capacitor = status.capacitor;
    m_capacitorMax = status.capacitor_max;
    m_velocity = status.velocity;
    m_maxVelocity = status.max_velocity;
}

void HUD::addLogMessage(const std::string& message) {
    m_combatLog.push_back(message);
    if (m_combatLog.size() > static_cast<size_t>(MAX_LOG_MESSAGES)) {
        m_combatLog.erase(m_combatLog.begin());
    }
}

} // namespace eve
