#ifndef INVENTORY_PANEL_H
#define INVENTORY_PANEL_H

#include <string>
#include <vector>
#include <functional>

namespace UI {

// Item in inventory
struct InventoryItem {
    std::string item_id;
    std::string name;
    int quantity;
    float volume;  // m³ per unit
    std::string type;  // module, ore, mineral, etc.
    std::string category;  // weapon, armor, mining, etc.
    
    InventoryItem() = default;
    InventoryItem(const std::string& id, const std::string& n, int qty, float vol, 
                  const std::string& t = "", const std::string& cat = "")
        : item_id(id), name(n), quantity(qty), volume(vol), type(t), category(cat) {}
};

// Inventory data structure
struct InventoryData {
    std::vector<InventoryItem> cargo_items;
    std::vector<InventoryItem> hangar_items;
    float cargo_capacity = 100.0f;
    float cargo_used = 0.0f;
    float hangar_capacity = 10000.0f;
    float hangar_used = 0.0f;
};

// Callback types
using TransferItemCallback = std::function<void(const std::string& item_id, bool to_hangar)>;
using JettisonItemCallback = std::function<void(const std::string& item_id, int quantity)>;
using DragDropCallback = std::function<void(const std::string& item_id, int quantity, bool from_cargo, bool to_cargo, bool to_space)>;

class InventoryPanel {
public:
    InventoryPanel();
    ~InventoryPanel() = default;
    
    // Render the inventory panel
    void Render();
    
    // Render just the panel contents (no Begin/End) — used by docking manager
    void RenderContents();
    
    // Update inventory data
    void SetInventoryData(const InventoryData& data);
    
    // Visibility
    void SetVisible(bool visible) { m_visible = visible; }
    bool IsVisible() const { return m_visible; }
    
    // Callbacks
    void SetTransferCallback(TransferItemCallback callback) { m_onTransfer = callback; }
    void SetJettisonCallback(JettisonItemCallback callback) { m_onJettison = callback; }
    void SetDragDropCallback(DragDropCallback callback) { m_onDragDrop = callback; }
    
    // Drag-and-drop settings
    void SetDragDropEnabled(bool enabled) { m_dragDropEnabled = enabled; }
    
    // Response feedback methods
    void ShowSuccess(const std::string& message);
    void ShowError(const std::string& message);
    void SetPendingOperation(bool pending) { m_pendingOperation = pending; }
    
private:
    bool m_visible;
    InventoryData m_data;
    
    // View mode: 0 = cargo, 1 = hangar
    int m_viewMode;
    
    // Selected item index (-1 if none)
    int m_selectedItem;
    
    // Callbacks
    TransferItemCallback m_onTransfer;
    JettisonItemCallback m_onJettison;
    DragDropCallback m_onDragDrop;
    
    // Drag-and-drop state
    bool m_dragDropEnabled;
    int m_draggedItemIndex;
    bool m_dragFromCargo;
    
    // Response feedback state
    bool m_pendingOperation;
    std::string m_feedbackMessage;
    bool m_feedbackIsError;
    float m_feedbackTimer;
    
    // Helper functions
    void RenderViewButtons();
    void RenderCapacityDisplay();
    void RenderItemList();
    void RenderActionButtons();
    void RenderItemRow(const InventoryItem& item, int index, bool selected);
    void HandleDragSource(const InventoryItem& item, int index);
    void HandleDropTarget(bool is_cargo_view);
    void RenderJettisonDropZone();
    
    const std::vector<InventoryItem>& GetCurrentItems() const;
    float GetCurrentCapacity() const;
    float GetCurrentUsed() const;
};

} // namespace UI

#endif // INVENTORY_PANEL_H
