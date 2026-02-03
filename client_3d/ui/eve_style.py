"""
EVE Online UI Style Configuration
Mimics the Photon UI color scheme and styling from EVE Online
"""

from panda3d.core import Vec4
from typing import Dict, Tuple


class EVEColorScheme:
    """
    EVE Online Photon UI color scheme
    Based on the official EVE Online interface design
    """
    
    # Primary UI Colors (RGBA normalized 0-1)
    # Dark blue-black backgrounds
    BACKGROUND_PRIMARY = Vec4(0.05, 0.08, 0.12, 0.85)      # Semi-transparent dark blue
    BACKGROUND_SECONDARY = Vec4(0.08, 0.12, 0.16, 0.75)    # Lighter variant
    BACKGROUND_PANEL = Vec4(0.02, 0.04, 0.08, 0.9)          # Very dark blue-black
    
    # Accent colors - Teal/Cyan (EVE's signature color)
    ACCENT_PRIMARY = Vec4(0.2, 0.6, 0.8, 1.0)              # Bright teal
    ACCENT_SECONDARY = Vec4(0.15, 0.45, 0.65, 1.0)         # Darker teal
    ACCENT_GLOW = Vec4(0.3, 0.8, 1.0, 0.5)                 # Glowing cyan
    
    # Border colors
    BORDER_NORMAL = Vec4(0.2, 0.3, 0.4, 0.6)               # Subtle blue-gray
    BORDER_HIGHLIGHT = Vec4(0.3, 0.7, 0.9, 0.8)            # Bright teal highlight
    BORDER_ACTIVE = Vec4(0.4, 0.9, 1.0, 1.0)               # Active bright cyan
    
    # Text colors
    TEXT_PRIMARY = Vec4(0.9, 0.95, 1.0, 1.0)               # Nearly white
    TEXT_SECONDARY = Vec4(0.7, 0.75, 0.8, 1.0)             # Gray-blue
    TEXT_DISABLED = Vec4(0.4, 0.45, 0.5, 0.6)              # Dark gray
    TEXT_HIGHLIGHT = Vec4(1.0, 1.0, 1.0, 1.0)              # Pure white
    
    # Status colors (Shield/Armor/Hull)
    SHIELD_COLOR = Vec4(0.2, 0.5, 1.0, 1.0)                # Blue for shields
    SHIELD_DAMAGED = Vec4(0.4, 0.6, 0.9, 1.0)              # Lighter blue when damaged
    ARMOR_COLOR = Vec4(1.0, 0.8, 0.2, 1.0)                 # Yellow-gold for armor
    ARMOR_DAMAGED = Vec4(0.9, 0.5, 0.1, 1.0)               # Orange when damaged
    HULL_COLOR = Vec4(0.8, 0.3, 0.3, 1.0)                  # Red for hull
    HULL_CRITICAL = Vec4(1.0, 0.2, 0.2, 1.0)               # Bright red critical
    
    # Capacitor color (Energy)
    CAPACITOR_FULL = Vec4(1.0, 0.9, 0.3, 1.0)              # Bright yellow
    CAPACITOR_LOW = Vec4(1.0, 0.5, 0.0, 1.0)               # Orange warning
    CAPACITOR_CRITICAL = Vec4(1.0, 0.2, 0.0, 1.0)          # Red critical
    
    # Target colors
    TARGET_HOSTILE = Vec4(1.0, 0.2, 0.2, 1.0)              # Red for enemies
    TARGET_FRIENDLY = Vec4(0.2, 1.0, 0.2, 1.0)             # Green for friendlies
    TARGET_NEUTRAL = Vec4(0.8, 0.8, 0.8, 1.0)              # Gray for neutral
    TARGET_LOCKED = Vec4(1.0, 0.8, 0.0, 1.0)               # Gold when locked
    
    # Module states
    MODULE_OFFLINE = Vec4(0.3, 0.3, 0.3, 0.6)              # Dark gray
    MODULE_ONLINE = Vec4(0.2, 0.6, 0.8, 1.0)               # Teal
    MODULE_ACTIVE = Vec4(0.3, 0.9, 0.3, 1.0)               # Green
    MODULE_OVERHEATED = Vec4(1.0, 0.3, 0.0, 1.0)           # Orange-red
    MODULE_CYCLING = Vec4(1.0, 0.8, 0.2, 1.0)              # Yellow
    
    # Warning colors
    WARNING_LOW = Vec4(1.0, 0.8, 0.2, 0.8)                 # Yellow
    WARNING_MEDIUM = Vec4(1.0, 0.5, 0.0, 0.9)              # Orange
    WARNING_HIGH = Vec4(1.0, 0.2, 0.0, 1.0)                # Red
    
    # Interactive states
    BUTTON_NORMAL = Vec4(0.15, 0.25, 0.35, 0.8)
    BUTTON_HOVER = Vec4(0.2, 0.35, 0.5, 0.9)
    BUTTON_PRESSED = Vec4(0.3, 0.5, 0.7, 1.0)
    BUTTON_DISABLED = Vec4(0.1, 0.12, 0.15, 0.5)


