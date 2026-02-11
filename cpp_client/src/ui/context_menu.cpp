#include "ui/context_menu.h"
#ifdef HAS_IMGUI
#include "ui/ui_manager.h"
#include "ui/eve_colors.h"
#include <imgui.h>
#endif
#include <iostream>

namespace UI {

ContextMenu::ContextMenu()
    : m_menuType(ContextMenuType::NONE)
    , m_targetIsLocked(false)
    , m_worldX(0.0f)
    , m_worldY(0.0f)
    , m_worldZ(0.0f)
{
}

void ContextMenu::ShowEntityMenu(const std::string& entity_id, bool is_locked) {
    m_menuType = ContextMenuType::ENTITY;
    m_targetEntityId = entity_id;
    m_targetIsLocked = is_locked;
#ifdef HAS_IMGUI
    ImGui::OpenPopup("EntityContextMenu");
#endif
}

void ContextMenu::ShowEmptySpaceMenu(float world_x, float world_y, float world_z) {
    m_menuType = ContextMenuType::EMPTY_SPACE;
    m_worldX = world_x;
    m_worldY = world_y;
    m_worldZ = world_z;
#ifdef HAS_IMGUI
    ImGui::OpenPopup("EmptySpaceContextMenu");
#endif
}

void ContextMenu::Close() {
    m_menuType = ContextMenuType::NONE;
#ifdef HAS_IMGUI
    ImGui::CloseCurrentPopup();
#endif
}

void ContextMenu::Render() {
    if (m_menuType == ContextMenuType::NONE) {
        return;
    }
    
#ifdef HAS_IMGUI
    // Photon UI menu colors — teal accent hover, dark blue-black background
    ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(
        EVEColors::BG_TOOLTIP[0], EVEColors::BG_TOOLTIP[1],
        EVEColors::BG_TOOLTIP[2], EVEColors::BG_TOOLTIP[3]));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(
        EVEColors::TEXT_PRIMARY[0], EVEColors::TEXT_PRIMARY[1],
        EVEColors::TEXT_PRIMARY[2], EVEColors::TEXT_PRIMARY[3]));
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(
        EVEColors::SELECTION[0], EVEColors::SELECTION[1],
        EVEColors::SELECTION[2], EVEColors::SELECTION[3]));
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(
        EVEColors::ACCENT_DIM[0], EVEColors::ACCENT_DIM[1],
        EVEColors::ACCENT_DIM[2], 0.8f));
    
    if (m_menuType == ContextMenuType::ENTITY) {
        if (ImGui::BeginPopup("EntityContextMenu")) {
            // Approach
            if (ImGui::MenuItem("Approach")) {
                if (m_onApproach) {
                    m_onApproach(m_targetEntityId);
                }
                Close();
            }
            
            // Orbit with submenu
            if (ImGui::BeginMenu("Orbit")) {
                RenderOrbitSubmenu();
                ImGui::EndMenu();
            }
            
            // Keep at Range with submenu
            if (ImGui::BeginMenu("Keep at Range")) {
                RenderKeepAtRangeSubmenu();
                ImGui::EndMenu();
            }
            
            ImGui::Separator();
            
            // Warp To with submenu
            if (ImGui::BeginMenu("Warp To")) {
                RenderWarpToSubmenu();
                ImGui::EndMenu();
            }
            
            ImGui::Separator();
            
            // Lock/Unlock Target
            if (m_targetIsLocked) {
                if (ImGui::MenuItem("Unlock Target")) {
                    if (m_onUnlockTarget) {
                        m_onUnlockTarget(m_targetEntityId);
                    }
                    Close();
                }
            } else {
                if (ImGui::MenuItem("Lock Target")) {
                    if (m_onLockTarget) {
                        m_onLockTarget(m_targetEntityId);
                    }
                    Close();
                }
            }
            
            ImGui::Separator();
            
            // Look At
            if (ImGui::MenuItem("Look At")) {
                if (m_onLookAt) {
                    m_onLookAt(m_targetEntityId);
                }
                Close();
            }
            
            // Show Info
            if (ImGui::MenuItem("Show Info")) {
                if (m_onShowInfo) {
                    m_onShowInfo(m_targetEntityId);
                }
                Close();
            }
            
            ImGui::Separator();
            
            // Cancel
            if (ImGui::MenuItem("Cancel")) {
                Close();
            }
            
            ImGui::EndPopup();
        } else {
            // Popup was closed externally
            m_menuType = ContextMenuType::NONE;
        }
    } else if (m_menuType == ContextMenuType::EMPTY_SPACE) {
        if (ImGui::BeginPopup("EmptySpaceContextMenu")) {
            // Navigate To
            if (ImGui::MenuItem("Navigate To")) {
                if (m_onNavigateTo) {
                    m_onNavigateTo(m_worldX, m_worldY, m_worldZ);
                }
                Close();
            }
            
            // Bookmark Location
            if (ImGui::MenuItem("Bookmark Location")) {
                if (m_onBookmark) {
                    m_onBookmark(m_worldX, m_worldY, m_worldZ);
                }
                Close();
            }
            
            ImGui::Separator();
            
            // Cancel
            if (ImGui::MenuItem("Cancel")) {
                Close();
            }
            
            ImGui::EndPopup();
        } else {
            // Popup was closed externally
            m_menuType = ContextMenuType::NONE;
        }
    }
    
    ImGui::PopStyleColor(4);
