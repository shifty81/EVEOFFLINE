#include "ui/eve_target_list.h"
#include "core/entity.h"
#include <imgui.h>
#include <algorithm>
#include <cmath>

namespace UI {

// Use centralized EVEColors from ui_manager.h for consistency
static const float COLOR_SHIELD[] = {0.2f, 0.6f, 1.0f, 1.0f};     // Blue — matches EVEColors::SHIELD_COLOR
static const float COLOR_ARMOR[] = {1.0f, 0.816f, 0.251f, 1.0f};   // Gold — matches EVEColors::ARMOR_COLOR
static const float COLOR_HULL[] = {0.902f, 0.271f, 0.271f, 1.0f};  // Red — matches EVEColors::HULL_COLOR
static const float COLOR_BACKGROUND[] = {0.051f, 0.067f, 0.090f, 0.92f};  // Dark blue-black — matches EVEColors::BG_PRIMARY
static const float COLOR_BORDER[] = {0.271f, 0.816f, 0.910f, 0.8f};       // Teal — matches EVEColors::ACCENT_PRIMARY
static const float COLOR_ACTIVE[] = {1.0f, 0.878f, 0.4f, 1.0f};          // Yellow-gold for active target

EVETargetList::EVETargetList()
    : m_posX(50.0f)
    , m_posY(50.0f)
    , m_iconSize(80.0f)
    , m_iconSpacing(10.0f)
{
}

EVETargetList::~EVETargetList() {
}

void EVETargetList::render() {
    if (m_targets.empty()) {
        return;
    }
    
    ImGui::SetNextWindowPos(ImVec2(m_posX, m_posY), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowBgAlpha(0.0f);  // Transparent background
    
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | 
                            ImGuiWindowFlags_NoMove |
                            ImGuiWindowFlags_AlwaysAutoResize |
                            ImGuiWindowFlags_NoFocusOnAppearing;
    
    if (ImGui::Begin("##TargetList", nullptr, flags)) {
        // Render targets in a horizontal row
        float currentX = 0.0f;
        
        for (size_t i = 0; i < m_targets.size(); ++i) {
            if (i > 0) {
                ImGui::SameLine();
                currentX += m_iconSize + m_iconSpacing;
            }
            
            ImGui::BeginGroup();
            renderTargetIcon(m_targets[i], currentX, 0.0f);
            ImGui::EndGroup();
        }
    }
    ImGui::End();
}

void EVETargetList::renderTargetIcon(const TargetData& target, float x, float y) {
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetCursorScreenPos();
    
    float centerX = pos.x + m_iconSize * 0.5f;
    float centerY = pos.y + m_iconSize * 0.5f;
    float radius = m_iconSize * 0.4f;
    
    // Draw background circle
    draw_list->AddCircleFilled(ImVec2(centerX, centerY), radius + 4.0f, 
                              ImColor(COLOR_BACKGROUND[0], COLOR_BACKGROUND[1], 
                                     COLOR_BACKGROUND[2], COLOR_BACKGROUND[3]));
    
    // Draw border (teal for normal, yellow for active target)
    const float* borderColor = target.isActive ? COLOR_ACTIVE : COLOR_BORDER;
    float borderThickness = target.isActive ? 3.0f : 2.0f;
    draw_list->AddCircle(ImVec2(centerX, centerY), radius + 4.0f, 
                        ImColor(borderColor[0], borderColor[1], borderColor[2], borderColor[3]),
                        32, borderThickness);
    
    // Draw health arcs (shield, armor, hull)
    // Each takes up 1/3 of the circle, starting from top
    float arcLength = (2.0f * 3.14159f) / 3.0f;  // 120 degrees each
    float startAngle = -3.14159f / 2.0f;  // Start at top
    
    // Shield arc (top, blue)
    renderHealthArc(centerX, centerY, radius + 2.0f,
                   startAngle, startAngle + arcLength,
                   target.shieldPercent, COLOR_SHIELD);
    
    // Armor arc (right, yellow)
    renderHealthArc(centerX, centerY, radius + 2.0f,
                   startAngle + arcLength, startAngle + arcLength * 2.0f,
                   target.armorPercent, COLOR_ARMOR);
    
    // Hull arc (left, red)
    renderHealthArc(centerX, centerY, radius + 2.0f,
                   startAngle + arcLength * 2.0f, startAngle + arcLength * 3.0f,
                   target.hullPercent, COLOR_HULL);
    
    // Draw ship icon/name in center
    ImVec2 textSize = ImGui::CalcTextSize(target.shipType.c_str());
    ImVec2 textPos(centerX - textSize.x * 0.5f, centerY - textSize.y * 0.5f);
    draw_list->AddText(textPos, IM_COL32(200, 200, 200, 255), target.shipType.c_str());
    
    // Draw distance below
    char distText[64];
    snprintf(distText, sizeof(distText), "%.0f m", target.distance);
    ImVec2 distSize = ImGui::CalcTextSize(distText);
    ImVec2 distPos(centerX - distSize.x * 0.5f, centerY + radius + 10.0f);
    draw_list->AddText(distPos, IM_COL32(150, 150, 150, 255), distText);
    
    // Advance cursor for next icon
    ImGui::Dummy(ImVec2(m_iconSize, m_iconSize + 30.0f));
    
    // Show tooltip on hover
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::Text("%s", target.shipName.c_str());
        ImGui::Text("%s", target.shipType.c_str());
        ImGui::Separator();
        ImGui::Text("Distance: %.0f m", target.distance);
        ImGui::Separator();
        ImGui::Text("Shield: %.0f / %.0f (%.0f%%)", 
                   target.shieldPercent * target.maxShield, target.maxShield, 
                   target.shieldPercent * 100.0f);
        ImGui::Text("Armor: %.0f / %.0f (%.0f%%)", 
                   target.armorPercent * target.maxArmor, target.maxArmor, 
                   target.armorPercent * 100.0f);
        ImGui::Text("Hull: %.0f / %.0f (%.0f%%)", 
                   target.hullPercent * target.maxHull, target.maxHull, 
                   target.hullPercent * 100.0f);
        ImGui::EndTooltip();
    }
}

void EVETargetList::renderHealthArc(float centerX, float centerY, float radius,
                                   float startAngle, float endAngle,
                                   float percent, const float color[4]) {
    if (percent <= 0.0f) return;
    
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    
    // Calculate actual end angle based on percentage
    float actualEndAngle = startAngle + (endAngle - startAngle) * percent;
    
    // Draw thick arc
    const int segments = 20;
    const float thickness = 6.0f;
    
    for (int i = 0; i < segments; ++i) {
        float t1 = (float)i / segments;
        float t2 = (float)(i + 1) / segments;
        
        float angle1 = startAngle + (actualEndAngle - startAngle) * t1;
        float angle2 = startAngle + (actualEndAngle - startAngle) * t2;
        
        float x1 = centerX + std::cos(angle1) * radius;
        float y1 = centerY + std::sin(angle1) * radius;
        float x2 = centerX + std::cos(angle2) * radius;
        float y2 = centerY + std::sin(angle2) * radius;
        
        draw_list->AddLine(ImVec2(x1, y1), ImVec2(x2, y2),
                          ImColor(color[0], color[1], color[2], color[3]),
                          thickness);
    }
}

void EVETargetList::updateTargets(const std::unordered_map<std::string, std::shared_ptr<eve::Entity>>& entities) {
    // Update existing targets
    for (auto& target : m_targets) {
        auto it = entities.find(target.entityId);
        if (it != entities.end()) {
            const auto& entity = it->second;
            const auto& health = entity->getHealth();
            
            target.shipName = entity->getShipName();
            target.shipType = entity->getShipType();
            target.distance = glm::length(entity->getPosition());  // Distance from origin
            
            // Update health percentages
            target.maxShield = health.maxShield;
            target.maxArmor = health.maxArmor;
            target.maxHull = health.maxHull;
            
            target.shieldPercent = (health.maxShield > 0.0f) ? 
                (health.currentShield / health.maxShield) : 0.0f;
            target.armorPercent = (health.maxArmor > 0.0f) ? 
                (health.currentArmor / health.maxArmor) : 0.0f;
            target.hullPercent = (health.maxHull > 0.0f) ? 
                (health.currentHull / health.maxHull) : 1.0f;
        }
    }
}

void EVETargetList::addTarget(const std::string& entityId) {
    // Check if already targeted
    if (isTargeted(entityId)) {
        return;
    }
    
    TargetData target;
    target.entityId = entityId;
    target.lockProgress = 0.0f;  // Start locking animation
    m_targets.push_back(target);
    
    // If this is the first target, make it active
    if (m_targets.size() == 1) {
        m_activeTargetId = entityId;
        target.isActive = true;
    }
}

void EVETargetList::removeTarget(const std::string& entityId) {
    auto it = std::remove_if(m_targets.begin(), m_targets.end(),
        [&entityId](const TargetData& target) {
            return target.entityId == entityId;
        });
    
    if (it != m_targets.end()) {
        m_targets.erase(it, m_targets.end());
        
        // If we removed the active target, make the first target active
        if (m_activeTargetId == entityId && !m_targets.empty()) {
            m_activeTargetId = m_targets[0].entityId;
            m_targets[0].isActive = true;
        } else if (m_targets.empty()) {
            m_activeTargetId.clear();
        }
    }
}

void EVETargetList::setActiveTarget(const std::string& entityId) {
    // Deactivate all targets
    for (auto& target : m_targets) {
        target.isActive = false;
    }
    
    // Activate the specified target
    for (auto& target : m_targets) {
        if (target.entityId == entityId) {
            target.isActive = true;
            m_activeTargetId = entityId;
            break;
        }
    }
}

bool EVETargetList::isTargeted(const std::string& entityId) const {
    return std::any_of(m_targets.begin(), m_targets.end(),
        [&entityId](const TargetData& target) {
            return target.entityId == entityId;
        });
}

} // namespace UI
