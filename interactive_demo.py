#!/usr/bin/env python3
"""
EVE OFFLINE - Interactive Gameplay Demo
Shows all features in an interactive text-based interface
"""

import sys
import os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from engine.core.ecs import World
from engine.components.game_components import (
    Position, Velocity, Health, Ship, Weapon, Player,
    Fitting, Skills, DroneBay, Drone, SkillTraining,
    Mission, WarpDrive, Celestial, Docking, Target, Capacitor
)
from engine.systems.game_systems import (
    MovementSystem, CapacitorSystem, ShieldRechargeSystem,
    WeaponSystem, AISystem, TargetingSystem, FittingSystem,
    DroneSystem, SkillSystem
)
from engine.systems.mission_system import MissionSystem
from engine.systems.navigation_system import NavigationSystem
from engine.utils.data_loader import DataLoader
import time


class InteractiveDemo:
    """Interactive gameplay demonstration"""
    
    def __init__(self):
        self.world = World()
        self.data_loader = DataLoader()
        self.data_loader.load_all()
        
        # Setup systems
        self.fitting_sys = FittingSystem(self.world)
        self.drone_sys = DroneSystem(self.world)
        self.skill_sys = SkillSystem(self.world)
        self.mission_sys = MissionSystem(self.world)
        self.nav_sys = NavigationSystem(self.world)
        self.weapon_sys = WeaponSystem(self.world)
        self.movement_sys = MovementSystem(self.world)
        self.targeting_sys = TargetingSystem(self.world)
        
        # Create player ship
        self.player = self.create_player_ship()
        
        # Create some enemies
        self.enemies = []
        self.create_enemies()
        
        # Create station
        self.station = self.create_station()
        
    def create_player_ship(self):
        """Create player's ship"""
        ship = self.world.create_entity("player_ship")
        
        ship.add_component(Position(x=0, y=0, z=0))
        ship.add_component(Velocity(vx=0, vy=0, vz=0, max_speed=350))
        ship.add_component(Health(
            hull_hp=450, hull_max=450,
            armor_hp=500, armor_max=500,
            shield_hp=600, shield_max=600,
            shield_recharge_rate=8.0,
            shield_em_resist=0.0,
            shield_thermal_resist=0.2,
            shield_kinetic_resist=0.4,
            shield_explosive_resist=0.5
        ))
        ship.add_component(Capacitor(capacitor=350, capacitor_max=350, recharge_rate=5.0))
        ship.add_component(Ship(
            ship_name="Vexor",
            ship_class="Cruiser",
            race="Gallente",
            cpu=0,
            cpu_max=270,
            powergrid=0,
            powergrid_max=75,
            signature_radius=125,
            scan_resolution=350
        ))
        ship.add_component(Player(player_id="player1", character_name="Commander_Shepard"))
        ship.add_component(Fitting(
            high_slots=[None, None, None, None],
            mid_slots=[None, None, None, None],
            low_slots=[None, None, None],
            rig_slots=[None, None],
            cargo=[],
            cargo_capacity=400
        ))
        ship.add_component(Skills(
            skills={'Gunnery': 4, 'Drones': 5, 'Navigation': 3},
            skill_points={'Gunnery': 512000, 'Drones': 1280000, 'Navigation': 128000}
        ))
        ship.add_component(DroneBay(
            drone_bandwidth=75.0,
            drone_bay_capacity=75.0,
            drones=['hammerhead_i'] * 5,
            active_drones=[]
        ))
        ship.add_component(WarpDrive(
            warp_speed=3.0,
            align_time=6.0
        ))
        ship.add_component(Target(locked_targets=[]))
        
        return ship
    
    def create_enemies(self):
        """Create enemy ships"""
        for i in range(3):
            enemy = self.world.create_entity(f"serpentis_{i}")
            
            enemy.add_component(Position(x=8000 + i*1000, y=2000, z=0))
            enemy.add_component(Velocity(max_speed=280))
            enemy.add_component(Health(
                hull_hp=300, hull_max=300,
                armor_hp=250, armor_max=250,
                shield_hp=400, shield_max=400,
                shield_recharge_rate=4.0
            ))
            enemy.add_component(Ship(
                ship_name=f"Serpentis Scout {i+1}",
                ship_class="Frigate",
                race="Serpentis"
            ))
            enemy.add_component(Weapon(
                weapon_type="Blasters",
                damage=15,
                optimal_range=5000,
                rate_of_fire=3.0
            ))
            
            self.enemies.append(enemy)
    
    def create_station(self):
        """Create a station"""
        station = self.world.create_entity("jita_4_4")
        
        station.add_component(Position(x=150000000, y=50000000, z=0))
        station.add_component(Celestial(
            celestial_type="station",
            celestial_name="Jita IV - Moon 4 - Caldari Navy Assembly Plant",
            dockable=True
        ))
        
        return station
    
    def print_banner(self):
        """Print game banner"""
        print("\n" + "="*80)
        print("  EVE OFFLINE - Interactive Gameplay Demo")
        print("  Your ship: Vexor (Gallente Cruiser)")
        print("="*80 + "\n")
    
    def print_status(self):
        """Print current ship status"""
        pos = self.player.get_component(Position)
        health = self.player.get_component(Health)
        ship = self.player.get_component(Ship)
        cap = self.player.get_component(Capacitor)
        fitting = self.player.get_component(Fitting)
        drones = self.player.get_component(DroneBay)
        
        print("\n" + "-"*80)
        print(f"📍 Position: ({pos.x:.0f}, {pos.y:.0f}, {pos.z:.0f})")
        print(f"🛡️  Shield: {health.shield_hp:.0f}/{health.shield_max:.0f} HP")
        print(f"🔧 Armor: {health.armor_hp:.0f}/{health.armor_max:.0f} HP")
        print(f"⚙️  Hull: {health.hull_hp:.0f}/{health.hull_max:.0f} HP")
        print(f"⚡ Capacitor: {cap.capacitor:.0f}/{cap.capacitor_max:.0f} GJ")
        print(f"🤖 Drones: {len(drones.active_drones)}/{len(drones.drones)} active")
        print(f"🔌 CPU: {ship.cpu:.0f}/{ship.cpu_max:.0f} tf")
        print(f"⚡ PowerGrid: {ship.powergrid:.0f}/{ship.powergrid_max:.0f} MW")
        print("-"*80)
    
    def print_menu(self):
        """Print command menu"""
        print("\n📋 Available Commands:")
        print("  1. Status      - Show ship status")
        print("  2. Fit         - Fit modules to ship")
        print("  3. Drones      - Launch and control drones")
        print("  4. Combat      - Engage enemies")
        print("  5. Skills      - View and train skills")
        print("  6. Navigate    - Warp to locations")
        print("  7. Mission     - Accept and run missions")
        print("  8. Scan        - Scan for nearby objects")
        print("  9. Help        - Show this menu")
        print("  0. Exit        - Quit demo")
        print()
    
    def cmd_fit_modules(self):
        """Fit modules to ship"""
        print("\n🔧 SHIP FITTING")
        print("-"*80)
        
        # Show available modules
        print("\nAvailable Modules:")
        print("  1. 200mm Railgun I (High)")
        print("  2. Medium Shield Booster I (Mid)")
        print("  3. 10MN Afterburner I (Mid)")
        print("  4. Magnetic Field Stabilizer I (Low)")
        print("  5. Damage Control I (Low)")
        
        choice = input("\nFit which module (1-5, 0 to cancel)? ").strip()
        
        modules = {
            '1': ('railgun_medium', 'high', 0),
            '2': ('medium_shield_booster', 'mid', 0),
            '3': ('10mn_afterburner', 'mid', 1),
            '4': ('magnetic_field_stabilizer', 'low', 0),
            '5': ('damage_control', 'low', 1)
        }
        
        if choice in modules:
            module_id, slot_type, slot_idx = modules[choice]
            success = self.fitting_sys.fit_module(self.player, module_id, slot_type, slot_idx)
            
            if success:
                print(f"✅ Successfully fitted {module_id} to {slot_type} slot {slot_idx}")
            else:
                print(f"❌ Failed to fit module (check CPU/PowerGrid)")
        
        fitting = self.player.get_component(Fitting)
        print(f"\n📋 Current Fitting:")
        print(f"  High: {fitting.high_slots}")
        print(f"  Mid: {fitting.mid_slots}")
        print(f"  Low: {fitting.low_slots}")
    
    def cmd_drones(self):
        """Control drones"""
        print("\n🤖 DRONE CONTROL")
        print("-"*80)
        
        drones = self.player.get_component(DroneBay)
        print(f"\nDrone Bay: {len(drones.drones)} drones available")
        print(f"Active: {len(drones.active_drones)}")
        print(f"Bandwidth: {len(drones.active_drones)*10}/{drones.drone_bandwidth} Mbit/s")
        
        print("\n1. Launch all drones")
        print("2. Recall all drones")
        print("3. Engage target")
        print("0. Back")
        
        choice = input("\nChoice? ").strip()
        
        if choice == '1':
            count = 0
            for i in range(len(drones.drones)):
                if self.drone_sys.launch_drone(self.player, i):
                    count += 1
            print(f"✅ Launched {count} drones")
            
        elif choice == '2':
            self.drone_sys.recall_all_drones(self.player)
            print("✅ All drones recalled")
            
        elif choice == '3':
            if self.enemies:
                target = self.enemies[0]
                self.drone_sys.engage_target(self.player, target.id)
                print(f"✅ Drones engaging {target.get_component(Ship).ship_name}")
    
    def cmd_combat(self):
        """Engage in combat"""
        print("\n⚔️  COMBAT")
        print("-"*80)
        
        print(f"\nEnemies in range:")
        for i, enemy in enumerate(self.enemies):
            ship = enemy.get_component(Ship)
            health = enemy.get_component(Health)
            pos = enemy.get_component(Position)
            
            player_pos = self.player.get_component(Position)
            distance = ((pos.x - player_pos.x)**2 + (pos.y - player_pos.y)**2)**0.5
            
            print(f"  {i+1}. {ship.ship_name}")
            print(f"     Shield: {health.shield_hp:.0f}/{health.shield_max:.0f}")
            print(f"     Distance: {distance:.0f}m")
        
        choice = input("\nTarget which enemy (1-3, 0 to cancel)? ").strip()
        
        if choice in ['1', '2', '3']:
            idx = int(choice) - 1
            target = self.enemies[idx]
            
            # Lock target
            target_comp = self.player.get_component(Target)
            if target.id not in target_comp.locked_targets:
                target_comp.locked_targets.append(target.id)
                print(f"✅ Target locked: {target.get_component(Ship).ship_name}")
            
            # Fire weapons
            print("\n🔫 Opening fire...")
            time.sleep(0.5)
            
            # Simulate damage
            health = target.get_component(Health)
            damage = 35 * (1 + self.skill_sys.get_skill_bonus(self.player, 'Gunnery', 'damage') / 100)
            health.shield_hp = max(0, health.shield_hp - damage)
            
            print(f"💥 HIT! Damage: {damage:.1f}")
            print(f"   Enemy shield: {health.shield_hp:.0f}/{health.shield_max:.0f}")
    
    def cmd_skills(self):
        """View and train skills"""
        print("\n📚 SKILLS")
        print("-"*80)
        
        skills = self.player.get_component(Skills)
        print("\nTrained Skills:")
        for skill_name, level in skills.skills.items():
            sp = skills.skill_points.get(skill_name, 0)
            bonus = self.skill_sys.get_skill_bonus(self.player, skill_name, 'damage')
            print(f"  {skill_name}: Level {level} ({sp:,} SP)")
            if bonus > 0:
                print(f"    Bonus: +{bonus:.1f}% damage")
    
    def cmd_navigate(self):
        """Navigate to locations"""
        print("\n🚀 NAVIGATION")
        print("-"*80)
        
        pos = self.player.get_component(Position)
        station_pos = self.station.get_component(Position)
        distance = ((station_pos.x - pos.x)**2 + (station_pos.y - pos.y)**2)**0.5
        
        print(f"\n1. Warp to {self.station.get_component(Celestial).celestial_name}")
        print(f"   Distance: {distance/149597870700:.2f} AU")
        print("2. Move forward")
        print("0. Back")
        
        choice = input("\nChoice? ").strip()
        
        if choice == '1':
            print("\n⏱️  Initiating warp...")
            print("   Aligning to destination...")
            time.sleep(1)
            print("   ✅ Entering warp!")
            time.sleep(1)
            print("   🌟 Traveling at FTL speeds...")
            time.sleep(1)
            
            # Warp to station
            pos.x = station_pos.x - 5000
            pos.y = station_pos.y - 5000
            pos.z = station_pos.z
            
            print("   ✅ Warp complete!")
            print(f"   Position: ({pos.x:.0f}, {pos.y:.0f}, {pos.z:.0f})")
            
        elif choice == '2':
            vel = self.player.get_component(Velocity)
            vel.vx = 100
            self.movement_sys.update(1.0)
            print("✅ Moving forward...")
    
    def cmd_scan(self):
        """Scan for nearby objects"""
        print("\n📡 DIRECTIONAL SCANNER")
        print("-"*80)
        
        player_pos = self.player.get_component(Position)
        
        print("\nNearby Objects:")
        
        # Show enemies
        for enemy in self.enemies:
            ship = enemy.get_component(Ship)
            pos = enemy.get_component(Position)
            distance = ((pos.x - player_pos.x)**2 + (pos.y - player_pos.y)**2)**0.5
            print(f"  🔴 {ship.ship_name} - {distance:.0f}m")
        
        # Show station
        station_pos = self.station.get_component(Position)
        distance = ((station_pos.x - player_pos.x)**2 + (station_pos.y - player_pos.y)**2)**0.5
        print(f"  🏢 {self.station.get_component(Celestial).celestial_name} - {distance/149597870700:.2f} AU")
    
    def cmd_mission(self):
        """Mission operations"""
        print("\n🎯 MISSION CONTROL")
        print("-"*80)
        
        print("\nAvailable Missions:")
        print("  1. Destroy Serpentis Rats (Combat)")
        print("     Reward: 150,000 ISK + 500 LP")
        print("  2. Reconnaissance Flight (Exploration)")
        print("     Reward: 100,000 ISK + 300 LP")
        
        choice = input("\nAccept mission (1-2, 0 to cancel)? ").strip()
        
        if choice in ['1', '2']:
            mission_name = "Destroy Serpentis Rats" if choice == '1' else "Reconnaissance Flight"
            print(f"\n✅ Mission accepted: {mission_name}")
            print("   Check your mission journal for details")
    
    def run(self):
        """Run interactive demo"""
        self.print_banner()
        self.print_menu()
        
        while True:
            try:
                cmd = input("\n⌨️  Command: ").strip()
                
                if cmd == '1':
                    self.print_status()
                elif cmd == '2':
                    self.cmd_fit_modules()
                elif cmd == '3':
                    self.cmd_drones()
                elif cmd == '4':
                    self.cmd_combat()
                elif cmd == '5':
                    self.cmd_skills()
                elif cmd == '6':
                    self.cmd_navigate()
                elif cmd == '7':
                    self.cmd_mission()
                elif cmd == '8':
                    self.cmd_scan()
                elif cmd == '9':
                    self.print_menu()
                elif cmd == '0':
                    print("\n👋 Thanks for playing EVE OFFLINE! Fly safe o7")
                    break
                else:
                    print("❌ Invalid command. Type 9 for help.")
                    
            except KeyboardInterrupt:
                print("\n\n👋 Thanks for playing EVE OFFLINE! Fly safe o7")
                break
            except Exception as e:
                print(f"❌ Error: {e}")


if __name__ == "__main__":
    demo = InteractiveDemo()
    demo.run()
