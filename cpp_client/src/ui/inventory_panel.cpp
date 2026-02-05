#include "ui/inventory_panel.h"
#include <imgui.h>
#include <algorithm>

namespace UI {

InventoryPanel::InventoryPanel()
    : m_visible(false)
    , m_viewMode(0)  // 0 = cargo
    , m_selectedItem(-1)
{
}

void InventoryPanel::Render() {
    if (!m_visible) return;
    
    // Set window size and position
    ImGui::SetNextWindowSize(ImVec2(600, 500), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(100, 100), ImGuiCond_FirstUseEver);
    
    // EVE-style window flags
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse;
    
    if (!ImGui::Begin("Inventory", &m_visible, flags)) {
        ImGui::End();
        return;
    }
    
    // Render UI elements
    RenderViewButtons();
    ImGui::Spacing();
    RenderCapacityDisplay();
    ImGui::Separator();
    ImGui::Spacing();
    RenderItemList();
    ImGui::Spacing();
    ImGui::Separator();
    RenderActionButtons();
    
    ImGui::End();
}

void InventoryPanel::SetInventoryData(const InventoryData& data) {
    m_data = data;
}

void InventoryPanel::RenderViewButtons() {
    // Cargo button
    if (m_viewMode == 0) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.8f, 0.8f));
    } else {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.15f, 0.2f, 0.8f));
    }
    
    if (ImGui::Button("Cargo Hold", ImVec2(150, 30))) {
        m_viewMode = 0;
        m_selectedItem = -1;
    }
    ImGui::PopStyleColor();
    
    ImGui::SameLine();
    
    // Hangar button
    if (m_viewMode == 1) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.8f, 0.8f));
    } else {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.15f, 0.2f, 0.8f));
    }
    
    if (ImGui::Button("Station Hangar", ImVec2(150, 30))) {
        m_viewMode = 1;
        m_selectedItem = -1;
    }
    ImGui::PopStyleColor();
}

void InventoryPanel::RenderCapacityDisplay() {
    float used = GetCurrentUsed();
    float capacity = GetCurrentCapacity();
    float percent = capacity > 0.0f ? (used / capacity) * 100.0f : 0.0f;
    
    // Right-aligned capacity text
    ImGui::Text("Capacity:");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.7f, 0.75f, 0.8f, 1.0f), "%.1f / %.1f m³ (%.1f%%)", 
                       used, capacity, percent);
    
    // Capacity bar
    ImGui::ProgressBar(used / capacity, ImVec2(-1.0f, 0.0f));
}

void InventoryPanel::RenderItemList() {
    const auto& items = GetCurrentItems();
    
    // Create a scrollable list
    ImGui::BeginChild("ItemList", ImVec2(0, -50), true);
    
    // Header
    ImGui::Columns(4, "ItemColumns");
    ImGui::Separator();
    ImGui::Text("Name"); ImGui::NextColumn();
    ImGui::Text("Type"); ImGui::NextColumn();
    ImGui::Text("Quantity"); ImGui::NextColumn();
    ImGui::Text("Volume"); ImGui::NextColumn();
    ImGui::Separator();
    
    // Items
    for (size_t i = 0; i < items.size(); ++i) {
        RenderItemRow(items[i], static_cast<int>(i), m_selectedItem == static_cast<int>(i));
    }
    
    ImGui::Columns(1);
    ImGui::EndChild();
}

void InventoryPanel::RenderItemRow(const InventoryItem& item, int index, bool selected) {
    // Make row selectable
    ImGuiSelectableFlags flags = ImGuiSelectableFlags_SpanAllColumns;
    
    if (ImGui::Selectable(("##item" + std::to_string(index)).c_str(), selected, flags)) {
        m_selectedItem = index;
    }
    
    ImGui::SameLine();
    ImGui::Text("%s", item.name.c_str()); ImGui::NextColumn();
    ImGui::Text("%s", item.type.c_str()); ImGui::NextColumn();
    ImGui::Text("%d", item.quantity); ImGui::NextColumn();
    ImGui::Text("%.2f m³", item.volume * item.quantity); ImGui::NextColumn();
}

void InventoryPanel::RenderActionButtons() {
    // Only enable buttons if an item is selected
    bool hasSelection = m_selectedItem >= 0 && 
                        m_selectedItem < static_cast<int>(GetCurrentItems().size());
    
    if (!hasSelection) {
        ImGui::BeginDisabled();
    }
    
    // Transfer button
    if (ImGui::Button("Transfer", ImVec2(120, 30))) {
        if (m_onTransfer && hasSelection) {
            const auto& items = GetCurrentItems();
            bool toHangar = (m_viewMode == 0);  // If viewing cargo, transfer to hangar
            m_onTransfer(items[m_selectedItem].item_id, toHangar);
        }
    }
    
    ImGui::SameLine();
    
    // Jettison button (only available for cargo)
    if (m_viewMode == 0) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.2f, 0.1f, 0.8f));
        if (ImGui::Button("Jettison", ImVec2(120, 30))) {
            if (m_onJettison && hasSelection) {
                const auto& items = GetCurrentItems();
                m_onJettison(items[m_selectedItem].item_id, items[m_selectedItem].quantity);
            }
        }
        ImGui::PopStyleColor();
    }
    
    if (!hasSelection) {
        ImGui::EndDisabled();
    }
}

const std::vector<InventoryItem>& InventoryPanel::GetCurrentItems() const {
    return m_viewMode == 0 ? m_data.cargo_items : m_data.hangar_items;
}

float InventoryPanel::GetCurrentCapacity() const {
    return m_viewMode == 0 ? m_data.cargo_capacity : m_data.hangar_capacity;
}

float InventoryPanel::GetCurrentUsed() const {
    return m_viewMode == 0 ? m_data.cargo_used : m_data.hangar_used;
}

} // namespace UI