class EVEPanelStyle:
    """
    Panel styling configuration for EVE-style UI
    """
    
    # Panel dimensions (normalized coordinates)
    PANEL_PADDING = 0.02
    PANEL_MARGIN = 0.01
    HEADER_HEIGHT = 0.05
    
    # Border widths
    BORDER_WIDTH_THIN = 1
    BORDER_WIDTH_NORMAL = 2
    BORDER_WIDTH_THICK = 3
    
    # Corner radius for rounded panels
    CORNER_RADIUS = 4
    
    # Opacity levels
    OPACITY_SOLID = 1.0
    OPACITY_HIGH = 0.9
    OPACITY_MEDIUM = 0.75
    OPACITY_LOW = 0.5
    OPACITY_SUBTLE = 0.3
    
    # Font sizes (relative scale)
    FONT_SIZE_TITLE = 0.06
    FONT_SIZE_LARGE = 0.05
    FONT_SIZE_NORMAL = 0.04
    FONT_SIZE_SMALL = 0.035
    FONT_SIZE_TINY = 0.03
    
    # Animation timing (seconds)
    ANIM_FAST = 0.1
    ANIM_NORMAL = 0.2
    ANIM_SLOW = 0.3
    
    # Glow effect parameters
    GLOW_INTENSITY = 0.5
    GLOW_RADIUS = 0.02


class EVELayoutPresets:
    """
    Common layout presets matching EVE Online's interface
    """
    
    # HUD Center (Ship status and capacitor)
    HUD_CENTER = {
        'pos': (0, 0, -0.6),
        'size': 0.3,  # Radius
    }
    
    # Ship Status Panel (Bottom left)
    SHIP_STATUS = {
        'pos': (-0.9, 0, -0.75),
        'size': (0.6, 0.3),  # width, height
        'anchor': 'bottom-left',
    }
    
    # Target Info Panel (Top right)
    TARGET_INFO = {
        'pos': (0.9, 0, 0.75),
        'size': (0.6, 0.3),
        'anchor': 'top-right',
    }
    
    # Speed/Navigation (Top left)
    SPEED_DISPLAY = {
        'pos': (-0.9, 0, 0.85),
        'size': (0.4, 0.2),
        'anchor': 'top-left',
    }
    
    # Overview Panel (Right side)
    OVERVIEW_PANEL = {
        'pos': (0.5, 0, 0.0),
        'size': (0.8, 1.2),
        'anchor': 'right',
    }
    
    # Module Rack (Bottom center)
    MODULE_RACK = {
        'pos': (0, 0, -0.85),
        'size': (1.0, 0.2),
        'anchor': 'bottom',
    }
    
    # Neocom (Left sidebar)
    NEOCOM = {
        'pos': (-1.0, 0, 0.0),
        'size': (0.12, 1.8),
        'anchor': 'left',
    }
    
    # Capacitor Display (Bottom center, around ship)
    CAPACITOR = {
        'pos': (0, 0, -0.6),
        'radius_inner': 0.15,
        'radius_outer': 0.18,
        'segments': 60,
    }
    
    # Target Lock Display (Top center)
    TARGET_LOCKS = {
        'pos': (0, 0, 0.4),
        'spacing': 0.25,
        'size': 0.15,
    }


