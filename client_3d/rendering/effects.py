"""
Visual Effects for 3D Client
Handles weapon fire effects, explosions, and other visual feedback
"""

from panda3d.core import Vec3, Vec4, Point3, NodePath, LineSegs
from panda3d.core import BillboardEffect
from direct.interval.IntervalGlobal import Sequence, Func, LerpScaleInterval, LerpColorScaleInterval, LerpPosInterval
import random


class EffectsManager:
    """
    Manages visual effects in the game
    Handles weapon fire, explosions, and other effects
    """
    
    def __init__(self, render, loader):
        self.render = render
        self.loader = loader
        self.active_effects = []  # List of active effect NodePaths
        
        # Effect settings
        self.beam_duration = 0.2  # seconds
        self.muzzle_flash_duration = 0.1
        
        print("[Effects] Effects manager initialized")
    
    def create_weapon_beam(self, start_pos: Vec3, end_pos: Vec3, color: Vec4 = Vec4(1, 0.3, 0.1, 1)):
        """
        Create a weapon beam effect from start to end position
        
        Args:
            start_pos: Starting position (weapon mount)
            end_pos: End position (target)
            color: Beam color (default: orange-red for lasers)
        """
        # Create a line segment for the beam
        segs = LineSegs()
        segs.setThickness(3.0)
        segs.setColor(color)
        
        # Draw line from start to end
        segs.moveTo(start_pos)
        segs.drawTo(end_pos)
        
        # Create node and attach to scene
        beam_node = segs.create()
        beam_np = self.render.attachNewNode(beam_node)
        beam_np.setTransparency(True)
        
        # Create fade-out sequence
        fade_seq = Sequence(
            LerpColorScaleInterval(beam_np, self.beam_duration, Vec4(1, 1, 1, 0)),
            Func(self._remove_effect, beam_np)
        )
        fade_seq.start()
        
        self.active_effects.append(beam_np)
        return beam_np
    
    def create_projectile_trail(self, start_pos: Vec3, end_pos: Vec3, color: Vec4 = Vec4(0.5, 0.7, 1, 1)):
        """
        Create a projectile trail effect (for autocannons, missiles, etc.)
        
        Args:
            start_pos: Starting position
            end_pos: End position
            color: Trail color (default: blue-white for projectiles)
        """
        # Similar to beam but thicker and different color
        segs = LineSegs()
        segs.setThickness(2.0)
        segs.setColor(color)
        
        segs.moveTo(start_pos)
        segs.drawTo(end_pos)
        
        trail_node = segs.create()
        trail_np = self.render.attachNewNode(trail_node)
        trail_np.setTransparency(True)
        
        # Fade out faster than beams
        fade_seq = Sequence(
            LerpColorScaleInterval(trail_np, self.beam_duration * 0.5, Vec4(1, 1, 1, 0)),
            Func(self._remove_effect, trail_np)
        )
        fade_seq.start()
        
        self.active_effects.append(trail_np)
        return trail_np
    
    def create_muzzle_flash(self, position: Vec3, color: Vec4 = Vec4(1, 0.8, 0.3, 1)):
        """
        Create a muzzle flash at weapon position
        
        Args:
            position: Position of weapon
            color: Flash color (default: yellow-orange)
        """
        # Create a simple sphere for muzzle flash
        try:
            flash = self.loader.loadModel("models/sphere")
            if flash:
                flash.reparentTo(self.render)
                flash.setPos(position)
                flash.setScale(1.5)
                flash.setColor(color)
                flash.setTransparency(True)
                
                # Billboard effect to always face camera
                flash.setBillboardPointEye()
                
                # Animate: scale up quickly then fade out
                flash_seq = Sequence(
                    LerpScaleInterval(flash, self.muzzle_flash_duration * 0.3, Vec3(3, 3, 3)),
                    LerpColorScaleInterval(flash, self.muzzle_flash_duration * 0.7, Vec4(1, 1, 1, 0)),
                    Func(self._remove_effect, flash)
                )
                flash_seq.start()
                
                self.active_effects.append(flash)
                return flash
        except Exception as e:
            # Fallback: don't create muzzle flash if models unavailable
            print(f"[Effects] Could not create muzzle flash: {e}")
            pass
        
        return None
    
    def create_impact_effect(self, position: Vec3, color: Vec4 = Vec4(1, 0.5, 0, 1)):
        """
        Create an impact effect at hit position
        
        Args:
            position: Impact position
            color: Impact color (default: orange)
        """
        # Create multiple small particles for impact
        for i in range(3):
            try:
                particle = self.loader.loadModel("models/sphere")
                if particle:
                    # Random offset around impact point
                    offset = Vec3(
                        random.uniform(-2, 2),
                        random.uniform(-2, 2),
                        random.uniform(-2, 2)
                    )
                    
                    particle.reparentTo(self.render)
                    particle.setPos(position + offset)
                    particle.setScale(0.5)
                    particle.setColor(color)
                    particle.setTransparency(True)
                    
                    # Billboard effect
                    particle.setBillboardPointEye()
                    
                    # Animate: expand and fade
                    particle_seq = Sequence(
                        LerpScaleInterval(particle, 0.3, Vec3(1.5, 1.5, 1.5)),
                        LerpColorScaleInterval(particle, 0.2, Vec4(1, 1, 1, 0)),
                        Func(self._remove_effect, particle)
                    )
                    particle_seq.start()
                    
                    self.active_effects.append(particle)
            except Exception as e:
                # Fallback if models unavailable
                pass
    
    def create_explosion_effect(self, position: Vec3, size: float = 5.0):
        """
        Create an explosion effect at position
        
        Args:
            position: Explosion position
            size: Size multiplier for the explosion
        """
        # Create expanding sphere for explosion
        try:
            explosion = self.loader.loadModel("models/sphere")
            if explosion:
                explosion.reparentTo(self.render)
                explosion.setPos(position)
                explosion.setScale(size * 0.5)
                explosion.setColor(1, 0.5, 0, 1)  # Orange
                explosion.setTransparency(True)
                explosion.setBillboardPointEye()
                
                # Animate: expand quickly and fade
                explosion_seq = Sequence(
                    LerpScaleInterval(explosion, 0.4, Vec3(size * 3, size * 3, size * 3)),
                    LerpColorScaleInterval(explosion, 0.3, Vec4(0.3, 0.1, 0, 0)),
                    Func(self._remove_effect, explosion)
                )
                explosion_seq.start()
                
                self.active_effects.append(explosion)
        except Exception as e:
            print(f"[Effects] Could not create explosion: {e}")
        
        # Create debris particles
        num_particles = int(size * 2)
        for i in range(num_particles):
            try:
                particle = self.loader.loadModel("models/sphere")
                if particle:
                    # Random direction
                    direction = Vec3(
                        random.uniform(-1, 1),
                        random.uniform(-1, 1),
                        random.uniform(-1, 1)
                    ).normalized()
                    
                    particle.reparentTo(self.render)
                    particle.setPos(position)
                    particle.setScale(0.3)
                    
                    # Random debris color (orange to dark gray)
                    brightness = random.uniform(0.3, 1.0)
                    particle.setColor(brightness, brightness * 0.5, 0, 1)
                    particle.setTransparency(True)
                    particle.setBillboardPointEye()
                    
                    # Animate: fly out and fade
                    end_pos = position + direction * size * random.uniform(3, 8)
                    particle_seq = Sequence(
                        LerpPosInterval(particle, 0.6, end_pos),
                        LerpColorScaleInterval(particle, 0.2, Vec4(1, 1, 1, 0)),
                        Func(self._remove_effect, particle)
                    )
                    particle_seq.start()
                    
                    self.active_effects.append(particle)
            except Exception as e:
                pass
    
    def create_shield_hit_effect(self, position: Vec3, normal: Vec3 = Vec3(0, 0, 1)):
        """
        Create a shield hit effect at position
        Shows the shield absorbing damage with a blue ripple
        
        Args:
            position: Hit position
            normal: Normal vector at hit point (direction)
        """
        try:
            # Create shield ripple
            ripple = self.loader.loadModel("models/sphere")
            if ripple:
                ripple.reparentTo(self.render)
                ripple.setPos(position)
                ripple.setScale(2)
                ripple.setColor(0.3, 0.5, 1, 0.6)  # Blue shield color
                ripple.setTransparency(True)
                ripple.setBillboardPointEye()
                
                # Animate: expand and fade quickly
                ripple_seq = Sequence(
                    LerpScaleInterval(ripple, 0.3, Vec3(6, 6, 6)),
                    LerpColorScaleInterval(ripple, 0.2, Vec4(1, 1, 1, 0)),
                    Func(self._remove_effect, ripple)
                )
                ripple_seq.start()
                
                self.active_effects.append(ripple)
        except Exception as e:
            print(f"[Effects] Could not create shield hit: {e}")
    
    def create_weapon_fire_effect(self, shooter_pos: Vec3, target_pos: Vec3, weapon_type: str = "laser"):
        """
        Create complete weapon fire effect with beam, flash, and impact
        
        Args:
            shooter_pos: Position of shooter
            target_pos: Position of target
            weapon_type: Type of weapon ("laser", "projectile", "missile")
        """
        # Determine effect colors based on weapon type
        if weapon_type == "laser" or weapon_type == "beam":
            beam_color = Vec4(1, 0.3, 0.1, 1)  # Red-orange
            flash_color = Vec4(1, 0.8, 0.3, 1)  # Yellow
            impact_color = Vec4(1, 0.5, 0, 1)   # Orange
        elif weapon_type == "projectile" or weapon_type == "autocannon":
            beam_color = Vec4(0.5, 0.7, 1, 1)   # Blue-white
            flash_color = Vec4(1, 1, 0.8, 1)    # White-yellow
            impact_color = Vec4(0.8, 0.8, 0.2, 1)  # Yellow
        elif weapon_type == "missile":
            beam_color = Vec4(1, 0.6, 0.2, 1)   # Orange trail
            flash_color = Vec4(1, 0.3, 0, 1)    # Red-orange
            impact_color = Vec4(1, 0.3, 0, 1)   # Explosion orange
        else:
            # Default colors
            beam_color = Vec4(1, 0.5, 0.2, 1)
            flash_color = Vec4(1, 0.8, 0.3, 1)
            impact_color = Vec4(1, 0.5, 0, 1)
        
        # Create muzzle flash at shooter
        self.create_muzzle_flash(shooter_pos, flash_color)
        
        # Create beam/trail
        if weapon_type == "projectile" or weapon_type == "autocannon":
            self.create_projectile_trail(shooter_pos, target_pos, beam_color)
        else:
            self.create_weapon_beam(shooter_pos, target_pos, beam_color)
        
        # Create impact at target
        self.create_impact_effect(target_pos, impact_color)
    
    def _remove_effect(self, effect_node: NodePath):
        """Remove an effect from the scene"""
        if effect_node in self.active_effects:
            self.active_effects.remove(effect_node)
        effect_node.removeNode()
    
    def clear_all_effects(self):
        """Clear all active effects"""
        for effect in self.active_effects[:]:
            self._remove_effect(effect)
        print("[Effects] All effects cleared")
    
    def update(self, dt: float):
        """
        Update active effects
        Called each frame
        
        Args:
            dt: Delta time since last frame
        """
        # Most effects are handled by Panda3D intervals
        # This is here for future effects that need per-frame updates
        pass
