"""
Enhanced Targeting Interface for 3D Client
Shows target locks with detailed information
"""

from direct.gui.DirectGui import DirectFrame, DirectLabel, DirectButton
from panda3d.core import TextNode, Vec3, Vec4, TransparencyAttrib
from typing import Dict, Optional, Callable, List


class TargetLockDisplay:
    """
    Individual target lock display
    Shows target info, distance, health, etc.
    """
    
    def __init__(self, parent, index: int, pos: tuple, size: tuple = (0.25, 0.1)):
        """
        Initialize target lock display
        
        Args:
            parent: Parent node
            index: Target slot index (0-based)
            pos: (x, y) position
            size: (width, height)
        """
        self.parent = parent
        self.index = index
        self.pos = pos
        self.size = size
        
        # Target data
        self.target_id = None
        self.target_name = "No Target"
        self.target_type = "Unknown"
        self.distance = 0.0
        self.shield_hp = 0.0
        self.shield_max = 1.0
        self.armor_hp = 0.0
        self.armor_max = 1.0
        self.hull_hp = 0.0
        self.hull_max = 1.0
        
        # Callbacks
        self.on_unlock: Optional[Callable] = None
        
        # Create display
        self._create_display()
    
    def _create_display(self):
        """Create target lock display UI"""
        # Background frame
        self.frame = DirectFrame(
            frameColor=(0.05, 0.08, 0.12, 0.9),
            frameSize=(0, self.size[0], -self.size[1], 0),
            pos=(self.pos[0], 0, self.pos[1]),
            parent=self.parent
        )
        self.frame.setTransparency(TransparencyAttrib.MAlpha)
        
        # Border (highlight when active)
        self.border = DirectFrame(
            frameColor=(0.2, 0.6, 0.8, 0.5),
            frameSize=(-0.002, self.size[0] + 0.002, -self.size[1] - 0.002, 0.002),
            pos=(0, 0, 0),
            parent=self.frame
        )
        self.border.setTransparency(TransparencyAttrib.MAlpha)
        
        # Target name
        self.name_label = DirectLabel(
            text=self.target_name,
            text_scale=0.035,
            text_fg=(0.9, 0.95, 1.0, 1.0),
            text_align=TextNode.ALeft,
            frameColor=(0, 0, 0, 0),
            pos=(0.01, 0, -0.02),
            parent=self.frame
        )
        
        # Target type
        self.type_label = DirectLabel(
            text=self.target_type,
            text_scale=0.025,
            text_fg=(0.7, 0.75, 0.8, 1.0),
            text_align=TextNode.ALeft,
            frameColor=(0, 0, 0, 0),
            pos=(0.01, 0, -0.045),
            parent=self.frame
        )
        
        # Distance
        self.distance_label = DirectLabel(
            text="0 km",
            text_scale=0.025,
            text_fg=(0.7, 0.75, 0.8, 1.0),
            text_align=TextNode.ARight,
            frameColor=(0, 0, 0, 0),
            pos=(self.size[0] - 0.01, 0, -0.02),
            parent=self.frame
        )
        
        # Health bars (shield, armor, hull)
        self._create_health_bars()
        
        # Unlock button
        self.unlock_button = DirectButton(
            text="X",
            text_scale=0.025,
            text_fg=(1.0, 0.3, 0.3, 1.0),
            frameColor=(0.3, 0.1, 0.1, 0.7),
            frameSize=(-0.015, 0.015, -0.015, 0.015),
            pos=(self.size[0] - 0.02, 0, -0.05),
            command=self._on_unlock,
            parent=self.frame
        )
        self.unlock_button.setTransparency(TransparencyAttrib.MAlpha)
    
    def _create_health_bars(self):
        """Create shield/armor/hull bars"""
        bar_y = -0.07
        bar_height = 0.008
        bar_spacing = 0.01
        bar_width = self.size[0] - 0.04
        
        # Shield bar
        self.shield_frame = DirectFrame(
            frameColor=(0.1, 0.1, 0.15, 0.8),
            frameSize=(0, bar_width, -bar_height/2, bar_height/2),
            pos=(0.02, 0, bar_y),
            parent=self.frame
        )
        self.shield_frame.setTransparency(TransparencyAttrib.MAlpha)
        
        self.shield_bar = DirectFrame(
            frameColor=(0.2, 0.5, 1.0, 0.9),  # Blue
            frameSize=(0, 0.01, -bar_height/2, bar_height/2),
            pos=(0, 0, 0),
            parent=self.shield_frame
        )
        self.shield_bar.setTransparency(TransparencyAttrib.MAlpha)
        
        # Armor bar
        bar_y -= bar_spacing
        self.armor_frame = DirectFrame(
            frameColor=(0.1, 0.1, 0.15, 0.8),
            frameSize=(0, bar_width, -bar_height/2, bar_height/2),
            pos=(0.02, 0, bar_y),
            parent=self.frame
        )
        self.armor_frame.setTransparency(TransparencyAttrib.MAlpha)
        
        self.armor_bar = DirectFrame(
            frameColor=(1.0, 0.8, 0.2, 0.9),  # Yellow
            frameSize=(0, 0.01, -bar_height/2, bar_height/2),
            pos=(0, 0, 0),
            parent=self.armor_frame
        )
        self.armor_bar.setTransparency(TransparencyAttrib.MAlpha)
        
        # Hull bar
        bar_y -= bar_spacing
        self.hull_frame = DirectFrame(
            frameColor=(0.1, 0.1, 0.15, 0.8),
            frameSize=(0, bar_width, -bar_height/2, bar_height/2),
            pos=(0.02, 0, bar_y),
            parent=self.frame
        )
        self.hull_frame.setTransparency(TransparencyAttrib.MAlpha)
        
        self.hull_bar = DirectFrame(
            frameColor=(0.8, 0.3, 0.3, 0.9),  # Red
            frameSize=(0, 0.01, -bar_height/2, bar_height/2),
            pos=(0, 0, 0),
            parent=self.hull_frame
        )
        self.hull_bar.setTransparency(TransparencyAttrib.MAlpha)
    
    def _on_unlock(self):
        """Handle unlock button"""
        if self.on_unlock and self.target_id:
            self.on_unlock(self.target_id)
    
    def update_target(self, target_id: str, name: str, target_type: str, distance: float,
                     shield_hp: float, shield_max: float,
                     armor_hp: float, armor_max: float,
                     hull_hp: float, hull_max: float):
        """Update target information"""
        self.target_id = target_id
        self.target_name = name
        self.target_type = target_type
        self.distance = distance
        
        # Update labels
        self.name_label['text'] = name
        self.type_label['text'] = target_type
        self.distance_label['text'] = f"{distance:.1f} km"
        
        # Update health bars
        bar_width = self.size[0] - 0.04
        
        shield_percent = shield_hp / shield_max if shield_max > 0 else 0
        self.shield_bar['frameSize'] = (0, bar_width * shield_percent, -0.004, 0.004)
        
        armor_percent = armor_hp / armor_max if armor_max > 0 else 0
        self.armor_bar['frameSize'] = (0, bar_width * armor_percent, -0.004, 0.004)
        
        hull_percent = hull_hp / hull_max if hull_max > 0 else 0
        self.hull_bar['frameSize'] = (0, bar_width * hull_percent, -0.004, 0.004)
    
    def clear_target(self):
        """Clear target lock"""
        self.target_id = None
        self.name_label['text'] = "[Empty Slot]"
        self.type_label['text'] = ""
        self.distance_label['text'] = ""
        
        # Clear health bars
        self.shield_bar['frameSize'] = (0, 0.01, -0.004, 0.004)
        self.armor_bar['frameSize'] = (0, 0.01, -0.004, 0.004)
        self.hull_bar['frameSize'] = (0, 0.01, -0.004, 0.004)
    
    def show(self):
        """Show target lock display"""
        self.frame.show()
    
    def hide(self):
        """Hide target lock display"""
        self.frame.hide()
    
    def destroy(self):
        """Destroy display"""
        if hasattr(self, 'frame'):
            self.frame.destroy()


