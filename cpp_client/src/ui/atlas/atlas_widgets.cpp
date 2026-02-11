#include "ui/atlas/atlas_widgets.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static constexpr float METERS_PER_AU = 149597870700.0f;

namespace atlas {

// ── Panel ───────────────────────────────────────────────────────────

bool panelBegin(AtlasContext& ctx, const char* title,
                Rect& bounds, const PanelFlags& flags,
                bool* open) {
    const Theme& t = ctx.theme();
    auto& r = ctx.renderer();

    ctx.pushID(title);

    // Panel background (sharp-cornered dark translucent rect)
    r.drawRect(bounds, t.bgPanel);

    // Border
    if (flags.drawBorder) {
        r.drawRectOutline(bounds, t.borderSubtle, t.borderWidth);
    }

    // Header bar
    if (flags.showHeader) {
        float hh = flags.compactMode ? t.headerHeight * 0.75f : t.headerHeight;
        Rect headerRect = {bounds.x, bounds.y, bounds.w, hh};
        r.drawRect(headerRect, t.bgHeader);

        // Title text
        float textY = bounds.y + (hh - 13.0f) * 0.5f;
        r.drawText(title, {bounds.x + t.padding, textY}, t.textPrimary);

        // Close button (×)
        if (flags.showClose && open) {
            float btnSz = 14.0f;
            Rect closeRect = {bounds.right() - btnSz - 4.0f,
                              bounds.y + (hh - btnSz) * 0.5f,
                              btnSz, btnSz};
            WidgetID closeID = ctx.currentID("_close");
            bool hovered = ctx.isHovered(closeRect);
            if (hovered) ctx.setHot(closeID);
            Color closeBg = hovered ? t.danger.withAlpha(0.6f)
                                    : t.bgSecondary;
            r.drawRect(closeRect, closeBg);
            r.drawText("x", {closeRect.x + 3.0f, closeRect.y + 1.0f},
                       t.textPrimary);
            if (ctx.buttonBehavior(closeRect, closeID)) {
                *open = false;
            }
        }

        // Minimize button (—)
        if (flags.showMinimize) {
            float btnSz = 14.0f;
            float offset = (flags.showClose && open) ? 22.0f : 4.0f;
            Rect minRect = {bounds.right() - btnSz - offset,
                            bounds.y + (hh - btnSz) * 0.5f,
                            btnSz, btnSz};
            WidgetID minID = ctx.currentID("_min");
            bool hovered = ctx.isHovered(minRect);
            if (hovered) ctx.setHot(minID);
            Color minBg = hovered ? t.hover : t.bgSecondary;
            r.drawRect(minRect, minBg);
            r.drawText("-", {minRect.x + 3.0f, minRect.y + 1.0f},
                       t.textPrimary);
        }

        // Thin accent line under header
        r.drawRect({bounds.x, bounds.y + hh - 1.0f, bounds.w, 1.0f},
                   t.accentDim);
    }

    return true;  // panel is open (minimize state not yet tracked)
}

void panelEnd(AtlasContext& ctx) {
    ctx.popID();
}

// ── Button ──────────────────────────────────────────────────────────

bool button(AtlasContext& ctx, const char* label, const Rect& r) {
    const Theme& t = ctx.theme();
    auto& rr = ctx.renderer();

    WidgetID id = ctx.currentID(label);
    bool clicked = ctx.buttonBehavior(r, id);

    Color bg = t.bgSecondary;
    if (ctx.isActive(id))      bg = t.selection;
    else if (ctx.isHot(id))    bg = t.hover;

    rr.drawRect(r, bg);
    rr.drawRectOutline(r, t.borderNormal);

    float tw = rr.measureText(label);
    float tx = r.x + (r.w - tw) * 0.5f;
    float ty = r.y + (r.h - 13.0f) * 0.5f;
    rr.drawText(label, {tx, ty}, t.textPrimary);

    return clicked;
}

bool iconButton(AtlasContext& ctx, WidgetID id, const Rect& r,
                const Color& iconColor) {
    const Theme& t = ctx.theme();
    auto& rr = ctx.renderer();

    bool clicked = ctx.buttonBehavior(r, id);

    Color bg = t.bgPrimary;
    if (ctx.isActive(id))      bg = t.selection;
    else if (ctx.isHot(id))    bg = t.hover;

    rr.drawRect(r, bg);

    // Draw a small filled circle as icon placeholder
    Vec2 c = r.center();
    float iconR = std::min(r.w, r.h) * 0.3f;
    rr.drawCircle(c, iconR, iconColor);

    return clicked;
}

// ── Progress Bar ────────────────────────────────────────────────────

void progressBar(AtlasContext& ctx, const Rect& r,
                 float fraction, const Color& fillColor,
                 const char* label) {
    const Theme& t = ctx.theme();
    auto& rr = ctx.renderer();

    // Background
    rr.drawRect(r, t.bgSecondary.withAlpha(0.5f));
    // Fill
    float fill = std::max(0.0f, std::min(1.0f, fraction));
    if (fill > 0.0f) {
        rr.drawRect({r.x, r.y, r.w * fill, r.h}, fillColor);
    }
    // Border
    rr.drawRectOutline(r, t.borderSubtle);

    // Label text
    if (label) {
        float ty = r.y + (r.h - 13.0f) * 0.5f;
        rr.drawText(label, {r.x + 4.0f, ty}, t.textPrimary);
    }
}

// ── Ship Status Arcs ────────────────────────────────────────────────

void shipStatusArcs(AtlasContext& ctx, Vec2 centre, float outerR,
                    float shieldPct, float armorPct, float hullPct) {
    const Theme& t = ctx.theme();
    auto& rr = ctx.renderer();

    // Arc layout (from screenshot):
    // - Arcs sweep the TOP half only (π to 0, i.e. left-to-right)
    // - Shield = outermost, Armor = middle, Hull = innermost
    // - Ring thickness ~8px each with ~2px gap
    float ringW = 8.0f;
    float gap = 2.0f;

    // Start/end angles: top semicircle = π to 0 (sweep right)
    float startAngle = static_cast<float>(M_PI);
    float endAngle   = 0.0f;

    struct ArcDef {
        float pct; Color full; Color empty; float innerR; float outerR;
    };

    float shieldInner = outerR - ringW;
    float armorOuter  = shieldInner - gap;
    float armorInner  = armorOuter - ringW;
    float hullOuter   = armorInner - gap;
    float hullInner   = hullOuter - ringW;

    ArcDef arcs[] = {
        {shieldPct, t.shield, t.bgSecondary.withAlpha(0.3f), shieldInner, outerR},
        {armorPct,  t.armor,  t.bgSecondary.withAlpha(0.3f), armorInner,  armorOuter},
        {hullPct,   t.hull,   t.bgSecondary.withAlpha(0.3f), hullInner,   hullOuter},
    };

    for (const auto& a : arcs) {
        // Draw empty (background) arc
        rr.drawArc(centre, a.innerR, a.outerR, startAngle, endAngle,
                   a.empty, 32);
        // Draw filled arc
        if (a.pct > 0.001f) {
            float fillEnd = startAngle + (endAngle - startAngle) * a.pct;
            rr.drawArc(centre, a.innerR, a.outerR, startAngle, fillEnd,
                       a.full, 32);
        }
    }

    // Percentage labels (left of arcs, stacked vertically)
    char buf[16];
    float labelX = centre.x - outerR - 40.0f;
    float labelBaseY = centre.y - outerR * 0.5f;

    std::snprintf(buf, sizeof(buf), "%d%%", static_cast<int>(shieldPct * 100));
    rr.drawText(buf, {labelX, labelBaseY}, t.shield);

    std::snprintf(buf, sizeof(buf), "%d%%", static_cast<int>(armorPct * 100));
    rr.drawText(buf, {labelX, labelBaseY + 16.0f}, t.armor);

    std::snprintf(buf, sizeof(buf), "%d%%", static_cast<int>(hullPct * 100));
    rr.drawText(buf, {labelX, labelBaseY + 32.0f}, t.hull);
}

// ── Capacitor Ring ──────────────────────────────────────────────────

void capacitorRing(AtlasContext& ctx, Vec2 centre,
                   float innerR, float outerR,
                   float fraction, int segments) {
    const Theme& t = ctx.theme();
    auto& rr = ctx.renderer();

    float gapAngle = 0.03f;  // small gap between segments
    float totalAngle = 2.0f * static_cast<float>(M_PI);
    float segAngle = totalAngle / segments - gapAngle;
    int filledCount = static_cast<int>(fraction * segments + 0.5f);

    for (int i = 0; i < segments; ++i) {
        float a0 = totalAngle * i / segments;
        float a1 = a0 + segAngle;
        bool filled = (i < filledCount);
        Color c = filled ? t.capacitor : t.bgSecondary.withAlpha(0.25f);
        rr.drawArc(centre, innerR, outerR, a0, a1, c, 4);
    }
}

// ── Module Slot ─────────────────────────────────────────────────────

bool moduleSlot(AtlasContext& ctx, Vec2 centre, float radius,
                bool active, float cooldownPct, const Color& color) {
    const Theme& t = ctx.theme();
    auto& rr = ctx.renderer();

    Rect hitbox = {centre.x - radius, centre.y - radius,
                   radius * 2, radius * 2};
    WidgetID id = hashID("mod") ^ static_cast<uint32_t>(centre.x * 1000)
                                ^ static_cast<uint32_t>(centre.y * 1000);
    bool clicked = ctx.buttonBehavior(hitbox, id);

    // Background circle
    Color bg = active ? color.withAlpha(0.4f) : t.bgSecondary.withAlpha(0.6f);
    if (ctx.isHot(id)) bg = t.hover;
    rr.drawCircle(centre, radius, bg);

    // Border ring
    Color border = active ? color : t.borderNormal;
    rr.drawCircleOutline(centre, radius, border, 1.5f);

    // Cooldown sweep overlay (clockwise from top)
    if (cooldownPct > 0.001f && cooldownPct < 1.0f) {
        float startA = -static_cast<float>(M_PI) * 0.5f;  // top
        float sweepA = 2.0f * static_cast<float>(M_PI) * cooldownPct;
        rr.drawArc(centre, 0.0f, radius - 1.0f, startA, startA + sweepA,
                   Color(0, 0, 0, 0.5f), 16);
    }

    // Active glow dot in centre
    if (active) {
        rr.drawCircle(centre, radius * 0.3f, color);
    }

    return clicked;
}

// ── Module Slot with Overheat ───────────────────────────────────────

bool moduleSlotEx(AtlasContext& ctx, Vec2 centre, float radius,
                  bool active, float cooldownPct, const Color& color,
                  float overheatPct, float time) {
    const Theme& t = ctx.theme();
    auto& rr = ctx.renderer();

    Rect hitbox = {centre.x - radius, centre.y - radius,
                   radius * 2, radius * 2};
    WidgetID id = hashID("modex") ^ static_cast<uint32_t>(centre.x * 1000)
                                  ^ static_cast<uint32_t>(centre.y * 1000);
    bool clicked = ctx.buttonBehavior(hitbox, id);

    // Background circle
    Color bg = active ? color.withAlpha(0.4f) : t.bgSecondary.withAlpha(0.6f);
    if (ctx.isHot(id)) bg = t.hover;
    rr.drawCircle(centre, radius, bg);

    // Overheat glow ring: pulsing red/orange border when overheated
    if (overheatPct > 0.01f) {
        // Pulse frequency increases with heat level
        float pulseRate = 2.0f + overheatPct * 4.0f;
        float pulse = 0.5f + 0.5f * std::sin(time * pulseRate * 2.0f * static_cast<float>(M_PI));
        float alpha = 0.3f + overheatPct * 0.7f * pulse;

        // Lerp from orange to red as heat increases
        Color heatColor = {1.0f, 0.5f * (1.0f - overheatPct), 0.0f, alpha};
        rr.drawCircleOutline(centre, radius + 1.0f, heatColor, 2.0f);

        // Inner heat tint
        rr.drawCircle(centre, radius - 1.0f, heatColor.withAlpha(alpha * 0.15f));
    }

    // Normal border ring
    Color border = active ? color : t.borderNormal;
    if (overheatPct >= 1.0f) {
        border = Color(0.5f, 0.1f, 0.1f, 0.8f);  // burnt out: dim red
    }
    rr.drawCircleOutline(centre, radius, border, 1.5f);

    // Cooldown sweep overlay (clockwise from top)
    if (cooldownPct > 0.001f && cooldownPct < 1.0f) {
        float startA = -static_cast<float>(M_PI) * 0.5f;
        float sweepA = 2.0f * static_cast<float>(M_PI) * cooldownPct;
        rr.drawArc(centre, 0.0f, radius - 1.0f, startA, startA + sweepA,
                   Color(0, 0, 0, 0.5f), 16);
    }

    // Active glow dot in centre
    if (active) {
        rr.drawCircle(centre, radius * 0.3f, color);
    }

    return clicked;
}

// ── Capacitor Ring Animated ─────────────────────────────────────────

void capacitorRingAnimated(AtlasContext& ctx, Vec2 centre,
                           float innerR, float outerR,
                           float targetFrac, float& displayFrac,
                           float dt, int segments, float lerpSpeed) {
    // Exponential ease toward target
    float diff = targetFrac - displayFrac;
    displayFrac += diff * std::min(1.0f, lerpSpeed * dt);
    // Snap when close enough to avoid floating imprecision
    if (std::fabs(diff) < 0.001f) displayFrac = targetFrac;

    // Clamp
    displayFrac = std::max(0.0f, std::min(1.0f, displayFrac));

    // Draw using the existing capacitorRing with smoothed value
    capacitorRing(ctx, centre, innerR, outerR, displayFrac, segments);
}

// ── Speed Indicator ─────────────────────────────────────────────────

void speedIndicator(AtlasContext& ctx, Vec2 pos,
                    float currentSpeed, float maxSpeed) {
    const Theme& t = ctx.theme();
    auto& rr = ctx.renderer();

    // Background bar
    float barW = 120.0f, barH = 20.0f;
    Rect bar = {pos.x - barW * 0.5f, pos.y, barW, barH};
    rr.drawRect(bar, t.bgSecondary.withAlpha(0.7f));
    rr.drawRectOutline(bar, t.borderSubtle);

    // Fill based on speed fraction
    float frac = (maxSpeed > 0.0f) ? currentSpeed / maxSpeed : 0.0f;
    frac = std::max(0.0f, std::min(1.0f, frac));
    if (frac > 0.0f) {
        rr.drawRect({bar.x, bar.y, bar.w * frac, bar.h},
                    t.accentDim.withAlpha(0.5f));
    }

    // Speed text
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%.1f m/s", currentSpeed);
    float tw = rr.measureText(buf);
    rr.drawText(buf, {pos.x - tw * 0.5f, pos.y + 3.0f}, t.textPrimary);

    // +/- buttons
    float btnSz = 16.0f;
    Rect minus = {bar.x - btnSz - 4.0f, pos.y + 2.0f, btnSz, btnSz};
    Rect plus  = {bar.right() + 4.0f,   pos.y + 2.0f, btnSz, btnSz};
    button(ctx, "-", minus);
    button(ctx, "+", plus);
}

// ── Overview ────────────────────────────────────────────────────────

void overviewHeader(AtlasContext& ctx, const Rect& r,
                    const std::vector<std::string>& tabs,
                    int activeTab) {
    const Theme& t = ctx.theme();
    auto& rr = ctx.renderer();

    // Header background
    rr.drawRect(r, t.bgHeader);

    // Tabs
    float tabX = r.x + 4.0f;
    float tabH = r.h - 4.0f;
    for (int i = 0; i < static_cast<int>(tabs.size()); ++i) {
        float tw = rr.measureText(tabs[i]) + 16.0f;
        Rect tabRect = {tabX, r.y + 2.0f, tw, tabH};

        if (i == activeTab) {
            rr.drawRect(tabRect, t.selection);
            rr.drawText(tabs[i], {tabX + 8.0f, r.y + 5.0f}, t.textPrimary);
        } else {
            rr.drawText(tabs[i], {tabX + 8.0f, r.y + 5.0f}, t.textSecondary);
        }
        tabX += tw + 2.0f;
    }

    // Bottom border
    rr.drawRect({r.x, r.bottom() - 1.0f, r.w, 1.0f}, t.borderSubtle);
}

bool overviewRow(AtlasContext& ctx, const Rect& r,
                 const OverviewEntry& entry, bool isAlternate) {
    const Theme& t = ctx.theme();
    auto& rr = ctx.renderer();

    WidgetID id = hashID(entry.name.c_str());
    bool clicked = ctx.buttonBehavior(r, id);

    // Row background
    Color bg = isAlternate ? t.bgSecondary.withAlpha(0.3f)
                           : t.bgPanel.withAlpha(0.2f);
    if (entry.selected) bg = t.selection;
    else if (ctx.isHot(id)) bg = t.hover;
    rr.drawRect(r, bg);

    // Standing color indicator (small square, 4px)
    rr.drawRect({r.x + 2.0f, r.y + 3.0f, 4.0f, r.h - 6.0f},
                entry.standingColor);

    // Columns: Distance | Name | Type | Velocity
    float colX = r.x + 10.0f;
    float textY = r.y + (r.h - 13.0f) * 0.5f;

    // Distance
    char distBuf[32];
    if (entry.distance >= 1000.0f) {
        std::snprintf(distBuf, sizeof(distBuf), "%.0f km", entry.distance / 1000.0f);
    } else {
        std::snprintf(distBuf, sizeof(distBuf), "%.0f m", entry.distance);
    }
    rr.drawText(distBuf, {colX, textY}, t.textSecondary);

    // Name
    colX += 80.0f;
    rr.drawText(entry.name, {colX, textY}, t.textPrimary);

    // Type
    colX += 120.0f;
    rr.drawText(entry.type, {colX, textY}, t.textSecondary);

    return clicked;
}

// ── Target Card ─────────────────────────────────────────────────────

bool targetCard(AtlasContext& ctx, const Rect& r,
                const TargetCardInfo& info) {
    const Theme& t = ctx.theme();
    auto& rr = ctx.renderer();

    WidgetID id = hashID(info.name.c_str());
    bool clicked = ctx.buttonBehavior(r, id);

    // Card background
    Color bg = t.bgPanel;
    if (info.isActive) bg = t.selection;
    else if (ctx.isHot(id)) bg = t.hover;
    rr.drawRect(r, bg);

    // Border (red if hostile, blue if friendly)
    Color borderColor = info.isHostile ? t.hostile : t.borderNormal;
    if (info.isActive) borderColor = t.accentPrimary;
    rr.drawRectOutline(r, borderColor, 2.0f);

    // Mini shield/armor/hull bars (horizontal, stacked at bottom)
    float barH = 3.0f;
    float barW = r.w - 8.0f;
    float barX = r.x + 4.0f;
    float barY = r.bottom() - 16.0f;

    rr.drawProgressBar({barX, barY, barW, barH}, info.shieldPct,
                       t.shield, t.bgSecondary.withAlpha(0.3f));
    rr.drawProgressBar({barX, barY + barH + 1.0f, barW, barH}, info.armorPct,
                       t.armor, t.bgSecondary.withAlpha(0.3f));
    rr.drawProgressBar({barX, barY + 2*(barH + 1.0f), barW, barH}, info.hullPct,
                       t.hull, t.bgSecondary.withAlpha(0.3f));

    // Name (truncated to fit)
    std::string displayName = info.name;
    if (rr.measureText(displayName) > r.w - 8.0f) {
        while (displayName.size() > 3 &&
               rr.measureText(displayName + "..") > r.w - 8.0f) {
            displayName.pop_back();
        }
        displayName += "..";
    }
    rr.drawText(displayName, {r.x + 4.0f, r.bottom() - 30.0f},
                t.textPrimary);

    // Distance
    char distBuf[32];
    if (info.distance >= 1000.0f) {
        std::snprintf(distBuf, sizeof(distBuf), "%.0f km", info.distance / 1000.0f);
    } else {
        std::snprintf(distBuf, sizeof(distBuf), "%.0f m", info.distance);
    }
    float tw = rr.measureText(distBuf);
    rr.drawText(distBuf, {r.x + (r.w - tw) * 0.5f, r.y + 4.0f},
                t.textSecondary);

    return clicked;
}

// ── Selected Item Panel ─────────────────────────────────────────────

void selectedItemPanel(AtlasContext& ctx, const Rect& r,
                       const SelectedItemInfo& info) {
    const Theme& t = ctx.theme();
    auto& rr = ctx.renderer();

    rr.drawRect(r, t.bgPanel);
    rr.drawRectOutline(r, t.borderSubtle);

    // Header: "Selected Item"
    rr.drawRect({r.x, r.y, r.w, t.headerHeight}, t.bgHeader);
    rr.drawText("Selected Item", {r.x + t.padding, r.y + 7.0f},
                t.textSecondary);
    rr.drawRect({r.x, r.y + t.headerHeight - 1.0f, r.w, 1.0f},
                t.accentDim);

    // Name
    float textY = r.y + t.headerHeight + 8.0f;
    rr.drawText(info.name, {r.x + t.padding, textY}, t.textPrimary);

    // Distance
    textY += 18.0f;
    char distBuf[64];
    std::snprintf(distBuf, sizeof(distBuf), "Distance: %.0f %s",
                  info.distance, info.distanceUnit.c_str());
    rr.drawText(distBuf, {r.x + t.padding, textY}, t.textSecondary);

    // Action buttons row (orbit, approach, warp, look at, info)
    float btnY = textY + 24.0f;
    float btnSz = 24.0f;
    float btnGap = 6.0f;
    float btnX = r.x + t.padding;
    const char* actions[] = {"O", ">>", "W", "i"};
    for (int i = 0; i < 4; ++i) {
        Rect btn = {btnX, btnY, btnSz, btnSz};
        button(ctx, actions[i], btn);
        btnX += btnSz + btnGap;
    }
}

// ── Utility Widgets ─────────────────────────────────────────────────

void label(AtlasContext& ctx, Vec2 pos, const std::string& text,
           const Color& color) {
    const Theme& t = ctx.theme();
    Color c = (color.a > 0.01f) ? color : t.textPrimary;
    ctx.renderer().drawText(text, pos, c);
}

void separator(AtlasContext& ctx, Vec2 start, float width) {
    const Theme& t = ctx.theme();
    ctx.renderer().drawRect({start.x, start.y, width, 1.0f}, t.borderSubtle);
}

bool treeNode(AtlasContext& ctx, const Rect& r,
              const char* nodeLabel, bool* expanded) {
    const Theme& t = ctx.theme();
    auto& rr = ctx.renderer();

    WidgetID id = ctx.currentID(nodeLabel);
    bool clicked = ctx.buttonBehavior(r, id);

    if (ctx.isHot(id)) {
        rr.drawRect(r, t.hover);
    }

    // Triangle indicator
    float triX = r.x + 4.0f;
    float triY = r.y + r.h * 0.5f;
    if (expanded && *expanded) {
        // Down-pointing triangle (▼)
        rr.drawLine({triX, triY - 4.0f}, {triX + 8.0f, triY - 4.0f},
                    t.textSecondary, 1.0f);
        rr.drawLine({triX, triY - 4.0f}, {triX + 4.0f, triY + 4.0f},
                    t.textSecondary, 1.0f);
        rr.drawLine({triX + 8.0f, triY - 4.0f}, {triX + 4.0f, triY + 4.0f},
                    t.textSecondary, 1.0f);
    } else {
        // Right-pointing triangle (▶)
        rr.drawLine({triX, triY - 5.0f}, {triX, triY + 5.0f},
                    t.textSecondary, 1.0f);
        rr.drawLine({triX, triY - 5.0f}, {triX + 8.0f, triY},
                    t.textSecondary, 1.0f);
        rr.drawLine({triX, triY + 5.0f}, {triX + 8.0f, triY},
                    t.textSecondary, 1.0f);
    }

    // Label text
    rr.drawText(nodeLabel, {r.x + 16.0f, r.y + (r.h - 13.0f) * 0.5f},
                t.textPrimary);

    if (clicked && expanded) {
        *expanded = !(*expanded);
    }
    return expanded ? *expanded : false;
}

void scrollbar(AtlasContext& ctx, const Rect& track,
               float scrollOffset, float contentHeight, float viewHeight) {
    const Theme& t = ctx.theme();
    auto& rr = ctx.renderer();

    if (contentHeight <= viewHeight) return;

    // Track background
    rr.drawRect(track, t.bgSecondary.withAlpha(0.3f));

    // Thumb
    float thumbRatio = viewHeight / contentHeight;
    float thumbH = std::max(20.0f, track.h * thumbRatio);
    float scrollRange = contentHeight - viewHeight;
    float thumbOffset = (scrollRange > 0.0f)
        ? (scrollOffset / scrollRange) * (track.h - thumbH) : 0.0f;

    Rect thumb = {track.x, track.y + thumbOffset, track.w, thumbH};
    rr.drawRect(thumb, t.accentDim);
}

// ── Sidebar Bar ─────────────────────────────────────────────────────

void sidebarBar(AtlasContext& ctx, float x, float width, float height,
               int icons, const std::function<void(int)>& callback) {
    const Theme& t = ctx.theme();
    auto& rr = ctx.renderer();

    // Full-height dark background
    Rect bar = {x, 0, width, height};
    rr.drawRect(bar, t.bgPrimary);
    rr.drawRect({x + width - 1.0f, 0, 1.0f, height}, t.borderSubtle);

    // Icon buttons (stacked vertically from top)
    float iconSz = width - 4.0f;
    float iconY = 8.0f;
    float iconGap = 4.0f;

    for (int i = 0; i < icons; ++i) {
        Rect iconRect = {x + 2.0f, iconY, iconSz, iconSz};
        WidgetID iconID = hashID("neocom") ^ static_cast<uint32_t>(i);

        // Alternate icon colors for visual variety
        Color iconColor = (i % 3 == 0) ? t.accentPrimary
                        : (i % 3 == 1) ? t.textSecondary
                                       : t.accentDim;

        if (iconButton(ctx, iconID, iconRect, iconColor)) {
            if (callback) callback(i);
        }

        iconY += iconSz + iconGap;
    }
}

// ── Tooltip ─────────────────────────────────────────────────────────

void tooltip(AtlasContext& ctx, const std::string& text) {
    const Theme& t = ctx.theme();
    auto& rr = ctx.renderer();

    float tw = rr.measureText(text) + 16.0f;
    float th = 24.0f;
    Vec2 mouse = ctx.input().mousePos;
    // Position tooltip slightly below and to the right of cursor
    Rect tipRect = {mouse.x + 12.0f, mouse.y + 16.0f, tw, th};

    // Clamp to window bounds
    float winW = static_cast<float>(ctx.input().windowW);
    float winH = static_cast<float>(ctx.input().windowH);
    if (tipRect.right() > winW) tipRect.x = winW - tw;
    if (tipRect.bottom() > winH) tipRect.y = mouse.y - th - 4.0f;

    rr.drawRect(tipRect, t.bgTooltip);
    rr.drawRectOutline(tipRect, t.borderNormal);
    rr.drawText(text, {tipRect.x + 8.0f, tipRect.y + 5.0f}, t.textPrimary);
}

// ── Checkbox ────────────────────────────────────────────────────────

bool checkbox(AtlasContext& ctx, const char* label,
              const Rect& r, bool* checked) {
    const Theme& t = ctx.theme();
    auto& rr = ctx.renderer();

    WidgetID id = ctx.currentID(label);

    float boxSz = std::min(r.h - 4.0f, 16.0f);
    Rect box = {r.x + 2.0f, r.y + (r.h - boxSz) * 0.5f, boxSz, boxSz};
    bool clicked = ctx.buttonBehavior(box, id);

    // Box background
    Color bg = t.bgSecondary;
    if (ctx.isHot(id)) bg = t.hover;
    rr.drawRect(box, bg);
    rr.drawRectOutline(box, t.borderNormal);

    // Check mark (simple filled inner square when checked)
    if (checked && *checked) {
        float inset = 3.0f;
        Rect inner = {box.x + inset, box.y + inset,
                      box.w - 2 * inset, box.h - 2 * inset};
        rr.drawRect(inner, t.accentPrimary);
    }

    // Label text
    float textX = box.right() + 6.0f;
    float textY = r.y + (r.h - 13.0f) * 0.5f;
    rr.drawText(label, {textX, textY}, t.textPrimary);

    if (clicked && checked) {
        *checked = !(*checked);
        return true;
    }
    return false;
}

// ── ComboBox ────────────────────────────────────────────────────────

bool comboBox(AtlasContext& ctx, const char* label,
              const Rect& r, const std::vector<std::string>& items,
              int* selected, bool* dropdownOpen) {
    const Theme& t = ctx.theme();
    auto& rr = ctx.renderer();

    WidgetID id = ctx.currentID(label);
    bool changed = false;

    // Main combo button
    bool mainClicked = ctx.buttonBehavior(r, id);

    Color bg = t.bgSecondary;
    if (ctx.isHot(id)) bg = t.hover;
    rr.drawRect(r, bg);
    rr.drawRectOutline(r, t.borderNormal);

    // Current selection text
    if (selected && *selected >= 0 && *selected < static_cast<int>(items.size())) {
        float textY = r.y + (r.h - 13.0f) * 0.5f;
        rr.drawText(items[*selected], {r.x + 6.0f, textY}, t.textPrimary);
    }

    // Dropdown arrow indicator
    float arrowX = r.right() - 14.0f;
    float arrowY = r.y + r.h * 0.5f;
    rr.drawLine({arrowX, arrowY - 3.0f}, {arrowX + 4.0f, arrowY + 3.0f},
                t.textSecondary, 1.0f);
    rr.drawLine({arrowX + 4.0f, arrowY + 3.0f}, {arrowX + 8.0f, arrowY - 3.0f},
                t.textSecondary, 1.0f);

    // Toggle dropdown on click
    if (mainClicked && dropdownOpen) {
        *dropdownOpen = !(*dropdownOpen);
    }

    // Dropdown list
    if (dropdownOpen && *dropdownOpen && !items.empty()) {
        float itemH = 22.0f;
        float dropH = itemH * items.size();
        Rect dropRect = {r.x, r.bottom(), r.w, dropH};

        rr.drawRect(dropRect, t.bgPanel);
        rr.drawRectOutline(dropRect, t.borderNormal);

        for (int i = 0; i < static_cast<int>(items.size()); ++i) {
            Rect itemRect = {r.x, r.bottom() + i * itemH, r.w, itemH};
            WidgetID itemID = ctx.currentID(items[i].c_str());

            bool itemClicked = ctx.buttonBehavior(itemRect, itemID);

            Color itemBg = (i % 2 == 0) ? t.bgSecondary.withAlpha(0.3f)
                                         : t.bgPanel.withAlpha(0.3f);
            if (selected && i == *selected) itemBg = t.selection;
            else if (ctx.isHot(itemID)) itemBg = t.hover;

            rr.drawRect(itemRect, itemBg);
            float textY = itemRect.y + (itemH - 13.0f) * 0.5f;
            rr.drawText(items[i], {itemRect.x + 6.0f, textY}, t.textPrimary);

            if (itemClicked && selected) {
                *selected = i;
                *dropdownOpen = false;
                changed = true;
            }
        }
    }

    return changed;
}

// ── Stateful Panel ──────────────────────────────────────────────────

bool panelBeginStateful(AtlasContext& ctx, const char* title,
                        PanelState& state, const PanelFlags& flags) {
    if (!state.open) return false;

    const Theme& t = ctx.theme();
    auto& rr = ctx.renderer();

    ctx.pushID(title);

    // Handle dragging (header-bar drag to move)
    if (!flags.locked && flags.showHeader) {
        float hh = flags.compactMode ? t.headerHeight * 0.75f : t.headerHeight;
        Rect headerHit = {state.bounds.x, state.bounds.y, state.bounds.w, hh};
        WidgetID dragID = ctx.currentID("_drag");

        if (ctx.isHovered(headerHit) && ctx.isMouseClicked()) {
            state.dragging = true;
            state.dragOffset = {ctx.input().mousePos.x - state.bounds.x,
                                ctx.input().mousePos.y - state.bounds.y};
            ctx.setActive(dragID);
        }

        if (state.dragging) {
            if (ctx.isMouseDown()) {
                state.bounds.x = ctx.input().mousePos.x - state.dragOffset.x;
                state.bounds.y = ctx.input().mousePos.y - state.dragOffset.y;

                // Clamp to window bounds
                float winW = static_cast<float>(ctx.input().windowW);
                float winH = static_cast<float>(ctx.input().windowH);
                state.bounds.x = std::max(0.0f, std::min(state.bounds.x, winW - state.bounds.w));
                state.bounds.y = std::max(0.0f, std::min(state.bounds.y, winH - t.headerHeight));
            } else {
                state.dragging = false;
                ctx.clearActive();
            }
        }
    }

    // Draw panel background
    Rect drawBounds = state.bounds;
    if (state.minimized) {
        float hh = flags.compactMode ? t.headerHeight * 0.75f : t.headerHeight;
        drawBounds.h = hh;
    }

    rr.drawRect(drawBounds, t.bgPanel);

    // Border
    if (flags.drawBorder) {
        rr.drawRectOutline(drawBounds, t.borderSubtle, t.borderWidth);
    }

    // Header bar
    if (flags.showHeader) {
        float hh = flags.compactMode ? t.headerHeight * 0.75f : t.headerHeight;
        Rect headerRect = {drawBounds.x, drawBounds.y, drawBounds.w, hh};
        rr.drawRect(headerRect, t.bgHeader);

        float textY = drawBounds.y + (hh - 13.0f) * 0.5f;
        rr.drawText(title, {drawBounds.x + t.padding, textY}, t.textPrimary);

        // Close button (×)
        if (flags.showClose) {
            float btnSz = 14.0f;
            Rect closeRect = {drawBounds.right() - btnSz - 4.0f,
                              drawBounds.y + (hh - btnSz) * 0.5f,
                              btnSz, btnSz};
            WidgetID closeID = ctx.currentID("_close");
            bool hovered = ctx.isHovered(closeRect);
            if (hovered) ctx.setHot(closeID);
            Color closeBg = hovered ? t.danger.withAlpha(0.6f) : t.bgSecondary;
            rr.drawRect(closeRect, closeBg);
            rr.drawText("x", {closeRect.x + 3.0f, closeRect.y + 1.0f}, t.textPrimary);
            if (ctx.buttonBehavior(closeRect, closeID)) {
                state.open = false;
            }
        }

        // Minimize button (—)
        if (flags.showMinimize) {
            float btnSz = 14.0f;
            float offset = flags.showClose ? 22.0f : 4.0f;
            Rect minRect = {drawBounds.right() - btnSz - offset,
                            drawBounds.y + (hh - btnSz) * 0.5f,
                            btnSz, btnSz};
            WidgetID minID = ctx.currentID("_min");
            bool hovered = ctx.isHovered(minRect);
            if (hovered) ctx.setHot(minID);
            Color minBg = hovered ? t.hover : t.bgSecondary;
            rr.drawRect(minRect, minBg);
            rr.drawText("-", {minRect.x + 3.0f, minRect.y + 1.0f}, t.textPrimary);
            if (ctx.buttonBehavior(minRect, minID)) {
                state.minimized = !state.minimized;
            }
        }

        // Thin accent line under header
        rr.drawRect({drawBounds.x, drawBounds.y + hh - 1.0f, drawBounds.w, 1.0f},
                    t.accentDim);
    }

    return !state.minimized;
}

// ── Slider ──────────────────────────────────────────────────────────

bool slider(AtlasContext& ctx, const char* label,
            const Rect& r, float* value,
            float minVal, float maxVal,
            const char* format) {
    const Theme& t = ctx.theme();
    auto& rr = ctx.renderer();

    WidgetID id = ctx.currentID(label);
    bool changed = false;

    // Track background
    rr.drawRect(r, t.bgSecondary.withAlpha(0.5f));
    rr.drawRectOutline(r, t.borderSubtle);

    if (!value) return false;

    float range = maxVal - minVal;
    if (range <= 0.0f) return false;

    float frac = (*value - minVal) / range;
    frac = std::max(0.0f, std::min(1.0f, frac));

    // Filled portion
    if (frac > 0.0f) {
        rr.drawRect({r.x, r.y, r.w * frac, r.h}, t.accentDim.withAlpha(0.6f));
    }

    // Thumb (small vertical bar)
    float thumbX = r.x + r.w * frac;
    float thumbW = 6.0f;
    Rect thumbRect = {thumbX - thumbW * 0.5f, r.y, thumbW, r.h};

    bool hovered = ctx.isHovered(r);
    if (hovered) ctx.setHot(id);

    Color thumbColor = t.accentPrimary;
    if (ctx.isActive(id)) thumbColor = t.accentSecondary;
    else if (ctx.isHot(id)) thumbColor = t.accentPrimary.withAlpha(0.8f);

    rr.drawRect(thumbRect, thumbColor);

    // Interaction: click or drag to set value
    if (hovered && ctx.isMouseClicked()) {
        ctx.setActive(id);
    }

    if (ctx.isActive(id)) {
        if (ctx.isMouseDown()) {
            float mouseX = ctx.input().mousePos.x;
            float newFrac = (mouseX - r.x) / r.w;
            newFrac = std::max(0.0f, std::min(1.0f, newFrac));
            float newValue = minVal + newFrac * range;
            if (newValue != *value) {
                *value = newValue;
                changed = true;
            }
        } else {
            ctx.clearActive();
        }
    }

    // Value label
    if (format) {
        char buf[64];
        std::snprintf(buf, sizeof(buf), format, *value);
        float tw = rr.measureText(buf);
        float tx = r.x + (r.w - tw) * 0.5f;
        float ty = r.y + (r.h - 13.0f) * 0.5f;
        rr.drawText(buf, {tx, ty}, t.textPrimary);
    }

    return changed;
}

// ── Text Input ──────────────────────────────────────────────────────

bool textInput(AtlasContext& ctx, const char* label,
               const Rect& r, TextInputState& state,
               const char* placeholder) {
    const Theme& t = ctx.theme();
    auto& rr = ctx.renderer();

    WidgetID id = ctx.currentID(label);
    bool changed = false;

    // Background
    Color bg = state.focused ? t.bgSecondary : t.bgSecondary.withAlpha(0.5f);
    rr.drawRect(r, bg);

    // Border
    Color borderCol = state.focused ? t.accentPrimary : t.borderNormal;
    rr.drawRectOutline(r, borderCol);

    // Click to focus
    bool hovered = ctx.isHovered(r);
    if (hovered) ctx.setHot(id);

    if (hovered && ctx.isMouseClicked()) {
        state.focused = true;
        ctx.setActive(id);
    } else if (ctx.isMouseClicked() && !hovered) {
        state.focused = false;
    }

    // Text display
    float textY = r.y + (r.h - 13.0f) * 0.5f;
    float textX = r.x + 6.0f;

    if (state.text.empty() && !state.focused && placeholder) {
        rr.drawText(placeholder, {textX, textY}, t.textDisabled);
    } else {
        rr.drawText(state.text, {textX, textY}, t.textPrimary);
    }

    // Blinking cursor (approximated — draw when focused)
    if (state.focused) {
        int pos = std::min(state.cursorPos, static_cast<int>(state.text.size()));
        std::string beforeCursor = state.text.substr(0, pos);
        float cursorX = textX + rr.measureText(beforeCursor);
        rr.drawRect({cursorX, r.y + 3.0f, 1.0f, r.h - 6.0f}, t.textPrimary);
    }

    return changed;
}

// ── Notification Toast ──────────────────────────────────────────────

void notification(AtlasContext& ctx, const std::string& text,
                  const Color& color) {
    const Theme& t = ctx.theme();
    auto& rr = ctx.renderer();

    float winW = static_cast<float>(ctx.input().windowW);
    float notifW = rr.measureText(text) + 32.0f;
    float notifH = 28.0f;
    float notifX = (winW - notifW) * 0.5f;
    float notifY = 40.0f;

    // Background
    rr.drawRect({notifX, notifY, notifW, notifH}, t.bgPanel);
    rr.drawRectOutline({notifX, notifY, notifW, notifH}, t.borderSubtle);

    // Left accent bar
    Color accentCol = (color.a > 0.01f) ? color : t.accentPrimary;
    rr.drawRect({notifX, notifY, 3.0f, notifH}, accentCol);

    // Text
    float textY = notifY + (notifH - 13.0f) * 0.5f;
    rr.drawText(text, {notifX + 12.0f, textY}, t.textPrimary);
}

// ── Mode Indicator ──────────────────────────────────────────────────

void modeIndicator(AtlasContext& ctx, Vec2 pos,
                   const char* modeText, const Color& color) {
    if (!modeText || modeText[0] == '\0') return;

    const Theme& t = ctx.theme();
    auto& rr = ctx.renderer();

    float tw = rr.measureText(modeText);
    float padding = 12.0f;
    float w = tw + padding * 2.0f;
    float h = 26.0f;
    float x = pos.x - w * 0.5f;
    float y = pos.y;

    Color accentCol = (color.a > 0.01f) ? color : t.accentPrimary;

    // Dark background pill
    rr.drawRoundedRect({x, y, w, h}, t.bgPanel.withAlpha(0.85f), 4.0f);
    rr.drawRoundedRectOutline({x, y, w, h}, accentCol.withAlpha(0.6f), 4.0f);

    // Left accent bar
    rr.drawRect({x, y + 2.0f, 3.0f, h - 4.0f}, accentCol);

    // Text
    float textY = y + (h - 13.0f) * 0.5f;
    rr.drawText(modeText, {x + padding, textY}, accentCol);
}

// ── Info Panel ──────────────────────────────────────────────────────

void infoPanelDraw(AtlasContext& ctx, PanelState& state,
                   const InfoPanelData& data) {
    if (!state.open || data.isEmpty()) return;

    PanelFlags flags;
    flags.showHeader   = true;
    flags.showClose    = true;
    flags.showMinimize = true;
    flags.drawBorder   = true;

    if (!panelBeginStateful(ctx, "Show Info", state, flags)) {
        panelEnd(ctx);
        return;
    }

    const Theme& t = ctx.theme();
    auto& rr = ctx.renderer();
    float hh = t.headerHeight;
    float x = state.bounds.x + t.padding;
    float y = state.bounds.y + hh + 8.0f;
    float contentW = state.bounds.w - t.padding * 2.0f;

    // Entity name
    rr.drawText(data.name, {x, y}, t.accentPrimary);
    y += 18.0f;

    // Type
    char typeBuf[128];
    std::snprintf(typeBuf, sizeof(typeBuf), "Type: %s", data.type.c_str());
    rr.drawText(typeBuf, {x, y}, t.textSecondary);
    y += 16.0f;

    // Faction
    if (!data.faction.empty()) {
        char facBuf[128];
        std::snprintf(facBuf, sizeof(facBuf), "Faction: %s", data.faction.c_str());
        rr.drawText(facBuf, {x, y}, t.textSecondary);
        y += 16.0f;
    }

    // Separator
    rr.drawRect({x, y, contentW, 1.0f}, t.borderSubtle);
    y += 8.0f;

    // Distance
    char distBuf[64];
    if (data.distance < 1000.0f) {
        std::snprintf(distBuf, sizeof(distBuf), "Distance: %.0f m", data.distance);
    } else if (data.distance < 1000000.0f) {
        std::snprintf(distBuf, sizeof(distBuf), "Distance: %.1f km", data.distance / 1000.0f);
    } else {
        std::snprintf(distBuf, sizeof(distBuf), "Distance: %.2f AU", data.distance / METERS_PER_AU);
    }
    rr.drawText(distBuf, {x, y}, t.textPrimary);
    y += 16.0f;

    // Velocity
    if (data.velocity > 0.0f) {
        char velBuf[64];
        std::snprintf(velBuf, sizeof(velBuf), "Velocity: %.0f m/s", data.velocity);
        rr.drawText(velBuf, {x, y}, t.textPrimary);
        y += 16.0f;
    }

    // Signature radius
    if (data.signature > 0.0f) {
        char sigBuf[64];
        std::snprintf(sigBuf, sizeof(sigBuf), "Signature: %.0f m", data.signature);
        rr.drawText(sigBuf, {x, y}, t.textPrimary);
        y += 16.0f;
    }

    // Health bars
    if (data.hasHealth) {
        rr.drawRect({x, y, contentW, 1.0f}, t.borderSubtle);
        y += 6.0f;

        float barW = contentW - 20.0f;
        float barH = 12.0f;
        Color bgBar = t.bgSecondary.withAlpha(0.4f);

        rr.drawText("S", {x, y + 1.0f}, t.textSecondary);
        rr.drawProgressBar({x + 14.0f, y, barW, barH}, data.shieldPct,
                           t.shield, bgBar);
        y += barH + 4.0f;

        rr.drawText("A", {x, y + 1.0f}, t.textSecondary);
        rr.drawProgressBar({x + 14.0f, y, barW, barH}, data.armorPct,
                           t.armor, bgBar);
        y += barH + 4.0f;

        rr.drawText("H", {x, y + 1.0f}, t.textSecondary);
        rr.drawProgressBar({x + 14.0f, y, barW, barH}, data.hullPct,
                           t.hull, bgBar);
        y += barH + 4.0f;
    }

    panelEnd(ctx);
}

// ── Overview Header Interactive ─────────────────────────────────────

int overviewHeaderInteractive(AtlasContext& ctx, const Rect& r,
                              const std::vector<std::string>& tabs,
                              int activeTab) {
    const Theme& t = ctx.theme();
    auto& rr = ctx.renderer();

    int clickedTab = -1;

    // Header background
    rr.drawRect(r, t.bgHeader);

    // Tabs (clickable)
    float tabX = r.x + 4.0f;
    float tabH = r.h - 4.0f;
    for (int i = 0; i < static_cast<int>(tabs.size()); ++i) {
        float tw = rr.measureText(tabs[i]) + 16.0f;
        Rect tabRect = {tabX, r.y + 2.0f, tw, tabH};

        WidgetID tabID = hashID("ovtab") ^ static_cast<uint32_t>(i);
        bool clicked = ctx.buttonBehavior(tabRect, tabID);
        if (clicked) clickedTab = i;

        bool hovered = ctx.isHot(tabID);

        if (i == activeTab) {
            rr.drawRect(tabRect, t.selection);
            rr.drawText(tabs[i], {tabX + 8.0f, r.y + 5.0f}, t.textPrimary);
        } else if (hovered) {
            rr.drawRect(tabRect, t.hover);
            rr.drawText(tabs[i], {tabX + 8.0f, r.y + 5.0f}, t.textPrimary);
        } else {
            rr.drawText(tabs[i], {tabX + 8.0f, r.y + 5.0f}, t.textSecondary);
        }
        tabX += tw + 2.0f;
    }

    // Bottom border
    rr.drawRect({r.x, r.bottom() - 1.0f, r.w, 1.0f}, t.borderSubtle);

    return clickedTab;
}

} // namespace atlas
