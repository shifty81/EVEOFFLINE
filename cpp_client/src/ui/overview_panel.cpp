#include "ui/overview_panel.h"
#include "ui/ui_manager.h"
#include "core/entity.h"
#include <imgui.h>
#include <algorithm>
#include <cmath>
#include <sstream>
#include <iomanip>

namespace UI {

OverviewPanel::OverviewPanel()
    : m_visible(true)
    , m_sortColumn(OverviewSortColumn::DISTANCE)
    , m_sortAscending(true)
{
    // Setup default filters
    OverviewFilter allFilter;
    allFilter.name = "All";
    m_savedFilters["All"] = allFilter;
    
    OverviewFilter hostileFilter;
    hostileFilter.name = "Hostile";
    hostileFilter.show_friendly = false;
    hostileFilter.show_neutral = false;
    m_savedFilters["Hostile"] = hostileFilter;
    
    OverviewFilter friendlyFilter;
    friendlyFilter.name = "Friendly";
    friendlyFilter.show_hostile = false;
    friendlyFilter.show_neutral = false;
    m_savedFilters["Friendly"] = friendlyFilter;
    
    OverviewFilter neutralFilter;
    neutralFilter.name = "Neutral";
    neutralFilter.show_hostile = false;
    neutralFilter.show_friendly = false;
    m_savedFilters["Neutral"] = neutralFilter;
    
    m_currentFilter = allFilter;
}

void OverviewPanel::Render() {
    if (!m_visible) return;
    
    // Set window size and position
    ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(50, 50), ImGuiCond_FirstUseEver);
    
    // EVE-style window flags
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse;
    
    if (!ImGui::Begin("Overview", &m_visible, flags)) {
        ImGui::End();
        return;
    }
    
    // Render filter tabs
    RenderFilterTabs();
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    // Render entity table
    if (ImGui::BeginTable("OverviewTable", 5, 
                         ImGuiTableFlags_Borders | 
                         ImGuiTableFlags_RowBg | 
                         ImGuiTableFlags_Resizable |
                         ImGuiTableFlags_Sortable |
                         ImGuiTableFlags_ScrollY |
                         ImGuiTableFlags_SizingStretchProp)) {
        
        RenderTableHeader();
        
        // Render rows
        for (size_t i = 0; i < m_filteredEntries.size(); i++) {
            RenderEntityRow(m_filteredEntries[i], static_cast<int>(i));
        }
        
        ImGui::EndTable();
    }
    
    ImGui::End();
}

void OverviewPanel::RenderFilterTabs() {
    // Create tabs for quick filter switching
    if (ImGui::BeginTabBar("OverviewFilters", ImGuiTabBarFlags_None)) {
        
        for (const auto& [name, filter] : m_savedFilters) {
            if (ImGui::BeginTabItem(name.c_str())) {
                if (m_currentFilter.name != name) {
                    SelectFilter(name);
                }
                ImGui::EndTabItem();
            }
        }
        
        ImGui::EndTabBar();
    }
}

void OverviewPanel::RenderTableHeader() {
    // Setup columns
    ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableSetupColumn("Distance", ImGuiTableColumnFlags_WidthFixed, 80.0f);
    ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 120.0f);
    ImGui::TableSetupColumn("Corp", ImGuiTableColumnFlags_WidthFixed, 100.0f);
    ImGui::TableSetupColumn("Standing", ImGuiTableColumnFlags_WidthFixed, 60.0f);
    
    // Enable sorting
    ImGui::TableSetupScrollFreeze(0, 1); // Freeze header row
    ImGui::TableHeadersRow();
    
    // Check for sort column changes
    ImGuiTableSortSpecs* sortSpecs = ImGui::TableGetSortSpecs();
    if (sortSpecs && sortSpecs->SpecsDirty) {
        if (sortSpecs->SpecsCount > 0) {
            const ImGuiTableColumnSortSpecs& spec = sortSpecs->Specs[0];
            
            OverviewSortColumn newColumn = OverviewSortColumn::NONE;
            switch (spec.ColumnIndex) {
                case 0: newColumn = OverviewSortColumn::NAME; break;
                case 1: newColumn = OverviewSortColumn::DISTANCE; break;
                case 2: newColumn = OverviewSortColumn::TYPE; break;
                case 3: newColumn = OverviewSortColumn::CORPORATION; break;
                case 4: newColumn = OverviewSortColumn::STANDING; break;
            }
            
            bool ascending = (spec.SortDirection == ImGuiSortDirection_Ascending);
            
            if (newColumn != m_sortColumn || ascending != m_sortAscending) {
                SetSortColumn(newColumn, ascending);
            }
        }
        sortSpecs->SpecsDirty = false;
    }
}

