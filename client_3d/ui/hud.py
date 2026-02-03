"""
HUD (Heads-Up Display) for 3D Client
Displays ship status, target info, navigation, and combat log

This module provides both legacy and EVE-styled HUD options
"""

from direct.gui.OnscreenText import OnscreenText
from direct.gui.OnscreenImage import OnscreenImage
from direct.gui.DirectGui import DirectFrame
from panda3d.core import TextNode, Vec3, Vec4, TransparencyAttrib
from typing import Optional, Dict, Any

# Import EVE-styled HUD
try:
    from .eve_hud import EVEStyledHUD
    EVE_HUD_AVAILABLE = True
except ImportError as e:
    print(f"[HUD] Warning: EVE-styled HUD not available: {e}")
    EVE_HUD_AVAILABLE = False


class HUDSystem:
    """
    Manages the HUD overlay for the 3D client
    Shows ship status, target info, speed, and combat log
    """
    
    def __init__(self, aspect2d):
        self.aspect2d = aspect2d
        
        # HUD elements
        self.ship_status_panel = None
        self.target_info_panel = None
        self.speed_display = None
        self.combat_log = None
        
        # Data
        self.ship_data = {}
        self.target_data = {}
        self.combat_messages = []
        self.max_combat_messages = 5
        
        # Initialize HUD
        self._create_hud()
        
        print("[HUD] HUD system initialized")
    
    def _create_hud(self):
        """Create all HUD elements"""
        self._create_ship_status_panel()
        self._create_target_info_panel()
        self._create_speed_display()
        self._create_combat_log()
    
    def _create_ship_status_panel(self):
        """Create ship status panel (bottom left)"""
        # Background frame
        self.ship_status_frame = DirectFrame(
            frameColor=(0, 0, 0, 0.5),
            frameSize=(-0.3, 0.3, -0.15, 0.15),
            pos=(-0.9, 0, -0.75),
            parent=self.aspect2d
        )
        
        # Ship name
        self.ship_name_text = OnscreenText(
            text="Ship: Loading...",
            pos=(-0.25, 0.1),
            scale=0.05,
            fg=(1, 1, 1, 1),
            align=TextNode.ALeft,
            parent=self.ship_status_frame
        )
        
        # Shield bar and text
        self.shield_text = OnscreenText(
            text="Shield: 0/0",
            pos=(-0.25, 0.04),
            scale=0.04,
            fg=(0.3, 0.5, 1, 1),  # Blue for shields
            align=TextNode.ALeft,
            parent=self.ship_status_frame
        )
        
        # Armor bar and text
        self.armor_text = OnscreenText(
            text="Armor: 0/0",
            pos=(-0.25, -0.02),
            scale=0.04,
            fg=(0.8, 0.8, 0.2, 1),  # Yellow for armor
            align=TextNode.ALeft,
            parent=self.ship_status_frame
        )
        
        # Hull bar and text
        self.hull_text = OnscreenText(
            text="Hull: 0/0",
            pos=(-0.25, -0.08),
            scale=0.04,
            fg=(0.8, 0.3, 0.3, 1),  # Red for hull
            align=TextNode.ALeft,
            parent=self.ship_status_frame
        )
        
        print("[HUD] Ship status panel created")
    
    def _create_target_info_panel(self):
        """Create target info panel (top right)"""
        # Background frame
        self.target_info_frame = DirectFrame(
            frameColor=(0, 0, 0, 0.5),
            frameSize=(-0.3, 0.3, -0.15, 0.15),
            pos=(0.9, 0, 0.75),
            parent=self.aspect2d
        )
        
        # Target name
        self.target_name_text = OnscreenText(
            text="Target: None",
            pos=(-0.25, 0.1),
            scale=0.05,
            fg=(1, 0.3, 0.3, 1),  # Red for target
            align=TextNode.ALeft,
            parent=self.target_info_frame
        )
        
        # Target distance
        self.target_distance_text = OnscreenText(
            text="Distance: -",
            pos=(-0.25, 0.04),
            scale=0.04,
            fg=(1, 1, 1, 1),
            align=TextNode.ALeft,
            parent=self.target_info_frame
        )
        
        # Target shield
        self.target_shield_text = OnscreenText(
            text="Shield: -",
            pos=(-0.25, -0.02),
            scale=0.04,
            fg=(0.3, 0.5, 1, 1),
            align=TextNode.ALeft,
            parent=self.target_info_frame
        )
        
        # Target armor
        self.target_armor_text = OnscreenText(
            text="Armor: -",
            pos=(-0.25, -0.08),
            scale=0.04,
            fg=(0.8, 0.8, 0.2, 1),
            align=TextNode.ALeft,
            parent=self.target_info_frame
        )
        
        # Initially hide target panel
        self.target_info_frame.hide()
        
        print("[HUD] Target info panel created")
    
    def _create_speed_display(self):
        """Create speed/navigation display (top left)"""
        self.speed_frame = DirectFrame(
            frameColor=(0, 0, 0, 0.5),
            frameSize=(-0.2, 0.2, -0.1, 0.1),
            pos=(-0.9, 0, 0.85),
            parent=self.aspect2d
        )
        
        # Speed text
        self.speed_text = OnscreenText(
            text="Speed: 0 m/s",
            pos=(-0.15, 0.04),
            scale=0.05,
            fg=(0.5, 1, 0.5, 1),  # Green for speed
            align=TextNode.ALeft,
            parent=self.speed_frame
        )
        
        # Position text (optional, can be hidden)
        self.position_text = OnscreenText(
            text="X: 0  Y: 0  Z: 0",
            pos=(-0.15, -0.03),
            scale=0.03,
            fg=(0.7, 0.7, 0.7, 1),
            align=TextNode.ALeft,
            parent=self.speed_frame
        )
        
        print("[HUD] Speed display created")
    
    def _create_combat_log(self):
        """Create combat log (bottom right)"""
        self.combat_log_frame = DirectFrame(
            frameColor=(0, 0, 0, 0.3),
            frameSize=(-0.4, 0.4, -0.2, 0.05),
            pos=(0.6, 0, -0.8),
            parent=self.aspect2d
        )
        
        # Combat log title
        self.combat_log_title = OnscreenText(
            text="Combat Log",
            pos=(-0.35, 0.0),
            scale=0.04,
            fg=(1, 0.8, 0.3, 1),
            align=TextNode.ALeft,
            parent=self.combat_log_frame
        )
        
        # Combat messages (stored as list of OnscreenText objects)
        self.combat_message_texts = []
        
        print("[HUD] Combat log created")
    
    def update_ship_status(self, ship_data: Dict[str, Any]):
        """
        Update ship status display
        
        Args:
            ship_data: Dictionary with ship stats
                - name: Ship name
                - shield_current: Current shield HP
                - shield_max: Max shield HP
                - armor_current: Current armor HP
                - armor_max: Max armor HP
                - hull_current: Current hull HP
                - hull_max: Max hull HP
        """
        self.ship_data = ship_data
        
        # Update ship name
        ship_name = ship_data.get('name', 'Unknown')
        self.ship_name_text.setText(f"Ship: {ship_name}")
        
        # Update shield
        shield_current = ship_data.get('shield_current', 0)
        shield_max = ship_data.get('shield_max', 1)
        shield_percent = (shield_current / shield_max * 100) if shield_max > 0 else 0
        self.shield_text.setText(f"Shield: {shield_current:.0f}/{shield_max:.0f} ({shield_percent:.1f}%)")
        
        # Update armor
        armor_current = ship_data.get('armor_current', 0)
        armor_max = ship_data.get('armor_max', 1)
        armor_percent = (armor_current / armor_max * 100) if armor_max > 0 else 0
        self.armor_text.setText(f"Armor: {armor_current:.0f}/{armor_max:.0f} ({armor_percent:.1f}%)")
        
        # Update hull
        hull_current = ship_data.get('hull_current', 0)
        hull_max = ship_data.get('hull_max', 1)
        hull_percent = (hull_current / hull_max * 100) if hull_max > 0 else 0
        self.hull_text.setText(f"Hull: {hull_current:.0f}/{hull_max:.0f} ({hull_percent:.1f}%)")
    
    def update_target_info(self, target_data: Optional[Dict[str, Any]]):
        """
        Update target info display
        
        Args:
            target_data: Dictionary with target stats (None to clear target)
                - name: Target name
                - distance: Distance in meters
                - shield_current: Current shield HP
                - shield_max: Max shield HP
                - armor_current: Current armor HP
                - armor_max: Max armor HP
        """
        if target_data is None:
            # Clear target
            self.target_info_frame.hide()
            self.target_data = {}
            return
        
        self.target_data = target_data
        self.target_info_frame.show()
        
        # Update target name
        target_name = target_data.get('name', 'Unknown')
        self.target_name_text.setText(f"Target: {target_name}")
        
        # Update distance
        distance = target_data.get('distance', 0)
        if distance >= 1000:
            self.target_distance_text.setText(f"Distance: {distance/1000:.1f} km")
        else:
            self.target_distance_text.setText(f"Distance: {distance:.0f} m")
        
        # Update shield
        shield_current = target_data.get('shield_current', 0)
        shield_max = target_data.get('shield_max', 1)
        shield_percent = (shield_current / shield_max * 100) if shield_max > 0 else 0
        self.target_shield_text.setText(f"Shield: {shield_percent:.1f}%")
        
        # Update armor
        armor_current = target_data.get('armor_current', 0)
        armor_max = target_data.get('armor_max', 1)
        armor_percent = (armor_current / armor_max * 100) if armor_max > 0 else 0
        self.target_armor_text.setText(f"Armor: {armor_percent:.1f}%")
    
    def update_speed(self, speed: float, position: Optional[Vec3] = None):
        """
        Update speed display
        
        Args:
            speed: Current speed in m/s
            position: Optional position vector
        """
        self.speed_text.setText(f"Speed: {speed:.1f} m/s")
        
        if position:
            self.position_text.setText(f"X: {position.x:.0f}  Y: {position.y:.0f}  Z: {position.z:.0f}")
    
    def add_combat_message(self, message: str, color: Vec4 = Vec4(1, 1, 1, 1)):
        """
        Add a message to combat log
        
        Args:
            message: Message text
            color: Message color (default: white)
        """
        self.combat_messages.append((message, color))
        
        # Limit number of messages
        if len(self.combat_messages) > self.max_combat_messages:
            self.combat_messages.pop(0)
        
        # Update display
        self._update_combat_log_display()
    
    def _update_combat_log_display(self):
        """Update combat log visual display"""
        # Clear old text objects
        for text_obj in self.combat_message_texts:
            text_obj.destroy()
        self.combat_message_texts.clear()
        
        # Create new text objects
        y_offset = -0.05
        for i, (message, color) in enumerate(reversed(self.combat_messages)):
            text = OnscreenText(
                text=message,
                pos=(-0.35, y_offset - (i * 0.035)),
                scale=0.03,
                fg=color,
                align=TextNode.ALeft,
                parent=self.combat_log_frame
            )
            self.combat_message_texts.append(text)
    
    def clear_combat_log(self):
        """Clear all combat messages"""
        self.combat_messages.clear()
        self._update_combat_log_display()
    
    def show(self):
        """Show all HUD elements"""
        if self.ship_status_frame:
            self.ship_status_frame.show()
        if self.target_info_frame and self.target_data:
            self.target_info_frame.show()
        if self.speed_frame:
            self.speed_frame.show()
        if self.combat_log_frame:
            self.combat_log_frame.show()
    
    def hide(self):
        """Hide all HUD elements"""
        if self.ship_status_frame:
            self.ship_status_frame.hide()
        if self.target_info_frame:
            self.target_info_frame.hide()
        if self.speed_frame:
            self.speed_frame.hide()
        if self.combat_log_frame:
            self.combat_log_frame.hide()
    
    def toggle_visibility(self):
        """Toggle HUD visibility"""
        if self.ship_status_frame.isHidden():
            self.show()
        else:
            self.hide()
    
    def cleanup(self):
        """Cleanup HUD resources"""
        # Destroy all frames and their children
        if self.ship_status_frame:
            self.ship_status_frame.destroy()
        if self.target_info_frame:
            self.target_info_frame.destroy()
        if self.speed_frame:
            self.speed_frame.destroy()
        if self.combat_log_frame:
            self.combat_log_frame.destroy()
        
        # Clear combat message texts
        for text_obj in self.combat_message_texts:
            text_obj.destroy()
        self.combat_message_texts.clear()
        
        print("[HUD] HUD system cleaned up")


def create_hud(aspect2d, render2d=None, style='eve'):
    """
    Factory function to create HUD with specified style
    
    Args:
        aspect2d: Panda3D aspect2d node
        render2d: Panda3D render2d node (optional, for EVE style)
        style: HUD style - 'eve' for EVE Online style, 'legacy' for original
        
    Returns:
        HUD instance (EVEStyledHUD or HUDSystem)
    """
    if style == 'eve' and EVE_HUD_AVAILABLE:
        print("[HUD] Creating EVE-styled HUD")
        return EVEStyledHUD(aspect2d, render2d)
    else:
        if style == 'eve' and not EVE_HUD_AVAILABLE:
            print("[HUD] EVE-styled HUD not available, falling back to legacy")
        print("[HUD] Creating legacy HUD")
        return HUDSystem(aspect2d)


__all__ = ['HUDSystem', 'create_hud']
