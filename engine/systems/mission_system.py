"""
Mission System
Handles mission acceptance, tracking, and completion
"""

from dataclasses import dataclass, field
from typing import Dict, List, Optional
from engine.core.ecs import System
from engine.components.game_components import Mission, Player, Position, NPC


class MissionSystem(System):
    """Handles mission lifecycle and objectives"""
    
    def __init__(self, world, data_loader=None):
        super().__init__(world)
        self.data_loader = data_loader
        self.active_missions: Dict[str, Mission] = {}  # player_id -> Mission
    
    def update(self, delta_time: float):
        """Update mission progress"""
        # Check mission objectives for all players
        from engine.components.game_components import Mission
        entities = self.get_entities(Mission, Player)
        
        for entity in entities:
            mission = entity.get_component(Mission)
            player = entity.get_component(Player)
            
            if mission.status == "active":
                self._check_objectives(entity, mission, player)
    
    def accept_mission(self, player_entity, mission_id: str) -> bool:
        """Player accepts a mission"""
        from engine.components.game_components import Mission
        
        if not self.data_loader:
            return False
        
        mission_data = self.data_loader.get_mission(mission_id)
        if not mission_data:
            return False
        
        # Check if player already has this mission
        existing_mission = player_entity.get_component(Mission)
        if existing_mission and existing_mission.status == "active":
            return False  # Can only have one active mission at a time (simplified)
        
        # Create mission component
        mission_component = Mission(
            mission_id=mission_id,
            mission_type=mission_data.get('type', 'combat'),
            objectives=mission_data.get('objectives', []),
            completed_objectives=[],
            rewards=mission_data.get('rewards', {}),
            status="active"
        )
        
        # Add or update mission component
        if existing_mission:
            player_entity.remove_component(Mission)
        player_entity.add_component(mission_component)
        
        return True
    
    def complete_mission(self, player_entity) -> Dict[str, any]:
        """Complete a mission and grant rewards"""
        mission = player_entity.get_component(Mission)
        player = player_entity.get_component(Player)
        
        if not mission or not player:
            return {'success': False, 'error': 'No active mission'}
        
        # Check if all objectives are complete
        if len(mission.completed_objectives) < len(mission.objectives):
            return {'success': False, 'error': 'Objectives not complete'}
        
        # Grant rewards
        rewards_granted = {}
        
        if 'isk' in mission.rewards:
            player.isk += mission.rewards['isk']
            rewards_granted['isk'] = mission.rewards['isk']
        
        if 'lp' in mission.rewards:
            # Loyalty points would go here
            rewards_granted['lp'] = mission.rewards['lp']
        
        # Update mission status
        mission.status = "completed"
        
        return {
            'success': True,
            'rewards': rewards_granted,
            'mission_id': mission.mission_id
        }
    
    def _check_objectives(self, player_entity, mission: 'Mission', player: 'Player'):
        """Check mission objectives for completion"""
        for idx, objective in enumerate(mission.objectives):
            if idx in mission.completed_objectives:
                continue  # Already completed
            
            obj_type = objective.get('type')
            
            if obj_type == 'kill':
                # Check if required NPCs are killed
                target_faction = objective.get('faction')
                target_count = objective.get('count', 1)
                
                # This would need to track kills - simplified for now
                # In full implementation, we'd track NPC kills per mission
                
            elif obj_type == 'reach_location':
                # Check if player is at location
                target_x = objective.get('x', 0)
                target_y = objective.get('y', 0)
                target_z = objective.get('z', 0)
                radius = objective.get('radius', 1000)
                
                pos = player_entity.get_component(Position)
                if pos:
                    import math
                    dx = pos.x - target_x
                    dy = pos.y - target_y
                    dz = pos.z - target_z
                    distance = math.sqrt(dx**2 + dy**2 + dz**2)
                    
                    if distance <= radius:
                        mission.completed_objectives.append(idx)
            
            elif obj_type == 'courier':
                # Check if item delivered
                # Would need inventory system
                pass
    
    def abandon_mission(self, player_entity) -> bool:
        """Abandon current mission"""
        mission = player_entity.get_component(Mission)
        
        if not mission:
            return False
        
        mission.status = "failed"
        return True
    
    def get_mission_progress(self, player_entity) -> Dict[str, any]:
        """Get current mission progress"""
        mission = player_entity.get_component(Mission)
        
        if not mission:
            return {'has_mission': False}
        
        total_objectives = len(mission.objectives)
        completed = len(mission.completed_objectives)
        
        return {
            'has_mission': True,
            'mission_id': mission.mission_id,
            'type': mission.mission_type,
            'status': mission.status,
            'progress': f"{completed}/{total_objectives}",
            'objectives': mission.objectives,
            'completed_objectives': mission.completed_objectives
        }
