#include "ui/module_browser_panel.h"
#include <imgui.h>
#include <algorithm>
#include <cstring>

namespace UI {

ModuleBrowserPanel::ModuleBrowserPanel()
    : m_visible(false)
    , m_selectedCategory("All")
    , m_selectedSlotType("All")
    , m_sortMode(0)
    , m_selectedIndex(-1)
{
    memset(m_searchBuffer, 0, sizeof(m_searchBuffer));
}

void ModuleBrowserPanel::Render() {
    if (!m_visible) return;
    
    // Set window size and position
    ImGui::SetNextWindowSize(ImVec2(900, 600), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(200, 100), ImGuiCond_FirstUseEver);
    
    // EVE-style window flags
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse;
    
    if (!ImGui::Begin("Module Browser", &m_visible, flags)) {
        ImGui::End();
        return;
    }
    
    // Search bar at top
    RenderSearchBar();
    ImGui::Spacing();
    
    // Filters
    RenderFilters();
    ImGui::Separator();
    ImGui::Spacing();
    
    // Split view: list on left, details on right
    ImGui::BeginChild("BrowserContent", ImVec2(0, 0), false);
    
    // Module list (left side, 60%)
    ImGui::BeginChild("ModuleList", ImVec2(ImGui::GetContentRegionAvail().x * 0.6f, 0), true);
    RenderModuleList();
    ImGui::EndChild();
    
    ImGui::SameLine();
    
    // Module details (right side, 40%)
    ImGui::BeginChild("ModuleDetails", ImVec2(0, 0), true);
    RenderModuleDetails();
    ImGui::EndChild();
    
    ImGui::EndChild();
    
    ImGui::End();
}

void ModuleBrowserPanel::SetModules(const std::vector<ModuleBrowserEntry>& modules) {
    m_modules = modules;
    ApplyFilters();
}

void ModuleBrowserPanel::AddModule(const ModuleBrowserEntry& module) {
    m_modules.push_back(module);
    ApplyFilters();
}

void ModuleBrowserPanel::ClearModules() {
    m_modules.clear();
    m_filteredModules.clear();
    m_selectedIndex = -1;
}

void ModuleBrowserPanel::RenderSearchBar() {
    ImGui::Text("Search:");
    ImGui::SameLine();
    
    if (ImGui::InputText("##search", m_searchBuffer, sizeof(m_searchBuffer))) {
        ApplyFilters();
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Clear")) {
        memset(m_searchBuffer, 0, sizeof(m_searchBuffer));
        ApplyFilters();
    }
}

void ModuleBrowserPanel::RenderFilters() {
    ImGui::Text("Filters:");
    ImGui::SameLine();
    
    // Category filter
    auto categories = GetCategories();
    if (ImGui::BeginCombo("Category", m_selectedCategory.c_str())) {
        for (const auto& cat : categories) {
            bool is_selected = (m_selectedCategory == cat);
            if (ImGui::Selectable(cat.c_str(), is_selected)) {
                m_selectedCategory = cat;
                ApplyFilters();
            }
            if (is_selected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
    
    ImGui::SameLine();
    
    // Slot type filter
    auto slots = GetSlotTypes();
    if (ImGui::BeginCombo("Slot", m_selectedSlotType.c_str())) {
        for (const auto& slot : slots) {
            bool is_selected = (m_selectedSlotType == slot);
            if (ImGui::Selectable(slot.c_str(), is_selected)) {
                m_selectedSlotType = slot;
                ApplyFilters();
            }
            if (is_selected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
    
    ImGui::SameLine();
    
    // Sort options
    const char* sort_options[] = { "Name", "CPU", "Powergrid", "Meta Level" };
    if (ImGui::Combo("Sort", &m_sortMode, sort_options, 4)) {
        SortModules();
    }
    
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), 
                       "(%zu modules)", m_filteredModules.size());
}

void ModuleBrowserPanel::RenderModuleList() {
    // Header
    ImGui::Columns(4, "ModuleColumns");
    ImGui::Separator();
    ImGui::Text("Name"); ImGui::NextColumn();
    ImGui::Text("Category"); ImGui::NextColumn();
    ImGui::Text("CPU"); ImGui::NextColumn();
    ImGui::Text("PG"); ImGui::NextColumn();
    ImGui::Separator();
    
    // Module rows
    for (size_t i = 0; i < m_filteredModules.size(); ++i) {
        RenderModuleRow(m_filteredModules[i], static_cast<int>(i), 
                       m_selectedIndex == static_cast<int>(i));
    }
    
    ImGui::Columns(1);
}

void ModuleBrowserPanel::RenderModuleRow(const ModuleBrowserEntry& module, int index, bool selected) {
    ImGuiSelectableFlags flags = ImGuiSelectableFlags_SpanAllColumns;
    
    if (ImGui::Selectable(("##module" + std::to_string(index)).c_str(), selected, flags)) {
        m_selectedIndex = index;
        if (m_onBrowseModule) {
            m_onBrowseModule(module.module_id);
        }
    }
    
    // Double-click to fit
    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
        if (m_onFitModule) {
            m_onFitModule(module.module_id);
        }
    }
    
    ImGui::SameLine();
    ImGui::Text("%s", module.name.c_str()); ImGui::NextColumn();
    ImGui::Text("%s", module.category.c_str()); ImGui::NextColumn();
    ImGui::Text("%.1f", module.cpu_cost); ImGui::NextColumn();
    ImGui::Text("%.1f", module.powergrid_cost); ImGui::NextColumn();
}

void ModuleBrowserPanel::RenderModuleDetails() {
    if (m_selectedIndex < 0 || m_selectedIndex >= static_cast<int>(m_filteredModules.size())) {
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Select a module to view details");
        return;
    }
    
    const auto& module = m_filteredModules[m_selectedIndex];
    
    // Module name (header)
    ImGui::TextColored(ImVec4(0.2f, 0.8f, 1.0f, 1.0f), "%s", module.name.c_str());
    ImGui::Separator();
    ImGui::Spacing();
    
    // Basic info
    ImGui::Text("Category: %s", module.category.c_str());
    ImGui::Text("Type: %s", module.type.c_str());
    ImGui::Text("Slot: %s", module.slot_type.c_str());
    ImGui::Spacing();
    
    // Resource costs
    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "Resource Requirements:");
    ImGui::Text("  CPU: %.1f tf", module.cpu_cost);
    ImGui::Text("  Powergrid: %.1f MW", module.powergrid_cost);
    ImGui::Spacing();
    
    // Module stats (if available)
    bool hasStats = false;
    if (module.damage > 0.0f) {
        if (!hasStats) {
            ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.4f, 1.0f), "Module Stats:");
            hasStats = true;
        }
        ImGui::Text("  Damage: %.1f", module.damage);
    }
    if (module.shield_hp > 0.0f) {
        if (!hasStats) {
            ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.4f, 1.0f), "Module Stats:");
            hasStats = true;
        }
        ImGui::Text("  Shield HP: %.1f", module.shield_hp);
    }
    if (module.armor_hp > 0.0f) {
        if (!hasStats) {
            ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.4f, 1.0f), "Module Stats:");
            hasStats = true;
        }
        ImGui::Text("  Armor HP: %.1f", module.armor_hp);
    }
    if (module.speed_bonus > 0.0f) {
        if (!hasStats) {
            ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.4f, 1.0f), "Module Stats:");
            hasStats = true;
        }
        ImGui::Text("  Speed Bonus: +%.1f%%", module.speed_bonus);
    }
    if (module.capacitor_use > 0.0f) {
        if (!hasStats) {
            ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.4f, 1.0f), "Module Stats:");
            hasStats = true;
        }
        ImGui::Text("  Capacitor Use: %.1f GJ", module.capacitor_use);
    }
    if (module.activation_time > 0.0f) {
        if (!hasStats) {
            ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.4f, 1.0f), "Module Stats:");
            hasStats = true;
        }
        ImGui::Text("  Activation Time: %.1fs", module.activation_time);
    }
    
    if (hasStats) {
        ImGui::Spacing();
    }
    
    // Description
    if (!module.description.empty()) {
        ImGui::TextWrapped("%s", module.description.c_str());
        ImGui::Spacing();
    }
    
    // Action buttons
    ImGui::Separator();
    ImGui::Spacing();
    
    if (ImGui::Button("Fit to Ship", ImVec2(-1, 30))) {
        if (m_onFitModule) {
            m_onFitModule(module.module_id);
        }
    }
}