void OverviewPanel::RenderEntityRow(const OverviewEntry& entry, int row_index) {
    ImGui::TableNextRow();
    
    // Determine row color based on standing
    float rowColor[4];
    GetStandingColor(entry.standing, rowColor);
    
    bool isSelected = (entry.entity_id == m_selectedEntity);
    
    // Name column (selectable)
    ImGui::TableSetColumnIndex(0);
    
    // Apply standing color
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(rowColor[0], rowColor[1], rowColor[2], rowColor[3]));
    
    // Enhance hover highlighting with Photon UI teal accent
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(
        EVEColors::SELECTION[0], EVEColors::SELECTION[1],
        EVEColors::SELECTION[2], 0.8f));
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(
        EVEColors::ACCENT_DIM[0], EVEColors::ACCENT_DIM[1],
        EVEColors::ACCENT_DIM[2], 0.9f));
    
    ImGuiSelectableFlags selectableFlags = ImGuiSelectableFlags_SpanAllColumns | 
                                           ImGuiSelectableFlags_AllowItemOverlap;
    
    if (ImGui::Selectable(entry.name.c_str(), isSelected, selectableFlags, ImVec2(0, 0))) {
        // Single click - select entity
        bool ctrlHeld = ImGui::GetIO().KeyCtrl;
        if (m_onSelect) {
            m_onSelect(entry.entity_id, ctrlHeld);
        }
        m_selectedEntity = entry.entity_id;
    }
    
    // Hover tooltip with entity details
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::TextColored(ImVec4(
            EVEColors::ACCENT_PRIMARY[0], EVEColors::ACCENT_PRIMARY[1],
            EVEColors::ACCENT_PRIMARY[2], EVEColors::ACCENT_PRIMARY[3]),
            "%s", entry.name.c_str());
        ImGui::Text("Type: %s", entry.ship_type.c_str());
        ImGui::Text("Distance: %s", FormatDistance(entry.distance).c_str());
        ImGui::Text("Corp: %s", entry.corporation.c_str());
        ImGui::EndTooltip();
    }
    
    // Double-click to align/warp
    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
        if (m_onAlignTo) {
            m_onAlignTo(entry.entity_id);
        }
    }
    
    // Right-click context menu
    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
        ImGui::OpenPopup("EntityContextMenu");
    }
    
    ImGui::PopStyleColor(3);
    
    // Distance column
    ImGui::TableSetColumnIndex(1);
    ImGui::Text("%s", FormatDistance(entry.distance).c_str());
    
    // Type column
    ImGui::TableSetColumnIndex(2);
    ImGui::Text("%s", entry.ship_type.c_str());
    
    // Corporation column
    ImGui::TableSetColumnIndex(3);
    ImGui::Text("%s", entry.corporation.c_str());
    
    // Standing column
    ImGui::TableSetColumnIndex(4);
    if (entry.standing > 0) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.6f, 1.0f, 1.0f)); // Blue (EVE style)
        ImGui::Text("+%d", entry.standing);
        ImGui::PopStyleColor();
    } else if (entry.standing < 0) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.2f, 0.2f, 1.0f)); // Red
        ImGui::Text("%d", entry.standing);
        ImGui::PopStyleColor();
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.7f, 0.7f, 1.0f)); // Grey
        ImGui::Text("0");
        ImGui::PopStyleColor();
    }
    
    // Context menu (enhanced)
    if (ImGui::BeginPopup("EntityContextMenu")) {
        // Approach
        if (ImGui::MenuItem("Approach")) {
            if (m_onApproach) {
                m_onApproach(entry.entity_id);
            }
        }
        
        // Orbit with submenu
        if (ImGui::BeginMenu("Orbit")) {
            if (ImGui::MenuItem("500m")) {
                if (m_onOrbit) {
                    m_onOrbit(entry.entity_id, 500);
                }
            }
            if (ImGui::MenuItem("1km")) {
                if (m_onOrbit) {
                    m_onOrbit(entry.entity_id, 1000);
                }
            }
            if (ImGui::MenuItem("5km")) {
                if (m_onOrbit) {
                    m_onOrbit(entry.entity_id, 5000);
                }
            }
            if (ImGui::MenuItem("10km")) {
                if (m_onOrbit) {
                    m_onOrbit(entry.entity_id, 10000);
                }
            }
            if (ImGui::MenuItem("20km")) {
                if (m_onOrbit) {
                    m_onOrbit(entry.entity_id, 20000);
                }
            }
            if (ImGui::MenuItem("50km")) {
                if (m_onOrbit) {
                    m_onOrbit(entry.entity_id, 50000);
                }
            }
            ImGui::EndMenu();
        }
        
        // Keep at Range with submenu
        if (ImGui::BeginMenu("Keep at Range")) {
            if (ImGui::MenuItem("1km")) {
                if (m_onKeepAtRange) {
                    m_onKeepAtRange(entry.entity_id, 1000);
                }
            }
            if (ImGui::MenuItem("5km")) {
                if (m_onKeepAtRange) {
                    m_onKeepAtRange(entry.entity_id, 5000);
                }
            }
            if (ImGui::MenuItem("10km")) {
                if (m_onKeepAtRange) {
                    m_onKeepAtRange(entry.entity_id, 10000);
                }
            }
            if (ImGui::MenuItem("20km")) {
                if (m_onKeepAtRange) {
                    m_onKeepAtRange(entry.entity_id, 20000);
                }
            }
            if (ImGui::MenuItem("50km")) {
                if (m_onKeepAtRange) {
                    m_onKeepAtRange(entry.entity_id, 50000);
                }
            }
            ImGui::EndMenu();
        }
        
        ImGui::Separator();
        
        // Warp To with submenu
        if (ImGui::BeginMenu("Warp To")) {
            if (ImGui::MenuItem("At 0km")) {
                if (m_onWarpTo) {
                    m_onWarpTo(entry.entity_id, 0);
                }
            }
            if (ImGui::MenuItem("At 10km")) {
                if (m_onWarpTo) {
                    m_onWarpTo(entry.entity_id, 10000);
                }
            }
            if (ImGui::MenuItem("At 50km")) {
                if (m_onWarpTo) {
                    m_onWarpTo(entry.entity_id, 50000);
                }
            }
            if (ImGui::MenuItem("At 100km")) {
                if (m_onWarpTo) {
                    m_onWarpTo(entry.entity_id, 100000);
                }
            }
            ImGui::EndMenu();
        }
        
        ImGui::Separator();
        
        // Lock Target
        if (ImGui::MenuItem("Lock Target")) {
            if (m_onLockTarget) {
                m_onLockTarget(entry.entity_id);
            }
        }
        
        // Look At
        if (ImGui::MenuItem("Look At")) {
            if (m_onLookAt) {
                m_onLookAt(entry.entity_id);
            }
        }
        
        // Show Info
        if (ImGui::MenuItem("Show Info")) {
            if (m_onShowInfo) {
                m_onShowInfo(entry.entity_id);
            }
        }
        
        ImGui::Separator();
        
        // Cancel
        if (ImGui::MenuItem("Cancel")) {
            // Just close the menu
        }
        
        ImGui::EndPopup();
    }
}

