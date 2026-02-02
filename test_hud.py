"""
Test script to verify HUD system functionality
Tests the HUD without requiring a full 3D client or server
"""

print("Testing HUD system...")

try:
    # Test import
    from panda3d.core import Vec3, Vec4
    from direct.gui.OnscreenText import OnscreenText
    print("✓ Panda3D imports successful")
    
    # Test HUD import
    from client_3d.ui.hud import HUDSystem
    print("✓ HUD import successful")
    
    # Create a minimal ShowBase for testing
    from direct.showbase.ShowBase import ShowBase
    
    class HUDTestApp(ShowBase):
        def __init__(self):
            ShowBase.__init__(self)
            
            # Create HUD
            self.hud = HUDSystem(self.aspect2d)
            print("✓ HUD system initialized")
            
            # Test ship status update
            ship_data = {
                'name': 'Rifter',
                'shield_current': 500,
                'shield_max': 1000,
                'armor_current': 800,
                'armor_max': 1000,
                'hull_current': 1200,
                'hull_max': 1200,
            }
            self.hud.update_ship_status(ship_data)
            print("✓ Ship status updated")
            
            # Test target info update
            target_data = {
                'name': 'Serpentis Fighter',
                'distance': 1500,
                'shield_current': 200,
                'shield_max': 500,
                'armor_current': 400,
                'armor_max': 500,
            }
            self.hud.update_target_info(target_data)
            print("✓ Target info updated")
            
            # Test speed update
            self.hud.update_speed(150.5, Vec3(1000, 2000, 500))
            print("✓ Speed updated")
            
            # Test combat messages
            self.hud.add_combat_message("Taking 50 damage!", Vec4(1, 0.3, 0.3, 1))
            self.hud.add_combat_message("Hit for 75 damage", Vec4(0.3, 1, 0.3, 1))
            self.hud.add_combat_message("Shield depleted!", Vec4(1, 0.5, 0, 1))
            print("✓ Combat messages added")
            
            # Test toggle
            self.hud.hide()
            print("✓ HUD hidden")
            self.hud.show()
            print("✓ HUD shown")
            
            print("\n" + "="*60)
            print("HUD TEST SUCCESSFUL!")
            print("All HUD components are working correctly.")
            print("="*60)
            print("\nPress ESC to close the test window.")
            
            # Setup exit
            self.accept('escape', self.userExit)
    
    # Run test
    app = HUDTestApp()
    app.run()
    
except ImportError as e:
    print(f"✗ Import error: {e}")
    print("\nPlease install Panda3D:")
    print("  pip install panda3d")
    exit(1)
except Exception as e:
    print(f"✗ Error during test: {e}")
    import traceback
    traceback.print_exc()
    exit(1)
