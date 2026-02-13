#include "ui/radial_menu.h"
#include "ui/atlas/atlas_context.h"
#include <cmath>
#include <cstdio>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace UI {

RadialMenu::RadialMenu()
    : m_open(false)
    , m_centerX(0.0f)
    , m_centerY(0.0f)
    , m_mouseX(0.0f)
    , m_mouseY(0.0f)
    , m_highlightedAction(Action::NONE)
    , m_rangeDistance(0)
{
    SetupSegments();
}

void RadialMenu::SetupSegments() {
    // 8 equal segments, 45 degrees each, starting from top (12 o'clock = -PI/2)
    const int numSegments = 8;
    float segmentAngle = 2.0f * static_cast<float>(M_PI) / numSegments;
    float startOffset = -static_cast<float>(M_PI) / 2.0f - segmentAngle / 2.0f;  // Center top segment at 12 o'clock

    struct SegmentDef {
        Action action;
        const char* label;
        const char* icon;
    };

    SegmentDef defs[] = {
        { Action::APPROACH,      "Approach",  "A"  },  // Top
        { Action::ORBIT,         "Orbit",     "O"  },  // Top-right
        { Action::WARP_TO,       "Warp To",   "W"  },  // Right
        { Action::LOCK_TARGET,   "Lock",      "L"  },  // Bottom-right
        { Action::KEEP_AT_RANGE, "Range",     "R"  },  // Bottom
        { Action::ALIGN_TO,      "Align",     ">"  },  // Bottom-left
        { Action::SHOW_INFO,     "Info",      "i"  },  // Left
        { Action::LOOK_AT,       "Look At",   "@"  },  // Top-left
    };

    m_segments.clear();
    for (int i = 0; i < numSegments; i++) {
        Segment seg;
        seg.action = defs[i].action;
        seg.label = defs[i].label;
        seg.icon = defs[i].icon;
        seg.startAngle = startOffset + i * segmentAngle;
        seg.endAngle = seg.startAngle + segmentAngle;
        m_segments.push_back(seg);
    }
}

void RadialMenu::Open(float screenX, float screenY, const std::string& entityId, float distanceToTarget) {
    m_open = true;
    m_centerX = screenX;
    m_centerY = screenY;
    m_mouseX = screenX;
    m_mouseY = screenY;
    m_entityId = entityId;
    m_highlightedAction = Action::NONE;
    m_rangeDistance = 0;
    m_distanceToTarget = distanceToTarget;
}

void RadialMenu::Close() {
    m_open = false;
    m_highlightedAction = Action::NONE;
    m_entityId.clear();
}

void RadialMenu::UpdateMousePosition(float mouseX, float mouseY) {
    if (!m_open) return;

    m_mouseX = mouseX;
    m_mouseY = mouseY;

    float dx = mouseX - m_centerX;
    float dy = mouseY - m_centerY;
    float dist = std::sqrt(dx * dx + dy * dy);

    if (dist < INNER_RADIUS) {
        // In dead zone — no selection
        m_highlightedAction = Action::NONE;
        m_rangeDistance = 0;
        return;
    }

    // Calculate angle from center
    float angle = std::atan2(dy, dx);
    int segIdx = GetSegmentAtAngle(angle);
    if (segIdx >= 0 && segIdx < static_cast<int>(m_segments.size())) {
        Action candidate = m_segments[segIdx].action;
        // Disable "Warp To" for on-grid entities (within 150km, same as ShipPhysics::MIN_WARP_DISTANCE)
        if (candidate == Action::WARP_TO && isWarpDisabled()) {
            m_highlightedAction = Action::NONE;
        } else {
            m_highlightedAction = candidate;
        }
    } else {
        m_highlightedAction = Action::NONE;
    }

    // EVE-style drag-to-range: dragging further from center selects
    // a larger distance for Orbit and Keep at Range actions
    UpdateRangeDistance(dist);
}

void RadialMenu::UpdateRangeDistance(float dist) {
    // Only compute range for distance-based actions
    if (m_highlightedAction != Action::ORBIT &&
        m_highlightedAction != Action::KEEP_AT_RANGE) {
        m_rangeDistance = 0;
        return;
    }

    // Map drag distance (OUTER_RADIUS to MAX_RANGE_RADIUS) to distance presets
    // Distance steps: 500m, 1000m, 2500m, 5000m, 10000m, 20000m, 50000m
    static const int distanceSteps[] = {500, 1000, 2500, 5000, 10000, 20000, 50000};
    static const int numSteps = 7;

    if (dist <= OUTER_RADIUS) {
        // Default: closest range
        m_rangeDistance = distanceSteps[0];
        return;
    }

    float rangeFrac = (dist - OUTER_RADIUS) / (MAX_RANGE_RADIUS - OUTER_RADIUS);
    rangeFrac = std::max(0.0f, std::min(1.0f, rangeFrac));
    int stepIdx = static_cast<int>(rangeFrac * (numSteps - 1));
    stepIdx = std::max(0, std::min(numSteps - 1, stepIdx));
    m_rangeDistance = distanceSteps[stepIdx];
}

RadialMenu::Action RadialMenu::Confirm() {
    if (!m_open) return Action::NONE;

    Action selected = m_highlightedAction;

    if (selected != Action::NONE) {
        // Fire ranged callback for distance-based actions
        if (m_rangeDistance > 0 && m_onRangedAction &&
            (selected == Action::ORBIT || selected == Action::KEEP_AT_RANGE)) {
            m_onRangedAction(selected, m_entityId, m_rangeDistance);
        } else if (m_onAction) {
            m_onAction(selected, m_entityId);
        }
    }

    Close();
    return selected;
}