void OverviewPanel::UpdateEntities(const std::unordered_map<std::string, std::shared_ptr<::eve::Entity>>& entities,
                                   const glm::vec3& playerPosition) {
    m_entries.clear();
    
    for (const auto& [id, entity] : entities) {
        if (!entity) continue;
        
        OverviewEntry entry;
        entry.entity_id = id;
        
        // Get entity data from client-side Entity
        entry.name = entity->getShipName();
        if (entry.name.empty()) {
            entry.name = "Unknown";
        }
        
        entry.ship_type = entity->getShipType();
        if (entry.ship_type.empty()) {
            entry.ship_type = "Unknown";
        }
        
        // Calculate distance from player position
        glm::vec3 pos = entity->getPosition();
        glm::vec3 delta = pos - playerPosition;
        entry.distance = std::sqrt(delta.x * delta.x + delta.y * delta.y + delta.z * delta.z);
        
        // Get health percentages
        const eve::Health& health = entity->getHealth();
        entry.shield_percent = (health.maxShield > 0) ? 
            (static_cast<float>(health.shield) / health.maxShield) : 0.0f;
        entry.armor_percent = (health.maxArmor > 0) ? 
            (static_cast<float>(health.armor) / health.maxArmor) : 0.0f;
        entry.hull_percent = (health.maxHull > 0) ? 
            (static_cast<float>(health.hull) / health.maxHull) : 0.0f;
        
        // Get faction/standing
        entry.corporation = entity->getFaction();
        if (entry.corporation.empty()) {
            entry.corporation = "Unknown";
        }
        
        // Determine if entity is a player (players have IDs starting with "player_")
        entry.is_player = (entry.entity_id.find("player_") == 0);
        
        // Calculate standing based on faction
        entry.standing = CalculateStanding(entry.corporation, entry.is_player);
        
        m_entries.push_back(entry);
    }
    
    // Apply filter and sort
    ApplyFilter();
    SortEntries();
}