class EVEIconPaths:
    """
    Placeholder paths for EVE-style icons
    In a full implementation, these would reference actual icon files
    """
    
    # Module types
    ICON_WEAPON_TURRET = "icons/turret.png"
    ICON_WEAPON_LAUNCHER = "icons/launcher.png"
    ICON_SHIELD_BOOSTER = "icons/shield_booster.png"
    ICON_ARMOR_REPAIRER = "icons/armor_rep.png"
    ICON_PROPULSION = "icons/propmod.png"
    ICON_EWAR = "icons/ewar.png"
    
    # Ship types
    ICON_SHIP_FRIGATE = "icons/frigate.png"
    ICON_SHIP_DESTROYER = "icons/destroyer.png"
    ICON_SHIP_CRUISER = "icons/cruiser.png"
    
    # Neocom menu items
    ICON_INVENTORY = "icons/inventory.png"
    ICON_FITTING = "icons/fitting.png"
    ICON_MAP = "icons/map.png"
    ICON_MARKET = "icons/market.png"
    ICON_CHARACTER = "icons/character.png"
    
    # Status icons
    ICON_LOCKED = "icons/locked.png"
    ICON_TARGETING = "icons/targeting.png"
    ICON_WARP = "icons/warp.png"


def get_health_color(current: float, maximum: float, health_type: str = 'shield') -> Vec4:
    """
    Get appropriate color based on health percentage
    
    Args:
        current: Current health value
        maximum: Maximum health value
        health_type: Type of health ('shield', 'armor', 'hull')
        
    Returns:
        Vec4 color for the health bar
    """
    if maximum <= 0:
        return EVEColorScheme.TEXT_DISABLED
    
    percent = current / maximum
    
    if health_type == 'shield':
        if percent > 0.5:
            return EVEColorScheme.SHIELD_COLOR
        else:
            return EVEColorScheme.SHIELD_DAMAGED
    elif health_type == 'armor':
        if percent > 0.3:
            return EVEColorScheme.ARMOR_COLOR
        else:
            return EVEColorScheme.ARMOR_DAMAGED
    elif health_type == 'hull':
        if percent > 0.25:
            return EVEColorScheme.HULL_COLOR
        else:
            return EVEColorScheme.HULL_CRITICAL
    
    return EVEColorScheme.TEXT_PRIMARY


def get_capacitor_color(current: float, maximum: float) -> Vec4:
    """
    Get appropriate color based on capacitor percentage
    
    Args:
        current: Current capacitor value
        maximum: Maximum capacitor value
        
    Returns:
        Vec4 color for the capacitor gauge
    """
    if maximum <= 0:
        return EVEColorScheme.CAPACITOR_CRITICAL
    
    percent = current / maximum
    
    if percent > 0.5:
        return EVEColorScheme.CAPACITOR_FULL
    elif percent > 0.25:
        return EVEColorScheme.CAPACITOR_LOW
    else:
        return EVEColorScheme.CAPACITOR_CRITICAL


def lerp_color(color1: Vec4, color2: Vec4, t: float) -> Vec4:
    """
    Linear interpolation between two colors
    
    Args:
        color1: Start color
        color2: End color
        t: Interpolation factor (0-1)
        
    Returns:
        Interpolated color
    """
    t = max(0.0, min(1.0, t))  # Clamp to [0, 1]
    return Vec4(
        color1[0] + (color2[0] - color1[0]) * t,
        color1[1] + (color2[1] - color1[1]) * t,
        color1[2] + (color2[2] - color1[2]) * t,
        color1[3] + (color2[3] - color1[3]) * t,
    )


# Export commonly used items
__all__ = [
    'EVEColorScheme',
    'EVEPanelStyle',
    'EVELayoutPresets',
    'EVEIconPaths',
    'get_health_color',
    'get_capacitor_color',
    'lerp_color',
]
