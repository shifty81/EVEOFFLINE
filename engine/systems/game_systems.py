"""
Game Systems
Systems process entities with specific components each frame
"""

import math
from engine.core.ecs import System
from engine.components.game_components import (
    Position, Velocity, Health, Capacitor, Weapon, Target, AI, Ship
)


class MovementSystem(System):
    """Handles entity movement and physics"""
    
    def update(self, delta_time: float):
        entities = self.get_entities(Position, Velocity)
        
        for entity in entities:
            pos = entity.get_component(Position)
            vel = entity.get_component(Velocity)
            
            # Update position based on velocity
            pos.x += vel.vx * delta_time
            pos.y += vel.vy * delta_time
            pos.z += vel.vz * delta_time
            pos.rotation += vel.angular_velocity * delta_time
            
            # Apply speed limit
            speed = math.sqrt(vel.vx**2 + vel.vy**2 + vel.vz**2)
            if speed > vel.max_speed:
                factor = vel.max_speed / speed
                vel.vx *= factor
                vel.vy *= factor
                vel.vz *= factor


class CapacitorSystem(System):
    """Handles capacitor recharge"""
    
    def update(self, delta_time: float):
        entities = self.get_entities(Capacitor)
        
        for entity in entities:
            cap = entity.get_component(Capacitor)
            
            # Recharge capacitor (EVE uses a percentage-based formula, simplified here)
            if cap.capacitor < cap.capacitor_max:
                recharge = cap.recharge_rate * delta_time
                cap.capacitor = min(cap.capacitor + recharge, cap.capacitor_max)


class ShieldRechargeSystem(System):
    """Handles shield recharge"""
    
    def update(self, delta_time: float):
        entities = self.get_entities(Health)
        
        for entity in entities:
            health = entity.get_component(Health)
            
            # Recharge shields
            if health.shield_hp < health.shield_max:
                recharge = health.shield_recharge_rate * delta_time
                health.shield_hp = min(health.shield_hp + recharge, health.shield_max)


class WeaponSystem(System):
    """Handles weapon firing and cooldowns"""
    
    def update(self, delta_time: float):
        entities = self.get_entities(Weapon)
        
        for entity in entities:
            weapon = entity.get_component(Weapon)
            
            # Update cooldown
            if weapon.cooldown > 0:
                weapon.cooldown -= delta_time
                
    def fire_weapon(self, shooter_entity, target_entity_id: str) -> bool:
        """Attempt to fire weapon at target"""
        weapon = shooter_entity.get_component(Weapon)
        if not weapon or weapon.cooldown > 0 or weapon.ammo_count <= 0:
            return False
            
        shooter_pos = shooter_entity.get_component(Position)
        target = self.world.get_entity(target_entity_id)
        
        if not target or not shooter_pos:
            return False
            
        target_pos = target.get_component(Position)
        if not target_pos:
            return False
            
        # Calculate distance
        dx = target_pos.x - shooter_pos.x
        dy = target_pos.y - shooter_pos.y
        dz = target_pos.z - shooter_pos.z
        distance = math.sqrt(dx**2 + dy**2 + dz**2)
        
        # Check range
        if distance > weapon.optimal_range + weapon.falloff_range:
            return False
            
        # Calculate damage based on range (simplified)
        damage_multiplier = 1.0
        if distance > weapon.optimal_range:
            falloff_distance = distance - weapon.optimal_range
            damage_multiplier = max(0.01, 1.0 - (falloff_distance / weapon.falloff_range))
            
        final_damage = weapon.damage * damage_multiplier
        
        # Apply damage to target
        self._apply_damage(target, final_damage, weapon.damage_type)
        
        # Consume ammo and start cooldown
        weapon.ammo_count -= 1
        weapon.cooldown = weapon.rate_of_fire
        
        return True
        
    def _apply_damage(self, target_entity, damage: float, damage_type: str):
        """Apply damage to target entity"""
        health = target_entity.get_component(Health)
        if not health:
            return
            
        # Get resistance based on damage type
        resist_map = {
            'em': ('em_resist', 'shield'),
            'thermal': ('thermal_resist', 'shield'),
            'kinetic': ('kinetic_resist', 'shield'),
            'explosive': ('explosive_resist', 'shield'),
        }
        
        resist_suffix, _ = resist_map.get(damage_type, ('kinetic_resist', 'shield'))
        
        # Apply to shields first
        if health.shield_hp > 0:
            resist = getattr(health, f'shield_{resist_suffix}', 0.0)
            actual_damage = damage * (1.0 - resist)
            health.shield_hp -= actual_damage
            
            if health.shield_hp < 0:
                # Overflow to armor
                overflow = -health.shield_hp
                health.shield_hp = 0
                
                resist = getattr(health, f'armor_{resist_suffix}', 0.0)
                actual_damage = overflow * (1.0 - resist)
                health.armor_hp -= actual_damage
        else:
            # Shields down, hit armor
            if health.armor_hp > 0:
                resist = getattr(health, f'armor_{resist_suffix}', 0.0)
                actual_damage = damage * (1.0 - resist)
                health.armor_hp -= actual_damage
                
                if health.armor_hp < 0:
                    # Overflow to hull
                    overflow = -health.armor_hp
                    health.armor_hp = 0
                    
                    resist = getattr(health, f'hull_{resist_suffix}', 0.0)
                    actual_damage = overflow * (1.0 - resist)
                    health.hull_hp -= actual_damage
            else:
                # Armor down, hit hull
                resist = getattr(health, f'hull_{resist_suffix}', 0.0)
                actual_damage = damage * (1.0 - resist)
                health.hull_hp -= actual_damage


