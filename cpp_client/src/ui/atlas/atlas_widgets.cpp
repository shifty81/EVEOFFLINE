#include "ui/atlas/atlas_widgets.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static constexpr float METERS_PER_AU = 149597870700.0f;

namespace atlas {

// Helper: truncate a string with "..." suffix if it exceeds maxWidth pixels
static std::string truncateText(AtlasRenderer& rr, const std::string& text, float maxWidth) {
    if (text.empty() || rr.measureText(text) <= maxWidth) return text;
    std::string result = text;
    while (result.size() > 1 && rr.measureText(result + "...") > maxWidth) {
        result.pop_back();
    }
    result += "...";
    return result;
}

// ── Panel ───────────────────────────────────────────────────────────

bool panelBegin(AtlasContext& ctx, const char* title,
                Rect& bounds, const PanelFlags& flags,
                bool* open) {
    const Theme& t = ctx.theme();
    auto& r = ctx.renderer();

    ctx.pushID(title);

    // Consume mouse if clicking within the panel body to prevent click-through
    if (ctx.isHovered(bounds) && ctx.isMouseClicked() && !ctx.isMouseConsumed()) {
        ctx.consumeMouse();
    }

    // Photon-style panel: dark translucent fill with skeletal frame
    r.drawRect(bounds, t.bgPanel);

    // Frame border (thin, skeletal — not a heavy box)
    if (flags.drawBorder) {
        r.drawRectOutline(bounds, t.borderNormal, t.borderWidth);
    }

    // Header bar
    if (flags.showHeader) {
        float hh = flags.compactMode ? t.headerHeight * 0.75f : t.headerHeight;
        Rect headerRect = {bounds.x, bounds.y, bounds.w, hh};
        r.drawRect(headerRect, t.bgHeader);

        // Title text (uppercase for headers per Photon guidelines)
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
                                    : Color(0, 0, 0, 0);
            r.drawRect(closeRect, closeBg);
            r.drawText("x", {closeRect.x + 3.0f, closeRect.y + 1.0f},
                       hovered ? t.textPrimary : t.textSecondary);
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
            Color minBg = hovered ? t.hover : Color(0, 0, 0, 0);
            r.drawRect(minRect, minBg);
            r.drawText("-", {minRect.x + 3.0f, minRect.y + 1.0f},
                       hovered ? t.textPrimary : t.textSecondary);
        }

        // Photon accent line under header (frame separator)
        r.drawRect({bounds.x, bounds.y + hh - 1.0f, bounds.w, 1.0f},
                   t.accentPrimary.withAlpha(0.3f));
    }

    return true;  // panel is open (minimize state not yet tracked)
}

