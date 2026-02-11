#include "ui/hud.h"
#include "ui/ui_manager.h"
#include "ui/hud_panels.h"
#include "ui/atlas/atlas_context.h"
#include <iostream>
#include <algorithm>

namespace eve {

HUD::HUD() {
}

HUD::~HUD() {
}

bool HUD::initialize() {
    std::cout << "HUD initialized" << std::endl;
    return true;
}

void HUD::render(atlas::AtlasContext& ctx) {
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

    UI::HUDPanels::RenderShipStatusCircular(ctx, status);

    // Render active damage flashes as screen overlays
    if (!m_damageFlashes.empty()) {
        auto& r = ctx.renderer();
        float outerRadius = 100.0f;
        atlas::Vec2 center(outerRadius + 15, outerRadius + 10);

        for (const auto& flash : m_damageFlashes) {
            float alpha = flash.intensity * (1.0f - flash.elapsed / flash.duration);
            if (alpha <= 0.0f) continue;

            atlas::Color flashColor;
            float flashRadius;
            switch (flash.layer) {
                case DamageLayer::SHIELD:
                    flashColor = atlas::Color::fromRGBA(0, 150, 255, static_cast<int>(80 * alpha));
                    flashRadius = outerRadius + 12.0f;
                    break;
                case DamageLayer::ARMOR:
                    flashColor = atlas::Color::fromRGBA(255, 200, 50, static_cast<int>(80 * alpha));
                    flashRadius = outerRadius - 2.0f;
                    break;
                case DamageLayer::HULL:
                    flashColor = atlas::Color::fromRGBA(255, 50, 50, static_cast<int>(100 * alpha));
                    flashRadius = outerRadius - 16.0f;
                    break;
            }

            r.drawCircleOutline(center, flashRadius, flashColor, 3.0f * alpha);
        }
    }
}

void HUD::update(float deltaTime) {
    // Update damage flashes
    for (auto& flash : m_damageFlashes) {
        flash.elapsed += deltaTime;
        flash.intensity = std::max(0.0f, 1.0f - flash.elapsed / flash.duration);
    }
    // Remove expired flashes
    m_damageFlashes.erase(
        std::remove_if(m_damageFlashes.begin(), m_damageFlashes.end(),
            [](const DamageFlash& f) { return f.elapsed >= f.duration; }),
        m_damageFlashes.end());
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

void HUD::triggerDamageFlash(DamageLayer layer) {
    // Replace existing flash for same layer or add new one
    for (auto& flash : m_damageFlashes) {
        if (flash.layer == layer) {
            flash.elapsed = 0.0f;
            flash.intensity = 1.0f;
            return;
        }
    }
    if (static_cast<int>(m_damageFlashes.size()) < MAX_FLASHES) {
        m_damageFlashes.emplace_back(layer);
    }
}

} // namespace eve
