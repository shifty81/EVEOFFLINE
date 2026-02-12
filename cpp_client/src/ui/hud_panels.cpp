#include "ui/hud_panels.h"
#include "ui/atlas/atlas_context.h"
#include <algorithm>
#include <cmath>
#include <cstdio>

static constexpr float PI = 3.14159265358979323846f;
static constexpr float METERS_PER_AU = 149597870700.0f;

static aatlas::Color toColor(const float c[4]) {
    return aatlas::Color(c[0], c[1], c[2], c[3]);
}

static aatlas::Color rgba(int r, int g, int b, int a = 255) {
    return aatlas::Color::fromRGBA(r, g, b, a);
}

namespace UI {
namespace HUDPanels {

void GetHealthColorForPercent(float percent, float out_color[4], const float base_color[4]) {
    out_color[0] = base_color[0];
    out_color[1] = base_color[1];
    out_color[2] = base_color[2];
    out_color[3] = base_color[3];

    if (percent < 0.3f) {
        out_color[0] = std::min(1.0f, base_color[0] * 1.3f);
        out_color[1] = base_color[1] * 0.8f;
        out_color[2] = base_color[2] * 0.8f;
    } else if (percent < 0.5f) {
        out_color[0] = base_color[0] * 1.1f;
        out_color[1] = base_color[1] * 0.9f;
        out_color[2] = base_color[2] * 0.9f;
    }
}

void RenderHealthBar(aatlas::AtlasContext& ctx, const char* label, float current, float max,
                     const float color[4], float width) {
    if (max <= 0.0f) max = 1.0f;

    float percent = std::max(0.0f, std::min(1.0f, current / max));
    float adjusted_color[4];
    GetHealthColorForPercent(percent, adjusted_color, color);

    char value_text[96];
    snprintf(value_text, sizeof(value_text), "%s  %.0f / %.0f (%.0f%%)",
             label, current, max, percent * 100.0f);

    auto& r = ctx.renderer();
    r.drawText(value_text, {0, 0}, toColor(SpaceColors::TEXT_PRIMARY));
    r.drawProgressBar(aatlas::Rect(0, 16, width, 14), percent,
                      toColor(adjusted_color), rgba(30, 40, 55, 180));
}

void RenderPanelHeader(aatlas::AtlasContext& ctx, const char* title,
                       const float accent_color[4]) {
    auto& r = ctx.renderer();
    r.drawText(title, {0, 0}, toColor(accent_color));
    r.drawLine({0, 18}, {200, 18}, toColor(SpaceColors::BORDER_SUBTLE));
}

void RenderShipStatus(aatlas::AtlasContext& ctx, const ShipStatus& status) {
    auto& r = ctx.renderer();
    float y = 0.0f;

    RenderPanelHeader(ctx, "SHIP STATUS", SpaceColors::ACCENT_PRIMARY);
    y += 24.0f;

    // Shields
    {
        float pct = (status.shields_max > 0) ?
            std::clamp(status.shields / status.shields_max, 0.0f, 1.0f) : 0.0f;
        float adj[4]; GetHealthColorForPercent(pct, adj, SpaceColors::SHIELD_COLOR);
        char buf[64]; snprintf(buf, sizeof(buf), "Shields  %.0f / %.0f (%.0f%%)",
                               status.shields, status.shields_max, pct * 100.0f);
        r.drawText(buf, {0, y}, toColor(SpaceColors::TEXT_PRIMARY));
        y += 14.0f;
        r.drawProgressBar(aatlas::Rect(0, y, 200, 12), pct, toColor(adj), rgba(30, 40, 55, 180));
        y += 18.0f;
    }

    // Armor
    {
        float pct = (status.armor_max > 0) ?
            std::clamp(status.armor / status.armor_max, 0.0f, 1.0f) : 0.0f;
        float adj[4]; GetHealthColorForPercent(pct, adj, SpaceColors::ARMOR_COLOR);
        char buf[64]; snprintf(buf, sizeof(buf), "Armor  %.0f / %.0f (%.0f%%)",
                               status.armor, status.armor_max, pct * 100.0f);
        r.drawText(buf, {0, y}, toColor(SpaceColors::TEXT_PRIMARY));
        y += 14.0f;
        r.drawProgressBar(aatlas::Rect(0, y, 200, 12), pct, toColor(adj), rgba(30, 40, 55, 180));
        y += 18.0f;
    }

    // Hull
    {
        float pct = (status.hull_max > 0) ?
            std::clamp(status.hull / status.hull_max, 0.0f, 1.0f) : 0.0f;
        float adj[4]; GetHealthColorForPercent(pct, adj, SpaceColors::HULL_COLOR);
        char buf[64]; snprintf(buf, sizeof(buf), "Hull  %.0f / %.0f (%.0f%%)",
                               status.hull, status.hull_max, pct * 100.0f);
        r.drawText(buf, {0, y}, toColor(SpaceColors::TEXT_PRIMARY));
        y += 14.0f;
        r.drawProgressBar(aatlas::Rect(0, y, 200, 12), pct, toColor(adj), rgba(30, 40, 55, 180));
        y += 18.0f;
    }

    r.drawLine({0, y}, {200, y}, toColor(SpaceColors::BORDER_SUBTLE));
    y += 8.0f;

    // Capacitor
    float cap_percent = (status.capacitor_max > 0.0f) ?
        (status.capacitor / status.capacitor_max) : 0.0f;
    aatlas::Color cap_color(1.0f, 0.9f, 0.3f, 1.0f);
    if (cap_percent < 0.25f) {
        cap_color = aatlas::Color(1.0f, 0.3f, 0.0f, 1.0f);
    } else if (cap_percent < 0.5f) {
        cap_color = aatlas::Color(1.0f, 0.6f, 0.0f, 1.0f);
    }

    char cap_text[64];
    snprintf(cap_text, sizeof(cap_text), "Capacitor  %.0f / %.0f (%.0f%%)",
             status.capacitor, status.capacitor_max, cap_percent * 100.0f);
    r.drawText(cap_text, {0, y}, toColor(SpaceColors::TEXT_PRIMARY));
    y += 16.0f;
    r.drawProgressBar(aatlas::Rect(0, y, 200, 12), cap_percent,
                      cap_color, rgba(30, 40, 55, 180));
}

void RenderTargetInfo(aatlas::AtlasContext& ctx, const TargetInfo& target) {
    auto& r = ctx.renderer();
    float y = 0.0f;

    if (!target.is_locked) {
        RenderPanelHeader(ctx, "TARGET INFO", SpaceColors::ACCENT_PRIMARY);
        y += 24.0f;
        r.drawText("No target locked", {0, y}, toColor(SpaceColors::TEXT_SECONDARY));
        return;
    }

    const float* header_color = target.is_hostile ?
        SpaceColors::TARGET_HOSTILE : SpaceColors::TARGET_FRIENDLY;
    RenderPanelHeader(ctx, "TARGET INFO", header_color);
    y += 24.0f;

    r.drawText(target.name, {0, y}, toColor(header_color));
    y += 18.0f;

    char dist_buf[64];
    snprintf(dist_buf, sizeof(dist_buf), "Distance: %.0f m", target.distance);
    r.drawText(dist_buf, {0, y}, toColor(SpaceColors::TEXT_PRIMARY));
    y += 18.0f;

    r.drawLine({0, y}, {250, y}, toColor(SpaceColors::BORDER_SUBTLE));
    y += 8.0f;

    aatlas::Color bgBar = rgba(30, 40, 55, 180);
    if (target.shields_max > 0.0f) {
        float pct = std::clamp(target.shields / target.shields_max, 0.0f, 1.0f);
        float adj[4]; GetHealthColorForPercent(pct, adj, SpaceColors::SHIELD_COLOR);
        r.drawText("Shields", {0, y}, toColor(SpaceColors::TEXT_PRIMARY));
        y += 14.0f;
        r.drawProgressBar(aatlas::Rect(0, y, 250, 12), pct, toColor(adj), bgBar);
        y += 18.0f;
    }
    if (target.armor_max > 0.0f) {
        float pct = std::clamp(target.armor / target.armor_max, 0.0f, 1.0f);
        float adj[4]; GetHealthColorForPercent(pct, adj, SpaceColors::ARMOR_COLOR);
        r.drawText("Armor", {0, y}, toColor(SpaceColors::TEXT_PRIMARY));
        y += 14.0f;
        r.drawProgressBar(aatlas::Rect(0, y, 250, 12), pct, toColor(adj), bgBar);
        y += 18.0f;
    }
    if (target.hull_max > 0.0f) {
        float pct = std::clamp(target.hull / target.hull_max, 0.0f, 1.0f);
        float adj[4]; GetHealthColorForPercent(pct, adj, SpaceColors::HULL_COLOR);
        r.drawText("Hull", {0, y}, toColor(SpaceColors::TEXT_PRIMARY));
        y += 14.0f;
        r.drawProgressBar(aatlas::Rect(0, y, 250, 12), pct, toColor(adj), bgBar);
    }
}

void RenderSpeedDisplay(aatlas::AtlasContext& ctx, float current_speed, float max_speed) {
    auto& r = ctx.renderer();
    float y = 0.0f;

    RenderPanelHeader(ctx, "SPEED", SpaceColors::ACCENT_PRIMARY);
    y += 24.0f;

    char speed_text[64];
    snprintf(speed_text, sizeof(speed_text), "%.1f m/s", current_speed);
    r.drawText(speed_text, {0, y}, toColor(SpaceColors::ACCENT_SECONDARY), 1.5f);
    y += 24.0f;

    char max_text[64];
    snprintf(max_text, sizeof(max_text), "Max: %.1f m/s", max_speed);
    r.drawText(max_text, {0, y}, toColor(SpaceColors::TEXT_PRIMARY));
    y += 18.0f;

    float speed_percent = (max_speed > 0.0f) ?
        std::min(1.0f, current_speed / max_speed) : 0.0f;
    r.drawProgressBar(aatlas::Rect(0, y, 200, 12), speed_percent,
                      toColor(SpaceColors::ACCENT_PRIMARY), rgba(30, 40, 55, 180));
}

// ============================================================================
// HUD-style circular gauges using Atlas renderer arcs
// ============================================================================

static void DrawBlockRingGauge(aatlas::AtlasRenderer& renderer,
                               aatlas::Vec2 center, float radius,
                               float pct, aatlas::Color filledColor,
                               aatlas::Color emptyColor, float thickness,
                               int segments = 32, float gapFraction = 0.08f) {
    float fullCircle = 2.0f * PI;
    float angle_step = fullCircle / static_cast<float>(segments);
    float gap = angle_step * gapFraction;
    int filledCount = static_cast<int>(pct * segments + 0.5f);
    float innerR = radius - thickness * 0.5f;
    float outerR = radius + thickness * 0.5f;

    for (int i = 0; i < segments; i++) {
        float a1 = -PI / 2.0f + angle_step * i + gap;
        float a2 = -PI / 2.0f + angle_step * (i + 1) - gap;
        const aatlas::Color& color = (i < filledCount) ? filledColor : emptyColor;
        renderer.drawArc(center, innerR, outerR, a1, a2, color);
    }
}

static void DrawHalfCircleBlockGauge(aatlas::AtlasRenderer& renderer,
                                     aatlas::Vec2 center, float radius,
                                     float pct, aatlas::Color filledColor,
                                     aatlas::Color emptyColor, float thickness,
                                     int segments = 20, float gapFraction = 0.06f) {
    float angle_min = PI;
    float angle_max = 2.0f * PI;
    float angle_step = (angle_max - angle_min) / static_cast<float>(segments);
    float gap = angle_step * gapFraction;
    float fillCount = pct * segments;
    float innerR = radius - thickness * 0.5f;
    float outerR = radius + thickness * 0.5f;

    for (int i = 0; i < segments; i++) {
        float a1 = angle_min + angle_step * i + gap;
        float a2 = angle_min + angle_step * (i + 1) - gap;
        const aatlas::Color& color =
            (static_cast<float>(i) < fillCount) ? filledColor : emptyColor;
        renderer.drawArc(center, innerR, outerR, a1, a2, color);
    }
}

void RenderShipStatusCircular(aatlas::AtlasContext& ctx, const ShipStatus& status) {
    auto& r = ctx.renderer();

    float outerRadius = 100.0f;
    float barThickness = 10.0f;
    float ringGap = 4.0f;
    float capThickness = 7.0f;

    aatlas::Vec2 center(outerRadius + 15, outerRadius + 10);

    // Dark background circle
    r.drawCircle(center, outerRadius + 8, rgba(8, 12, 18, 230));
    r.drawCircleOutline(center, outerRadius + 8, rgba(35, 50, 65, 100), 1.5f);

    // SHIELD
    float shieldPct = (status.shields_max > 0) ?
        std::clamp(status.shields / status.shields_max, 0.0f, 1.0f) : 0.0f;
    DrawHalfCircleBlockGauge(r, center, outerRadius, shieldPct,
                             rgba(0, 180, 255, 240), rgba(20, 45, 80, 70),
                             barThickness, 20, 0.06f);

    // ARMOR
    float armorPct = (status.armor_max > 0) ?
        std::clamp(status.armor / status.armor_max, 0.0f, 1.0f) : 0.0f;
    float armorRadius = outerRadius - barThickness - ringGap;
    DrawHalfCircleBlockGauge(r, center, armorRadius, armorPct,
                             rgba(255, 210, 60, 240), rgba(80, 65, 15, 70),
                             barThickness, 20, 0.06f);

    // HULL
    float hullPct = (status.hull_max > 0) ?
        std::clamp(status.hull / status.hull_max, 0.0f, 1.0f) : 0.0f;
    float hullRadius = armorRadius - barThickness - ringGap;
    DrawHalfCircleBlockGauge(r, center, hullRadius, hullPct,
                             rgba(230, 55, 55, 240), rgba(80, 20, 20, 70),
                             barThickness, 20, 0.06f);

    // CAPACITOR
    float capPct = (status.capacitor_max > 0) ?
        std::clamp(status.capacitor / status.capacitor_max, 0.0f, 1.0f) : 0.0f;
    float capRadius = hullRadius - barThickness - 8.0f;
    DrawBlockRingGauge(r, center, capRadius, capPct,
                       rgba(255, 225, 70, 220), rgba(45, 40, 15, 90),
                       capThickness, 32, 0.10f);

    // Center text: speed readout
    char speedText[32];
    snprintf(speedText, sizeof(speedText), "%.0f", status.velocity);
    float speedW = r.measureText(speedText, 1.4f);
    r.drawText(speedText, {center.x - speedW / 2, center.y - 12},
               rgba(210, 235, 250, 245), 1.4f);

    const char* unitLabel = "m/s";
    float unitW = r.measureText(unitLabel);
    r.drawText(unitLabel, {center.x - unitW / 2, center.y + 6},
               rgba(130, 150, 170, 170));

    // HP/CAP readout text above the gauge
    float infoY = center.y - outerRadius - 22;
    float infoX = center.x - 60;
    char hpText[96];
    snprintf(hpText, sizeof(hpText), "S:%.0f  A:%.0f  H:%.0f  C:%.0f%%",
             status.shields, status.armor, status.hull, capPct * 100.0f);
    r.drawText(hpText, {infoX, infoY}, rgba(160, 180, 200, 180));

    // MODULE RACK below the gauge
    float rackY = center.y + outerRadius + 18;
    float slotRadius = 13.0f;
    float slotSpacing = 30.0f;
    int totalSlots = 8;
    float rackWidth = totalSlots * slotSpacing;
    float rackStartX = center.x - rackWidth / 2.0f + slotSpacing / 2.0f;

    aatlas::Rect rackBg(rackStartX - slotRadius - 4, rackY - slotRadius - 6,
                        (totalSlots - 1) * slotSpacing + slotRadius * 2 + 8,
                        slotRadius * 2 + 12);
    r.drawRoundedRect(rackBg, rgba(12, 16, 22, 200), 3.0f);
    r.drawRoundedRectOutline(rackBg, rgba(40, 55, 72, 120), 3.0f);

    r.drawText("HIGH         MID          LOW",
               {rackStartX - 4, rackY - slotRadius - 20},
               rgba(120, 150, 180, 140));

    for (int i = 0; i < totalSlots; i++) {
        aatlas::Vec2 slotPos(rackStartX + i * slotSpacing, rackY);

        aatlas::Color inactiveCol;
        if (i < 3) {
            inactiveCol = rgba(22, 38, 28, 190);
        } else if (i < 6) {
            inactiveCol = rgba(22, 30, 45, 190);
        } else {
            inactiveCol = rgba(45, 32, 18, 190);
        }

        r.drawCircle(slotPos, slotRadius, inactiveCol);
        r.drawCircleOutline(slotPos, slotRadius, rgba(55, 75, 95, 140));

        char fLabel[4];
        snprintf(fLabel, sizeof(fLabel), "F%d", i + 1);
        float tw = r.measureText(fLabel);
        r.drawText(fLabel, {slotPos.x - tw / 2, slotPos.y - 6},
                   rgba(200, 220, 240, 220));
    }
}

void RenderSpeedGauge(aatlas::AtlasContext& ctx, float current_speed, float max_speed,
                      bool* approach_active, bool* orbit_active,
                      bool* keep_range_active) {
    auto& r = ctx.renderer();

    float gaugeRadius = 50.0f;
    aatlas::Vec2 center(gaugeRadius + 10, gaugeRadius + 10);

    r.drawCircle(center, gaugeRadius, rgba(13, 17, 23, 200));
    r.drawCircleOutline(center, gaugeRadius, rgba(40, 56, 72, 150));

    float speedPct = (max_speed > 0) ?
        std::clamp(current_speed / max_speed, 0.0f, 1.0f) : 0.0f;
    int segments = 15;
    float arcStart = 0.75f * PI;
    float arcRange = 1.5f * PI;
    float stepAngle = arcRange / static_cast<float>(segments);
    float pctFill = speedPct * segments;
    float gap = stepAngle * 0.08f;
    float innerR = gaugeRadius - 6.0f;
    float outerR = gaugeRadius;

    for (int i = 0; i < segments; i++) {
        float a1 = arcStart + stepAngle * i + gap;
        float a2 = arcStart + stepAngle * (i + 1) - gap;
        aatlas::Color color = (static_cast<float>(i) < pctFill) ?
            rgba(69, 208, 232, 220) : rgba(30, 50, 70, 100);
        r.drawArc(center, innerR, outerR, a1, a2, color);
    }

    char speedText[32];
    snprintf(speedText, sizeof(speedText), "%.0f", current_speed);
    float tw = r.measureText(speedText);
    r.drawText(speedText, {center.x - tw / 2, center.y - 12},
               rgba(200, 230, 245, 240));

    const char* unitLabel = "m/s";
    float uw = r.measureText(unitLabel);
    r.drawText(unitLabel, {center.x - uw / 2, center.y + 4},
               rgba(140, 160, 180, 180));

    // Motion command buttons below the gauge
    float btnY = center.y + gaugeRadius + 16;
    float buttonWidth = 70.0f;
    float buttonHeight = 22.0f;
    float btnX = 0.0f;

    auto drawButton = [&](const char* label, float bx, float by,
                          float bw, float bh, bool highlight) -> bool {
        aatlas::Rect br(bx, by, bw, bh);
        aatlas::Color bg = highlight ?
            aatlas::Color(0.15f, 0.4f, 0.5f, 0.9f) :
            rgba(30, 42, 58, 200);
        r.drawRoundedRect(br, bg, 3.0f);
        r.drawRoundedRectOutline(br, rgba(55, 75, 95, 160), 3.0f);
        float lw = r.measureText(label);
        r.drawText(label, {bx + (bw - lw) / 2, by + 4}, rgba(200, 220, 240, 220));
        return ctx.buttonBehavior(br, ctx.currentID(label));
    };

    bool approachState = approach_active ? *approach_active : false;
    if (drawButton("Approach", btnX, btnY, buttonWidth, buttonHeight, approachState)) {
        if (approach_active) *approach_active = !*approach_active;
    }
    btnX += buttonWidth + 4;

    bool orbitState = orbit_active ? *orbit_active : false;
    if (drawButton("Orbit", btnX, btnY, buttonWidth, buttonHeight, orbitState)) {
        if (orbit_active) *orbit_active = !*orbit_active;
    }
    btnX += buttonWidth + 4;

    bool keepState = keep_range_active ? *keep_range_active : false;
    if (drawButton("Keep", btnX, btnY, buttonWidth, buttonHeight, keepState)) {
        if (keep_range_active) *keep_range_active = !*keep_range_active;
    }

    btnY += buttonHeight + 4;
    float stopWidth = buttonWidth * 3 + 8;
    if (drawButton("STOP", 0, btnY, stopWidth, buttonHeight, false)) {
        if (approach_active) *approach_active = false;
        if (orbit_active) *orbit_active = false;
        if (keep_range_active) *keep_range_active = false;
    }

    btnY += buttonHeight + 4;
    char maxBuf[64];
    snprintf(maxBuf, sizeof(maxBuf), "Max: %.0f m/s", max_speed);
    r.drawText(maxBuf, {0, btnY}, rgba(140, 148, 158, 255));
}

void RenderCombatLog(aatlas::AtlasContext& ctx,
                     const std::vector<std::string>& messages) {
    auto& r = ctx.renderer();
    float y = 0.0f;

    RenderPanelHeader(ctx, "COMBAT LOG", SpaceColors::ACCENT_PRIMARY);
    y += 24.0f;

    if (messages.empty()) {
        r.drawText("No combat activity", {0, y}, toColor(SpaceColors::TEXT_SECONDARY));
    } else {
        for (const auto& message : messages) {
            r.drawText(message, {0, y}, toColor(SpaceColors::TEXT_PRIMARY));
            y += 14.0f;
        }
    }
}

// ============================================================================
// RenderModuleRack
// ============================================================================
void RenderModuleRack(aatlas::AtlasContext& ctx,
                      const ModuleSlotState slots[], int count) {
    if (count <= 0) return;

    auto& r = ctx.renderer();

    float slotRadius = 16.0f;
    float slotSpacing = 36.0f;
    float startX = 10.0f;
    float centerY = slotRadius + 8.0f;

    aatlas::Rect rackBg(startX - slotRadius - 4, centerY - slotRadius - 8,
                        (count - 1) * slotSpacing + slotRadius * 2 + 8,
                        slotRadius * 2 + 16);
    r.drawRoundedRect(rackBg, rgba(12, 16, 22, 210), 3.0f);
    r.drawRoundedRectOutline(rackBg, rgba(40, 55, 72, 120), 3.0f);

    for (int i = 0; i < count; i++) {
        const auto& s = slots[i];
        aatlas::Vec2 slotPos(startX + i * slotSpacing, centerY);

        aatlas::Color activeCol, inactiveCol, emptyCol;
        switch (s.slotType) {
            case ModuleSlotState::HIGH:
                activeCol   = rgba(50, 180, 50, 230);
                inactiveCol = rgba(30, 65, 35, 200);
                emptyCol    = rgba(22, 35, 25, 140);
                break;
            case ModuleSlotState::MID:
                activeCol   = rgba(50, 120, 200, 230);
                inactiveCol = rgba(25, 40, 75, 200);
                emptyCol    = rgba(22, 30, 45, 140);
                break;
            case ModuleSlotState::LOW:
                activeCol   = rgba(200, 140, 40, 230);
                inactiveCol = rgba(70, 50, 20, 200);
                emptyCol    = rgba(45, 32, 18, 140);
                break;
        }

        aatlas::Color bgColor;
        if (!s.fitted) {
            bgColor = emptyCol;
        } else if (s.active) {
            bgColor = activeCol;
        } else {
            bgColor = inactiveCol;
        }

        r.drawCircle(slotPos, slotRadius, bgColor);

        aatlas::Color borderColor = rgba(55, 75, 95, 160);
        if (s.overheated) {
            borderColor = rgba(255, 100, 30, 220);
        } else if (s.active) {
            borderColor = rgba(0, 200, 230, 200);
        }
        r.drawCircleOutline(slotPos, slotRadius, borderColor, s.active ? 2.0f : 1.0f);

        if (s.fitted && s.cooldown_pct > 0.0f) {
            float angle = s.cooldown_pct * 2.0f * PI;
            r.drawArc(slotPos, slotRadius - 3.0f, slotRadius - 1.0f,
                      -PI / 2.0f, -PI / 2.0f + angle, rgba(0, 200, 230, 130));
        }

        char fLabel[4];
        snprintf(fLabel, sizeof(fLabel), "F%d", i + 1);
        float ftw = r.measureText(fLabel);
        int textAlpha = s.fitted ? 240 : 120;
        r.drawText(fLabel, {slotPos.x - ftw / 2, slotPos.y - 6},
                   rgba(200, 220, 240, textAlpha));
    }
}

// ============================================================================
// RenderAlertStack
// ============================================================================
void RenderAlertStack(aatlas::AtlasContext& ctx,
                      const std::vector<HUDAlert>& alerts,
                      float centerX, float baseY) {
    if (alerts.empty()) return;

    auto& r = ctx.renderer();

    float alertHeight = 22.0f;
    float alertWidth = 200.0f;
    float alertGap = 3.0f;
    float startY = baseY;

    for (int i = 0; i < static_cast<int>(alerts.size()); ++i) {
        const auto& alert = alerts[i];

        float remaining = alert.duration - alert.elapsed;
        float alpha = (remaining < 1.0f) ? remaining : 1.0f;
        if (alpha <= 0.0f) continue;

        float y = startY - (i + 1) * (alertHeight + alertGap);
        float x = centerX - alertWidth / 2.0f;

        aatlas::Color bgColor, borderColor, textColor;
        switch (alert.priority) {
            case HUDAlertPriority::CRITICAL:
                bgColor     = rgba(120, 20, 20, static_cast<int>(200 * alpha));
                borderColor = rgba(255, 60, 60, static_cast<int>(220 * alpha));
                textColor   = rgba(255, 100, 100, static_cast<int>(255 * alpha));
                break;
            case HUDAlertPriority::WARNING:
                bgColor     = rgba(100, 80, 10, static_cast<int>(190 * alpha));
                borderColor = rgba(255, 200, 50, static_cast<int>(200 * alpha));
                textColor   = rgba(255, 210, 80, static_cast<int>(255 * alpha));
                break;
            default:
                bgColor     = rgba(20, 50, 80, static_cast<int>(170 * alpha));
                borderColor = rgba(69, 208, 232, static_cast<int>(160 * alpha));
                textColor   = rgba(180, 220, 240, static_cast<int>(240 * alpha));
                break;
        }

        aatlas::Rect box(x, y, alertWidth, alertHeight);
        r.drawRoundedRect(box, bgColor, 2.0f);
        r.drawRoundedRectOutline(box, borderColor, 2.0f);

        float tw = r.measureText(alert.message);
        float textX = x + (alertWidth - tw) / 2.0f;
        float textY = y + (alertHeight - 13) / 2.0f;
        r.drawText(alert.message, {textX, textY}, textColor);
    }
}

// ============================================================================
// RenderSelectedItem
// ============================================================================
void RenderSelectedItem(aatlas::AtlasContext& ctx, const SelectedItemData& item,
                        bool* approach_clicked, bool* orbit_clicked,
                        bool* lock_clicked, bool* warp_clicked) {
    auto& r = ctx.renderer();
    float y = 0.0f;

    if (item.isEmpty()) {
        r.drawText("No item selected", {0, y}, toColor(SpaceColors::TEXT_SECONDARY));
        return;
    }

    aatlas::Color nameColor = item.is_hostile ?
        toColor(SpaceColors::TARGET_HOSTILE) : toColor(SpaceColors::ACCENT_PRIMARY);
    r.drawText(item.name, {0, y}, nameColor);
    y += 16.0f;

    r.drawText(item.type, {0, y}, toColor(SpaceColors::TEXT_SECONDARY));
    y += 16.0f;

    char dist_buf[64];
    if (item.distance < 1000.0f) {
        snprintf(dist_buf, sizeof(dist_buf), "Distance: %.0f m", item.distance);
    } else if (item.distance < 1000000.0f) {
        snprintf(dist_buf, sizeof(dist_buf), "Distance: %.1f km", item.distance / 1000.0f);
    } else {
        snprintf(dist_buf, sizeof(dist_buf), "Distance: %.2f AU", item.distance / METERS_PER_AU);
    }
    r.drawText(dist_buf, {0, y}, toColor(SpaceColors::TEXT_PRIMARY));
    y += 16.0f;

    if (item.velocity > 0.0f) {
        char vel_buf[64];
        snprintf(vel_buf, sizeof(vel_buf), "Velocity: %.0f m/s", item.velocity);
        r.drawText(vel_buf, {0, y}, toColor(SpaceColors::TEXT_PRIMARY));
        y += 16.0f;
    }
    if (item.angular_velocity > 0.001f) {
        char ang_buf[64];
        snprintf(ang_buf, sizeof(ang_buf), "Angular: %.3f rad/s", item.angular_velocity);
        r.drawText(ang_buf, {0, y}, toColor(SpaceColors::TEXT_PRIMARY));
        y += 16.0f;
    }

    if (item.has_health) {
        r.drawLine({0, y}, {180, y}, toColor(SpaceColors::BORDER_SUBTLE));
        y += 6.0f;
        aatlas::Color bgBar = rgba(30, 40, 55, 180);

        float sAdj[4]; GetHealthColorForPercent(item.shields_pct, sAdj, SpaceColors::SHIELD_COLOR);
        r.drawText("S", {0, y}, toColor(SpaceColors::TEXT_PRIMARY));
        r.drawProgressBar(aatlas::Rect(14, y, 180, 12), item.shields_pct, toColor(sAdj), bgBar);
        y += 16.0f;

        float aAdj[4]; GetHealthColorForPercent(item.armor_pct, aAdj, SpaceColors::ARMOR_COLOR);
        r.drawText("A", {0, y}, toColor(SpaceColors::TEXT_PRIMARY));
        r.drawProgressBar(aatlas::Rect(14, y, 180, 12), item.armor_pct, toColor(aAdj), bgBar);
        y += 16.0f;

        float hAdj[4]; GetHealthColorForPercent(item.hull_pct, hAdj, SpaceColors::HULL_COLOR);
        r.drawText("H", {0, y}, toColor(SpaceColors::TEXT_PRIMARY));
        r.drawProgressBar(aatlas::Rect(14, y, 180, 12), item.hull_pct, toColor(hAdj), bgBar);
        y += 16.0f;
    }

    r.drawLine({0, y}, {280, y}, toColor(SpaceColors::BORDER_SUBTLE));
    y += 6.0f;

    float buttonWidth = 65.0f;
    float buttonHeight = 22.0f;
    float btnX = 0.0f;

    auto drawBtn = [&](const char* label, bool* clicked) {
        aatlas::Rect br(btnX, y, buttonWidth, buttonHeight);
        r.drawRoundedRect(br, rgba(30, 42, 58, 200), 3.0f);
        r.drawRoundedRectOutline(br, rgba(55, 75, 95, 160), 3.0f);
        float lw = r.measureText(label);
        r.drawText(label, {btnX + (buttonWidth - lw) / 2, y + 4},
                   rgba(200, 220, 240, 220));
        bool hit = ctx.buttonBehavior(br, ctx.currentID(label));
        if (clicked) *clicked = hit;
        btnX += buttonWidth + 4;
    };

    drawBtn("Approach", approach_clicked);
    drawBtn("Orbit", orbit_clicked);
    drawBtn(item.is_locked ? "Unlock" : "Lock", lock_clicked);
    drawBtn("Warp To", warp_clicked);
}

} // namespace HUDPanels
} // namespace UI
