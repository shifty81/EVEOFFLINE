"""
Context Menu System for 3D Client
EVE Online-style right-click menus for tactical commands
"""

from direct.gui.DirectGui import DirectFrame, DirectButton, DGG
from panda3d.core import Vec3, Vec4, TextNode
from typing import Callable, List, Tuple, Optional


class ContextMenu:
    """
    Right-click context menu system
    Shows tactical options based on what was clicked
    """
    
    # EVE-style color scheme
    MENU_BG_COLOR = Vec4(0.1, 0.1, 0.12, 0.95)  # Dark semi-transparent
    MENU_TEXT_COLOR = Vec4(0.9, 0.9, 0.9, 1.0)  # Light gray
    MENU_HIGHLIGHT_COLOR = Vec4(0.9, 0.8, 0.4, 1.0)  # Gold
    MENU_HOVER_BG = Vec4(0.2, 0.2, 0.25, 0.95)  # Lighter on hover
    
    def __init__(self, parent, aspect2d):
        self.parent = parent
        self.aspect2d = aspect2d
        
        # Menu state
        self.menu_frame: Optional[DirectFrame] = None
        self.menu_items: List[DirectButton] = []
        self.is_visible = False
        
        # Menu positioning
        self.menu_x = 0
        self.menu_y = 0
        
        print("[ContextMenu] Context menu system initialized")
    
    def show(self, mouse_x: float, mouse_y: float, menu_items: List[Tuple[str, Callable]]):
        """
        Show context menu at mouse position
        
        Args:
            mouse_x: Mouse X position in screen coords (-1 to 1)
            mouse_y: Mouse Y position in screen coords (-1 to 1)
            menu_items: List of (label, callback) tuples
        """
        # Hide existing menu if any
        self.hide()
        
        if not menu_items:
            return
        
        # Calculate menu dimensions
        item_height = 0.06
        menu_width = 0.4
        menu_height = len(menu_items) * item_height + 0.02
        
        # Position menu at mouse (with bounds checking)
        menu_x = max(-0.9, min(mouse_x, 0.9 - menu_width))
        menu_y = max(-0.9 + menu_height, min(mouse_y, 0.9))
        
        # Create menu background frame
        self.menu_frame = DirectFrame(
            frameColor=self.MENU_BG_COLOR,
            frameSize=(-0.01, menu_width, -menu_height, 0.01),
            pos=(menu_x, 0, menu_y),
            parent=self.aspect2d
        )
        
        # Create menu items
        for i, (label, callback) in enumerate(menu_items):
            y_offset = -i * item_height - 0.03
            
            btn = DirectButton(
                text=label,
                text_align=TextNode.ALeft,
                text_scale=0.04,
                text_fg=self.MENU_TEXT_COLOR,
                text_pos=(0.02, 0),
                frameColor=self.MENU_BG_COLOR,
                frameSize=(0, menu_width - 0.02, -item_height + 0.01, 0),
                pos=(0, 0, y_offset),
                command=self._on_menu_item_click,
                extraArgs=[callback],
                parent=self.menu_frame,
                relief=DGG.FLAT,
                pressEffect=0
            )
            
            # Hover effects
            btn.bind(DGG.ENTER, self._on_menu_item_enter, [btn])
            btn.bind(DGG.EXIT, self._on_menu_item_exit, [btn])
            
            self.menu_items.append(btn)
        
        self.is_visible = True
        print(f"[ContextMenu] Showing menu with {len(menu_items)} items at ({mouse_x:.2f}, {mouse_y:.2f})")
    
    def show_entity_menu(self, mouse_x: float, mouse_y: float, entity_id: str, entity_data: dict):
        """
        Show context menu for an entity
        
        Args:
            mouse_x: Mouse X position
            mouse_y: Mouse Y position
            entity_id: Entity ID
            entity_data: Entity data (name, type, etc.)
        """
        entity_name = entity_data.get('name', 'Unknown')
        entity_type = entity_data.get('ship_type', 'Entity')
        
        menu_items = [
            (f"▸ {entity_name}", None),  # Header (non-clickable)
            ("─────────────", None),  # Separator
            ("Approach", lambda: self.parent.on_menu_action('approach', entity_id)),
            ("Orbit", lambda: self.parent.on_menu_action('orbit', entity_id)),
            ("Keep at Range", lambda: self.parent.on_menu_action('keep_range', entity_id)),
            ("─────────────", None),  # Separator
            ("Look at", lambda: self.parent.on_menu_action('look_at', entity_id)),
            ("Lock Target", lambda: self.parent.on_menu_action('lock', entity_id)),
            ("Show Info", lambda: self.parent.on_menu_action('show_info', entity_id)),
        ]
        
        self.show(mouse_x, mouse_y, menu_items)
    
    def show_space_menu(self, mouse_x: float,_y: float, world_pos: Vec3):
        """
        Show context menu for empty space
        
        Args:
            mouse_x: Mouse X position
            mouse_y: Mouse Y position
            world_pos: 3D world position clicked
        """
        menu_items = [
            ("▸ Space", None),  # Header
            ("─────────────", None),  # Separator
            ("Navigate to", lambda: self.parent.on_menu_action('navigate_to', world_pos)),
            ("Look at", lambda: self.parent.on_menu_action('look_at_pos', world_pos)),
        ]
        
        self.show(mouse_x, mouse_y, menu_items)
    
    def _on_menu_item_click(self, callback):
        """Handle menu item click"""
        # Hide menu first
        self.hide()
        
        # Execute callback if provided
        if callback:
            callback()
    
    def _on_menu_item_enter(self, btn, event):
        """Handle mouse enter menu item"""
        btn['frameColor'] = self.MENU_HOVER_BG
        btn['text_fg'] = self.MENU_HIGHLIGHT_COLOR
    
    def _on_menu_item_exit(self, btn, event):
        """Handle mouse exit menu item"""
        btn['frameColor'] = self.MENU_BG_COLOR
        btn['text_fg'] = self.MENU_TEXT_COLOR
    
    def hide(self):
        """Hide context menu"""
        if self.menu_frame:
            self.menu_frame.destroy()
            self.menu_frame = None
        
        for btn in self.menu_items:
            btn.destroy()
        self.menu_items.clear()
        
        self.is_visible = False
    
    def is_showing(self) -> bool:
        """Check if menu is currently visible"""
        return self.is_visible
    
    def cleanup(self):
        """Cleanup resources"""
        self.hide()
        print("[ContextMenu] Context menu cleaned up")
