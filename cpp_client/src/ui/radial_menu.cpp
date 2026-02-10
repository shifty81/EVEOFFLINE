#include "ui/radial_menu.h"
#include <imgui.h>
#include <cmath>

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

void RadialMenu::Open(float screenX, float screenY, const std::string& entityId) {
    m_open = true;
    m_centerX = screenX;
    m_centerY = screenY;
    m_mouseX = screenX;
    m_mouseY = screenY;
    m_entityId = entityId;
    m_highlightedAction = Action::NONE;
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
        return;
    }

    // Calculate angle from center
    float angle = std::atan2(dy, dx);
    int segIdx = GetSegmentAtAngle(angle);
    if (segIdx >= 0 && segIdx < static_cast<int>(m_segments.size())) {
        m_highlightedAction = m_segments[segIdx].action;
    } else {
        m_highlightedAction = Action::NONE;
    }
}

RadialMenu::Action RadialMenu::Confirm() {
    if (!m_open) return Action::NONE;

    Action selected = m_highlightedAction;

    if (selected != Action::NONE && m_onAction) {
        m_onAction(selected, m_entityId);
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
    if (!m_open) return;

    ImDrawList* drawList = ImGui::GetForegroundDrawList();
    ImVec2 center(m_centerX, m_centerY);

    // Draw semi-transparent background circle
    drawList->AddCircleFilled(center, OUTER_RADIUS, IM_COL32(10, 15, 25, 200));
    drawList->AddCircle(center, OUTER_RADIUS, IM_COL32(69, 208, 232, 120), 0, 2.0f);

    // Draw inner dead zone circle
    drawList->AddCircleFilled(center, INNER_RADIUS, IM_COL32(5, 8, 15, 220));
    drawList->AddCircle(center, INNER_RADIUS, IM_COL32(69, 208, 232, 80), 0, 1.0f);

    // Draw segments
    const int arcSegments = 32;
    for (int i = 0; i < static_cast<int>(m_segments.size()); i++) {
        const auto& seg = m_segments[i];
        bool highlighted = (seg.action == m_highlightedAction && m_highlightedAction != Action::NONE);

        // Draw segment divider lines
        float lineAngle = seg.startAngle;
        ImVec2 innerPt(center.x + INNER_RADIUS * std::cos(lineAngle),
                       center.y + INNER_RADIUS * std::sin(lineAngle));
        ImVec2 outerPt(center.x + OUTER_RADIUS * std::cos(lineAngle),
                       center.y + OUTER_RADIUS * std::sin(lineAngle));
        drawList->AddLine(innerPt, outerPt, IM_COL32(69, 208, 232, 50), 1.0f);

        // Highlight the selected segment
        if (highlighted) {
            float midAngle = (seg.startAngle + seg.endAngle) * 0.5f;
            float segAngle = seg.endAngle - seg.startAngle;

            // Draw highlighted arc fill
            int steps = arcSegments;
            std::vector<ImVec2> points;
            // Inner arc
            for (int s = 0; s <= steps; s++) {
                float a = seg.startAngle + (segAngle * s / steps);
                points.push_back(ImVec2(center.x + INNER_RADIUS * std::cos(a),
                                        center.y + INNER_RADIUS * std::sin(a)));
            }
            // Outer arc (reverse)
            for (int s = steps; s >= 0; s--) {
                float a = seg.startAngle + (segAngle * s / steps);
                points.push_back(ImVec2(center.x + OUTER_RADIUS * std::cos(a),
                                        center.y + OUTER_RADIUS * std::sin(a)));
            }
            if (points.size() >= 3) {
                drawList->AddConvexPolyFilled(points.data(), static_cast<int>(points.size()),
                                              IM_COL32(69, 208, 232, 60));
            }
        }

        // Draw label at mid-angle, at ICON_RADIUS distance
        float midAngle = (seg.startAngle + seg.endAngle) * 0.5f;
        ImVec2 labelPos(center.x + ICON_RADIUS * std::cos(midAngle),
                        center.y + ICON_RADIUS * std::sin(midAngle));

        ImU32 textColor = highlighted ? IM_COL32(69, 208, 232, 255) : IM_COL32(200, 215, 230, 200);

        // Center the text approximately
        const char* label = seg.label;
        ImVec2 textSize = ImGui::CalcTextSize(label);
        drawList->AddText(ImVec2(labelPos.x - textSize.x * 0.5f, labelPos.y - textSize.y * 0.5f),
                          textColor, label);
    }

    // Draw mouse direction indicator line
    float dx = m_mouseX - m_centerX;
    float dy = m_mouseY - m_centerY;
    float dist = std::sqrt(dx * dx + dy * dy);
    if (dist > INNER_RADIUS) {
        float indicatorLen = std::min(dist, OUTER_RADIUS);
        float angle = std::atan2(dy, dx);
        ImVec2 indicator(center.x + indicatorLen * std::cos(angle),
                         center.y + indicatorLen * std::sin(angle));
        drawList->AddLine(center, indicator, IM_COL32(69, 208, 232, 100), 1.5f);
    }
}

} // namespace UI