#endif
}

void ContextMenu::RenderOrbitSubmenu() {
#ifdef HAS_IMGUI
    if (ImGui::MenuItem("500m")) {
        if (m_onOrbit) {
            m_onOrbit(m_targetEntityId, static_cast<int>(OrbitDistance::ORBIT_500M));
        }
        Close();
    }
    if (ImGui::MenuItem("1km")) {
        if (m_onOrbit) {
            m_onOrbit(m_targetEntityId, static_cast<int>(OrbitDistance::ORBIT_1KM));
        }
        Close();
    }
    if (ImGui::MenuItem("5km")) {
        if (m_onOrbit) {
            m_onOrbit(m_targetEntityId, static_cast<int>(OrbitDistance::ORBIT_5KM));
        }
        Close();
    }
    if (ImGui::MenuItem("10km")) {
        if (m_onOrbit) {
            m_onOrbit(m_targetEntityId, static_cast<int>(OrbitDistance::ORBIT_10KM));
        }
        Close();
    }
    if (ImGui::MenuItem("20km")) {
        if (m_onOrbit) {
            m_onOrbit(m_targetEntityId, static_cast<int>(OrbitDistance::ORBIT_20KM));
        }
        Close();
    }
    if (ImGui::MenuItem("50km")) {
        if (m_onOrbit) {
            m_onOrbit(m_targetEntityId, static_cast<int>(OrbitDistance::ORBIT_50KM));
        }
        Close();
    }
#endif
}

void ContextMenu::RenderKeepAtRangeSubmenu() {
#ifdef HAS_IMGUI
    if (ImGui::MenuItem("1km")) {
        if (m_onKeepAtRange) {
            m_onKeepAtRange(m_targetEntityId, static_cast<int>(KeepAtRangeDistance::RANGE_1KM));
        }
        Close();
    }
    if (ImGui::MenuItem("5km")) {
        if (m_onKeepAtRange) {
            m_onKeepAtRange(m_targetEntityId, static_cast<int>(KeepAtRangeDistance::RANGE_5KM));
        }
        Close();
    }
    if (ImGui::MenuItem("10km")) {
        if (m_onKeepAtRange) {
            m_onKeepAtRange(m_targetEntityId, static_cast<int>(KeepAtRangeDistance::RANGE_10KM));
        }
        Close();
    }
    if (ImGui::MenuItem("20km")) {
        if (m_onKeepAtRange) {
            m_onKeepAtRange(m_targetEntityId, static_cast<int>(KeepAtRangeDistance::RANGE_20KM));
        }
        Close();
    }
    if (ImGui::MenuItem("50km")) {
        if (m_onKeepAtRange) {
            m_onKeepAtRange(m_targetEntityId, static_cast<int>(KeepAtRangeDistance::RANGE_50KM));
        }
        Close();
    }
#endif
}

void ContextMenu::RenderWarpToSubmenu() {
#ifdef HAS_IMGUI
    if (ImGui::MenuItem("At 0km")) {
        if (m_onWarpTo) {
            m_onWarpTo(m_targetEntityId, static_cast<int>(WarpToDistance::WARP_0KM));
        }
        Close();
    }
    if (ImGui::MenuItem("At 10km")) {
        if (m_onWarpTo) {
            m_onWarpTo(m_targetEntityId, static_cast<int>(WarpToDistance::WARP_10KM));
        }
        Close();
    }
    if (ImGui::MenuItem("At 50km")) {
        if (m_onWarpTo) {
            m_onWarpTo(m_targetEntityId, static_cast<int>(WarpToDistance::WARP_50KM));
        }
        Close();
    }
    if (ImGui::MenuItem("At 100km")) {
        if (m_onWarpTo) {
            m_onWarpTo(m_targetEntityId, static_cast<int>(WarpToDistance::WARP_100KM));
        }
        Close();
    }
#endif
}

} // namespace UI
