"""
Camera System for 3D Client
EVE Online-style camera controls
"""

from panda3d.core import Vec3, Point3
import math


class CameraSystem:
    """
    EVE-style camera system with orbit, zoom, and pan
    """
    
    # Camera mode enum
    MODE_ORBIT = 0
    MODE_FREE = 1
    MODE_FOLLOW = 2
    
    def __init__(self, camera, render):
        self.camera = camera
        self.render = render
        
        # Camera state
        self.mode = self.MODE_ORBIT
        self.target = Vec3(0, 0, 0)  # What we're looking at
        self.distance = 50.0  # Distance from target
        self.min_distance = 10.0
        self.max_distance = 1000.0
        
        # Orbit angles (in degrees)
        self.theta = 45.0  # Horizontal angle
        self.phi = 30.0    # Vertical angle
        self.min_phi = -85.0
        self.max_phi = 85.0
        
        # Pan offset
        self.pan_x = 0.0
        self.pan_y = 0.0
        
        # Smooth movement
        self.smooth_factor = 0.1  # Lower = smoother (0-1)
        self.target_distance = self.distance
        self.target_theta = self.theta
        self.target_phi = self.phi
        
        # Mouse state
        self.last_mouse_x = 0
        self.last_mouse_y = 0
        
    def update(self, dt):
        """
        Update camera position
        
        Args:
            dt: Delta time (seconds)
        """
        # Smooth interpolation
        self.distance += (self.target_distance - self.distance) * self.smooth_factor
        self.theta += (self.target_theta - self.theta) * self.smooth_factor
        self.phi += (self.target_phi - self.phi) * self.smooth_factor
        
        # Calculate camera position based on mode
        if self.mode == self.MODE_ORBIT or self.mode == self.MODE_FOLLOW:
            # Convert spherical coordinates to Cartesian
            theta_rad = math.radians(self.theta)
            phi_rad = math.radians(self.phi)
            
            x = self.distance * math.cos(phi_rad) * math.cos(theta_rad)
            y = self.distance * math.cos(phi_rad) * math.sin(theta_rad)
            z = self.distance * math.sin(phi_rad)
            
            # Apply pan offset
            x += self.pan_x
            y += self.pan_y
            
            # Set camera position relative to target
            self.camera.setPos(
                self.target.x + x,
                self.target.y + y,
                self.target.z + z
            )
            
            # Look at target
            self.camera.lookAt(Point3(self.target.x, self.target.y, self.target.z))
            
        elif self.mode == self.MODE_FREE:
            # Free camera mode - manual control
            pass
    
    def set_target(self, target_pos: Vec3):
        """Set camera target position"""
        self.target = Vec3(target_pos)
    
    def set_target_entity(self, entity):
        """Set camera to follow an entity"""
        if entity:
            pos = entity.get_position()
            self.target = Vec3(pos[0], pos[1], pos[2])
    
    def rotate(self, dx: float, dy: float, sensitivity: float = 0.5):
        """
        Rotate camera (orbit mode)
        
        Args:
            dx: Mouse delta X
            dy: Mouse delta Y
            sensitivity: Mouse sensitivity multiplier
        """
        self.target_theta -= dx * sensitivity
        self.target_phi = max(self.min_phi, min(self.max_phi, self.target_phi + dy * sensitivity))
        
        # Keep theta in 0-360 range
        if self.target_theta < 0:
            self.target_theta += 360
        elif self.target_theta >= 360:
            self.target_theta -= 360
    
    def zoom(self, delta: float):
        """
        Zoom camera in/out
        
        Args:
            delta: Zoom delta (positive = zoom in, negative = zoom out)
        """
        zoom_speed = self.distance * 0.1  # Zoom faster when farther away
        self.target_distance = max(
            self.min_distance,
            min(self.max_distance, self.target_distance - delta * zoom_speed)
        )
    
    def pan(self, dx: float, dy: float, sensitivity: float = 0.5):
        """
        Pan camera
        
        Args:
            dx: Mouse delta X
            dy: Mouse delta Y
            sensitivity: Pan sensitivity multiplier
        """
        pan_speed = self.distance * 0.01 * sensitivity
        self.pan_x += dx * pan_speed
        self.pan_y += dy * pan_speed
    
    def reset_pan(self):
        """Reset pan to center on target"""
        self.pan_x = 0.0
        self.pan_y = 0.0
    
    def reset(self):
        """Reset camera to default position"""
        self.target_distance = 50.0
        self.target_theta = 45.0
        self.target_phi = 30.0
        self.reset_pan()
    
    def set_mode(self, mode: int):
        """Set camera mode"""
        self.mode = mode
        print(f"[Camera] Mode set to: {['ORBIT', 'FREE', 'FOLLOW'][mode]}")
    
    def toggle_mode(self):
        """Toggle between camera modes"""
        self.mode = (self.mode + 1) % 3
        self.set_mode(self.mode)
    
    def get_forward_vector(self) -> Vec3:
        """Get camera forward direction vector"""
        return Vec3(
            math.cos(math.radians(self.phi)) * math.cos(math.radians(self.theta)),
            math.cos(math.radians(self.phi)) * math.sin(math.radians(self.theta)),
            math.sin(math.radians(self.phi))
        )
    
    def get_right_vector(self) -> Vec3:
        """Get camera right direction vector"""
        return Vec3(
            -math.sin(math.radians(self.theta)),
            math.cos(math.radians(self.theta)),
            0
        )