void panelEnd(AtlasContext& ctx) {
    // Consume any leftover mouse clicks within the panel body so they
    // don't fall through to the 3D world or widgets behind the panel.
    Rect bounds = ctx.popPanelBounds();
    if (bounds.w > 0.0f && bounds.h > 0.0f) {
        if (ctx.isHovered(bounds) && ctx.isMouseClicked() && !ctx.isMouseConsumed()) {
            ctx.consumeMouse();
        }
    }
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
                const Color& iconColor, const char* symbol) {
    const Theme& t = ctx.theme();
    auto& rr = ctx.renderer();

    bool clicked = ctx.buttonBehavior(r, id);

    // Neocom-style: subtle background that only shows on hover/active
    bool hovered = ctx.isHot(id);
    bool active  = ctx.isActive(id);

    if (active) {
        rr.drawRect(r, t.selection);
        // Left accent bar when active
        rr.drawRect({r.x, r.y, 2.0f, r.h}, t.accentPrimary);
    } else if (hovered) {
        rr.drawRect(r, t.hover);
        // Left accent bar on hover
        rr.drawRect({r.x, r.y, 2.0f, r.h}, t.accentPrimary.withAlpha(0.6f));
    }

    // Draw letter/symbol icon centered in the slot
    if (symbol && symbol[0]) {
        float tw = rr.measureText(symbol);
        float tx = r.x + (r.w - tw) * 0.5f;
        float ty = r.y + (r.h - 13.0f) * 0.5f;
        Color textCol = hovered ? t.accentPrimary : iconColor;
        rr.drawText(symbol, {tx, ty}, textCol);
    }

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

int speedIndicator(AtlasContext& ctx, Vec2 pos,
                    float currentSpeed, float maxSpeed) {
    const Theme& t = ctx.theme();
    auto& rr = ctx.renderer();

    int result = 0;

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
    if (button(ctx, "-", minus)) result = -1;
    if (button(ctx, "+", plus))  result =  1;

    return result;
}

// ── Warp Progress Indicator ────────────────────────────────────────

void warpProgressIndicator(AtlasContext& ctx, Vec2 pos,
                           int phase, float progress, float speedAU) {
    if (phase <= 0) return;

    const Theme& t = ctx.theme();
    auto& rr = ctx.renderer();

    float barW = 200.0f, barH = 22.0f;
    float totalH = barH + 18.0f;  // bar + text row
    Rect bar = {pos.x - barW * 0.5f, pos.y, barW, barH};

    // Phase-dependent accent colour
    Color phaseColor;
    const char* phaseLabel = "";
    switch (phase) {
        case 1:
            phaseColor = {0.3f, 0.8f, 0.8f, 0.9f};  // teal
            phaseLabel = "ALIGNING";
            break;
        case 2:
            phaseColor = {0.3f, 0.5f, 1.0f, 0.9f};  // blue
            phaseLabel = "ACCELERATING";
            break;
        case 3:
            phaseColor = {0.4f, 0.6f, 1.0f, 0.95f};  // bright blue
            phaseLabel = "WARP";
            break;
        case 4:
            phaseColor = {0.5f, 0.5f, 0.8f, 0.8f};  // fading purple-blue
            phaseLabel = "DECELERATING";
            break;
        default:
            phaseColor = t.accentPrimary;
            phaseLabel = "WARP";
            break;
    }

    // Background
    rr.drawRoundedRect(bar, t.bgPanel.withAlpha(0.85f), 3.0f);
    rr.drawRoundedRectOutline(bar, phaseColor.withAlpha(0.6f), 3.0f);

    // Fill bar based on progress
    float frac = std::max(0.0f, std::min(1.0f, progress));
    if (frac > 0.0f) {
        Rect fill = {bar.x + 2.0f, bar.y + 2.0f,
                     (bar.w - 4.0f) * frac, bar.h - 4.0f};
        rr.drawRoundedRect(fill, phaseColor.withAlpha(0.45f), 2.0f);
    }

    // Phase label (left-aligned inside bar)
    rr.drawText(phaseLabel,
        {bar.x + 8.0f, bar.y + (barH - 13.0f) * 0.5f}, phaseColor);

    // Warp speed readout (right-aligned inside bar)
    if (speedAU > 0.01f) {
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%.1f AU/s", speedAU);
        float tw = rr.measureText(buf);
        rr.drawText(buf,
            {bar.right() - tw - 8.0f, bar.y + (barH - 13.0f) * 0.5f},
            t.textPrimary);
    }

    // Progress percentage below bar
    {
        char buf[16];
        std::snprintf(buf, sizeof(buf), "%.0f%%", progress * 100.0f);
        float tw = rr.measureText(buf);
        rr.drawText(buf,
            {pos.x - tw * 0.5f, bar.bottom() + 2.0f},
            t.textSecondary);
    }
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

    // Photon-style: neutral row background (alternating for scanability)
    Color bg = isAlternate ? t.bgSecondary.withAlpha(0.2f)
                           : Color(0, 0, 0, 0);
    if (ctx.isHot(id)) bg = t.hover;
    rr.drawRect(r, bg);

    // Photon selection: thin 2px accent bar on the left (not row fill)
    if (entry.selected) {
        rr.drawRect({r.x, r.y, t.selectionBarWidth, r.h}, t.accentPrimary);
    }

    // Standing/threat icon indicator (small square — color the icon, not the row)
    float iconEnd = r.x + 16.0f;
    rr.drawRect({r.x + 4.0f, r.y + (r.h - 8.0f) * 0.5f, 8.0f, 8.0f},
                entry.standingColor);

    // Columns: Distance | Name | Type — proportional to row width
    float textY = r.y + (r.h - 13.0f) * 0.5f;
    float usableW = r.w - 16.0f;  // subtract icon area
    float distColW = usableW * 0.28f;
    float nameColW = usableW * 0.40f;
    float typeColW = usableW * 0.32f;

    float distColStart = iconEnd;
    float nameColStart = distColStart + distColW;
    float typeColStart = nameColStart + nameColW;

    // Distance (right-aligned in its column for readability)
    char distBuf[32];
    if (entry.distance >= METERS_PER_AU * 0.01f) {
        std::snprintf(distBuf, sizeof(distBuf), "%.1f AU", entry.distance / METERS_PER_AU);
    } else if (entry.distance >= 1000.0f) {
        std::snprintf(distBuf, sizeof(distBuf), "%.0f km", entry.distance / 1000.0f);
    } else {
        std::snprintf(distBuf, sizeof(distBuf), "%.0f m", entry.distance);
    }
    float distW = rr.measureText(distBuf);
    float distEnd = nameColStart - 4.0f;
    rr.drawText(distBuf, {distEnd - distW, textY}, t.textSecondary);

    // Name — truncate with "..." if it exceeds column width
    float nameMaxW = nameColW - 6.0f;
    std::string displayName = truncateText(rr, entry.name, nameMaxW);
    rr.drawText(displayName, {nameColStart + 2.0f, textY}, t.textPrimary);

    // Type — truncate with "..." if it exceeds column width
    float typeMaxW = typeColW - 4.0f;
    std::string displayType = truncateText(rr, entry.type, typeMaxW);
    rr.drawText(displayType, {typeColStart + 2.0f, textY}, t.textSecondary);

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

// ── Sidebar Icon Shape Helpers ──────────────────────────────────────
// Draw shape-based icons modelled after EVE Online's Neocom Photon UI.
// Each icon uses simple geometric primitives (rect, circle, line, arc)
// to create a distinctive silhouette recognizable at a glance.

static void drawIconInventory(AtlasRenderer& rr, Vec2 c, float sz, const Color& col) {
    // Open box: rectangle body with angled lid
    float hs = sz * 0.38f;
    rr.drawRectOutline({c.x - hs, c.y - hs * 0.6f, hs * 2, hs * 1.6f}, col, 1.5f);
    // Lid top line
    rr.drawLine({c.x - hs * 1.1f, c.y - hs * 0.6f},
                {c.x + hs * 1.1f, c.y - hs * 0.6f}, col, 1.5f);
    // Handle tab at top center
    rr.drawLine({c.x - hs * 0.3f, c.y - hs * 0.6f},
                {c.x - hs * 0.3f, c.y - hs * 0.95f}, col, 1.5f);
    rr.drawLine({c.x + hs * 0.3f, c.y - hs * 0.6f},
                {c.x + hs * 0.3f, c.y - hs * 0.95f}, col, 1.5f);
    rr.drawLine({c.x - hs * 0.3f, c.y - hs * 0.95f},
                {c.x + hs * 0.3f, c.y - hs * 0.95f}, col, 1.5f);
}

static void drawIconFitting(AtlasRenderer& rr, Vec2 c, float sz, const Color& col) {
    // Wrench: diagonal tool shape
    float hs = sz * 0.38f;
    // Shaft (diagonal line)
    rr.drawLine({c.x - hs * 0.7f, c.y + hs * 0.7f},
                {c.x + hs * 0.4f, c.y - hs * 0.4f}, col, 2.0f);
    // Jaw (U shape at top-right end)
    rr.drawArc(c + Vec2(hs * 0.5f, -hs * 0.5f),
               hs * 0.2f, hs * 0.45f,
               -0.5f, 2.1f, col, 6);
    // Handle end (small rect at bottom-left)
    rr.drawCircle(c + Vec2(-hs * 0.7f, hs * 0.7f), hs * 0.15f, col, 6);
}

static void drawIconMarket(AtlasRenderer& rr, Vec2 c, float sz, const Color& col) {
    // Bar chart: three vertical bars ascending left-to-right
    float hs = sz * 0.35f;
    float bw = hs * 0.45f;
    float gap = hs * 0.15f;
    float baseY = c.y + hs;
    // Bar 1 (short)
    float x0 = c.x - hs;
    rr.drawRect({x0, baseY - hs * 0.8f, bw, hs * 0.8f}, col.withAlpha(0.7f));
    // Bar 2 (medium)
    float x1 = x0 + bw + gap;
    rr.drawRect({x1, baseY - hs * 1.3f, bw, hs * 1.3f}, col.withAlpha(0.85f));
    // Bar 3 (tall)
    float x2 = x1 + bw + gap;
    rr.drawRect({x2, baseY - hs * 1.8f, bw, hs * 1.8f}, col);
    // Baseline
    rr.drawLine({c.x - hs * 1.1f, baseY}, {c.x + hs * 1.1f, baseY}, col.withAlpha(0.5f), 1.0f);
}

static void drawIconMission(AtlasRenderer& rr, Vec2 c, float sz, const Color& col) {
    // Journal/document: rectangle with horizontal text lines
    float hs = sz * 0.35f;
    Rect doc = {c.x - hs, c.y - hs * 1.1f, hs * 2, hs * 2.2f};
    rr.drawRectOutline(doc, col, 1.5f);
    // Text lines inside
    float lineX = doc.x + hs * 0.3f;
    float lineW = hs * 1.4f;
    for (int i = 0; i < 4; ++i) {
        float ly = doc.y + hs * 0.4f + i * hs * 0.45f;
        rr.drawLine({lineX, ly}, {lineX + lineW * (i == 3 ? 0.6f : 1.0f), ly},
                    col.withAlpha(0.6f), 1.0f);
    }
}

static void drawIconDScan(AtlasRenderer& rr, Vec2 c, float sz, const Color& col) {
    // Radar sweep: circle outline with a sweep wedge
    float r = sz * 0.38f;
    rr.drawCircleOutline(c, r, col.withAlpha(0.5f), 1.0f, 16);
    // Sweep arc (bright wedge)
    rr.drawArc(c, 0.0f, r, -0.3f, 0.8f, col.withAlpha(0.7f), 8);
    // Centre dot
    rr.drawCircle(c, r * 0.12f, col, 6);
    // Cross-hairs
    rr.drawLine({c.x, c.y - r * 0.5f}, {c.x, c.y + r * 0.5f}, col.withAlpha(0.3f), 1.0f);
    rr.drawLine({c.x - r * 0.5f, c.y}, {c.x + r * 0.5f, c.y}, col.withAlpha(0.3f), 1.0f);
}

static void drawIconOverview(AtlasRenderer& rr, Vec2 c, float sz, const Color& col) {
    // Table/list: horizontal lines in a frame
    float hs = sz * 0.35f;
    Rect frame = {c.x - hs, c.y - hs * 0.9f, hs * 2, hs * 1.8f};
    rr.drawRectOutline(frame, col.withAlpha(0.6f), 1.0f);
    // Header line
    rr.drawRect({frame.x, frame.y, frame.w, hs * 0.35f}, col.withAlpha(0.3f));
    // Data rows
    for (int i = 0; i < 3; ++i) {
        float ly = frame.y + hs * 0.5f + i * hs * 0.45f;
        rr.drawLine({frame.x + 2, ly}, {frame.right() - 2, ly},
                    col.withAlpha(0.4f), 1.0f);
    }
}

static void drawIconChat(AtlasRenderer& rr, Vec2 c, float sz, const Color& col) {
    // Speech bubble: rounded rect with a tail
    float hs = sz * 0.35f;
    Rect bubble = {c.x - hs, c.y - hs * 0.8f, hs * 2, hs * 1.3f};
    rr.drawRectOutline(bubble, col, 1.5f);
    // Tail (triangle pointing down-left)
    float tx = bubble.x + hs * 0.4f;
    float ty = bubble.bottom();
    rr.drawLine({tx, ty}, {tx - hs * 0.3f, ty + hs * 0.5f}, col, 1.5f);
    rr.drawLine({tx - hs * 0.3f, ty + hs * 0.5f}, {tx + hs * 0.3f, ty}, col, 1.5f);
    // Dots (ellipsis inside bubble)
    float dotY = bubble.center().y;
    for (int i = 0; i < 3; ++i) {
        float dx = c.x - hs * 0.4f + i * hs * 0.4f;
        rr.drawCircle({dx, dotY}, 1.5f, col.withAlpha(0.7f), 4);
    }
}

static void drawIconDrone(AtlasRenderer& rr, Vec2 c, float sz, const Color& col) {
    // Drone: disc body with symmetric wing struts
    float r = sz * 0.2f;
    rr.drawCircle(c, r, col.withAlpha(0.6f), 8);
    rr.drawCircleOutline(c, r, col, 1.0f, 8);
    // Wing struts (4 arms extending outward)
    float armLen = sz * 0.35f;
    float angles[] = {0.6f, 2.5f, 3.7f, 5.6f};
    for (float a : angles) {
        Vec2 tip = c + Vec2(std::cos(a) * armLen, std::sin(a) * armLen);
        rr.drawLine(c + Vec2(std::cos(a) * r, std::sin(a) * r), tip, col, 1.5f);
        rr.drawCircle(tip, r * 0.35f, col.withAlpha(0.5f), 4);
    }
}

// Dispatch: draw the shape icon for a given sidebar slot index.
static void drawSidebarIconShape(AtlasRenderer& rr, int index,
                                  Vec2 centre, float slotSz,
                                  const Color& col) {
    switch (index) {
        case 0: drawIconInventory(rr, centre, slotSz, col); break;
        case 1: drawIconFitting(rr, centre, slotSz, col);   break;
        case 2: drawIconMarket(rr, centre, slotSz, col);    break;
        case 3: drawIconMission(rr, centre, slotSz, col);   break;
        case 4: drawIconDScan(rr, centre, slotSz, col);     break;
        case 5: drawIconOverview(rr, centre, slotSz, col);  break;
        case 6: drawIconChat(rr, centre, slotSz, col);      break;
        case 7: drawIconDrone(rr, centre, slotSz, col);     break;
        default: {
            // Fallback: draw "?" letter
            float tw = rr.measureText("?");
            rr.drawText("?", {centre.x - tw * 0.5f, centre.y - 6.5f}, col);
            break;
        }
    }
}

// ── Sidebar Bar ─────────────────────────────────────────────────────

void sidebarBar(AtlasContext& ctx, float x, float width, float height,
               int icons, const std::function<void(int)>& callback,
               const bool* activeIcons, float skillQueuePct) {
    const Theme& t = ctx.theme();
    auto& rr = ctx.renderer();

    // ── Background: dark semi-transparent panel ─────────────────────
    Rect bar = {x, 0, width, height};
    rr.drawRect(bar, t.bgPanel.withAlpha(0.92f));
    // Right edge border (thin skeletal frame)
    rr.drawRect({x + width - 1.0f, 0, 1.0f, height}, t.borderSubtle);

    float iconY = 4.0f;
    float slotSz = width - 6.0f;
    float iconGap = 2.0f;
    float pad = 3.0f;

    // ── "A" Menu Button (top of sidebar) ────────────────────────────
    {
        Rect eRect = {x + pad, iconY, slotSz, slotSz};
        WidgetID eID = hashID("sidebar_emenu");

        bool hovered = ctx.isHovered(eRect);
        if (hovered) ctx.setHot(eID);
        bool active = ctx.isActive(eID);

        // Accent-coloured background for the A button
        Color eBg = active  ? t.accentPrimary.withAlpha(0.5f) :
                    hovered ? t.accentPrimary.withAlpha(0.3f) :
                              t.accentPrimary.withAlpha(0.12f);
        rr.drawRect(eRect, eBg);
        rr.drawRectOutline(eRect, t.accentPrimary.withAlpha(0.4f));

        // "A" letter centered (Atlas menu icon)
        float tw = rr.measureText("A");
        float tx = eRect.x + (eRect.w - tw) * 0.5f;
        float ty = eRect.y + (eRect.h - 13.0f) * 0.5f;
        rr.drawText("A", {tx, ty}, t.accentPrimary);

        // Click behavior (currently informational — master menu)
        ctx.buttonBehavior(eRect, eID);
        if (ctx.isHot(eID)) {
            tooltip(ctx, "Atlas Menu");
        }

        iconY += slotSz + iconGap;
    }

    // ── Character Portrait Area ─────────────────────────────────────
    {
        float portraitSz = slotSz;
        Rect portraitRect = {x + pad, iconY, portraitSz, portraitSz};

        // Dark recessed area for portrait
        rr.drawRect(portraitRect, t.bgSecondary.withAlpha(0.8f));
        rr.drawRectOutline(portraitRect, t.borderSubtle);

        // Placeholder silhouette (draw a simple head/shoulders shape)
        Vec2 pCentre = portraitRect.center();
        float headR = portraitSz * 0.2f;
        rr.drawCircle(pCentre + Vec2(0, -headR * 0.5f), headR,
                       t.textMuted.withAlpha(0.4f), 12);
        // Shoulders arc
        rr.drawArc(pCentre + Vec2(0, headR * 0.8f),
                    headR * 0.8f, headR * 1.6f,
                    static_cast<float>(M_PI), 2.0f * static_cast<float>(M_PI),
                    t.textMuted.withAlpha(0.3f), 8);

        if (ctx.isHovered(portraitRect)) {
            tooltip(ctx, "Character Sheet");
        }

        iconY += portraitSz + 2.0f;
    }

    // ── Skill Queue Progress Bar ────────────────────────────────────
    {
        float barH = 3.0f;
        Rect sqRect = {x + pad, iconY, slotSz, barH};
        rr.drawRect(sqRect, t.bgSecondary.withAlpha(0.6f));
        float fill = std::max(0.0f, std::min(1.0f, skillQueuePct));
        if (fill > 0.0f) {
            rr.drawRect({sqRect.x, sqRect.y, sqRect.w * fill, barH},
                         t.accentSecondary.withAlpha(0.8f));
        }
        rr.drawRectOutline(sqRect, t.borderSubtle.withAlpha(0.4f), 1.0f);

        if (ctx.isHovered(sqRect)) {
            tooltip(ctx, "Skill Queue");
        }

        iconY += barH + 6.0f;
    }

    // ── Separator ───────────────────────────────────────────────────
    rr.drawRect({x + 6.0f, iconY, width - 12.0f, 1.0f},
                t.borderSubtle.withAlpha(0.6f));
    iconY += 5.0f;

    // ── Service icon definitions ────────────────────────────────────
    static const char* sidebarTooltips[] = {
        "Inventory",
        "Ship Fitting",
        "Market",
        "Missions",
        "Directional Scan",
        "Overview",
        "Chat",
        "Drones",
    };
    // Per-icon accent colours (EVE lets users colour-code icons)
    static const Color iconAccentColors[] = {
        Color(0.35f, 0.75f, 0.45f, 1.0f),  // Inventory: green
        Color(0.85f, 0.50f, 0.25f, 1.0f),  // Fitting: orange
        Color(0.40f, 0.65f, 0.90f, 1.0f),  // Market: blue
        Color(0.80f, 0.75f, 0.30f, 1.0f),  // Missions: gold
        Color(0.60f, 0.80f, 0.90f, 1.0f),  // D-Scan: light cyan
        Color(0.50f, 0.70f, 0.85f, 1.0f),  // Overview: steel blue
        Color(0.65f, 0.55f, 0.85f, 1.0f),  // Chat: purple
        Color(0.45f, 0.80f, 0.65f, 1.0f),  // Drones: teal
    };

    // ── Top group: Inventory & Fitting (icons 0–1) ──────────────────
    int topGroup = std::min(icons, 2);
    for (int i = 0; i < topGroup; ++i) {
        Rect iconRect = {x + pad, iconY, slotSz, slotSz};
        WidgetID iconID = hashID("sidebar") ^ static_cast<uint32_t>(i);

        bool isOpen = activeIcons ? activeIcons[i] : false;
        bool hovered = ctx.isHovered(iconRect);
        if (hovered) ctx.setHot(iconID);
        bool pressing = ctx.isActive(iconID);

        // Background: show persistent highlight when panel is open
        Color bg(0, 0, 0, 0);
        Color accentCol = (i < 8) ? iconAccentColors[i] : t.accentPrimary;
        if (pressing) {
            bg = accentCol.withAlpha(0.35f);
        } else if (isOpen) {
            bg = accentCol.withAlpha(0.18f);
        } else if (hovered) {
            bg = t.hover;
        }
        rr.drawRect(iconRect, bg);

        // Left accent bar: bright when open, subtle on hover
        if (isOpen) {
            rr.drawRect({iconRect.x, iconRect.y, 2.0f, iconRect.h},
                         accentCol);
        } else if (hovered) {
            rr.drawRect({iconRect.x, iconRect.y, 2.0f, iconRect.h},
                         accentCol.withAlpha(0.5f));
        }

        // Shape-based icon (EVE Photon style)
        Color iconCol = isOpen ? accentCol : (hovered ? t.textPrimary : t.textSecondary);
        drawSidebarIconShape(rr, i, iconRect.center(), slotSz, iconCol);

        if (ctx.buttonBehavior(iconRect, iconID)) {
            if (callback) callback(i);
        }

        if (ctx.isHot(iconID) && i < 8) {
            tooltip(ctx, sidebarTooltips[i]);
        }

        iconY += slotSz + iconGap;
    }

    // ── Separator ───────────────────────────────────────────────────
    if (topGroup > 0 && icons > topGroup) {
        iconY += 2.0f;
        rr.drawRect({x + 6.0f, iconY, width - 12.0f, 1.0f},
                    t.borderSubtle.withAlpha(0.6f));
        iconY += 5.0f;
    }

    // ── Middle group: Market, Missions, D-Scan, Overview (icons 2–5) ─
    int midGroup = std::min(icons, 6);
    for (int i = topGroup; i < midGroup; ++i) {
        Rect iconRect = {x + pad, iconY, slotSz, slotSz};
        WidgetID iconID = hashID("sidebar") ^ static_cast<uint32_t>(i);

        bool isOpen = activeIcons ? activeIcons[i] : false;
        bool hovered = ctx.isHovered(iconRect);
        if (hovered) ctx.setHot(iconID);
        bool pressing = ctx.isActive(iconID);

        Color accentCol = (i < 8) ? iconAccentColors[i] : t.accentPrimary;
        Color bg(0, 0, 0, 0);
        if (pressing) {
            bg = accentCol.withAlpha(0.35f);
        } else if (isOpen) {
            bg = accentCol.withAlpha(0.18f);
        } else if (hovered) {
            bg = t.hover;
        }
        rr.drawRect(iconRect, bg);

        if (isOpen) {
            rr.drawRect({iconRect.x, iconRect.y, 2.0f, iconRect.h}, accentCol);
        } else if (hovered) {
            rr.drawRect({iconRect.x, iconRect.y, 2.0f, iconRect.h},
                         accentCol.withAlpha(0.5f));
        }

        // Shape-based icon (EVE Photon style)
        Color iconCol = isOpen ? accentCol : (hovered ? t.textPrimary : t.textSecondary);
        drawSidebarIconShape(rr, i, iconRect.center(), slotSz, iconCol);

        if (ctx.buttonBehavior(iconRect, iconID)) {
            if (callback) callback(i);
        }

        if (ctx.isHot(iconID) && i < 8) {
            tooltip(ctx, sidebarTooltips[i]);
        }

        iconY += slotSz + iconGap;
    }

    // ── Separator before bottom group ───────────────────────────────
    if (midGroup > topGroup && icons > midGroup) {
        iconY += 2.0f;
        rr.drawRect({x + 6.0f, iconY, width - 12.0f, 1.0f},
                    t.borderSubtle.withAlpha(0.6f));
        iconY += 5.0f;
    }

    // ── Bottom group: Chat, Drones (icons 6+) ───────────────────────
    for (int i = midGroup; i < icons; ++i) {
        Rect iconRect = {x + pad, iconY, slotSz, slotSz};
        WidgetID iconID = hashID("sidebar") ^ static_cast<uint32_t>(i);

        bool isOpen = activeIcons ? activeIcons[i] : false;
        bool hovered = ctx.isHovered(iconRect);
        if (hovered) ctx.setHot(iconID);
        bool pressing = ctx.isActive(iconID);

        Color accentCol = (i < 8) ? iconAccentColors[i] : t.accentPrimary;
        Color bg(0, 0, 0, 0);
        if (pressing) {
            bg = accentCol.withAlpha(0.35f);
        } else if (isOpen) {
            bg = accentCol.withAlpha(0.18f);
        } else if (hovered) {
            bg = t.hover;
        }
        rr.drawRect(iconRect, bg);

        if (isOpen) {
            rr.drawRect({iconRect.x, iconRect.y, 2.0f, iconRect.h}, accentCol);
        } else if (hovered) {
            rr.drawRect({iconRect.x, iconRect.y, 2.0f, iconRect.h},
                         accentCol.withAlpha(0.5f));
        }

        // Shape-based icon (EVE Photon style)
        Color iconCol = isOpen ? accentCol : (hovered ? t.textPrimary : t.textSecondary);
        drawSidebarIconShape(rr, i, iconRect.center(), slotSz, iconCol);

        if (ctx.buttonBehavior(iconRect, iconID)) {
            if (callback) callback(i);
        }

        if (ctx.isHot(iconID) && i < 8) {
            tooltip(ctx, sidebarTooltips[i]);
        }

        iconY += slotSz + iconGap;
    }

    // Consume mouse clicks within the sidebar AFTER icon processing,
    // to prevent click-through to the 3D world while still allowing
    // icon buttons to receive their clicks first
    if (ctx.isHovered(bar) && ctx.isMouseClicked() && !ctx.isMouseConsumed()) {
        ctx.consumeMouse();
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

    // Effective locked state: PanelFlags OR PanelState lock
    bool effectivelyLocked = flags.locked || state.locked;

    // Pre-compute header button rects so the drag handler can exclude them
    float hh = flags.compactMode ? t.headerHeight * 0.75f : t.headerHeight;
    float btnSz = 14.0f;

    // Right-side buttons: [gear] [minimize] [close]  (right to left)
    Rect closeRect = {state.bounds.right() - btnSz - 4.0f,
                      state.bounds.y + (hh - btnSz) * 0.5f,
                      btnSz, btnSz};
    float minOffset = flags.showClose ? 22.0f : 4.0f;
    Rect minRect = {state.bounds.right() - btnSz - minOffset,
                    state.bounds.y + (hh - btnSz) * 0.5f,
                    btnSz, btnSz};
    float gearOffset = minOffset + (flags.showMinimize ? 18.0f : 0.0f);
    Rect gearRect = {state.bounds.right() - btnSz - gearOffset,
                     state.bounds.y + (hh - btnSz) * 0.5f,
                     btnSz, btnSz};

    // Left-side button: [lock] just after title
    float titleW = rr.measureText(title);
    Rect lockRect = {state.bounds.x + t.padding + titleW + 6.0f,
                     state.bounds.y + (hh - btnSz) * 0.5f,
                     btnSz, btnSz};

    // Process close/minimize/lock/gear buttons BEFORE drag, so they get priority
    bool closeBtnClicked = false;
    bool minBtnClicked = false;
    bool lockBtnClicked = false;
    bool gearBtnClicked = false;
    if (flags.showHeader && !ctx.isMouseConsumed()) {
        if (flags.showClose) {
            WidgetID closeID = ctx.currentID("_close");
            closeBtnClicked = ctx.buttonBehavior(closeRect, closeID);
            if (closeBtnClicked) {
                state.open = false;
                ctx.consumeMouse();
            }
        }
        if (flags.showMinimize && !ctx.isMouseConsumed()) {
            WidgetID minID = ctx.currentID("_min");
            minBtnClicked = ctx.buttonBehavior(minRect, minID);
            if (minBtnClicked) {
                state.minimized = !state.minimized;
                ctx.consumeMouse();
            }
        }
        // Lock button
        if (!ctx.isMouseConsumed()) {
            WidgetID lockID = ctx.currentID("_lock");
            lockBtnClicked = ctx.buttonBehavior(lockRect, lockID);
            if (lockBtnClicked) {
                state.locked = !state.locked;
                ctx.consumeMouse();
            }
        }
        // Gear/settings button
        if (!ctx.isMouseConsumed()) {
            WidgetID gearID = ctx.currentID("_gear");
            gearBtnClicked = ctx.buttonBehavior(gearRect, gearID);
            if (gearBtnClicked) {
                state.settingsOpen = !state.settingsOpen;
                ctx.consumeMouse();
            }
        }
    }

    // Handle dragging (header-bar drag to move) — skip if locked or a button was clicked
    bool anyBtnClicked = closeBtnClicked || minBtnClicked || lockBtnClicked || gearBtnClicked;
    if (!effectivelyLocked && flags.showHeader && !anyBtnClicked) {
        Rect headerHit = {state.bounds.x, state.bounds.y, state.bounds.w, hh};
        WidgetID dragID = ctx.currentID("_drag");

        if (!ctx.isMouseConsumed() && ctx.isHovered(headerHit) && ctx.isMouseClicked()) {
            // Don't start drag if mouse is over a header button
            Vec2 mp = ctx.input().mousePos;
            bool overButton = (flags.showClose && closeRect.contains(mp)) ||
                              (flags.showMinimize && minRect.contains(mp)) ||
                              lockRect.contains(mp) || gearRect.contains(mp);
            if (!overButton) {
                state.dragging = true;
                state.dragOffset = {mp.x - state.bounds.x, mp.y - state.bounds.y};
                ctx.setActive(dragID);
                ctx.consumeMouse();
            }
        }

        if (state.dragging) {
            ctx.consumeMouse();
            if (ctx.isMouseDown()) {
                state.bounds.x = ctx.input().mousePos.x - state.dragOffset.x;
                state.bounds.y = ctx.input().mousePos.y - state.dragOffset.y;

                // Clamp to window bounds (sidebar is the left border)
                float winW = static_cast<float>(ctx.input().windowW);
                float winH = static_cast<float>(ctx.input().windowH);
                float leftEdge = ctx.sidebarWidth();
                state.bounds.x = std::max(leftEdge, std::min(state.bounds.x, winW - state.bounds.w));
                state.bounds.y = std::max(0.0f, std::min(state.bounds.y, winH - t.headerHeight));

                // ── EVE-style edge magnetism / snapping ────────────────
                // Panels snap to screen edges and the Neocom sidebar
                // when within the snap threshold (Photon UI principle).
                constexpr float snapThreshold = 15.0f;

                // Snap left edge to sidebar
                if (std::fabs(state.bounds.x - leftEdge) < snapThreshold)
                    state.bounds.x = leftEdge;
                // Snap right edge to window right
                if (std::fabs(state.bounds.right() - winW) < snapThreshold)
                    state.bounds.x = winW - state.bounds.w;
                // Snap top edge to window top
                if (std::fabs(state.bounds.y) < snapThreshold)
                    state.bounds.y = 0.0f;
                // Snap bottom edge to window bottom
                if (std::fabs(state.bounds.bottom() - winH) < snapThreshold)
                    state.bounds.y = winH - state.bounds.h;
            } else {
                state.dragging = false;
                ctx.clearActive();
            }
        }
    }

    // Handle resizing (edge/corner drag) — skip if locked or minimized
    if (!effectivelyLocked && !state.minimized && !anyBtnClicked && !state.dragging) {
        constexpr float edgeGrab = 6.0f;  // grab zone in pixels
        const Rect& b = state.bounds;

        // Edge hit-test rects
        Rect leftEdge   = {b.x - edgeGrab, b.y + edgeGrab, edgeGrab * 2, b.h - edgeGrab * 2};
        Rect rightEdge  = {b.right() - edgeGrab, b.y + edgeGrab, edgeGrab * 2, b.h - edgeGrab * 2};
        Rect topEdge    = {b.x + edgeGrab, b.y - edgeGrab, b.w - edgeGrab * 2, edgeGrab * 2};
        Rect bottomEdge = {b.x + edgeGrab, b.bottom() - edgeGrab, b.w - edgeGrab * 2, edgeGrab * 2};

        // Corner hit-test rects
        Rect topLeft     = {b.x - edgeGrab, b.y - edgeGrab, edgeGrab * 2, edgeGrab * 2};
        Rect topRight    = {b.right() - edgeGrab, b.y - edgeGrab, edgeGrab * 2, edgeGrab * 2};
        Rect bottomLeft  = {b.x - edgeGrab, b.bottom() - edgeGrab, edgeGrab * 2, edgeGrab * 2};
        Rect bottomRight = {b.right() - edgeGrab, b.bottom() - edgeGrab, edgeGrab * 2, edgeGrab * 2};

        if (!ctx.isMouseConsumed() && ctx.isMouseClicked() && !state.resizing) {
            int edge = 0;
            Vec2 mp = ctx.input().mousePos;
            if (bottomRight.contains(mp))      edge = 2 | 8;
            else if (bottomLeft.contains(mp))  edge = 1 | 8;
            else if (topRight.contains(mp))    edge = 2 | 4;
            else if (topLeft.contains(mp))     edge = 1 | 4;
            else if (leftEdge.contains(mp))    edge = 1;
            else if (rightEdge.contains(mp))   edge = 2;
            else if (topEdge.contains(mp))     edge = 4;
            else if (bottomEdge.contains(mp))  edge = 8;

            if (edge != 0) {
                state.resizing = true;
                state.resizeEdge = edge;
                state.resizeAnchor = mp;
                state.resizeOrigBounds = state.bounds;
                ctx.consumeMouse();
            }
        }

        if (state.resizing) {
            ctx.consumeMouse();
            if (ctx.isMouseDown()) {
                Vec2 delta = ctx.input().mousePos - state.resizeAnchor;
                Rect nb = state.resizeOrigBounds;

                if (state.resizeEdge & 1) { // left
                    float newX = nb.x + delta.x;
                    float newW = nb.w - delta.x;
                    float leftEdge = ctx.sidebarWidth();
                    // Clamp left edge to sidebar boundary
                    if (newX < leftEdge) {
                        newW -= (leftEdge - newX);
                        newX = leftEdge;
                    }
                    if (newW >= state.minW) { state.bounds.x = newX; state.bounds.w = newW; }
                }
                if (state.resizeEdge & 2) { // right
                    float newW = nb.w + delta.x;
                    if (newW >= state.minW) { state.bounds.w = newW; }
                }
                if (state.resizeEdge & 4) { // top
                    float newY = nb.y + delta.y;
                    float newH = nb.h - delta.y;
                    if (newH >= state.minH) { state.bounds.y = newY; state.bounds.h = newH; }
                }
                if (state.resizeEdge & 8) { // bottom
                    float newH = nb.h + delta.y;
                    if (newH >= state.minH) { state.bounds.h = newH; }
                }
            } else {
                state.resizing = false;
                state.resizeEdge = 0;
            }
        }
    }

    // Push panel bounds so panelEnd can consume leftover clicks
    // (deferred to let content widgets process clicks first)
    ctx.pushPanelBounds(state.bounds);

    // Draw panel background (apply opacity)
    Rect drawBounds = state.bounds;
    if (state.minimized) {
        drawBounds.h = hh;
    }

    Color panelBg = t.bgPanel.withAlpha(t.bgPanel.a * state.opacity);
    rr.drawRect(drawBounds, panelBg);

    // Border (Photon skeletal frame)
    if (flags.drawBorder) {
        rr.drawRectOutline(drawBounds, t.borderNormal, t.borderWidth);
    }

    // Resize grip indicator (bottom-right corner dots when not locked)
    if (!effectivelyLocked && !state.minimized) {
        float gx = drawBounds.right() - 10.0f;
        float gy = drawBounds.bottom() - 10.0f;
        Color gripColor = t.textMuted.withAlpha(0.4f);
        rr.drawRect({gx + 4.0f, gy + 4.0f, 3.0f, 3.0f}, gripColor);
        rr.drawRect({gx,        gy + 4.0f, 3.0f, 3.0f}, gripColor);
        rr.drawRect({gx + 4.0f, gy,        3.0f, 3.0f}, gripColor);
    }

    // Header bar
    if (flags.showHeader) {
        Rect headerRect = {drawBounds.x, drawBounds.y, drawBounds.w, hh};
        rr.drawRect(headerRect, t.bgHeader.withAlpha(t.bgHeader.a * state.opacity));

        float textY = drawBounds.y + (hh - 13.0f) * 0.5f;
        rr.drawText(title, {drawBounds.x + t.padding, textY}, t.textPrimary);

        // Lock button (left side, after title) — padlock icon
        {
            WidgetID lockID = ctx.currentID("_lock");
            bool hovered = ctx.isHovered(lockRect);
            if (hovered) ctx.setHot(lockID);
            Color lockBg = hovered ? t.hover : Color(0, 0, 0, 0);
            rr.drawRect(lockRect, lockBg);
            const char* lockIcon = state.locked ? "L" : "U";
            Color lockColor = state.locked ? t.accentPrimary : t.textMuted;
            if (hovered) lockColor = t.textPrimary;
            rr.drawText(lockIcon, {lockRect.x + 2.0f, lockRect.y + 1.0f}, lockColor);
        }

        // Close button (×) — visual only, behavior handled above
        if (flags.showClose) {
            WidgetID closeID = ctx.currentID("_close");
            bool hovered = ctx.isHovered(closeRect);
            if (hovered) ctx.setHot(closeID);
            Color closeBg = hovered ? t.danger.withAlpha(0.6f) : Color(0, 0, 0, 0);
            rr.drawRect(closeRect, closeBg);
            rr.drawText("x", {closeRect.x + 3.0f, closeRect.y + 1.0f},
                        hovered ? t.textPrimary : t.textSecondary);
        }

        // Minimize button (—) — visual only, behavior handled above
        if (flags.showMinimize) {
            WidgetID minID = ctx.currentID("_min");
            bool hovered = ctx.isHovered(minRect);
            if (hovered) ctx.setHot(minID);
            Color minBg = hovered ? t.hover : Color(0, 0, 0, 0);
            rr.drawRect(minRect, minBg);
            rr.drawText("-", {minRect.x + 3.0f, minRect.y + 1.0f},
                        hovered ? t.textPrimary : t.textSecondary);
        }

        // Gear/settings button — visual only, behavior handled above
        {
            WidgetID gearID = ctx.currentID("_gear");
            bool hovered = ctx.isHovered(gearRect);
            if (hovered) ctx.setHot(gearID);
            Color gearBg = (hovered || state.settingsOpen) ? t.hover : Color(0, 0, 0, 0);
            rr.drawRect(gearRect, gearBg);
            Color gearColor = state.settingsOpen ? t.accentPrimary : (hovered ? t.textPrimary : t.textSecondary);
            rr.drawText("*", {gearRect.x + 3.0f, gearRect.y + 1.0f}, gearColor);
        }

        // Photon accent line under header
        rr.drawRect({drawBounds.x, drawBounds.y + hh - 1.0f, drawBounds.w, 1.0f},
                    t.accentPrimary.withAlpha(0.3f));
    }

    // Settings dropdown popup (below gear button)
    if (state.settingsOpen && flags.showHeader) {
        float popW = 180.0f;
        float popH = 72.0f;
        float popX = std::min(gearRect.x, static_cast<float>(ctx.input().windowW) - popW - 4.0f);
        float popY = state.bounds.y + hh + 2.0f;

        Rect popBg(popX, popY, popW, popH);
        rr.drawRect(popBg, t.bgPanel);
        rr.drawRectOutline(popBg, t.borderNormal);

        // Consume mouse within popup
        if (ctx.isHovered(popBg) && ctx.isMouseClicked() && !ctx.isMouseConsumed()) {
            ctx.consumeMouse();
        }

        float py = popY + 4.0f;

        // Opacity slider — use 20–100 range for display; convert back to 0.0–1.0
        rr.drawText("Opacity", {popX + 6.0f, py + 1.0f}, t.textSecondary);
        py += 16.0f;
        Rect opacitySlider(popX + 6.0f, py, popW - 12.0f, 14.0f);
        float opacityPct = state.opacity * 100.0f;
        if (slider(ctx, "_opacity", opacitySlider, &opacityPct, 20.0f, 100.0f, "%.0f%%")) {
            state.opacity = opacityPct / 100.0f;
        }
        py += 22.0f;

        // Compact rows toggle
        Rect compactRect(popX + 6.0f, py, popW - 12.0f, 14.0f);
        checkbox(ctx, "Compact rows", compactRect, &state.compactRows);
    }

    // Close settings popup when clicking outside it
    if (state.settingsOpen && ctx.isMouseClicked()) {
        float popW = 180.0f;
        float popH = 72.0f;
        float popX = std::min(gearRect.x, static_cast<float>(ctx.input().windowW) - popW - 4.0f);
        float popY = state.bounds.y + hh + 2.0f;
        Rect popBg(popX, popY, popW, popH);
        if (!ctx.isHovered(popBg) && !ctx.isHovered(gearRect)) {
            state.settingsOpen = false;
        }
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

    // Photon-style tabs: text-only with accent underline on active
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
            // Active tab: bright text + accent underline indicator
            rr.drawText(tabs[i], {tabX + 8.0f, r.y + 5.0f}, t.textPrimary);
            rr.drawRect({tabX + 4.0f, tabRect.bottom() - 2.0f, tw - 8.0f, 2.0f},
                        t.accentPrimary);
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


// ── Tab Bar ─────────────────────────────────────────────────────────

int tabBar(AtlasContext& ctx, const Rect& r,
           const std::vector<std::string>& labels, int activeIdx) {
    const Theme& t = ctx.theme();
    auto& rr = ctx.renderer();

    int clicked = -1;

    // Background
    rr.drawRect(r, t.bgHeader);

    float tabX = r.x + 4.0f;
    float tabH = r.h - 4.0f;
    for (int i = 0; i < static_cast<int>(labels.size()); ++i) {
        float tw = rr.measureText(labels[i]) + 16.0f;
        Rect tabRect = {tabX, r.y + 2.0f, tw, tabH};

        WidgetID tabID = hashID("tab") ^ static_cast<uint32_t>(i);
        if (ctx.buttonBehavior(tabRect, tabID)) clicked = i;

        bool hovered = ctx.isHot(tabID);

        if (i == activeIdx) {
            rr.drawText(labels[i], {tabX + 8.0f, r.y + 5.0f}, t.textPrimary);
            rr.drawRect({tabX + 4.0f, tabRect.bottom() - 2.0f, tw - 8.0f, 2.0f},
                        t.accentPrimary);
        } else if (hovered) {
            rr.drawRect(tabRect, t.hover);
            rr.drawText(labels[i], {tabX + 8.0f, r.y + 5.0f}, t.textPrimary);
        } else {
            rr.drawText(labels[i], {tabX + 8.0f, r.y + 5.0f}, t.textSecondary);
        }
        tabX += tw + 2.0f;
    }

    rr.drawRect({r.x, r.bottom() - 1.0f, r.w, 1.0f}, t.borderSubtle);
    return clicked;
}

// ── Combat Log Widget ───────────────────────────────────────────────

void combatLogWidget(AtlasContext& ctx, const Rect& r,
                     const std::vector<std::string>& messages,
                     float& scrollOff, int maxVisible) {
    const Theme& t = ctx.theme();
    auto& rr = ctx.renderer();

    // Panel background
    rr.drawRect(r, t.bgPanel);
    rr.drawRectOutline(r, t.borderSubtle, t.borderWidth);

    // Header
    float hh = 18.0f;
    rr.drawRect({r.x, r.y, r.w, hh}, t.bgHeader);
    rr.drawText("Combat Log", {r.x + 6.0f, r.y + 3.0f}, t.textSecondary);

    // Content area
    float contentY = r.y + hh + 2.0f;
    float contentH = r.h - hh - 2.0f;
    float rowH = 16.0f;
    int visRows = maxVisible > 0 ? maxVisible : static_cast<int>(contentH / rowH);
    if (visRows <= 0) visRows = 1;

    // Handle scrolling
    if (ctx.isHovered(r)) {
        scrollOff -= ctx.input().scrollY * rowH * 2.0f;
    }

    int total = static_cast<int>(messages.size());
    float maxScroll = std::max(0.0f, (total - visRows) * rowH);
    if (scrollOff < 0.0f) scrollOff = 0.0f;
    if (scrollOff > maxScroll) scrollOff = maxScroll;

    // Draw visible messages (newest at bottom)
    int firstRow = static_cast<int>(scrollOff / rowH);
    for (int i = 0; i < visRows + 1 && (firstRow + i) < total; ++i) {
        int msgIdx = firstRow + i;
        float y = contentY + i * rowH - std::fmod(scrollOff, rowH);
        if (y + rowH < contentY || y > r.bottom()) continue;

        // Fade older messages
        float age = static_cast<float>(total - 1 - msgIdx) / std::max(1.0f, static_cast<float>(total));
        float alpha = 1.0f - age * 0.4f;
        Color textCol = t.textPrimary.withAlpha(alpha);

        rr.drawText(messages[msgIdx], {r.x + 6.0f, y + 1.0f}, textCol);
    }

    // Scrollbar
    if (total > visRows) {
        Rect scrollTrack = {r.right() - t.scrollbarWidth, contentY,
                           t.scrollbarWidth, contentH};
        scrollbar(ctx, scrollTrack, scrollOff, total * rowH, contentH);
    }
}

// ── Damage Flash Overlay ────────────────────────────────────────────

void damageFlashOverlay(AtlasContext& ctx, Vec2 centre, float radius,
                        int layer, float intensity) {
    if (intensity <= 0.0f) return;

    const Theme& t = ctx.theme();
    Color flashColor;
    switch (layer) {
        case 0: flashColor = t.shield;  break;  // blue
        case 1: flashColor = t.armor;   break;  // gold
        case 2: flashColor = t.hull;    break;  // red
        default: flashColor = t.shield; break;
    }

    float alpha = intensity * 0.35f;

    // Draw concentric rings that fade outward
    int rings = 3;
    for (int i = 0; i < rings; ++i) {
        float innerR = radius + i * 15.0f;
        float outerR = innerR + 12.0f;
        float ringAlpha = alpha * (1.0f - static_cast<float>(i) / rings);
        Color ringColor = flashColor.withAlpha(ringAlpha);

        ctx.renderer().drawArc(centre, innerR, outerR,
                               0.0f, 2.0f * static_cast<float>(M_PI),
                               ringColor, 32);
    }
}

// ── Drone Status Bar ────────────────────────────────────────────────

void droneStatusBar(AtlasContext& ctx, const Rect& r,
                    int inSpace, int inBay,
                    int bandwidthUsed, int bandwidthMax) {
    const Theme& t = ctx.theme();
    auto& rr = ctx.renderer();

    // Background
    rr.drawRect(r, t.bgPanel);
    rr.drawRectOutline(r, t.borderSubtle, t.borderWidth);

    float pad = 4.0f;
    float textY = r.y + (r.h - 13.0f) * 0.5f;

    // Drone counts
    char buf[64];
    std::snprintf(buf, sizeof(buf), "Drones: %d/%d", inSpace, inSpace + inBay);
    rr.drawText(buf, {r.x + pad, textY}, t.textPrimary);

    // Bandwidth bar
    float barX = r.x + 120.0f;
    float barW = r.w - 120.0f - pad * 2 - 60.0f;
    float barH = 8.0f;
    float barY = r.y + (r.h - barH) * 0.5f;
    float fraction = bandwidthMax > 0
        ? static_cast<float>(bandwidthUsed) / static_cast<float>(bandwidthMax)
        : 0.0f;
    Color barColor = fraction > 0.9f ? t.danger : t.accentSecondary;
    rr.drawProgressBar({barX, barY, barW, barH}, fraction, barColor, t.bgSecondary);

    // Bandwidth label
    std::snprintf(buf, sizeof(buf), "%d/%d Mb", bandwidthUsed, bandwidthMax);
    rr.drawText(buf, {barX + barW + pad, textY}, t.textSecondary);
}

// ── Fleet Broadcast Banner ──────────────────────────────────────────

void fleetBroadcastBanner(AtlasContext& ctx, const Rect& r,
                          const std::vector<FleetBroadcast>& broadcasts) {
    if (broadcasts.empty()) return;

    const Theme& t = ctx.theme();
    auto& rr = ctx.renderer();

    float rowH = 20.0f;
    int maxShow = static_cast<int>(r.h / rowH);
    if (maxShow <= 0) maxShow = 1;

    // Show most recent broadcasts (newest last → draw bottom to top)
    int start = std::max(0, static_cast<int>(broadcasts.size()) - maxShow);
    int count = static_cast<int>(broadcasts.size()) - start;

    for (int i = 0; i < count; ++i) {
        const auto& bc = broadcasts[start + i];
        float safeMaxAge = bc.maxAge > 0.0f ? bc.maxAge : 0.01f;
        float alpha = 1.0f - (bc.age / safeMaxAge);
        if (alpha <= 0.0f) continue;

        float y = r.y + i * rowH;

        // Accent left border
        Color accentFade = bc.color.withAlpha(alpha * 0.8f);
        rr.drawRect({r.x, y, 3.0f, rowH - 2.0f}, accentFade);

        // Background
        rr.drawRect({r.x + 3.0f, y, r.w - 3.0f, rowH - 2.0f},
                    t.bgPanel.withAlpha(alpha * 0.7f));

        // Sender + message
        char buf[256];
        std::snprintf(buf, sizeof(buf), "%s: %s",
                      bc.sender.c_str(), bc.message.c_str());
        rr.drawText(buf, {r.x + 8.0f, y + 3.0f},
                    t.textPrimary.withAlpha(alpha));
    }
}

} // namespace atlas
