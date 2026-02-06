"""
Advanced Particle System for 3D Client
Generates particle effects for various game events
"""

from panda3d.core import Vec3, Vec4, Point3, NodePath
from panda3d.core import GeomVertexFormat, GeomVertexData, Geom, GeomNode
from panda3d.core import GeomPoints, GeomVertexWriter
from direct.interval.IntervalGlobal import Sequence, Func, LerpScaleInterval, LerpColorScaleInterval, LerpPosInterval
import random
import math


class ParticleEffect:
    """Represents a single particle effect instance"""
    
    def __init__(self, node: NodePath, lifetime: float):
        self.node = node
        self.lifetime = lifetime
        self.age = 0.0
        self.is_alive = True
    
    def update(self, dt: float):
        """Update particle effect"""
        self.age += dt
        if self.age >= self.lifetime:
            self.is_alive = False
            self.cleanup()
    
    def cleanup(self):
        """Clean up particle effect"""
        if self.node:
            self.node.removeNode()
            self.node = None


class ParticleSystem:
    """
    Advanced particle system for various visual effects
    
    Generates and manages particle effects for:
    - Engine trails
    - Shield impacts
    - Explosions
    - Debris
    - Warp effects
    """
    
    def __init__(self, render):
        self.render = render
        self.active_particles = []
        self.max_particles = 1000
        
        print("[Particles] Particle system initialized")
    
    def create_engine_trail(self, position: Vec3, velocity: Vec3, color: Vec4 = Vec4(0.2, 0.4, 1.0, 0.8)):
        """
        Create engine trail particles
        
        Args:
            position: Engine position
            velocity: Ship velocity (for trail direction)
            color: Trail color (default: blue)
        """
        # Create multiple small particles
        num_particles = 5
        
        for i in range(num_particles):
            # Create particle
            particle = self._create_point_particle(
                position,
                size=random.uniform(0.3, 0.8),
                color=color
            )
            
            if particle:
                # Add random spread
                spread = Vec3(
                    random.uniform(-0.5, 0.5),
                    random.uniform(-0.5, 0.5),
                    random.uniform(-0.5, 0.5)
                )
                
                # Move particle in opposite direction of velocity
                end_pos = position - velocity * 2.0 + spread
                
                # Animate particle
                lifetime = random.uniform(0.5, 1.0)
                particle_seq = Sequence(
                    LerpPosInterval(particle, lifetime, end_pos),
                    LerpColorScaleInterval(particle, lifetime * 0.5, Vec4(1, 1, 1, 0)),
                    Func(self._remove_particle, particle)
                )
                particle_seq.start()
                
                effect = ParticleEffect(particle, lifetime)
                self.active_particles.append(effect)
    
    def create_shield_impact_particles(self, position: Vec3, normal: Vec3 = Vec3(0, 0, 1)):
        """
        Create particles for shield impact
        
        Args:
            position: Impact position
            normal: Impact normal direction
        """
        num_particles = 10
        
        for i in range(num_particles):
            # Shield impact particles are blue/cyan
            color = Vec4(
                random.uniform(0.2, 0.5),
                random.uniform(0.5, 1.0),
                random.uniform(0.8, 1.0),
                1.0
            )
            
            particle = self._create_point_particle(
                position,
                size=random.uniform(0.5, 1.2),
                color=color
            )
            
            if particle:
                # Particles bounce away from impact point
                angle = random.uniform(0, 2 * math.pi)
                spread_radius = random.uniform(1.0, 3.0)
                
                offset = Vec3(
                    math.cos(angle) * spread_radius,
                    math.sin(angle) * spread_radius,
                    random.uniform(0, 2.0)
                )
                
                end_pos = position + offset + normal * 2.0
                
                lifetime = random.uniform(0.3, 0.6)
                particle_seq = Sequence(
                    LerpPosInterval(particle, lifetime, end_pos),
                    LerpScaleInterval(particle, lifetime * 0.5, Vec3(0.1, 0.1, 0.1)),
                    LerpColorScaleInterval(particle, lifetime * 0.5, Vec4(1, 1, 1, 0)),
                    Func(self._remove_particle, particle)
                )
                particle_seq.start()
                
                effect = ParticleEffect(particle, lifetime)
                self.active_particles.append(effect)
    
    def create_explosion_particles(self, position: Vec3, size: float = 5.0, count: int = 20):
        """
        Create explosion particle burst
        
        Args:
            position: Explosion center
            size: Explosion radius
            count: Number of particles
        """
        for i in range(count):
            # Explosion colors: orange to yellow
            color = Vec4(
                random.uniform(0.8, 1.0),
                random.uniform(0.3, 0.7),
                random.uniform(0.0, 0.3),
                1.0
            )
            
            particle = self._create_point_particle(
                position,
                size=random.uniform(0.8, 2.0),
                color=color
            )
            
            if particle:
                # Random direction outward
                theta = random.uniform(0, 2 * math.pi)
                phi = random.uniform(0, math.pi)
                
                direction = Vec3(
                    math.sin(phi) * math.cos(theta),
                    math.sin(phi) * math.sin(theta),
                    math.cos(phi)
                )
                
                speed = random.uniform(size * 0.5, size * 1.5)
                end_pos = position + direction * speed
                
                lifetime = random.uniform(0.5, 1.2)
                particle_seq = Sequence(
                    LerpPosInterval(particle, lifetime, end_pos),
                    LerpScaleInterval(particle, lifetime * 0.7, Vec3(0.2, 0.2, 0.2)),
                    LerpColorScaleInterval(particle, lifetime * 0.5, Vec4(0.2, 0.1, 0.0, 0)),
                    Func(self._remove_particle, particle)
                )
                particle_seq.start()
                
                effect = ParticleEffect(particle, lifetime)
                self.active_particles.append(effect)
    
    def create_debris_particles(self, position: Vec3, velocity: Vec3, count: int = 15):
        """
        Create debris particles (for destroyed ships)
        
        Args:
            position: Debris origin
            velocity: Initial velocity
            count: Number of debris pieces
        """
        for i in range(count):
            # Debris is gray/metallic
            color = Vec4(
                random.uniform(0.3, 0.6),
                random.uniform(0.3, 0.6),
                random.uniform(0.3, 0.6),
                1.0
            )
            
            particle = self._create_point_particle(
                position,
                size=random.uniform(0.5, 1.5),
                color=color
            )
            
            if particle:
                # Random tumbling motion
                random_vel = Vec3(
                    random.uniform(-5, 5),
                    random.uniform(-5, 5),
                    random.uniform(-5, 5)
                )
                
                end_pos = position + velocity * 2.0 + random_vel
                
                lifetime = random.uniform(1.0, 2.0)
                particle_seq = Sequence(
                    LerpPosInterval(particle, lifetime, end_pos),
                    LerpColorScaleInterval(particle, lifetime * 0.7, Vec4(0.1, 0.1, 0.1, 0)),
                    Func(self._remove_particle, particle)
                )
                particle_seq.start()
                
                effect = ParticleEffect(particle, lifetime)
                self.active_particles.append(effect)
    
    def create_warp_effect(self, position: Vec3, direction: Vec3):
        """
        Create warp tunnel effect particles
        
        Args:
            position: Warp origin
            direction: Warp direction
        """
        num_particles = 30
        
        for i in range(num_particles):
            # Warp particles are bright blue/white
            color = Vec4(
                random.uniform(0.7, 1.0),
                random.uniform(0.8, 1.0),
                random.uniform(0.9, 1.0),
                1.0
            )
            
            # Create particle at random position in a cone
            angle = random.uniform(0, 2 * math.pi)
            radius = random.uniform(0, 3.0)
            height = random.uniform(0, 10.0)
            
            offset = Vec3(
                math.cos(angle) * radius,
                math.sin(angle) * radius,
                0
            )
            
            start_pos = position + offset + direction * height
            
            particle = self._create_point_particle(
                start_pos,
                size=random.uniform(0.3, 1.0),
                color=color
            )
            
            if particle:
                # Particles streak along warp direction
                end_pos = start_pos + direction * 50.0
                
                lifetime = random.uniform(0.5, 1.5)
                particle_seq = Sequence(
                    LerpPosInterval(particle, lifetime, end_pos),
                    LerpColorScaleInterval(particle, lifetime * 0.5, Vec4(1, 1, 1, 0)),
                    Func(self._remove_particle, particle)
                )
                particle_seq.start()
                
                effect = ParticleEffect(particle, lifetime)
                self.active_particles.append(effect)
    
    def _create_point_particle(self, position: Vec3, size: float, color: Vec4) -> NodePath:
        """
        Create a simple point particle
        
        Args:
            position: Particle position
            size: Particle size
            color: Particle color
        
        Returns:
            NodePath for the particle
        """
        # Create a very simple geometry for the particle
        format = GeomVertexFormat.getV3c4()
        vdata = GeomVertexData('particle', format, Geom.UHDynamic)
        
        vertex = GeomVertexWriter(vdata, 'vertex')
        color_writer = GeomVertexWriter(vdata, 'color')
        
        vertex.addData3(0, 0, 0)
        color_writer.addData4(color)
        
        points = GeomPoints(Geom.UHDynamic)
        points.addVertex(0)
        points.closePrimitive()
        
        geom = Geom(vdata)
        geom.addPrimitive(points)
        
        node = GeomNode('particle')
        node.addGeom(geom)
        
        particle_np = self.render.attachNewNode(node)
        particle_np.setPos(position)
        particle_np.setScale(size)
        particle_np.setTransparency(True)
        particle_np.setBillboardPointEye()
        
        return particle_np
    
    def _remove_particle(self, particle: NodePath):
        """Remove a particle from the scene"""
        if particle:
            particle.removeNode()
        
        # Remove from active list
        self.active_particles = [p for p in self.active_particles if p.node != particle]
    
    def update(self, dt: float):
        """
        Update all active particles
        
        Args:
            dt: Delta time since last update
        """
        # Update all particles
        for particle in self.active_particles[:]:  # Copy list to allow removal
            particle.update(dt)
            
            # Remove dead particles
            if not particle.is_alive:
                self.active_particles.remove(particle)
        
        # Limit max particles
        if len(self.active_particles) > self.max_particles:
            # Remove oldest particles
            excess = len(self.active_particles) - self.max_particles
            for particle in self.active_particles[:excess]:
                particle.cleanup()
            self.active_particles = self.active_particles[excess:]
    
    def clear(self):
        """Clear all particles"""
        for particle in self.active_particles:
            particle.cleanup()
        self.active_particles.clear()
    
    def get_particle_count(self) -> int:
        """Get current number of active particles"""
        return len(self.active_particles)
