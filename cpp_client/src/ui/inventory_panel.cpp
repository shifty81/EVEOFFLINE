#include "ui/inventory_panel.h"
#include <imgui.h>
#include <algorithm>
#include <cstring>  // for strncpy

namespace UI {

InventoryPanel::InventoryPanel()
    : m_visible(false)
    , m_viewMode(0)  // 0 = cargo
    , m_selectedItem(-1)
    , m_dragDropEnabled(true)
    , m_draggedItemIndex(-1)
    , m_dragFromCargo(false)
    , m_pendingOperation(false)
    , m_feedbackIsError(false)
    , m_feedbackTimer(0.0f)
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
    
    // Jettison drop zone (only for cargo view and when drag-drop enabled)
    if (m_dragDropEnabled && m_viewMode == 0) {
        RenderJettisonDropZone();
    }
    
    RenderItemList();
    ImGui::Spacing();
    ImGui::Separator();
    RenderActionButtons();
    
    // Render feedback message if active
    if (m_feedbackTimer > 0.0f) {
        ImGui::Spacing();
        ImGui::Separator();
        ImVec4 color = m_feedbackIsError ? ImVec4(1.0f, 0.3f, 0.3f, 1.0f) : ImVec4(0.3f, 1.0f, 0.3f, 1.0f);
        ImGui::PushStyleColor(ImGuiCol_Text, color);
        ImGui::TextWrapped("%s %s", m_feedbackIsError ? "✗" : "✓", m_feedbackMessage.c_str());
        ImGui::PopStyleColor();
        m_feedbackTimer -= ImGui::GetIO().DeltaTime;
    }
    
    // Show pending operation indicator
    if (m_pendingOperation) {
        ImGui::Spacing();
        ImGui::Text("⏳ Operation in progress...");
    }
    
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
    
    // Handle drop target for the list area
    if (m_dragDropEnabled) {
        HandleDropTarget(m_viewMode == 0);
    }
    
    ImGui::EndChild();
}

void InventoryPanel::RenderItemRow(const InventoryItem& item, int index, bool selected) {
    // Make row selectable
    ImGuiSelectableFlags flags = ImGuiSelectableFlags_SpanAllColumns;
    
    if (ImGui::Selectable(("##item" + std::to_string(index)).c_str(), selected, flags)) {
        m_selectedItem = index;
    }
    
    // Add drag-and-drop source
    if (m_dragDropEnabled) {
        HandleDragSource(item, index);
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

void InventoryPanel::HandleDragSource(const InventoryItem& item, int index) {
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
        // Store drag data
        m_draggedItemIndex = index;
        m_dragFromCargo = (m_viewMode == 0);
        
        // Set payload (item_id and quantity)
        struct DragPayload {
            char item_id[64];
            int quantity;
            bool from_cargo;
        };
        
        DragPayload payload;
        strncpy(payload.item_id, item.item_id.c_str(), sizeof(payload.item_id) - 1);
        payload.item_id[sizeof(payload.item_id) - 1] = '\0';
        payload.quantity = item.quantity;
        payload.from_cargo = m_dragFromCargo;
        
        ImGui::SetDragDropPayload("INVENTORY_ITEM", &payload, sizeof(payload));
        
        // Drag preview
        ImGui::Text("🔹 %s (x%d)", item.name.c_str(), item.quantity);
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "%.2f m³", item.volume * item.quantity);
        
        ImGui::EndDragDropSource();
    }
}

void InventoryPanel::HandleDropTarget(bool is_cargo_view) {
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("INVENTORY_ITEM")) {
            struct DragPayload {
                char item_id[64];
                int quantity;
                bool from_cargo;
            };
            
            if (payload->DataSize == sizeof(DragPayload)) {
                const DragPayload* data = static_cast<const DragPayload*>(payload->Data);
                
                // Only transfer if dropping to different view
                if (data->from_cargo != is_cargo_view) {
                    if (m_onDragDrop) {
                        m_onDragDrop(data->item_id, data->quantity, data->from_cargo, is_cargo_view, false);
                    }
                }
            }
        }
        ImGui::EndDragDropTarget();
    }
}

void InventoryPanel::RenderJettisonDropZone() {
    // Create a colored drop zone for jettisoning items
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.2f, 0.1f, 0.3f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.6f, 0.3f, 0.2f, 0.5f));
    ImGui::Button("⚠️ Jettison Zone - Drop items here to jettison into space", ImVec2(-1, 40));
    ImGui::PopStyleColor(2);
    
    // Handle drop target
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("INVENTORY_ITEM")) {
            struct DragPayload {
                char item_id[64];
                int quantity;
                bool from_cargo;
            };
            
            if (payload->DataSize == sizeof(DragPayload)) {
                const DragPayload* data = static_cast<const DragPayload*>(payload->Data);
                
                // Only jettison from cargo
                if (data->from_cargo) {
                    if (m_onDragDrop) {
                        m_onDragDrop(data->item_id, data->quantity, true, false, true);
                    }
                }
            }
        }
        ImGui::EndDragDropTarget();
    }
    
    ImGui::Spacing();
}

void InventoryPanel::ShowSuccess(const std::string& message) {
    m_feedbackMessage = message;
    m_feedbackIsError = false;
    m_feedbackTimer = 3.0f;  // Show for 3 seconds
    m_pendingOperation = false;
}

void InventoryPanel::ShowError(const std::string& message) {
    m_feedbackMessage = message;
    m_feedbackIsError = true;
    m_feedbackTimer = 5.0f;  // Show errors longer
    m_pendingOperation = false;
}

} // namespace UI
