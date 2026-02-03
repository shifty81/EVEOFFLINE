"""
Base Panel System for EVE-styled UI Windows
Provides reusable window/panel infrastructure for the 3D client
"""

from direct.gui.DirectGui import (
    DirectFrame, DirectButton, DirectLabel, 
    DirectScrolledFrame, DirectEntry
)
from panda3d.core import TextNode, Vec4, TransparencyAttrib
from typing import Optional, Callable, List, Tuple


class EVEPanel:
    """
    Base class for EVE-styled UI panels/windows
    Provides common functionality like dragging, closing, resizing
    """
    
    def __init__(
        self,
        parent,
        title: str,
        pos: Tuple[float, float] = (0, 0),
        size: Tuple[float, float] = (0.8, 0.6),
        closeable: bool = True,
        draggable: bool = True
    ):
        """
        Initialize base panel
        
        Args:
            parent: Parent node for this panel
            title: Window title text
            pos: (x, y) position in normalized coordinates
            size: (width, height) in normalized coordinates
            closeable: Whether panel can be closed
            draggable: Whether panel can be dragged
        """
        self.parent = parent
        self.title = title
        self.pos = pos
        self.size = size
        self.closeable = closeable
        self.draggable = draggable
        self.visible = True
        
        # Callbacks
        self.on_close_callback: Optional[Callable] = None
        
        # Dragging state
        self.is_dragging = False
        self.drag_start_pos = None
        self.drag_start_mouse = None
        
        # Create the panel
        self._create_panel()
    
    def _create_panel(self):
        """Create the panel structure"""
        # Main frame (background)
        self.frame = DirectFrame(
            frameColor=(0.05, 0.08, 0.12, 0.95),  # EVE dark blue-black
            frameSize=(0, self.size[0], -self.size[1], 0),
            pos=(self.pos[0], 0, self.pos[1]),
            parent=self.parent
        )
        self.frame.setTransparency(TransparencyAttrib.MAlpha)
        
        # Title bar
        self._create_title_bar()
        
        # Content area (subclasses will populate this)
        self.content_frame = DirectFrame(
            frameColor=(0.02, 0.04, 0.08, 0.9),  # Slightly lighter
            frameSize=(0.01, self.size[0] - 0.01, -self.size[1] + 0.08, -0.01),
            pos=(0, 0, -0.08),
            parent=self.frame
        )
        self.content_frame.setTransparency(TransparencyAttrib.MAlpha)
    
    def _create_title_bar(self):
        """Create title bar with title text and close button"""
        # Title bar background
        title_bar = DirectFrame(
            frameColor=(0.2, 0.6, 0.8, 0.3),  # EVE teal accent
            frameSize=(0, self.size[0], -0.08, 0),
            pos=(0, 0, 0),
            parent=self.frame
        )
        title_bar.setTransparency(TransparencyAttrib.MAlpha)
        
        # Title text
        self.title_label = DirectLabel(
            text=self.title,
            text_scale=0.05,
            text_fg=(0.9, 0.95, 1.0, 1.0),  # EVE white text
            text_align=TextNode.ALeft,
            frameColor=(0, 0, 0, 0),
            pos=(0.02, 0, -0.055),
            parent=title_bar
        )
        
        # Close button (if closeable)
        if self.closeable:
            self.close_button = DirectButton(
                text="X",
                text_scale=0.04,
                text_fg=(1, 0.3, 0.3, 1),  # Red
                frameColor=(0.3, 0.1, 0.1, 0.7),
                frameSize=(-0.02, 0.02, -0.025, 0.025),
                pos=(self.size[0] - 0.04, 0, -0.04),
                command=self.close,
                parent=title_bar
            )
            self.close_button.setTransparency(TransparencyAttrib.MAlpha)
        
        # Make title bar draggable
        if self.draggable:
            title_bar.bind(DirectFrame.B1PRESS, self._start_drag)
            title_bar.bind(DirectFrame.B1RELEASE, self._stop_drag)
    
    def _start_drag(self, event):
        """Start dragging the panel"""
        if self.draggable and hasattr(event, 'getMouse'):
            self.is_dragging = True
            mouse_pos = event.getMouse()
            self.drag_start_mouse = (mouse_pos.getX(), mouse_pos.getY())
            self.drag_start_pos = self.frame.getPos()
    
    def _stop_drag(self, event):
        """Stop dragging the panel"""
        self.is_dragging = False
    
    def update_drag(self, mouse_watcher):
        """Update panel position while dragging"""
        if self.is_dragging and mouse_watcher.hasMouse():
            mouse = mouse_watcher.getMouse()
            if self.drag_start_mouse and self.drag_start_pos:
                dx = mouse.getX() - self.drag_start_mouse[0]
                dy = mouse.getY() - self.drag_start_mouse[1]
                new_x = self.drag_start_pos[0] + dx
                new_z = self.drag_start_pos[2] + dy
                self.frame.setPos(new_x, 0, new_z)
    
    def show(self):
        """Show the panel"""
        self.frame.show()
        self.visible = True
    
    def hide(self):
        """Hide the panel"""
        self.frame.hide()
        self.visible = False
    
    def close(self):
        """Close the panel"""
        self.hide()
        if self.on_close_callback:
            self.on_close_callback(self)
    
    def toggle(self):
        """Toggle panel visibility"""
        if self.visible:
            self.hide()
        else:
            self.show()
    
    def destroy(self):
        """Destroy the panel and clean up"""
        if hasattr(self, 'frame'):
            self.frame.destroy()
    
    def set_on_close_callback(self, callback: Callable):
        """Set callback to be called when panel is closed"""
        self.on_close_callback = callback


