"""
Inventory Management UI Panel
Displays ship cargo hold and station hangars
"""

from direct.gui.DirectGui import DirectButton, DirectLabel
from panda3d.core import TextNode, TransparencyAttrib
from typing import Dict, Optional, Callable
from .base_panel import EVEListPanel


class InventoryPanel(EVEListPanel):
    """
    Inventory management panel
    Shows cargo hold, hangar, and allows item management
    """
    
    def __init__(self, parent, pos=(0.1, 0.3), size=(0.6, 0.7)):
        """Initialize inventory panel"""
        super().__init__(
            parent=parent,
            title="Inventory",
            pos=pos,
            size=size,
            closeable=True,
            draggable=True
        )
        
        # Current inventory view mode
        self.view_mode = "cargo"  # "cargo" or "hangar"
        
        # Callbacks for inventory actions
        self.on_transfer_item: Optional[Callable] = None
        self.on_jettison_item: Optional[Callable] = None
        
        # Create additional UI elements
        self._create_inventory_ui()
        
        print("[InventoryPanel] Inventory panel initialized")
    
    def _create_inventory_ui(self):
        """Create inventory-specific UI elements"""
        # View mode buttons
        self._create_view_buttons()
        
        # Capacity display
        self._create_capacity_display()
        
        # Action buttons
        self._create_action_buttons()
    
    def _create_view_buttons(self):
        """Create buttons to switch between cargo/hangar views"""
        button_width = 0.15
        button_height = 0.05
        
        # Cargo button
        self.cargo_button = DirectButton(
            text="Cargo Hold",
            text_scale=0.035,
            text_fg=(0.9, 0.95, 1.0, 1.0),
            frameColor=(0.2, 0.6, 0.8, 0.8),
            frameSize=(-button_width/2, button_width/2, -button_height/2, button_height/2),
            pos=(0.15, 0, -0.025),
            command=self._switch_to_cargo,
            parent=self.content_frame
        )
        self.cargo_button.setTransparency(TransparencyAttrib.MAlpha)
        
        # Hangar button
        self.hangar_button = DirectButton(
            text="Station Hangar",
            text_scale=0.035,
            text_fg=(0.9, 0.95, 1.0, 1.0),
            frameColor=(0.1, 0.15, 0.2, 0.8),
            frameSize=(-button_width/2, button_width/2, -button_height/2, button_height/2),
            pos=(0.35, 0, -0.025),
            command=self._switch_to_hangar,
            parent=self.content_frame
        )
        self.hangar_button.setTransparency(TransparencyAttrib.MAlpha)
    
    def _create_capacity_display(self):
        """Create capacity display (used space / total space)"""
        self.capacity_label = DirectLabel(
            text="Capacity: 0.0 / 100.0 m³",
            text_scale=0.04,
            text_fg=(0.7, 0.75, 0.8, 1.0),
            text_align=TextNode.ARight,
            frameColor=(0, 0, 0, 0),
            pos=(self.size[0] - 0.02, 0, -0.025),
            parent=self.content_frame
        )
    
    def _create_action_buttons(self):
        """Create action buttons (transfer, jettison, etc.)"""
        button_y = -self.size[1] + 0.04
        
        # Transfer button (move item between cargo/hangar)
        self.transfer_button = DirectButton(
            text="Transfer",
            text_scale=0.035,
            text_fg=(0.9, 0.95, 1.0, 1.0),
            frameColor=(0.2, 0.6, 0.8, 0.8),
            frameSize=(-0.08, 0.08, -0.02, 0.02),
            pos=(0.15, 0, button_y),
            command=self._on_transfer_clicked,
            parent=self.content_frame
        )
        self.transfer_button.setTransparency(TransparencyAttrib.MAlpha)
        
        # Jettison button (drop item into space)
        self.jettison_button = DirectButton(
            text="Jettison",
            text_scale=0.035,
            text_fg=(1.0, 0.5, 0.3, 1.0),
            frameColor=(0.5, 0.2, 0.1, 0.8),
            frameSize=(-0.08, 0.08, -0.02, 0.02),
            pos=(0.35, 0, button_y),
            command=self._on_jettison_clicked,
            parent=self.content_frame
        )
        self.jettison_button.setTransparency(TransparencyAttrib.MAlpha)
    
    def _switch_to_cargo(self):
        """Switch view to cargo hold"""
        self.view_mode = "cargo"
        self.cargo_button['frameColor'] = (0.2, 0.6, 0.8, 0.8)
        self.hangar_button['frameColor'] = (0.1, 0.15, 0.2, 0.8)
        self.refresh_inventory()
    
    def _switch_to_hangar(self):
        """Switch view to station hangar"""
        self.view_mode = "hangar"
        self.cargo_button['frameColor'] = (0.1, 0.15, 0.2, 0.8)
        self.hangar_button['frameColor'] = (0.2, 0.6, 0.8, 0.8)
        self.refresh_inventory()
    
    def _on_transfer_clicked(self):
        """Handle transfer button click"""
        if self.on_transfer_item:
            # Get selected item (for now, use first item)
            if self.list_items:
                item_data = self.list_items[0].getPythonTag("data")
                self.on_transfer_item(item_data, self.view_mode)
    
    def _on_jettison_clicked(self):
        """Handle jettison button click"""
        if self.on_jettison_item:
            # Get selected item (for now, use first item)
            if self.list_items:
                item_data = self.list_items[0].getPythonTag("data")
                self.on_jettison_item(item_data)
    
    def update_inventory(self, items: Dict[str, int], capacity_used: float = 0.0, capacity_max: float = 100.0):
        """
        Update inventory display
        
        Args:
            items: Dictionary of {item_id: quantity}
            capacity_used: Used capacity in m³
            capacity_max: Maximum capacity in m³
        """
        # Clear existing items
        self.clear_list()
        
        # Add items to list
        for item_id, quantity in sorted(items.items()):
            item_text = f"{item_id} x{quantity}"
            self.add_list_item(item_text, data={"item_id": item_id, "quantity": quantity})
        
        # Update capacity display
        capacity_percent = (capacity_used / capacity_max * 100) if capacity_max > 0 else 0
        self.capacity_label['text'] = f"Capacity: {capacity_used:.1f} / {capacity_max:.1f} m³ ({capacity_percent:.0f}%)"
        
        # Update capacity label color based on usage
        if capacity_percent > 90:
            self.capacity_label['text_fg'] = (1.0, 0.3, 0.3, 1.0)  # Red
        elif capacity_percent > 75:
            self.capacity_label['text_fg'] = (1.0, 0.7, 0.2, 1.0)  # Orange
        else:
            self.capacity_label['text_fg'] = (0.7, 0.75, 0.8, 1.0)  # Normal
    
    def refresh_inventory(self):
        """Refresh inventory from current data (placeholder for now)"""
        # This would normally fetch data from the game client
        # For now, just update with empty inventory
        self.update_inventory({}, 0.0, 100.0)
    
    def set_transfer_callback(self, callback: Callable):
        """Set callback for transfer action"""
        self.on_transfer_item = callback
    
    def set_jettison_callback(self, callback: Callable):
        """Set callback for jettison action"""
        self.on_jettison_item = callback