int OverviewPanel::CalculateStanding(const std::string& faction, bool is_player) const {
    // Players are neutral by default
    if (is_player) {
        return 0;
    }
    
    // Pirate factions are hostile
    if (faction == "Serpentis" || faction == "Guristas" || 
        faction == "Blood Raiders" || faction == "Sansha's Nation" ||
        faction == "Angel Cartel" || faction == "Mordu's Legion") {
        return -5;
    }
    
    // Rogue drones are hostile
    if (faction == "Rogue Drones") {
        return -5;
    }
    
    // Empire factions and corporations are neutral by default
    // In a full implementation, this would check player's standings with the faction
    if (faction == "Caldari" || faction == "Gallente" || 
        faction == "Amarr" || faction == "Minmatar") {
        return 0;
    }
    
    // CONCORD and other friendly factions
    if (faction == "CONCORD" || faction == "ORE" || faction == "Sisters of EVE") {
        return 5;
    }
    
    // Unknown/neutral by default
    return 0;
}

void OverviewPanel::ApplyFilter() {
    m_filteredEntries.clear();
    
    for (const auto& entry : m_entries) {
        // Check standing filter
        bool showByStanding = false;
        if (entry.standing < 0 && m_currentFilter.show_hostile) {
            showByStanding = true;
        } else if (entry.standing > 0 && m_currentFilter.show_friendly) {
            showByStanding = true;
        } else if (entry.standing == 0 && m_currentFilter.show_neutral) {
            showByStanding = true;
        }
        
        if (!showByStanding) continue;
        
        // Check player/NPC filter
        if (entry.is_player && !m_currentFilter.show_players) continue;
        if (!entry.is_player && !m_currentFilter.show_npcs) continue;
        
        // Check distance filter
        if (m_currentFilter.max_distance_km > 0.0f) {
            float distance_km = entry.distance / 1000.0f;
            if (distance_km > m_currentFilter.max_distance_km) continue;
        }
        
        // Check ship type filter
        if (!m_currentFilter.show_ship_types.empty()) {
            bool typeMatch = false;
            for (const auto& type : m_currentFilter.show_ship_types) {
                if (entry.ship_type.find(type) != std::string::npos) {
                    typeMatch = true;
                    break;
                }
            }
            if (!typeMatch) continue;
        }
        
        m_filteredEntries.push_back(entry);
    }
}

