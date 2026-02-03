#!/usr/bin/env python3
"""
Standalone test for EVE-styled HUD
Tests the new UI components without requiring a server connection
"""

import sys
import os
import math

# Add parent directory to path
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

try:
    from direct.showbase.ShowBase import ShowBase
    from panda3d.core import loadPrcFileData, Vec3
except ImportError:
    print("ERROR: Panda3D not installed!")
    print("Install with: pip install panda3d")
    sys.exit(1)

from client_3d.ui.eve_hud import EVEStyledHUD
from client_3d.rendering.starfield import StarField


class HUDDemo(ShowBase):
    """
    Demo application to showcase the EVE-styled HUD
    """
    
    def __init__(self):
        # Configure Panda3D
        loadPrcFileData("", "window-title EVE OFFLINE - UI Demo")
        loadPrcFileData("", "win-size 1280 720")
        loadPrcFileData("", "sync-video #t")
        
        # Initialize Panda3D
        ShowBase.__init__(self)
        
        # Setup background
        self.setBackgroundColor(0.0, 0.0, 0.02, 1)
        
        # Add star field for visual context
        self.star_field = StarField(self.render, self.camera)
        self.star_field.create(num_stars=1500)
        
        # Initialize EVE-styled HUD
        self.hud = EVEStyledHUD(self.aspect2d, self.render2d)
        
        # Demo state
        self.time = 0
        self.has_target = False
        
        # Setup demo data
        self._update_demo_data()
        
        # Setup tasks
        self.taskMgr.add(self.update_task, "update_task")
        
        # Setup input
        self.accept('escape', sys.exit)
        self.accept('t', self.toggle_target)
        self.accept('space', self.simulate_damage)
        
        print("=" * 60)
        print("EVE OFFLINE - UI Demo")
        print("=" * 60)
        print("Controls:")
        print("  T      - Toggle target display")
        print("  SPACE  - Simulate damage")
        print("  ESC    - Exit")
        print("=" * 60)
        print()
        print("Watch the UI elements update:")
        print("- Capacitor ring depletes and regenerates")
        print("- Health rings show damage")
        print("- Ship status panel updates")
        print("- Target panel toggles")
        print("- Combat log shows messages")
        print("=" * 60)
    
    def _update_demo_data(self):
        """Update ship status with demo data"""
        # Ship data (will animate)
        ship_data = {
            'name': 'Rifter',
            'shield_current': 500,
            'shield_max': 500,
            'armor_current': 400,
            'armor_max': 400,
            'hull_current': 350,
            'hull_max': 350,
            'capacitor_current': 100,
            'capacitor_max': 100,
        }
        self.hud.update_ship_status(ship_data)
        
        # Speed data
        self.hud.update_speed(150.5, Vec3(1000, 2000, 500))
        
        # Initial combat message
        self.hud.add_combat_message("◢ System initialized")
        self.hud.add_combat_message("◢ UI demo active")
    
    def toggle_target(self):
        """Toggle target display"""
        self.has_target = not self.has_target
        
        if self.has_target:
            target_data = {
                'name': 'Guristas Frigate',
                'distance': 5500,
                'shield_current': 300,
                'shield_max': 400,
                'armor_current': 250,
                'armor_max': 300,
            }
            self.hud.update_target_info(target_data)
            self.hud.add_combat_message("◢ Target locked: Guristas Frigate")
        else:
            self.hud.update_target_info(None)
            self.hud.add_combat_message("◢ Target unlocked")
    
    def simulate_damage(self):
        """Simulate taking damage"""
        # Get current values and reduce them
        ship_data = {
            'name': 'Rifter',
            'shield_current': max(0, 500 - (self.time % 5) * 100),
            'shield_max': 500,
            'armor_current': max(0, 400 - (self.time % 7) * 80),
            'armor_max': 400,
            'hull_current': max(50, 350 - (self.time % 10) * 50),
            'hull_max': 350,
            'capacitor_current': 100,
            'capacitor_max': 100,
        }
        self.hud.update_ship_status(ship_data)
        self.hud.add_combat_message("◢ Taking damage!")
    
    def update_task(self, task):
        """Update task - animates the demo"""
        dt = globalClock.getDt()
        self.time += dt
        
        # Animate capacitor (depletes and regenerates)
        cap_cycle = (math.sin(self.time * 0.5) + 1) / 2  # 0 to 1
        cap_current = cap_cycle * 100
        
        # Animate speed
        speed = 150 + math.sin(self.time * 0.3) * 50
        
        # Animate shield (slow regeneration)
        shield_pct = (math.sin(self.time * 0.2) + 1) / 2
        shield_current = 500 * shield_pct
        
        # Update ship status
        ship_data = {
            'name': 'Rifter',
            'shield_current': shield_current,
            'shield_max': 500,
            'armor_current': 400,
            'armor_max': 400,
            'hull_current': 350,
            'hull_max': 350,
            'capacitor_current': cap_current,
            'capacitor_max': 100,
        }
        self.hud.update_ship_status(ship_data)
        
        # Update speed
        self.hud.update_speed(speed, Vec3(1000 + self.time * 10, 2000, 500))
        
        # Update target if active
        if self.has_target:
            distance = 5500 + math.sin(self.time * 0.8) * 1000
            target_data = {
                'name': 'Guristas Frigate',
                'distance': distance,
                'shield_current': 300,
                'shield_max': 400,
                'armor_current': 250,
                'armor_max': 300,
            }
            self.hud.update_target_info(target_data)
        
        # Periodic combat messages
        if int(self.time) % 5 == 0 and int(self.time * 10) % 50 == 0:
            messages = [
                "◢ Capacitor stable",
                "◢ Shields holding",
                "◢ All systems nominal",
            ]
            msg = messages[int(self.time / 5) % len(messages)]
            self.hud.add_combat_message(msg)
        
        return task.cont


if __name__ == "__main__":
    app = HUDDemo()
    app.run()