class AISystem(System):
    """Controls NPC behavior"""
    
    def update(self, delta_time: float):
        entities = self.get_entities(AI, Position, Velocity)
        
        for entity in entities:
            ai = entity.get_component(AI)
            pos = entity.get_component(Position)
            vel = entity.get_component(Velocity)
            
            if ai.state == "idle":
                self._idle_behavior(entity, ai, pos, vel)
            elif ai.state == "approaching":
                self._approach_behavior(entity, ai, pos, vel)
            elif ai.state == "orbiting":
                self._orbit_behavior(entity, ai, pos, vel)
            elif ai.state == "attacking":
                self._attack_behavior(entity, ai, pos, vel)
                
    def _idle_behavior(self, entity, ai, pos, vel):
        """Idle behavior - look for targets"""
        # Find nearest player or hostile
        all_entities = self.world.get_entities(Position)
        nearest_target = None
        nearest_distance = ai.awareness_range
        
        for potential_target in all_entities:
            if potential_target.id == entity.id:
                continue
                
            # Check if target has player component (is a player)
            from engine.components.game_components import Player
            if not potential_target.has_component(Player):
                continue
                
            target_pos = potential_target.get_component(Position)
            dx = target_pos.x - pos.x
            dy = target_pos.y - pos.y
            dz = target_pos.z - pos.z
            distance = math.sqrt(dx**2 + dy**2 + dz**2)
            
            if distance < nearest_distance:
                nearest_distance = distance
                nearest_target = potential_target
                
        if nearest_target:
            ai.target_entity_id = nearest_target.id
            ai.state = "approaching"
            
    def _approach_behavior(self, entity, ai, pos, vel):
        """Approach target"""
        if not ai.target_entity_id:
            ai.state = "idle"
            return
            
        target = self.world.get_entity(ai.target_entity_id)
        if not target:
            ai.state = "idle"
            ai.target_entity_id = None
            return
            
        target_pos = target.get_component(Position)
        dx = target_pos.x - pos.x
        dy = target_pos.y - pos.y
        dz = target_pos.z - pos.z
        distance = math.sqrt(dx**2 + dy**2 + dz**2)
        
        if distance < ai.orbit_distance:
            ai.state = "orbiting"
        else:
            # Move towards target
            if distance > 0:
                vel.vx = (dx / distance) * vel.max_speed
                vel.vy = (dy / distance) * vel.max_speed
                vel.vz = (dz / distance) * vel.max_speed
                
    def _orbit_behavior(self, entity, ai, pos, vel):
        """Orbit around target"""
        if not ai.target_entity_id:
            ai.state = "idle"
            return
            
        target = self.world.get_entity(ai.target_entity_id)
        if not target:
            ai.state = "idle"
            ai.target_entity_id = None
            return
            
        # Simple orbit logic - move perpendicular to target direction
        target_pos = target.get_component(Position)
        dx = target_pos.x - pos.x
        dy = target_pos.y - pos.y
        distance = math.sqrt(dx**2 + dy**2)
        
        if distance > 0:
            # Perpendicular velocity for orbiting
            vel.vx = -(dy / distance) * vel.max_speed * 0.5
            vel.vy = (dx / distance) * vel.max_speed * 0.5
            
        # Switch to attacking if we have weapons
        if entity.has_component(Weapon):
            ai.state = "attacking"
            
    def _attack_behavior(self, entity, ai, pos, vel):
        """Attack target while orbiting"""
        self._orbit_behavior(entity, ai, pos, vel)
        
        # Try to fire weapons (handled by WeaponSystem)
        if ai.target_entity_id and entity.has_component(Weapon):
            weapon_system = None
            for system in self.world.systems:
                if isinstance(system, WeaponSystem):
                    weapon_system = system
                    break
                    
            if weapon_system:
                weapon_system.fire_weapon(entity, ai.target_entity_id)


class TargetingSystem(System):
    """Handles target locking"""
    
    def update(self, delta_time: float):
        entities = self.get_entities(Target, Ship)
        
        for entity in entities:
            target_comp = entity.get_component(Target)
            ship = entity.get_component(Ship)
            
            # Process locking targets
            completed_locks = []
            for target_id, progress in list(target_comp.locking_targets.items()):
                # Lock time based on scan resolution (simplified)
                lock_time = 1000.0 / ship.scan_resolution  # seconds
                progress += delta_time / lock_time
                
                if progress >= 1.0:
                    completed_locks.append(target_id)
                else:
                    target_comp.locking_targets[target_id] = progress
                    
            # Complete locks
            for target_id in completed_locks:
                if len(target_comp.locked_targets) < ship.max_locked_targets:
                    target_comp.locked_targets.append(target_id)
                del target_comp.locking_targets[target_id]
                
    def start_lock(self, entity, target_entity_id: str) -> bool:
        """Start locking a target"""
        target_comp = entity.get_component(Target)
        ship = entity.get_component(Ship)
        
        if not target_comp or not ship:
            return False
            
        # Check if already locked or locking
        if target_entity_id in target_comp.locked_targets:
            return True
        if target_entity_id in target_comp.locking_targets:
            return True
            
        # Check max targets
        total_targets = len(target_comp.locked_targets) + len(target_comp.locking_targets)
        if total_targets >= ship.max_locked_targets:
            return False
            
        # Start locking
        target_comp.locking_targets[target_entity_id] = 0.0
        return True
