"""
Enhanced HUD System with EVE Online Photon UI Styling
Comprehensive heads-up display mimicking EVE Online's interface
"""

from direct.gui.OnscreenText import OnscreenText
from direct.gui.DirectGui import DirectFrame, DirectButton
from panda3d.core import TextNode, Vec3, Vec4, TransparencyAttrib
from typing import Optional, Dict, Any, List

from .eve_style import (
    EVEColorScheme, EVEPanelStyle, EVELayoutPresets,
    get_health_color, get_capacitor_color
)
from .capacitor_display import CapacitorDisplay, ShipHealthRings


class EVEStyledHUD:
    """
    Enhanced HUD system with EVE Online Photon UI styling
    Provides complete interface similar to EVE Online
    """
    
    def __init__(self, aspect2d, render2d):
        """
        Initialize EVE-styled HUD
        
        Args:
            aspect2d: Panda3D aspect2d node for 2D UI
            render2d: Panda3D render2d node for additional 2D elements
        """
        self.aspect2d = aspect2d
        self.render2d = render2d
        
        # UI Components
        self.ship_status_panel = None
        self.target_info_panel = None
        self.speed_panel = None
        self.combat_log_panel = None
        self.overview_panel = None
        self.neocom_panel = None
        
        # EVE-style circular displays
        self.capacitor_display = None
        self.health_rings = None
        
        # Target lock displays
        self.target_locks = []
        
        # Data storage
        self.ship_data = {}
        self.target_data = {}
        self.combat_messages = []
        self.max_combat_messages = 8
        
        # Initialize all UI components
        self._create_all_panels()
        
        print("[EVEStyledHUD] Enhanced EVE-style HUD initialized")
    
    def _create_all_panels(self):
        """Create all HUD panels and components"""
        # Create capacitor and health rings (center HUD)
        self._create_center_hud()
        
        # Create information panels
        self._create_ship_status_panel()
        self._create_target_info_panel()
        self._create_speed_panel()
        self._create_combat_log_panel()
        
        # Create EVE-style overview panel
        self._create_overview_panel()
        
        # Create Neocom menu (left sidebar)
        self._create_neocom_panel()
    
    def _create_center_hud(self):
        """Create the center HUD with capacitor and health rings"""
        # EVE-style capacitor ring
        self.capacitor_display = CapacitorDisplay(
            parent=self.aspect2d,
            pos=(0, 0, -0.6),
            radius=0.18,
            thickness=0.025,
            segments=60
        )
        
        # Health rings (shield/armor/hull) around ship
        self.health_rings = ShipHealthRings(
            parent=self.aspect2d,
            pos=(0, 0, -0.6),
            base_radius=0.08,
            ring_thickness=0.012,
            spacing=0.005
        )
        
        # Center ship indicator (small dot)
        self._create_ship_indicator()
    
    def _create_ship_indicator(self):
        """Create a small indicator for the ship in the center"""
        # Small frame representing the ship
        self.ship_indicator = DirectFrame(
            frameColor=EVEColorScheme.ACCENT_PRIMARY,
            frameSize=(-0.015, 0.015, -0.015, 0.015),
            pos=(0, 0, -0.6),
            parent=self.aspect2d
        )
        self.ship_indicator.setTransparency(TransparencyAttrib.MAlpha)
    
    def _create_ship_status_panel(self):
        """Create EVE-style ship status panel (bottom left)"""
        layout = EVELayoutPresets.SHIP_STATUS
        
        # Main panel background with EVE styling
        self.ship_status_panel = DirectFrame(
            frameColor=EVEColorScheme.BACKGROUND_PANEL,
            frameSize=(-0.3, 0.3, -0.2, 0.2),
            pos=layout['pos'],
            parent=self.aspect2d
        )
        self.ship_status_panel.setTransparency(TransparencyAttrib.MAlpha)
        
        # Panel header
        self.ship_header = DirectFrame(
            frameColor=EVEColorScheme.BACKGROUND_PRIMARY,
            frameSize=(-0.3, 0.3, -0.025, 0.025),
            pos=(0, 0, 0.175),
            parent=self.ship_status_panel
        )
        self.ship_header.setTransparency(TransparencyAttrib.MAlpha)
        
        # Border accent (top)
        self.ship_border = DirectFrame(
            frameColor=EVEColorScheme.BORDER_HIGHLIGHT,
            frameSize=(-0.3, 0.3, -0.002, 0.002),
            pos=(0, 0, 0.2),
            parent=self.ship_status_panel
        )
        self.ship_border.setTransparency(TransparencyAttrib.MAlpha)
        
        # Title
        self.ship_title = OnscreenText(
            text="◢ SHIP STATUS",
            pos=(-0.28, 0.16),
            scale=EVEPanelStyle.FONT_SIZE_NORMAL,
            fg=EVEColorScheme.ACCENT_PRIMARY,
            align=TextNode.ALeft,
            parent=self.ship_status_panel
        )
        
        # Ship name
        self.ship_name_text = OnscreenText(
            text="Ship: Loading...",
            pos=(-0.28, 0.10),
            scale=EVEPanelStyle.FONT_SIZE_SMALL,
            fg=EVEColorScheme.TEXT_PRIMARY,
            align=TextNode.ALeft,
            parent=self.ship_status_panel
        )
        
        # Shield status
        self.shield_label = OnscreenText(
            text="SHIELD",
            pos=(-0.28, 0.04),
            scale=EVEPanelStyle.FONT_SIZE_TINY,
            fg=EVEColorScheme.SHIELD_COLOR,
            align=TextNode.ALeft,
            parent=self.ship_status_panel
        )
        self.shield_text = OnscreenText(
            text="0 / 0 (0%)",
            pos=(0.28, 0.04),
            scale=EVEPanelStyle.FONT_SIZE_SMALL,
            fg=EVEColorScheme.SHIELD_COLOR,
            align=TextNode.ARight,
            parent=self.ship_status_panel
        )
        
        # Armor status
        self.armor_label = OnscreenText(
            text="ARMOR",
            pos=(-0.28, -0.02),
            scale=EVEPanelStyle.FONT_SIZE_TINY,
            fg=EVEColorScheme.ARMOR_COLOR,
            align=TextNode.ALeft,
            parent=self.ship_status_panel
        )
        self.armor_text = OnscreenText(
            text="0 / 0 (0%)",
            pos=(0.28, -0.02),
            scale=EVEPanelStyle.FONT_SIZE_SMALL,
            fg=EVEColorScheme.ARMOR_COLOR,
            align=TextNode.ARight,
            parent=self.ship_status_panel
        )
        
        # Hull status
        self.hull_label = OnscreenText(
            text="STRUCTURE",
            pos=(-0.28, -0.08),
            scale=EVEPanelStyle.FONT_SIZE_TINY,
            fg=EVEColorScheme.HULL_COLOR,
            align=TextNode.ALeft,
            parent=self.ship_status_panel
        )
        self.hull_text = OnscreenText(
            text="0 / 0 (0%)",
            pos=(0.28, -0.08),
            scale=EVEPanelStyle.FONT_SIZE_SMALL,
            fg=EVEColorScheme.HULL_COLOR,
            align=TextNode.ARight,
            parent=self.ship_status_panel
        )
        
        # Capacitor status (text display)
        self.cap_label = OnscreenText(
            text="CAPACITOR",
            pos=(-0.28, -0.14),
            scale=EVEPanelStyle.FONT_SIZE_TINY,
            fg=EVEColorScheme.CAPACITOR_FULL,
            align=TextNode.ALeft,
            parent=self.ship_status_panel
        )
        self.cap_text = OnscreenText(
            text="100 / 100 (100%)",
            pos=(0.28, -0.14),
            scale=EVEPanelStyle.FONT_SIZE_SMALL,
            fg=EVEColorScheme.CAPACITOR_FULL,
            align=TextNode.ARight,
            parent=self.ship_status_panel
        )
    
    def _create_target_info_panel(self):
        """Create EVE-style target info panel (top right)"""
        layout = EVELayoutPresets.TARGET_INFO
        
        # Main panel
        self.target_info_panel = DirectFrame(
            frameColor=EVEColorScheme.BACKGROUND_PANEL,
            frameSize=(-0.3, 0.3, -0.2, 0.2),
            pos=layout['pos'],
            parent=self.aspect2d
        )
        self.target_info_panel.setTransparency(TransparencyAttrib.MAlpha)
        
        # Header
        self.target_header = DirectFrame(
            frameColor=EVEColorScheme.BACKGROUND_PRIMARY,
            frameSize=(-0.3, 0.3, -0.025, 0.025),
            pos=(0, 0, 0.175),
            parent=self.target_info_panel
        )
        self.target_header.setTransparency(TransparencyAttrib.MAlpha)
        
        # Border accent
        self.target_border = DirectFrame(
            frameColor=EVEColorScheme.TARGET_HOSTILE,
            frameSize=(-0.3, 0.3, -0.002, 0.002),
            pos=(0, 0, 0.2),
            parent=self.target_info_panel
        )
        self.target_border.setTransparency(TransparencyAttrib.MAlpha)
        
        # Title
        self.target_title = OnscreenText(
            text="◢ TARGET",
            pos=(-0.28, 0.16),
            scale=EVEPanelStyle.FONT_SIZE_NORMAL,
            fg=EVEColorScheme.TARGET_HOSTILE,
            align=TextNode.ALeft,
            parent=self.target_info_panel
        )
        
        # Target name
        self.target_name_text = OnscreenText(
            text="No Target",
            pos=(-0.28, 0.10),
            scale=EVEPanelStyle.FONT_SIZE_SMALL,
            fg=EVEColorScheme.TEXT_PRIMARY,
            align=TextNode.ALeft,
            parent=self.target_info_panel
        )
        
        # Distance
        self.target_distance_text = OnscreenText(
            text="Distance: --",
            pos=(-0.28, 0.04),
            scale=EVEPanelStyle.FONT_SIZE_SMALL,
            fg=EVEColorScheme.TEXT_SECONDARY,
            align=TextNode.ALeft,
            parent=self.target_info_panel
        )
        
        # Shield
        self.target_shield_label = OnscreenText(
            text="SHIELD",
            pos=(-0.28, -0.02),
            scale=EVEPanelStyle.FONT_SIZE_TINY,
            fg=EVEColorScheme.SHIELD_COLOR,
            align=TextNode.ALeft,
            parent=self.target_info_panel
        )
        self.target_shield_text = OnscreenText(
            text="--",
            pos=(0.28, -0.02),
            scale=EVEPanelStyle.FONT_SIZE_SMALL,
            fg=EVEColorScheme.SHIELD_COLOR,
            align=TextNode.ARight,
            parent=self.target_info_panel
        )
        
        # Armor
        self.target_armor_label = OnscreenText(
            text="ARMOR",
            pos=(-0.28, -0.08),
            scale=EVEPanelStyle.FONT_SIZE_TINY,
            fg=EVEColorScheme.ARMOR_COLOR,
            align=TextNode.ALeft,
            parent=self.target_info_panel
        )
        self.target_armor_text = OnscreenText(
            text="--",
            pos=(0.28, -0.08),
            scale=EVEPanelStyle.FONT_SIZE_SMALL,
            fg=EVEColorScheme.ARMOR_COLOR,
            align=TextNode.ARight,
            parent=self.target_info_panel
        )
        
        # Hide by default
        self.target_info_panel.hide()
    
    def _create_speed_panel(self):
        """Create speed/navigation panel (top left)"""
        layout = EVELayoutPresets.SPEED_DISPLAY
        
        self.speed_panel = DirectFrame(
            frameColor=EVEColorScheme.BACKGROUND_PANEL,
            frameSize=(-0.25, 0.25, -0.12, 0.12),
            pos=layout['pos'],
            parent=self.aspect2d
        )
        self.speed_panel.setTransparency(TransparencyAttrib.MAlpha)
        
        # Header
        speed_header = DirectFrame(
            frameColor=EVEColorScheme.BACKGROUND_PRIMARY,
            frameSize=(-0.25, 0.25, -0.02, 0.02),
            pos=(0, 0, 0.1),
            parent=self.speed_panel
        )
        speed_header.setTransparency(TransparencyAttrib.MAlpha)
        
        # Border
        speed_border = DirectFrame(
            frameColor=EVEColorScheme.ACCENT_SECONDARY,
            frameSize=(-0.25, 0.25, -0.002, 0.002),
            pos=(0, 0, 0.12),
            parent=self.speed_panel
        )
        speed_border.setTransparency(TransparencyAttrib.MAlpha)
        
        # Title
        OnscreenText(
            text="◢ NAVIGATION",
            pos=(-0.23, 0.09),
            scale=EVEPanelStyle.FONT_SIZE_NORMAL,
            fg=EVEColorScheme.ACCENT_SECONDARY,
            align=TextNode.ALeft,
            parent=self.speed_panel
        )
        
        # Speed display
        self.speed_text = OnscreenText(
            text="Speed: 0.0 m/s",
            pos=(-0.23, 0.03),
            scale=EVEPanelStyle.FONT_SIZE_NORMAL,
            fg=EVEColorScheme.TEXT_PRIMARY,
            align=TextNode.ALeft,
            parent=self.speed_panel
        )
        
        # Position (optional, can be hidden)
        self.position_text = OnscreenText(
            text="X: 0  Y: 0  Z: 0",
            pos=(-0.23, -0.04),
            scale=EVEPanelStyle.FONT_SIZE_TINY,
            fg=EVEColorScheme.TEXT_SECONDARY,
            align=TextNode.ALeft,
            parent=self.speed_panel
        )
    
    def _create_combat_log_panel(self):
        """Create combat log panel (bottom right)"""
        self.combat_log_panel = DirectFrame(
            frameColor=EVEColorScheme.BACKGROUND_PANEL,
            frameSize=(-0.45, 0.45, -0.25, 0.05),
            pos=(0.45, 0, -0.75),
            parent=self.aspect2d
        )
        self.combat_log_panel.setTransparency(TransparencyAttrib.MAlpha)
        
        # Header
        log_header = DirectFrame(
            frameColor=EVEColorScheme.BACKGROUND_PRIMARY,
            frameSize=(-0.45, 0.45, -0.02, 0.02),
            pos=(0, 0, 0.03),
            parent=self.combat_log_panel
        )
        log_header.setTransparency(TransparencyAttrib.MAlpha)
        
        # Border
        log_border = DirectFrame(
            frameColor=EVEColorScheme.ACCENT_GLOW,
            frameSize=(-0.45, 0.45, -0.002, 0.002),
            pos=(0, 0, 0.05),
            parent=self.combat_log_panel
        )
        log_border.setTransparency(TransparencyAttrib.MAlpha)
        
        # Title
        OnscreenText(
            text="◢ COMBAT LOG",
            pos=(-0.43, 0.02),
            scale=EVEPanelStyle.FONT_SIZE_NORMAL,
            fg=EVEColorScheme.ACCENT_GLOW,
            align=TextNode.ALeft,
            parent=self.combat_log_panel
        )
        
        # Combat messages list
        self.combat_message_texts = []
    
    def _create_overview_panel(self):
        """Create EVE-style overview panel (right side)"""
        # Simplified overview for now - can be expanded
        self.overview_panel = DirectFrame(
            frameColor=EVEColorScheme.BACKGROUND_PANEL,
            frameSize=(-0.35, 0.35, -0.5, 0.05),
            pos=(0.65, 0, 0.2),
            parent=self.aspect2d
        )
        self.overview_panel.setTransparency(TransparencyAttrib.MAlpha)
        
        # Header
        overview_header = DirectFrame(
            frameColor=EVEColorScheme.BACKGROUND_PRIMARY,
            frameSize=(-0.35, 0.35, -0.02, 0.02),
            pos=(0, 0, 0.03),
            parent=self.overview_panel
        )
        overview_header.setTransparency(TransparencyAttrib.MAlpha)
        
        # Border
        overview_border = DirectFrame(
            frameColor=EVEColorScheme.ACCENT_PRIMARY,
            frameSize=(-0.35, 0.35, -0.002, 0.002),
            pos=(0, 0, 0.05),
            parent=self.overview_panel
        )
        overview_border.setTransparency(TransparencyAttrib.MAlpha)
        
        # Title
        OnscreenText(
            text="◢ OVERVIEW",
            pos=(-0.33, 0.02),
            scale=EVEPanelStyle.FONT_SIZE_NORMAL,
            fg=EVEColorScheme.ACCENT_PRIMARY,
            align=TextNode.ALeft,
            parent=self.overview_panel
        )
        
        # Placeholder text
        OnscreenText(
            text="[Nearby Objects]",
            pos=(0, -0.05),
            scale=EVEPanelStyle.FONT_SIZE_SMALL,
            fg=EVEColorScheme.TEXT_SECONDARY,
            align=TextNode.ACenter,
            parent=self.overview_panel
        )
    
    def _create_neocom_panel(self):
        """Create Neocom-style left sidebar menu"""
        self.neocom_panel = DirectFrame(
            frameColor=EVEColorScheme.BACKGROUND_PRIMARY,
            frameSize=(-0.06, 0.06, -0.9, 0.9),
            pos=(-0.94, 0, 0),
            parent=self.aspect2d
        )
        self.neocom_panel.setTransparency(TransparencyAttrib.MAlpha)
        
        # Border
        neocom_border = DirectFrame(
            frameColor=EVEColorScheme.BORDER_HIGHLIGHT,
            frameSize=(-0.002, 0.002, -0.9, 0.9),
            pos=(0.06, 0, 0),
            parent=self.neocom_panel
        )
        neocom_border.setTransparency(TransparencyAttrib.MAlpha)
        
        # Neocom title (top)
        OnscreenText(
            text="≡",
            pos=(0, 0.85),
            scale=0.08,
            fg=EVEColorScheme.ACCENT_PRIMARY,
            align=TextNode.ACenter,
            parent=self.neocom_panel
        )
        
        # Menu items (simplified - would normally be icons)
        menu_items = [
            ("I", "Inventory", 0.6),
            ("F", "Fitting", 0.4),
            ("M", "Map", 0.2),
            ("$", "Market", 0.0),
            ("@", "Character", -0.2),
        ]
        
        for icon, name, y_pos in menu_items:
            btn_frame = DirectFrame(
                frameColor=EVEColorScheme.BUTTON_NORMAL,
                frameSize=(-0.05, 0.05, -0.05, 0.05),
                pos=(0, 0, y_pos),
                parent=self.neocom_panel
            )
            btn_frame.setTransparency(TransparencyAttrib.MAlpha)
            
            OnscreenText(
                text=icon,
                pos=(0, -0.015),
                scale=0.06,
                fg=EVEColorScheme.TEXT_PRIMARY,
                align=TextNode.ACenter,
                parent=btn_frame
            )
    
    # Update methods
    
    def update_ship_status(self, ship_data: Dict[str, Any]):
        """Update ship status display"""
        self.ship_data = ship_data
        
        # Update text displays
        ship_name = ship_data.get('name', 'Unknown')
        self.ship_name_text.setText(f"Ship: {ship_name}")
        
        # Shield
        shield_cur = ship_data.get('shield_current', 0)
        shield_max = ship_data.get('shield_max', 1)
        shield_pct = (shield_cur / shield_max * 100) if shield_max > 0 else 0
        self.shield_text.setText(f"{shield_cur:.0f} / {shield_max:.0f} ({shield_pct:.0f}%)")
        shield_color = get_health_color(shield_cur, shield_max, 'shield')
        self.shield_text.setFg(shield_color)
        
        # Armor
        armor_cur = ship_data.get('armor_current', 0)
        armor_max = ship_data.get('armor_max', 1)
        armor_pct = (armor_cur / armor_max * 100) if armor_max > 0 else 0
        self.armor_text.setText(f"{armor_cur:.0f} / {armor_max:.0f} ({armor_pct:.0f}%)")
        armor_color = get_health_color(armor_cur, armor_max, 'armor')
        self.armor_text.setFg(armor_color)
        
        # Hull
        hull_cur = ship_data.get('hull_current', 0)
        hull_max = ship_data.get('hull_max', 1)
        hull_pct = (hull_cur / hull_max * 100) if hull_max > 0 else 0
        self.hull_text.setText(f"{hull_cur:.0f} / {hull_max:.0f} ({hull_pct:.0f}%)")
        hull_color = get_health_color(hull_cur, hull_max, 'hull')
        self.hull_text.setFg(hull_color)
        
        # Capacitor
        cap_cur = ship_data.get('capacitor_current', 100)
        cap_max = ship_data.get('capacitor_max', 100)
        cap_pct = (cap_cur / cap_max * 100) if cap_max > 0 else 0
        self.cap_text.setText(f"{cap_cur:.0f} / {cap_max:.0f} ({cap_pct:.0f}%)")
        cap_color = get_capacitor_color(cap_cur, cap_max)
        self.cap_text.setFg(cap_color)
        
        # Update circular displays
        if self.health_rings:
            self.health_rings.update(shield_cur, shield_max, armor_cur, armor_max, hull_cur, hull_max)
        if self.capacitor_display:
            self.capacitor_display.update(cap_cur, cap_max)
    
    def update_target_info(self, target_data: Optional[Dict[str, Any]]):
        """Update target information"""
        if target_data is None:
            self.target_info_panel.hide()
            return
        
        self.target_data = target_data
        self.target_info_panel.show()
        
        # Name
        name = target_data.get('name', 'Unknown')
        self.target_name_text.setText(f"{name}")
        
        # Distance
        distance = target_data.get('distance', 0)
        if distance >= 1000:
            self.target_distance_text.setText(f"Distance: {distance/1000:.1f} km")
        else:
            self.target_distance_text.setText(f"Distance: {distance:.0f} m")
        
        # Shield
        shield_cur = target_data.get('shield_current', 0)
        shield_max = target_data.get('shield_max', 1)
        shield_pct = (shield_cur / shield_max * 100) if shield_max > 0 else 0
        self.target_shield_text.setText(f"{shield_pct:.0f}%")
        
        # Armor
        armor_cur = target_data.get('armor_current', 0)
        armor_max = target_data.get('armor_max', 1)
        armor_pct = (armor_cur / armor_max * 100) if armor_max > 0 else 0
        self.target_armor_text.setText(f"{armor_pct:.0f}%")
    
    def update_speed(self, speed: float, position: Optional[Vec3] = None):
        """Update speed and position"""
        self.speed_text.setText(f"Speed: {speed:.1f} m/s")
        
        if position:
            self.position_text.setText(f"X: {position.x:.0f}  Y: {position.y:.0f}  Z: {position.z:.0f}")
    
    def add_combat_message(self, message: str, color: Optional[Vec4] = None):
        """Add a message to the combat log"""
        if color is None:
            color = EVEColorScheme.TEXT_PRIMARY
        
        self.combat_messages.append(message)
        if len(self.combat_messages) > self.max_combat_messages:
            self.combat_messages.pop(0)
        
        # Clear old message texts
        for text in self.combat_message_texts:
            text.destroy()
        self.combat_message_texts.clear()
        
        # Create new message texts
        y_pos = -0.05
        for msg in reversed(self.combat_messages):
            text = OnscreenText(
                text=msg,
                pos=(-0.43, y_pos),
                scale=EVEPanelStyle.FONT_SIZE_TINY,
                fg=color,
                align=TextNode.ALeft,
                parent=self.combat_log_panel
            )
            self.combat_message_texts.append(text)
            y_pos -= 0.03
    
    def show(self):
        """Show all HUD elements"""
        if self.ship_status_panel:
            self.ship_status_panel.show()
        if self.speed_panel:
            self.speed_panel.show()
        if self.combat_log_panel:
            self.combat_log_panel.show()
        if self.overview_panel:
            self.overview_panel.show()
        if self.neocom_panel:
            self.neocom_panel.show()
        if self.capacitor_display:
            self.capacitor_display.show()
        if self.health_rings:
            self.health_rings.show()
    
    def hide(self):
        """Hide all HUD elements"""
        if self.ship_status_panel:
            self.ship_status_panel.hide()
        if self.target_info_panel:
            self.target_info_panel.hide()
        if self.speed_panel:
            self.speed_panel.hide()
        if self.combat_log_panel:
            self.combat_log_panel.hide()
        if self.overview_panel:
            self.overview_panel.hide()
        if self.neocom_panel:
            self.neocom_panel.hide()
        if self.capacitor_display:
            self.capacitor_display.hide()
        if self.health_rings:
            self.health_rings.hide()
    
    def destroy(self):
        """Clean up all HUD elements"""
        if self.capacitor_display:
            self.capacitor_display.destroy()
        if self.health_rings:
            self.health_rings.destroy()
        
        # DirectGui elements clean themselves up when parent is destroyed


__all__ = ['EVEStyledHUD']