int RadialMenu::GetSegmentAtAngle(float angle) const {
    for (int i = 0; i < static_cast<int>(m_segments.size()); i++) {
        float start = m_segments[i].startAngle;
        float end = m_segments[i].endAngle;

        // Normalize angle to segment range
        float a = angle;
        // Handle wrap-around
        while (a < start) a += 2.0f * static_cast<float>(M_PI);
        while (a > start + 2.0f * static_cast<float>(M_PI)) a -= 2.0f * static_cast<float>(M_PI);

        if (a >= start && a < end) {
            return i;
        }
    }
    return -1;
}

void RadialMenu::Render() {
    // Legacy stub — rendering now goes through RenderAtlas()
}

void RadialMenu::RenderAtlas(atlas::AtlasContext& ctx) {
    if (!m_open) return;

    const atlas::Theme& t = ctx.theme();
    auto& r = ctx.renderer();

    atlas::Vec2 center(m_centerX, m_centerY);

    // EVE-style teal accent color
    atlas::Color accentTeal  = t.accentPrimary;
    atlas::Color accentDim   = t.accentDim;
    atlas::Color bgDark      = atlas::Color(0.04f, 0.06f, 0.09f, 0.85f);
    atlas::Color bgHighlight = atlas::Color(0.1f, 0.25f, 0.32f, 0.9f);

    // Draw outer ring background
    r.drawArc(center, INNER_RADIUS, OUTER_RADIUS,
              0.0f, 2.0f * static_cast<float>(M_PI), bgDark, 48);

    // Draw inner dead-zone circle (darker)
    r.drawCircle(center, INNER_RADIUS, atlas::Color(0.02f, 0.03f, 0.05f, 0.9f));
    r.drawCircleOutline(center, INNER_RADIUS, accentDim, 1.0f);

    // Draw outer ring outline
    r.drawCircleOutline(center, OUTER_RADIUS, accentDim, 1.5f);

    // Draw segment divider lines and labels
    float pi2 = 2.0f * static_cast<float>(M_PI);
    for (int i = 0; i < static_cast<int>(m_segments.size()); ++i) {
        const auto& seg = m_segments[i];
        bool highlighted = (seg.action == m_highlightedAction && m_highlightedAction != Action::NONE);

        // Check if this action is disabled (e.g. Warp To when target is on-grid)
        bool disabled = (seg.action == Action::WARP_TO && isWarpDisabled());

        // Highlight the selected segment
        if (highlighted && !disabled) {
            r.drawArc(center, INNER_RADIUS + 1.0f, OUTER_RADIUS - 1.0f,
                      seg.startAngle, seg.endAngle, bgHighlight, 8);
            // Teal accent border on highlighted segment
            r.drawArc(center, OUTER_RADIUS - 3.0f, OUTER_RADIUS,
                      seg.startAngle, seg.endAngle, accentTeal, 8);
        }

        // Divider line from inner to outer radius
        float ca = std::cos(seg.startAngle);
        float sa = std::sin(seg.startAngle);
        atlas::Vec2 lineStart(center.x + ca * INNER_RADIUS,
                              center.y + sa * INNER_RADIUS);
        atlas::Vec2 lineEnd(center.x + ca * OUTER_RADIUS,
                            center.y + sa * OUTER_RADIUS);
        r.drawLine(lineStart, lineEnd, accentDim.withAlpha(0.4f), 1.0f);

        // Label at midpoint of segment arc
        float midAngle = (seg.startAngle + seg.endAngle) * 0.5f;
        float labelX = center.x + std::cos(midAngle) * ICON_RADIUS;
        float labelY = center.y + std::sin(midAngle) * ICON_RADIUS;

        // Center the text approximately
        float textW = r.measureText(seg.label);
        atlas::Color labelColor = disabled ? t.textDisabled
                                 : highlighted ? accentTeal
                                 : t.textPrimary;
        r.drawText(seg.label,
                   {labelX - textW * 0.5f, labelY - 6.0f},
                   labelColor);
    }

    // Draw crosshair / mouse direction indicator
    float dx = m_mouseX - m_centerX;
    float dy = m_mouseY - m_centerY;
    float dist = std::sqrt(dx * dx + dy * dy);
    if (dist > INNER_RADIUS) {
        float nx = dx / dist;
        float ny = dy / dist;
        atlas::Vec2 indicatorPos(center.x + nx * (INNER_RADIUS + 6.0f),
                                 center.y + ny * (INNER_RADIUS + 6.0f));
        r.drawCircle(indicatorPos, 3.0f, accentTeal);
    }

    // ── Drag-to-range distance indicator ───────────────────────────
    // When dragging beyond the outer radius on Orbit/Keep at Range,
    // show the selected distance as an arc + text label
    if (m_rangeDistance > 0 && dist > OUTER_RADIUS &&
        (m_highlightedAction == Action::ORBIT || m_highlightedAction == Action::KEEP_AT_RANGE)) {
        // Draw range indicator arc at current drag radius
        float rangeR = std::min(dist, MAX_RANGE_RADIUS);
        r.drawCircleOutline(center, rangeR, accentTeal.withAlpha(0.3f), 1.0f);

        // Format distance text
        char rangeBuf[32];
        if (m_rangeDistance >= 1000) {
            std::snprintf(rangeBuf, sizeof(rangeBuf), "%d km", m_rangeDistance / 1000);
        } else {
            std::snprintf(rangeBuf, sizeof(rangeBuf), "%d m", m_rangeDistance);
        }

        // Draw distance label near the mouse
        float labelW = r.measureText(rangeBuf);
        r.drawText(rangeBuf,
                   {m_mouseX - labelW * 0.5f, m_mouseY - 18.0f},
                   accentTeal);
    }
}

} // namespace UI
