#!/usr/bin/env python3
"""
EVE OFFLINE - Standalone GUI Demo
Demonstrates visual capabilities without requiring server
"""

import sys
import os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

try:
    import pygame
    PYGAME_AVAILABLE = True
except ImportError:
    PYGAME_AVAILABLE = False
    print("=" * 80)
    print("ERROR: pygame is not installed!")
    print("=" * 80)
    print("\nTo install pygame, run:")
    print("  pip install pygame")
    print("\nOr install all dependencies:")
    print("  pip install -r requirements.txt")
    print("\n" + "=" * 80)
    sys.exit(1)

from engine.core.ecs import World
from engine.components.game_components import (
    Position, Velocity, Health, Ship, Weapon, Player, Target
)
from engine.systems.game_systems import (
    MovementSystem, WeaponSystem, ShieldRechargeSystem
)
import random
import time


class GUIDemo:
    """Standalone GUI demonstration"""
    
    def __init__(self):
        # Game engine
        self.world = World()
        self.movement_sys = MovementSystem(self.world)
        self.weapon_sys = WeaponSystem(self.world)
        self.shield_sys = ShieldRechargeSystem(self.world)
        
        # Pygame
        self.width = 1280
        self.height = 720
        self.screen = None
        self.clock = None
        self.font = None
        self.font_small = None
        self.font_large = None
        
        # Colors
        self.COLOR_BG = (0, 0, 20)
        self.COLOR_STARS = (255, 255, 255)
        self.COLOR_PLAYER = (0, 255, 0)
        self.COLOR_ENEMY = (255, 0, 0)
        self.COLOR_TEXT = (200, 200, 255)
        self.COLOR_TEXT_BRIGHT = (255, 255, 255)
        self.COLOR_SHIELD = (0, 150, 255)
        self.COLOR_ARMOR = (255, 200, 0)
        self.COLOR_HULL = (180, 180, 180)
        self.COLOR_LASER = (255, 100, 100)
        
        # Camera
        self.camera_x = 0.0
        self.camera_y = 0.0
        self.zoom = 1.0
        
        # Star field
        self.stars = []
        
        # Entities
        self.player = None
        self.enemies = []
        
        # Game state
        self.running = True
        self.paused = False
        self.show_help = True
        self.combat_log = []
        self.max_log_entries = 5
        
        # Animation
        self.weapon_effects = []  # [(start_x, start_y, end_x, end_y, time)]
        
    def initialize(self):
        """Initialize Pygame and create entities"""
        pygame.init()
        self.screen = pygame.display.set_mode((self.width, self.height))
        pygame.display.set_caption("EVE OFFLINE - GUI Demo")
        self.clock = pygame.time.Clock()
        
        # Fonts
        self.font_large = pygame.font.Font(None, 36)
        self.font = pygame.font.Font(None, 24)
        self.font_small = pygame.font.Font(None, 18)
        
        # Generate star field
        for _ in range(300):
            x = random.randint(0, self.width)
            y = random.randint(0, self.height)
            brightness = random.randint(100, 255)
            size = random.choice([1, 1, 1, 2])
            self.stars.append((x, y, brightness, size))
        
        # Create player ship
        self.create_player()
        
        # Create enemy ships
        self.create_enemies()
        
        print("GUI Demo initialized successfully!")
        print("Press H to toggle help overlay")
    
    def create_player(self):
        """Create player ship"""
        self.player = self.world.create_entity("player_ship")
        
        self.player.add_component(Position(x=0, y=0, z=0))
        self.player.add_component(Velocity(vx=0, vy=0, vz=0, max_speed=350))
        self.player.add_component(Health(
            hull_hp=450, hull_max=450,
            armor_hp=500, armor_max=500,
            shield_hp=600, shield_max=600,
            shield_recharge_rate=8.0
        ))
        self.player.add_component(Ship(
            ship_name="Vexor",
            ship_class="Cruiser",
            race="Gallente"
        ))
        self.player.add_component(Player(
            player_id="demo_player",
            character_name="Demo Pilot"
        ))
        self.player.add_component(Target(
            locked_targets=[],
            max_targets=5,
            lock_time=2.0
        ))
        self.player.add_component(Weapon(
            weapon_type="Light Ion Blaster",
            damage=35,
            damage_type="thermal",
            optimal_range=5000,
            falloff_range=2500,
            cycle_time=2.0
        ))
    
    def create_enemies(self):
        """Create enemy ships"""
        enemy_configs = [
            ("Serpentis Scout", "Frigate", 3000, -2000),
            ("Guristas Pithum", "Frigate", -4000, 1500),
            ("Blood Raider", "Destroyer", 5000, 3000),
        ]
        
        for name, ship_class, x, y in enemy_configs:
            enemy = self.world.create_entity(f"enemy_{name}")
            
            enemy.add_component(Position(x=x, y=y, z=0))
            enemy.add_component(Velocity(vx=0, vy=0, vz=0, max_speed=250))
            enemy.add_component(Health(
                hull_hp=300, hull_max=300,
                armor_hp=350, armor_max=350,
                shield_hp=400, shield_max=400,
                shield_recharge_rate=5.0
            ))
            enemy.add_component(Ship(
                ship_name=name,
                ship_class=ship_class,
                race="Pirate"
            ))
            
            self.enemies.append(enemy)
    
    def add_log(self, message: str):
        """Add message to combat log"""
        self.combat_log.append((message, time.time()))
        if len(self.combat_log) > self.max_log_entries:
            self.combat_log.pop(0)
    
    def world_to_screen(self, x: float, y: float):
        """Convert world to screen coordinates"""
        screen_x = int((x - self.camera_x) * self.zoom + self.width / 2)
        screen_y = int((y - self.camera_y) * self.zoom + self.height / 2)
        return screen_x, screen_y
    
    def draw_star_field(self):
        """Draw background stars"""
        for x, y, brightness, size in self.stars:
            color = (brightness, brightness, brightness)
            if size == 1:
                pygame.draw.circle(self.screen, color, (x, y), 1)
            else:
                pygame.draw.circle(self.screen, color, (x, y), 2)
    
    def draw_entity(self, entity, is_player=False):
        """Draw a ship entity"""
        pos = entity.get_component(Position)
        health = entity.get_component(Health)
        ship = entity.get_component(Ship)
        
        screen_x, screen_y = self.world_to_screen(pos.x, pos.y)
        
        # Skip if off-screen
        if screen_x < -100 or screen_x > self.width + 100:
            return
        if screen_y < -100 or screen_y > self.height + 100:
            return
        
        # Draw ship
        if is_player:
            color = self.COLOR_PLAYER
            size = 10
        else:
            color = self.COLOR_ENEMY
            size = 8
        
        # Ship triangle
        points = [
            (screen_x, screen_y - size),
            (screen_x - size, screen_y + size),
            (screen_x + size, screen_y + size)
        ]
        pygame.draw.polygon(self.screen, color, points)
        pygame.draw.polygon(self.screen, self.COLOR_TEXT_BRIGHT, points, 1)
        
        # Selection circle for player
        if is_player:
            pygame.draw.circle(self.screen, color, (screen_x, screen_y), size * 2, 2)
        
        # Ship name
        name_text = self.font_small.render(ship.ship_name, True, self.COLOR_TEXT)
        name_rect = name_text.get_rect(center=(screen_x, screen_y - size - 20))
        self.screen.blit(name_text, name_rect)
        
        # Health bars
        self.draw_health_bar(screen_x, screen_y + size + 5,
                           health.shield_hp, health.shield_max,
                           health.armor_hp, health.armor_max,
                           health.hull_hp, health.hull_max)
        
        # Distance indicator
        if not is_player and self.player:
            player_pos = self.player.get_component(Position)
            dx = pos.x - player_pos.x
            dy = pos.y - player_pos.y
            distance = (dx*dx + dy*dy) ** 0.5
            
            dist_text = self.font_small.render(f"{distance:.0f}m", True, self.COLOR_TEXT)
            dist_rect = dist_text.get_rect(center=(screen_x, screen_y + size + 25))
            self.screen.blit(dist_text, dist_rect)
    
    def draw_health_bar(self, x, y, shield, shield_max, armor, armor_max, hull, hull_max):
        """Draw health bars"""
        bar_width = 60
        bar_height = 4
        spacing = 1
        
        # Shield
        if shield_max > 0:
            percent = shield / shield_max
            pygame.draw.rect(self.screen, (30, 60, 100),
                           (x - bar_width//2, y, bar_width, bar_height))
            pygame.draw.rect(self.screen, self.COLOR_SHIELD,
                           (x - bar_width//2, y, int(bar_width * percent), bar_height))
            y += bar_height + spacing
        
        # Armor
        if armor_max > 0:
            percent = armor / armor_max
            pygame.draw.rect(self.screen, (100, 80, 30),
                           (x - bar_width//2, y, bar_width, bar_height))
            pygame.draw.rect(self.screen, self.COLOR_ARMOR,
                           (x - bar_width//2, y, int(bar_width * percent), bar_height))
            y += bar_height + spacing
        
        # Hull
        if hull_max > 0:
            percent = hull / hull_max
            pygame.draw.rect(self.screen, (60, 60, 60),
                           (x - bar_width//2, y, bar_width, bar_height))
            pygame.draw.rect(self.screen, self.COLOR_HULL,
                           (x - bar_width//2, y, int(bar_width * percent), bar_height))
    
    def draw_weapon_effects(self):
        """Draw weapon fire effects"""
        current_time = time.time()
        effects_to_remove = []
        
        for i, (start_x, start_y, end_x, end_y, effect_time) in enumerate(self.weapon_effects):
            age = current_time - effect_time
            if age > 0.3:  # Effect lasts 0.3 seconds
                effects_to_remove.append(i)
                continue
            
            # Fade out
            alpha = int(255 * (1 - age / 0.3))
            color = (255, 100, 100, alpha)
            
            # Draw laser line
            pygame.draw.line(self.screen, self.COLOR_LASER, 
                           (start_x, start_y), (end_x, end_y), 2)
        
        # Remove old effects
        for i in reversed(effects_to_remove):
            self.weapon_effects.pop(i)
    
    def draw_ui(self):
        """Draw UI overlay"""
        # Top bar
        ui_surface = pygame.Surface((self.width, 100), pygame.SRCALPHA)
        ui_surface.fill((20, 20, 40, 200))
        self.screen.blit(ui_surface, (0, 0))
        
        # Title
        title = self.font_large.render("EVE OFFLINE - GUI Demo", True, self.COLOR_TEXT_BRIGHT)
        self.screen.blit(title, (10, 10))
        
        # Status
        if self.player:
            health = self.player.get_component(Health)
            ship = self.player.get_component(Ship)
            pos = self.player.get_component(Position)
            
            y = 50
            status_lines = [
                f"Ship: {ship.ship_name} ({ship.ship_class})",
                f"Position: ({pos.x:.0f}, {pos.y:.0f})",
                f"Shield: {health.shield_hp:.0f}/{health.shield_max:.0f}  "
                f"Armor: {health.armor_hp:.0f}/{health.armor_max:.0f}  "
                f"Hull: {health.hull_hp:.0f}/{health.hull_max:.0f}"
            ]
            
            for line in status_lines:
                text = self.font_small.render(line, True, self.COLOR_TEXT)
                self.screen.blit(text, (10, y))
                y += 20
        
        # Combat log (bottom left)
        log_y = self.height - 150
        log_surface = pygame.Surface((500, 140), pygame.SRCALPHA)
        log_surface.fill((20, 20, 40, 180))
        self.screen.blit(log_surface, (10, log_y))
        
        title = self.font.render("Combat Log", True, self.COLOR_TEXT_BRIGHT)
        self.screen.blit(title, (20, log_y + 10))
        
        y = log_y + 40
        current_time = time.time()
        for message, msg_time in self.combat_log:
            age = current_time - msg_time
            alpha = int(255 * max(0, 1 - age / 5))  # Fade over 5 seconds
            if alpha > 0:
                text = self.font_small.render(message, True, self.COLOR_TEXT)
                self.screen.blit(text, (20, y))
                y += 20
        
        # Controls (bottom right)
        controls_y = self.height - 120
        controls_x = self.width - 410
        controls_surface = pygame.Surface((400, 110), pygame.SRCALPHA)
        controls_surface.fill((20, 20, 40, 180))
        self.screen.blit(controls_surface, (controls_x, controls_y))
        
        y = controls_y + 10
        control_lines = [
            "Controls:",
            "WASD - Move player ship",
            "SPACE - Fire weapons at nearest enemy",
            "Arrow Keys - Pan camera | +/- - Zoom",
            "H - Toggle help | P - Pause | ESC - Quit"
        ]
        for line in control_lines:
            text = self.font_small.render(line, True, self.COLOR_TEXT)
            self.screen.blit(text, (controls_x + 10, y))
            y += 20
        
        # Help overlay
        if self.show_help:
            self.draw_help()
    
    def draw_help(self):
        """Draw help overlay"""
        help_width = 700
        help_height = 500
        help_x = (self.width - help_width) // 2
        help_y = (self.height - help_height) // 2
        
        # Background
        help_surface = pygame.Surface((help_width, help_height), pygame.SRCALPHA)
        help_surface.fill((0, 0, 0, 230))
        self.screen.blit(help_surface, (help_x, help_y))
        pygame.draw.rect(self.screen, self.COLOR_TEXT_BRIGHT,
                        (help_x, help_y, help_width, help_height), 3)
        
        # Title
        title = self.font_large.render("EVE OFFLINE - GUI Demo Help", True, self.COLOR_TEXT_BRIGHT)
        title_rect = title.get_rect(center=(self.width // 2, help_y + 30))
        self.screen.blit(title, title_rect)
        
        # Help content
        y = help_y + 80
        help_lines = [
            "WELCOME TO EVE OFFLINE GUI DEMO",
            "",
            "This demo showcases the visual capabilities of the game engine.",
            "",
            "WHAT YOU SEE:",
            "  • 2D space visualization with star field background",
            "  • Player ship (green triangle) and enemy ships (red triangles)",
            "  • Real-time health bars showing Shield (blue), Armor (yellow), Hull (gray)",
            "  • Distance indicators between ships",
            "  • Weapon fire effects and combat feedback",
            "  • Interactive camera controls with pan and zoom",
            "",
            "CONTROLS:",
            "  W/A/S/D       - Move your ship in space",
            "  SPACE         - Fire weapons at nearest enemy",
            "  Arrow Keys    - Pan camera view",
            "  +/-           - Zoom in/out",
            "  H             - Toggle this help",
            "  P             - Pause/Resume simulation",
            "  ESC           - Quit demo",
            "",
            "Press H to close this help and start exploring!",
        ]
        
        for line in help_lines:
            if line.isupper() and ":" in line:
                text = self.font.render(line, True, self.COLOR_TEXT_BRIGHT)
            else:
                text = self.font_small.render(line, True, self.COLOR_TEXT)
            text_rect = text.get_rect(center=(self.width // 2, y))
            self.screen.blit(text, text_rect)
            y += 25 if line.isupper() else 20
    
    def handle_input(self):
        """Handle user input"""
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                self.running = False
            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_ESCAPE:
                    self.running = False
                elif event.key == pygame.K_h:
                    self.show_help = not self.show_help
                elif event.key == pygame.K_p:
                    self.paused = not self.paused
                    self.add_log("Paused" if self.paused else "Resumed")
                elif event.key == pygame.K_SPACE:
                    self.fire_weapons()
        
        if self.paused:
            return
        
        # Player movement
        if self.player:
            keys = pygame.key.get_pressed()
            vel = self.player.get_component(Velocity)
            speed = 5.0
            
            if keys[pygame.K_w]:
                vel.vy = -speed
            elif keys[pygame.K_s]:
                vel.vy = speed
            else:
                vel.vy = 0
            
            if keys[pygame.K_a]:
                vel.vx = -speed
            elif keys[pygame.K_d]:
                vel.vx = speed
            else:
                vel.vx = 0
        
        # Camera controls
        keys = pygame.key.get_pressed()
        cam_speed = 10 / self.zoom
        
        if keys[pygame.K_LEFT]:
            self.camera_x -= cam_speed
        if keys[pygame.K_RIGHT]:
            self.camera_x += cam_speed
        if keys[pygame.K_UP]:
            self.camera_y -= cam_speed
        if keys[pygame.K_DOWN]:
            self.camera_y += cam_speed
        if keys[pygame.K_EQUALS] or keys[pygame.K_PLUS]:
            self.zoom = min(3.0, self.zoom * 1.02)
        if keys[pygame.K_MINUS]:
            self.zoom = max(0.3, self.zoom / 1.02)
    
    def fire_weapons(self):
        """Fire weapons at nearest enemy"""
        if not self.player or not self.enemies:
            return
        
        player_pos = self.player.get_component(Position)
        
        # Find nearest enemy
        nearest = None
        nearest_dist = float('inf')
        
        for enemy in self.enemies:
            enemy_health = enemy.get_component(Health)
            if enemy_health.hull_hp <= 0:
                continue
            
            enemy_pos = enemy.get_component(Position)
            dx = enemy_pos.x - player_pos.x
            dy = enemy_pos.y - player_pos.y
            dist = (dx*dx + dy*dy) ** 0.5
            
            if dist < nearest_dist:
                nearest = enemy
                nearest_dist = dist
        
        if nearest:
            # Deal damage
            weapon = self.player.get_component(Weapon)
            enemy_health = nearest.get_component(Health)
            enemy_ship = nearest.get_component(Ship)
            
            damage = weapon.damage
            
            # Apply to shield first
            if enemy_health.shield_hp > 0:
                actual_damage = min(damage, enemy_health.shield_hp)
                enemy_health.shield_hp -= actual_damage
                self.add_log(f"Hit {enemy_ship.ship_name} shield for {actual_damage:.0f} damage")
            elif enemy_health.armor_hp > 0:
                actual_damage = min(damage, enemy_health.armor_hp)
                enemy_health.armor_hp -= actual_damage
                self.add_log(f"Hit {enemy_ship.ship_name} armor for {actual_damage:.0f} damage")
            else:
                actual_damage = min(damage, enemy_health.hull_hp)
                enemy_health.hull_hp -= actual_damage
                self.add_log(f"Hit {enemy_ship.ship_name} hull for {actual_damage:.0f} damage")
                if enemy_health.hull_hp <= 0:
                    self.add_log(f"💥 {enemy_ship.ship_name} DESTROYED!")
            
            # Visual effect
            enemy_pos = nearest.get_component(Position)
            start_x, start_y = self.world_to_screen(player_pos.x, player_pos.y)
            end_x, end_y = self.world_to_screen(enemy_pos.x, enemy_pos.y)
            self.weapon_effects.append((start_x, start_y, end_x, end_y, time.time()))
    
    def update(self, dt: float):
        """Update game state"""
        if self.paused:
            return
        
        # Update systems
        self.movement_sys.update(dt)
        self.shield_sys.update(dt)
        
        # Center camera on player
        if self.player:
            pos = self.player.get_component(Position)
            # Smoothly follow player
            self.camera_x = self.camera_x * 0.95 + pos.x * 0.05
            self.camera_y = self.camera_y * 0.95 + pos.y * 0.05
    
    def run(self):
        """Main game loop"""
        self.initialize()
        self.add_log("Welcome to EVE OFFLINE GUI Demo!")
        self.add_log("Press H for help")
        
        while self.running:
            dt = self.clock.tick(60) / 1000.0  # 60 FPS, dt in seconds
            
            self.handle_input()
            self.update(dt)
            
            # Render
            self.screen.fill(self.COLOR_BG)
            self.draw_star_field()
            
            # Draw entities
            for enemy in self.enemies:
                self.draw_entity(enemy, is_player=False)
            
            if self.player:
                self.draw_entity(self.player, is_player=True)
            
            self.draw_weapon_effects()
            self.draw_ui()
            
            pygame.display.flip()
        
        pygame.quit()
        print("Demo closed. Thank you for trying EVE OFFLINE!")


if __name__ == "__main__":
    print("=" * 80)
    print("EVE OFFLINE - GUI Demo")
    print("=" * 80)
    print()
    
    demo = GUIDemo()
    demo.run()
