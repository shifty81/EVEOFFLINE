#!/usr/bin/env python3
"""
Unit tests for EVE-styled UI components
Tests that the UI components can be imported and instantiated correctly
"""

import sys
import os

# Add parent directory to path
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

def test_imports():
    """Test that all UI modules can be imported"""
    print("Testing imports...")
    
    try:
        from client_3d.ui.eve_style import (
            EVEColorScheme, EVEPanelStyle, EVELayoutPresets,
            get_health_color, get_capacitor_color, lerp_color
        )
        print("✓ eve_style module imported successfully")
    except Exception as e:
        print(f"✗ Failed to import eve_style: {e}")
        return False
    
    try:
        from client_3d.ui.capacitor_display import CapacitorDisplay, ShipHealthRings
        print("✓ capacitor_display module imported successfully")
    except Exception as e:
        print(f"✗ Failed to import capacitor_display: {e}")
        return False
    
    try:
        from client_3d.ui.eve_hud import EVEStyledHUD
        print("✓ eve_hud module imported successfully")
    except Exception as e:
        print(f"✗ Failed to import eve_hud: {e}")
        return False
    
    try:
        from client_3d.ui.hud import create_hud, HUDSystem
        print("✓ hud module imported successfully")
    except Exception as e:
        print(f"✗ Failed to import hud: {e}")
        return False
    
    return True


def test_color_scheme():
    """Test color scheme values"""
    print("\nTesting color scheme...")
    
    from client_3d.ui.eve_style import EVEColorScheme, get_health_color, get_capacitor_color
    from panda3d.core import Vec4
    
    # Test that colors are Vec4
    assert isinstance(EVEColorScheme.BACKGROUND_PRIMARY, Vec4)
    assert isinstance(EVEColorScheme.ACCENT_PRIMARY, Vec4)
    assert isinstance(EVEColorScheme.SHIELD_COLOR, Vec4)
    print("✓ Color scheme values are valid Vec4")
    
    # Test health color function
    shield_color = get_health_color(100, 100, 'shield')
    assert isinstance(shield_color, Vec4)
    print("✓ get_health_color() works")
    
    # Test capacitor color function
    cap_color = get_capacitor_color(50, 100)
    assert isinstance(cap_color, Vec4)
    print("✓ get_capacitor_color() works")
    
    return True


def test_panel_style():
    """Test panel style constants"""
    print("\nTesting panel style...")
    
    from client_3d.ui.eve_style import EVEPanelStyle
    
    # Test constants exist
    assert hasattr(EVEPanelStyle, 'FONT_SIZE_NORMAL')
    assert hasattr(EVEPanelStyle, 'PANEL_PADDING')
    assert hasattr(EVEPanelStyle, 'ANIM_FAST')
    print("✓ Panel style constants defined")
    
    # Test values are reasonable
    assert 0 < EVEPanelStyle.FONT_SIZE_NORMAL < 1
    assert 0 < EVEPanelStyle.PANEL_PADDING < 1
    print("✓ Panel style values are reasonable")
    
    return True


def test_layout_presets():
    """Test layout presets"""
    print("\nTesting layout presets...")
    
    from client_3d.ui.eve_style import EVELayoutPresets
    
    # Test that presets exist
    assert hasattr(EVELayoutPresets, 'SHIP_STATUS')
    assert hasattr(EVELayoutPresets, 'TARGET_INFO')
    assert hasattr(EVELayoutPresets, 'NEOCOM')
    print("✓ Layout presets defined")
    
    # Test structure
    assert 'pos' in EVELayoutPresets.SHIP_STATUS
    assert 'size' in EVELayoutPresets.SHIP_STATUS
    print("✓ Layout presets have correct structure")
    
    return True


def test_hud_factory():
    """Test HUD factory function"""
    print("\nTesting HUD factory...")
    
    from client_3d.ui.hud import create_hud
    
    # Factory function exists
    assert callable(create_hud)
    print("✓ create_hud() factory function exists")
    
    return True


def main():
    """Run all tests"""
    print("=" * 60)
    print("EVE-Styled UI Component Tests")
    print("=" * 60)
    
    all_passed = True
    
    # Run tests
    if not test_imports():
        all_passed = False
    
    if not test_color_scheme():
        all_passed = False
    
    if not test_panel_style():
        all_passed = False
    
    if not test_layout_presets():
        all_passed = False
    
    if not test_hud_factory():
        all_passed = False
    
    print("\n" + "=" * 60)
    if all_passed:
        print("✓ ALL TESTS PASSED")
        print("=" * 60)
        print("\nEVE-styled UI components are properly configured!")
        print("\nKey Features:")
        print("  • EVE Online Photon UI color scheme")
        print("  • Circular capacitor ring display")
        print("  • Concentric health rings (shield/armor/hull)")
        print("  • EVE-styled panels with borders and headers")
        print("  • Neocom-style left sidebar")
        print("  • Overview panel")
        print("  • Combat log with proper styling")
        print("  • Target info panel")
        print("  • Speed/navigation display")
        print("\nTo test with 3D client:")
        print("  python client_3d.py \"TestPilot\"")
        print("\nTo test standalone UI demo:")
        print("  python test_eve_hud.py")
        return 0
    else:
        print("✗ SOME TESTS FAILED")
        print("=" * 60)
        return 1


if __name__ == "__main__":
    sys.exit(main())