class EVEListPanel(EVEPanel):
    """
    Panel with a scrollable list of items
    Useful for inventory, market orders, etc.
    """
    
    def __init__(self, *args, **kwargs):
        self.list_items = []
        super().__init__(*args, **kwargs)
        self._create_list()
    
    def _create_list(self):
        """Create scrollable list area"""
        # Scrolled frame for list
        self.scroll_frame = DirectScrolledFrame(
            frameColor=(0.02, 0.04, 0.08, 0.5),
            frameSize=(0, self.size[0] - 0.04, 0, self.size[1] - 0.12),
            canvasSize=(0, self.size[0] - 0.06, -2.0, 0),
            scrollBarWidth=0.02,
            pos=(0.02, 0, -0.02),
            parent=self.content_frame
        )
        self.scroll_frame.setTransparency(TransparencyAttrib.MAlpha)
        
        # Canvas for list items
        self.canvas = self.scroll_frame.getCanvas()
    
    def add_list_item(self, text: str, data: any = None) -> DirectLabel:
        """Add an item to the list"""
        y_pos = -0.06 * len(self.list_items)
        
        item = DirectLabel(
            text=text,
            text_scale=0.04,
            text_fg=(0.9, 0.95, 1.0, 1.0),
            text_align=TextNode.ALeft,
            frameColor=(0.1, 0.15, 0.2, 0.3),
            frameSize=(0, self.size[0] - 0.08, -0.05, 0.05),
            pos=(0.02, 0, y_pos),
            parent=self.canvas
        )
        item.setTransparency(TransparencyAttrib.MAlpha)
        
        # Store data with item
        item.setPythonTag("data", data)
        
        self.list_items.append(item)
        
        # Update canvas size
        canvas_height = max(2.0, len(self.list_items) * 0.06 + 0.1)
        self.scroll_frame['canvasSize'] = (0, self.size[0] - 0.06, -canvas_height, 0)
        
        return item
    
    def clear_list(self):
        """Clear all list items"""
        for item in self.list_items:
            item.destroy()
        self.list_items = []
        self.scroll_frame['canvasSize'] = (0, self.size[0] - 0.06, -2.0, 0)
