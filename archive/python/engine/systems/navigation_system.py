"""
Navigation and Warp System
Handles warp travel, docking, and stargate jumps
"""

import math
from typing import Optional, List, Dict
from engine.core.ecs import System
from engine.components.game_components import (
    Position, Velocity, Ship, WarpDrive, Celestial, Docking
)


class NavigationSystem(System):
    """Handles warp, docking, and navigation"""
    
    def update(self, delta_time: float):
        """Update navigation states"""
        # Update warp progress
        entities = self.get_entities(WarpDrive, Position, Velocity)
        
        for entity in entities:
            warp = entity.get_component(WarpDrive)
            pos = entity.get_component(Position)
            vel = entity.get_component(Velocity)
            
            if warp.is_aligning:
                # Update align progress
                warp.align_progress += delta_time / warp.align_time
                
                if warp.align_progress >= 1.0:
                    # Alignment complete, start warp
                    warp.is_aligning = False
                    warp.align_progress = 0.0
                    
                    if warp.warp_destination:
                        warp.is_warping = True
                        warp.warp_progress = 0.0
            
            elif warp.is_warping and warp.warp_destination:
                # Update warp progress
                dest_x, dest_y, dest_z = warp.warp_destination
                
                # Calculate distance
                dx = dest_x - pos.x
                dy = dest_y - pos.y
                dz = dest_z - pos.z
                total_distance = math.sqrt(dx**2 + dy**2 + dz**2)
                
                if total_distance < 1000:  # Close enough, drop out of warp
                    warp.is_warping = False
                    warp.warp_destination = None
                    warp.warp_progress = 0.0
                    
                    # Set velocity to zero
                    vel.vx = 0
                    vel.vy = 0
                    vel.vz = 0
                else:
                    # Move towards destination at warp speed
                    # Convert warp speed (AU/s) to m/s
                    # 1 AU = 150,000,000,000 meters (simplified to 150,000,000 for game)
                    au_to_m = 150_000_000
                    warp_speed_ms = warp.warp_speed * au_to_m
                    
                    # Normalize direction and apply warp speed
                    norm = total_distance
                    vel.vx = (dx / norm) * warp_speed_ms
                    vel.vy = (dy / norm) * warp_speed_ms
                    vel.vz = (dz / norm) * warp_speed_ms
                    
                    # Update warp progress
                    warp.warp_progress += delta_time
    
    def initiate_warp(self, entity, dest_x: float, dest_y: float, dest_z: float) -> bool:
        """Initiate warp to destination"""
        warp = entity.get_component(WarpDrive)
        docking = entity.get_component(Docking)
        
        if not warp:
            return False
        
        # Can't warp while docked
        if docking and docking.is_docked:
            return False
        
        # Can't warp while already warping
        if warp.is_warping or warp.is_aligning:
            return False
        
        # Set destination and start alignment
        warp.warp_destination = (dest_x, dest_y, dest_z)
        warp.is_aligning = True
        warp.align_progress = 0.0
        
        return True
    
    def warp_to_celestial(self, entity, celestial_entity) -> bool:
        """Warp to a celestial object"""
        pos = celestial_entity.get_component(Position)
        if not pos:
            return False
        
        return self.initiate_warp(entity, pos.x, pos.y, pos.z)
    
    def cancel_warp(self, entity) -> bool:
        """Cancel warp/alignment"""
        warp = entity.get_component(WarpDrive)
        vel = entity.get_component(Velocity)
        
        if not warp:
            return False
        
        warp.is_warping = False
        warp.is_aligning = False
        warp.warp_destination = None
        warp.warp_progress = 0.0
        warp.align_progress = 0.0
        
        # Stop movement
        if vel:
            vel.vx = 0
            vel.vy = 0
            vel.vz = 0
        
        return True
    
    def dock(self, ship_entity, station_entity) -> bool:
        """Dock at a station"""
        ship_pos = ship_entity.get_component(Position)
        station_pos = station_entity.get_component(Position)
        station = station_entity.get_component(Celestial)
        
        if not ship_pos or not station_pos or not station:
            return False
        
        if not station.dockable:
            return False
        
        # Check distance
        dx = station_pos.x - ship_pos.x
        dy = station_pos.y - ship_pos.y
        dz = station_pos.z - ship_pos.z
        distance = math.sqrt(dx**2 + dy**2 + dz**2)
        
        if distance > 5000:  # Must be within 5km
            return False
        
        # Dock the ship
        docking = ship_entity.get_component(Docking)
        if not docking:
            docking = Docking()
            ship_entity.add_component(docking)
        
        docking.is_docked = True
        docking.docked_at = station_entity.id
        
        return True
    
    def undock(self, ship_entity) -> bool:
        """Undock from station"""
        docking = ship_entity.get_component(Docking)
        
        if not docking or not docking.is_docked:
            return False
        
        if not docking.can_undock:
            return False
        
        # Get docked station position
        station_entity = self.world.get_entity(docking.docked_at)
        if station_entity:
            station_pos = station_entity.get_component(Position)
            ship_pos = ship_entity.get_component(Position)
            
            if station_pos and ship_pos:
                # Place ship near station (1km away)
                ship_pos.x = station_pos.x + 1000
                ship_pos.y = station_pos.y
                ship_pos.z = station_pos.z
        
        docking.is_docked = False
        docking.docked_at = None
        
        return True
    
    def jump_stargate(self, ship_entity, gate_entity) -> Dict[str, any]:
        """
        Jump through a stargate
        
        Returns:
            dict: {'success': bool, 'destination_system': str, 'destination_gate': str, 'error': str}
        """
        ship_pos = ship_entity.get_component(Position)
        gate_pos = gate_entity.get_component(Position)
        gate = gate_entity.get_component(Celestial)
        
        if not ship_pos or not gate_pos or not gate:
            return {'success': False, 'error': 'Missing required components'}
        
        if gate.celestial_type != "stargate":
            return {'success': False, 'error': 'Not a stargate'}
        
        if not gate.destination_system:
            return {'success': False, 'error': 'No destination configured'}
        
        # Check distance
        dx = gate_pos.x - ship_pos.x
        dy = gate_pos.y - ship_pos.y
        dz = gate_pos.z - ship_pos.z
        distance = math.sqrt(dx**2 + dy**2 + dz**2)
        
        if distance > 2500:  # Must be within 2.5km
            return {'success': False, 'error': 'Too far from gate'}
        
        # In a full implementation, this would:
        # 1. Load the destination system
        # 2. Move the ship to the destination gate
        # 3. Update all clients
        
        return {
            'success': True,
            'destination_system': gate.destination_system,
            'destination_gate': gate.destination_gate
        }
    
    def approach(self, ship_entity, target_entity, approach_distance: float = 1000.0) -> bool:
        """Approach a target entity"""
        ship_pos = ship_entity.get_component(Position)
        ship_vel = ship_entity.get_component(Velocity)
        target_pos = target_entity.get_component(Position)
        
        if not ship_pos or not ship_vel or not target_pos:
            return False
        
        # Calculate direction
        dx = target_pos.x - ship_pos.x
        dy = target_pos.y - ship_pos.y
        dz = target_pos.z - ship_pos.z
        distance = math.sqrt(dx**2 + dy**2 + dz**2)
        
        if distance <= approach_distance:
            # Already close enough
            ship_vel.vx = 0
            ship_vel.vy = 0
            ship_vel.vz = 0
            return True
        
        # Set velocity towards target
        if distance > 0:
            ship_vel.vx = (dx / distance) * ship_vel.max_speed
            ship_vel.vy = (dy / distance) * ship_vel.max_speed
            ship_vel.vz = (dz / distance) * ship_vel.max_speed
        
        return True
    
    def orbit(self, ship_entity, target_entity, orbit_distance: float = 5000.0) -> bool:
        """Orbit a target entity"""
        ship_pos = ship_entity.get_component(Position)
        ship_vel = ship_entity.get_component(Velocity)
        target_pos = target_entity.get_component(Position)
        
        if not ship_pos or not ship_vel or not target_pos:
            return False
        
        # Calculate direction to target
        dx = target_pos.x - ship_pos.x
        dy = target_pos.y - ship_pos.y
        dz = target_pos.z - ship_pos.z
        distance = math.sqrt(dx**2 + dy**2 + dz**2)
        
        if distance == 0:
            return False
        
        # If too far, approach first
        if distance > orbit_distance * 1.5:
            ship_vel.vx = (dx / distance) * ship_vel.max_speed
            ship_vel.vy = (dy / distance) * ship_vel.max_speed
            ship_vel.vz = (dz / distance) * ship_vel.max_speed
        else:
            # Orbit: move perpendicular to direction
            # In 2D this is simple rotation, in 3D we pick XY plane
            ship_vel.vx = -(dy / distance) * ship_vel.max_speed * 0.7
            ship_vel.vy = (dx / distance) * ship_vel.max_speed * 0.7
            ship_vel.vz = 0
        
        return True