void ModuleBrowserPanel::ApplyFilters() {
    m_filteredModules.clear();
    std::string search = m_searchBuffer;
    std::transform(search.begin(), search.end(), search.begin(), ::tolower);
    
    for (const auto& module : m_modules) {
        // Category filter
        if (m_selectedCategory != "All" && module.category != m_selectedCategory) {
            continue;
        }
        
        // Slot type filter
        if (m_selectedSlotType != "All" && module.slot_type != m_selectedSlotType) {
            continue;
        }
        
        // Search filter
        if (!search.empty()) {
            std::string name = module.name;
            std::string desc = module.description;
            std::transform(name.begin(), name.end(), name.begin(), ::tolower);
            std::transform(desc.begin(), desc.end(), desc.begin(), ::tolower);
            
            if (name.find(search) == std::string::npos && 
                desc.find(search) == std::string::npos) {
                continue;
            }
        }
        
        m_filteredModules.push_back(module);
    }
    
    SortModules();
    m_selectedIndex = -1;
}

void ModuleBrowserPanel::SortModules() {
    switch (m_sortMode) {
        case 0: // Name
            std::sort(m_filteredModules.begin(), m_filteredModules.end(),
                     [](const ModuleBrowserEntry& a, const ModuleBrowserEntry& b) {
                         return a.name < b.name;
                     });
            break;
        case 1: // CPU
            std::sort(m_filteredModules.begin(), m_filteredModules.end(),
                     [](const ModuleBrowserEntry& a, const ModuleBrowserEntry& b) {
                         return a.cpu_cost < b.cpu_cost;
                     });
            break;
        case 2: // Powergrid
            std::sort(m_filteredModules.begin(), m_filteredModules.end(),
                     [](const ModuleBrowserEntry& a, const ModuleBrowserEntry& b) {
                         return a.powergrid_cost < b.powergrid_cost;
                     });
            break;
        case 3: // Meta level
            std::sort(m_filteredModules.begin(), m_filteredModules.end(),
                     [](const ModuleBrowserEntry& a, const ModuleBrowserEntry& b) {
                         return a.meta_level < b.meta_level;
                     });
            break;
    }
}

std::vector<std::string> ModuleBrowserPanel::GetCategories() const {
    std::vector<std::string> categories = { "All" };
    for (const auto& module : m_modules) {
        if (std::find(categories.begin(), categories.end(), module.category) == categories.end()) {
            categories.push_back(module.category);
        }
    }
    return categories;
}

std::vector<std::string> ModuleBrowserPanel::GetSlotTypes() const {
    std::vector<std::string> slots = { "All", "high", "mid", "low", "rig" };
    return slots;
}

} // namespace UI