class EnhancedTargetingInterface:
    """
    Enhanced targeting interface
    Manages multiple target locks
    """
    
    def __init__(self, parent, max_targets: int = 5, pos: tuple = (0.68, 0.8)):
        """
        Initialize targeting interface
        
        Args:
            parent: Parent node
            max_targets: Maximum number of simultaneous targets
            pos: Starting position for first target display
        """
        self.parent = parent
        self.max_targets = max_targets
        self.start_pos = pos
        
        # Target lock displays
        self.target_displays: List[TargetLockDisplay] = []
        
        # Callbacks
        self.on_unlock_target: Optional[Callable] = None
        
        # Create target displays
        self._create_target_displays()
        
        print(f"[EnhancedTargetingInterface] Targeting interface initialized ({max_targets} slots)")
    
    def _create_target_displays(self):
        """Create target lock display slots"""
        display_height = 0.11
        
        for i in range(self.max_targets):
            y_pos = self.start_pos[1] - (i * display_height)
            display = TargetLockDisplay(
                parent=self.parent,
                index=i,
                pos=(self.start_pos[0], y_pos),
                size=(0.25, 0.1)
            )
            display.on_unlock = self._handle_unlock
            display.clear_target()
            self.target_displays.append(display)
    
    def _handle_unlock(self, target_id: str):
        """Handle target unlock"""
        if self.on_unlock_target:
            self.on_unlock_target(target_id)
    
    def update_target(self, slot_index: int, target_id: str, name: str, target_type: str,
                     distance: float, shield_hp: float, shield_max: float,
                     armor_hp: float, armor_max: float, hull_hp: float, hull_max: float):
        """Update target in specific slot"""
        if 0 <= slot_index < len(self.target_displays):
            self.target_displays[slot_index].update_target(
                target_id, name, target_type, distance,
                shield_hp, shield_max, armor_hp, armor_max, hull_hp, hull_max
            )
    
    def clear_target(self, slot_index: int):
        """Clear target from slot"""
        if 0 <= slot_index < len(self.target_displays):
            self.target_displays[slot_index].clear_target()
    
    def clear_all_targets(self):
        """Clear all target locks"""
        for display in self.target_displays:
            display.clear_target()
    
    def set_unlock_callback(self, callback: Callable):
        """Set callback for target unlock"""
        self.on_unlock_target = callback
    
    def show(self):
        """Show all target displays"""
        for display in self.target_displays:
            display.show()
    
    def hide(self):
        """Hide all target displays"""
        for display in self.target_displays:
            display.hide()
    
    def destroy(self):
        """Destroy all target displays"""
        for display in self.target_displays:
            display.destroy()