void OverviewPanel::SortEntries() {
    if (m_sortColumn == OverviewSortColumn::NONE) return;
    
    std::sort(m_filteredEntries.begin(), m_filteredEntries.end(), 
        [this](const OverviewEntry& a, const OverviewEntry& b) {
            bool less = false;
            
            switch (m_sortColumn) {
                case OverviewSortColumn::NAME:
                    less = a.name < b.name;
                    break;
                case OverviewSortColumn::DISTANCE:
                    less = a.distance < b.distance;
                    break;
                case OverviewSortColumn::TYPE:
                    less = a.ship_type < b.ship_type;
                    break;
                case OverviewSortColumn::CORPORATION:
                    less = a.corporation < b.corporation;
                    break;
                case OverviewSortColumn::STANDING:
                    less = a.standing < b.standing;
                    break;
                default:
                    break;
            }
            
            return m_sortAscending ? less : !less;
        });
}

std::string OverviewPanel::FormatDistance(float meters) const {
    if (meters < 1000.0f) {
        // Format as meters
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(0) << meters << " m";
        return oss.str();
    } else if (meters < 1000000.0f) {
        // Format as kilometers
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2) << (meters / 1000.0f) << " km";
        return oss.str();
    } else {
        // Format as AU (1 AU = 149,597,870,700 meters, simplified to 150M for games)
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2) << (meters / 150000000.0f) << " AU";
        return oss.str();
    }
}

void OverviewPanel::GetStandingColor(int standing, float color[4]) const {
    if (standing < 0) {
        // Hostile - red
        color[0] = 0.8f; color[1] = 0.2f; color[2] = 0.2f; color[3] = 1.0f;
    } else if (standing > 0) {
        // Friendly - blue (EVE uses blue for friendlies)
        color[0] = 0.2f; color[1] = 0.6f; color[2] = 1.0f; color[3] = 1.0f;
    } else {
        // Neutral - grey
        color[0] = 0.667f; color[1] = 0.667f; color[2] = 0.667f; color[3] = 1.0f;
    }
}

void OverviewPanel::SetFilter(const OverviewFilter& filter) {
    m_currentFilter = filter;
    ApplyFilter();
    SortEntries();
}

void OverviewPanel::AddFilter(const std::string& name, const OverviewFilter& filter) {
    OverviewFilter newFilter = filter;
    newFilter.name = name;
    m_savedFilters[name] = newFilter;
}

void OverviewPanel::SelectFilter(const std::string& name) {
    auto it = m_savedFilters.find(name);
    if (it != m_savedFilters.end()) {
        SetFilter(it->second);
    }
}

void OverviewPanel::SetSortColumn(OverviewSortColumn column, bool ascending) {
    m_sortColumn = column;
    m_sortAscending = ascending;
    SortEntries();
}

void OverviewPanel::RenderContents() {
    // Render filter tabs
    RenderFilterTabs();
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    // Render entity table
    if (ImGui::BeginTable("OverviewTable", 5, 
                         ImGuiTableFlags_Borders | 
                         ImGuiTableFlags_RowBg | 
                         ImGuiTableFlags_Resizable |
                         ImGuiTableFlags_Sortable |
                         ImGuiTableFlags_ScrollY |
                         ImGuiTableFlags_SizingStretchProp)) {
        
        RenderTableHeader();
        
        // Render rows
        for (size_t i = 0; i < m_filteredEntries.size(); i++) {
            RenderEntityRow(m_filteredEntries[i], static_cast<int>(i));
        }
        
        ImGui::EndTable();
    }
}

} // namespace UI
