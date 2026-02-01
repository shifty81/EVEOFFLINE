"""
Game Systems
Systems process entities with specific components each frame
"""

import math
from dataclasses import dataclass, field
from typing import Dict, Optional, List, Tuple
from engine.core.ecs import System, Component
from engine.components.game_components import (
    Position, Velocity, Health, Capacitor, Weapon, Target, AI, Ship,
    Module, Drone, DroneBay, SkillTraining, EWAREffect
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


class FittingSystem(System):
    """Handles module fitting and CPU/PowerGrid management"""
    
    def validate_fitting(self, entity) -> Dict[str, any]:
        """Validate if current fitting is valid (CPU/PG constraints)"""
        from engine.components.game_components import Fitting
        ship = entity.get_component(Ship)
        fitting = entity.get_component(Fitting)
        
        if not ship or not fitting:
            return {'valid': False, 'error': 'Missing ship or fitting component'}
        
        # Calculate total CPU and PowerGrid usage
        # This would need module data loaded
        # For now, returning placeholder
        return {'valid': True, 'cpu_used': ship.cpu, 'pg_used': ship.powergrid}
    
    def fit_module(self, entity, module_id: str, slot_type: str, slot_index: int) -> bool:
        """Fit a module to a ship slot"""
        from engine.components.game_components import Fitting
        fitting = entity.get_component(Fitting)
        ship = entity.get_component(Ship)
        
        if not fitting or not ship:
            return False
        
        # Get appropriate slot list
        if slot_type == 'high':
            slots = fitting.high_slots
        elif slot_type == 'mid':
            slots = fitting.mid_slots
        elif slot_type == 'low':
            slots = fitting.low_slots
        elif slot_type == 'rig':
            slots = fitting.rig_slots
        else:
            return False
        
        # Check slot index
        if slot_index < 0 or slot_index >= len(slots):
            return False
        
        # Fit the module
        slots[slot_index] = module_id
        return True
    
    def unfit_module(self, entity, slot_type: str, slot_index: int) -> Optional[str]:
        """Unfit a module from a slot, returns module_id if successful"""
        from engine.components.game_components import Fitting
        fitting = entity.get_component(Fitting)
        
        if not fitting:
            return None
        
        # Get appropriate slot list
        if slot_type == 'high':
            slots = fitting.high_slots
        elif slot_type == 'mid':
            slots = fitting.mid_slots
        elif slot_type == 'low':
            slots = fitting.low_slots
        elif slot_type == 'rig':
            slots = fitting.rig_slots
        else:
            return None
        
        if slot_index < 0 or slot_index >= len(slots):
            return None
        
        module_id = slots[slot_index]
        slots[slot_index] = None
        return module_id


class ModuleActivationSystem(System):
    """Handles active module cycles and effects"""
    
    def update(self, delta_time: float):
        """Update all active modules"""
        from engine.components.game_components import Fitting
        entities = self.get_entities(Fitting, Capacitor)
        
        for entity in entities:
            fitting = entity.get_component(Fitting)
            capacitor = entity.get_component(Capacitor)
            
            # Check all fitted modules for active ones
            all_slots = (fitting.high_slots + fitting.mid_slots + 
                        fitting.low_slots + fitting.rig_slots)
            
            # This is a simplified version - in full implementation
            # we'd track Module components for each fitted module
            # For now, this is a placeholder for the system
    
    def activate_module(self, entity, slot_type: str, slot_index: int) -> bool:
        """Activate a module in a specific slot"""
        from engine.components.game_components import Fitting
        fitting = entity.get_component(Fitting)
        capacitor = entity.get_component(Capacitor)
        
        if not fitting or not capacitor:
            return False
        
        # Get the module from the slot
        # This would need full module data implementation
        # Placeholder for now
        return True
    
    def deactivate_module(self, entity, slot_type: str, slot_index: int) -> bool:
        """Deactivate a module"""
        # Placeholder
        return True


class DroneSystem(System):
    """Handles drone control and behavior"""
    
    def update(self, delta_time: float):
        """Update all drones"""
        drones = self.get_entities(Drone, Position, Velocity)
        
        for drone_entity in drones:
            drone = drone_entity.get_component(Drone)
            pos = drone_entity.get_component(Position)
            vel = drone_entity.get_component(Velocity)
            
            if drone.state == "active" and drone.target_entity_id:
                # Move towards target and attack
                target = self.world.get_entity(drone.target_entity_id)
                if target:
                    target_pos = target.get_component(Position)
                    if target_pos:
                        # Move towards target
                        dx = target_pos.x - pos.x
                        dy = target_pos.y - pos.y
                        dz = target_pos.z - pos.z
                        distance = math.sqrt(dx**2 + dy**2 + dz**2)
                        
                        if distance > drone.optimal_range:
                            # Approach
                            if distance > 0:
                                vel.vx = (dx / distance) * drone.speed
                                vel.vy = (dy / distance) * drone.speed
                                vel.vz = (dz / distance) * drone.speed
                        else:
                            # In range, orbit and shoot
                            vel.vx = -(dy / distance) * drone.speed * 0.3
                            vel.vy = (dx / distance) * drone.speed * 0.3
                            vel.vz = 0
                            
                            # Apply damage (simplified)
                            target_health = target.get_component(Health)
                            if target_health:
                                # Use weapon system's damage application
                                self._apply_drone_damage(target, drone.damage, drone.damage_type)
            
            elif drone.state == "returning" and drone.owner_entity_id:
                # Return to owner
                owner = self.world.get_entity(drone.owner_entity_id)
                if owner:
                    owner_pos = owner.get_component(Position)
                    if owner_pos:
                        dx = owner_pos.x - pos.x
                        dy = owner_pos.y - pos.y
                        dz = owner_pos.z - pos.z
                        distance = math.sqrt(dx**2 + dy**2 + dz**2)
                        
                        if distance < 100:  # Close enough to be recovered
                            drone.state = "in_bay"
                        else:
                            # Move towards owner
                            if distance > 0:
                                vel.vx = (dx / distance) * drone.speed
                                vel.vy = (dy / distance) * drone.speed
                                vel.vz = (dz / distance) * drone.speed
    
    def launch_drone(self, ship_entity, drone_index: int) -> bool:
        """Launch a drone from the bay"""
        drone_bay = ship_entity.get_component(DroneBay)
        if not drone_bay or drone_index >= len(drone_bay.drones):
            return False
        
        drone_entity_id = drone_bay.drones[drone_index]
        drone_entity = self.world.get_entity(drone_entity_id)
        if not drone_entity:
            return False
        
        drone = drone_entity.get_component(Drone)
        
        # Check bandwidth
        if drone_bay.bandwidth_used + drone.bandwidth > drone_bay.drone_bandwidth:
            return False
        
        # Launch drone
        drone.state = "launching"
        drone.owner_entity_id = ship_entity.id
        drone_bay.active_drones.append(drone_entity_id)
        drone_bay.bandwidth_used += drone.bandwidth
        
        # Position drone near ship
        ship_pos = ship_entity.get_component(Position)
        drone_pos = drone_entity.get_component(Position)
        if ship_pos and drone_pos:
            drone_pos.x = ship_pos.x + 100
            drone_pos.y = ship_pos.y + 100
            drone_pos.z = ship_pos.z
        
        drone.state = "active"
        return True
    
    def recall_drone(self, ship_entity, drone_entity_id: str) -> bool:
        """Recall a drone to the bay"""
        drone_bay = ship_entity.get_component(DroneBay)
        if not drone_bay or drone_entity_id not in drone_bay.active_drones:
            return False
        
        drone_entity = self.world.get_entity(drone_entity_id)
        if not drone_entity:
            return False
        
        drone = drone_entity.get_component(Drone)
        drone.state = "returning"
        drone.target_entity_id = None
        
        return True
    
    def recall_all_drones(self, ship_entity) -> bool:
        """Recall all active drones"""
        drone_bay = ship_entity.get_component(DroneBay)
        if not drone_bay:
            return False
        
        for drone_id in list(drone_bay.active_drones):
            self.recall_drone(ship_entity, drone_id)
        
        return True
    
    def engage_target(self, ship_entity, target_entity_id: str) -> bool:
        """Command active drones to engage a target"""
        drone_bay = ship_entity.get_component(DroneBay)
        if not drone_bay:
            return False
        
        for drone_id in drone_bay.active_drones:
            drone_entity = self.world.get_entity(drone_id)
            if drone_entity:
                drone = drone_entity.get_component(Drone)
                if drone and drone.state == "active":
                    drone.target_entity_id = target_entity_id
        
        return True
    
    def _apply_drone_damage(self, target_entity, damage: float, damage_type: str):
        """Apply damage from drone to target (reuses weapon system logic)"""
        health = target_entity.get_component(Health)
        if not health:
            return
        
        resist_map = {
            'em': 'em_resist',
            'thermal': 'thermal_resist',
            'kinetic': 'kinetic_resist',
            'explosive': 'explosive_resist',
        }
        
        resist_suffix = resist_map.get(damage_type, 'kinetic_resist')
        
        # Apply to shields first
        if health.shield_hp > 0:
            resist = getattr(health, f'shield_{resist_suffix}', 0.0)
            actual_damage = damage * (1.0 - resist)
            health.shield_hp -= actual_damage
            
            if health.shield_hp < 0:
                overflow = -health.shield_hp
                health.shield_hp = 0
                resist = getattr(health, f'armor_{resist_suffix}', 0.0)
                actual_damage = overflow * (1.0 - resist)
                health.armor_hp -= actual_damage
        elif health.armor_hp > 0:
            resist = getattr(health, f'armor_{resist_suffix}', 0.0)
            actual_damage = damage * (1.0 - resist)
            health.armor_hp -= actual_damage
            
            if health.armor_hp < 0:
                overflow = -health.armor_hp
                health.armor_hp = 0
                resist = getattr(health, f'hull_{resist_suffix}', 0.0)
                actual_damage = overflow * (1.0 - resist)
                health.hull_hp -= actual_damage
        else:
            resist = getattr(health, f'hull_{resist_suffix}', 0.0)
            actual_damage = damage * (1.0 - resist)
            health.hull_hp -= actual_damage


class SkillSystem(System):
    """Handles skill training and bonus application"""
    
    def update(self, delta_time: float):
        """Update skill training progress"""
        from engine.components.game_components import Skills
        entities = self.get_entities(Skills, SkillTraining)
        
        for entity in entities:
            skills = entity.get_component(Skills)
            training = entity.get_component(SkillTraining)
            
            if training.current_skill:
                # Calculate SP gain
                sp_gain = training.sp_per_minute * (delta_time / 60.0)
                training.current_sp += int(sp_gain)
                
                # Check if level completed
                sp_needed = self._calculate_sp_needed(training.current_target_level)
                if training.current_sp >= sp_needed:
                    # Level up!
                    current_level = skills.skills.get(training.current_skill, 0)
                    skills.skills[training.current_skill] = training.current_target_level
                    skills.skill_points[training.current_skill] = training.current_sp
                    
                    # Start next in queue
                    if training.training_queue:
                        next_skill, next_level = training.training_queue.pop(0)
                        training.current_skill = next_skill
                        training.current_target_level = next_level
                        training.current_sp = skills.skill_points.get(next_skill, 0)
                    else:
                        training.current_skill = None
                        training.current_target_level = 0
    
    def start_training(self, entity, skill_name: str, target_level: int) -> bool:
        """Start training a skill"""
        from engine.components.game_components import Skills
        skills = entity.get_component(Skills)
        training = entity.get_component(SkillTraining)
        
        if not skills or not training:
            return False
        
        current_level = skills.skills.get(skill_name, 0)
        if target_level <= current_level or target_level > 5:
            return False
        
        if not training.current_skill:
            # Start immediately
            training.current_skill = skill_name
            training.current_target_level = target_level
            training.current_sp = skills.skill_points.get(skill_name, 0)
        else:
            # Add to queue
            training.training_queue.append((skill_name, target_level))
        
        return True
    
    def _calculate_sp_needed(self, level: int) -> int:
        """Calculate SP needed for a skill level"""
        # EVE formula: 250 * multiplier * sqrt(32)^(level-1)
        # Using multiplier of 1 for simplicity
        multiplier = 1
        base_sp = 250
        return int(base_sp * multiplier * (5.657 ** (level - 1)))
    
    def get_skill_bonus(self, entity, skill_name: str, bonus_type: str) -> float:
        """Get bonus from a skill"""
        from engine.components.game_components import Skills
        skills = entity.get_component(Skills)
        if not skills:
            return 0.0
        
        level = skills.skills.get(skill_name, 0)
        
        # Define skill bonuses (simplified)
        skill_bonuses = {
            'Gunnery': {'damage': 0.02},  # +2% per level
            'Weapon Upgrades': {'cpu': -0.05},  # -5% CPU per level
            'Shield Management': {'capacity': 0.05},  # +5% shield per level
            'Drones': {'damage': 0.05, 'hp': 0.05},  # +5% per level
        }
        
        bonus_per_level = skill_bonuses.get(skill_name, {}).get(bonus_type, 0.0)
        return bonus_per_level * level
