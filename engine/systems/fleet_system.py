"""
Fleet System
Handles fleet mechanics, fleet bonuses, and fleet coordination
Based on EVE Online's fleet system
"""

from typing import Optional, Dict, List, Set
from dataclasses import dataclass, field
from enum import Enum
from engine.core.ecs import World, Entity


class FleetRole(Enum):
    """Fleet member roles"""
    MEMBER = "member"
    SQUAD_COMMANDER = "squad_commander"
    WING_COMMANDER = "wing_commander"
    FLEET_COMMANDER = "fleet_commander"


class FleetBoosterType(Enum):
    """Types of fleet boosters"""
    ARMOR = "armor"
    SHIELD = "shield"
    SKIRMISH = "skirmish"  # Speed/agility
    INFORMATION = "information"  # Targeting/scanning


@dataclass
class FleetMember:
    """Individual fleet member"""
    entity_id: str
    character_name: str
    role: FleetRole = FleetRole.MEMBER
    squad_id: Optional[int] = None
    wing_id: Optional[int] = None
    is_online: bool = True


@dataclass
class Fleet:
    """Fleet data structure"""
    fleet_id: str
    fleet_name: str
    commander_id: str  # Entity ID of fleet commander
    members: Dict[str, FleetMember] = field(default_factory=dict)  # {entity_id: FleetMember}
    max_members: int = 256  # EVE allows up to 256 in a fleet
    is_free_move: bool = True  # Can members move freely or must stay in formation
    
    # Fleet bonuses
    active_boosters: Dict[FleetBoosterType, str] = field(default_factory=dict)  # {type: booster_entity_id}
    
    # Organization
    squads: Dict[int, List[str]] = field(default_factory=dict)  # {squad_id: [entity_ids]}
    wings: Dict[int, List[int]] = field(default_factory=dict)  # {wing_id: [squad_ids]}


@dataclass
class FleetMembership:
    """Component for entities that are in a fleet"""
    fleet_id: str
    role: FleetRole = FleetRole.MEMBER
    bonuses_active: Dict[str, float] = field(default_factory=dict)  # {bonus_type: value}


class FleetSystem:
    """
    Handles fleet operations and bonuses
    """
    
    def __init__(self, world: World):
        self.world = world
        self.fleet_counter = 0
        
        # Active fleets: {fleet_id: Fleet}
        self.fleets: Dict[str, Fleet] = {}
    
    def create_fleet(
        self,
        commander_entity: Entity,
        fleet_name: str
    ) -> Optional[str]:
        """
        Create a new fleet
        
        Returns:
            fleet_id if successful
        """
        self.fleet_counter += 1
        fleet_id = f"fleet_{self.fleet_counter}"
        
        # Create fleet
        fleet = Fleet(
            fleet_id=fleet_id,
            fleet_name=fleet_name,
            commander_id=commander_entity.id
        )
        
        # Add commander as first member
        member = FleetMember(
            entity_id=commander_entity.id,
            character_name="Commander",
            role=FleetRole.FLEET_COMMANDER
        )
        fleet.members[commander_entity.id] = member
        
        # Add fleet component to commander
        membership = FleetMembership(
            fleet_id=fleet_id,
            role=FleetRole.FLEET_COMMANDER
        )
        commander_entity.add_component(membership)
        
        self.fleets[fleet_id] = fleet
        
        return fleet_id
    
    def disband_fleet(self, fleet_id: str) -> bool:
        """Disband a fleet"""
        fleet = self.fleets.get(fleet_id)
        if not fleet:
            return False
        
        # Remove fleet component from all members
        for member_id in list(fleet.members.keys()):
            entity = self.world.get_entity(member_id)
            if entity:
                membership = entity.get_component(FleetMembership)
                if membership:
                    entity.remove_component(FleetMembership)
        
        # Remove fleet
        del self.fleets[fleet_id]
        
        return True
    
    def invite_to_fleet(
        self,
        fleet_id: str,
        inviter_entity: Entity,
        invitee_entity: Entity
    ) -> bool:
        """Invite a player to join the fleet"""
        fleet = self.fleets.get(fleet_id)
        if not fleet:
            return False
        
        # Check if inviter has permission
        inviter_member = fleet.members.get(inviter_entity.id)
        if not inviter_member:
            return False
        
        # Check fleet capacity
        if len(fleet.members) >= fleet.max_members:
            return False
        
        # Add member
        member = FleetMember(
            entity_id=invitee_entity.id,
            character_name=f"Pilot_{invitee_entity.id[:8]}",
            role=FleetRole.MEMBER
        )
        fleet.members[invitee_entity.id] = member
        
        # Add fleet component
        membership = FleetMembership(
            fleet_id=fleet_id,
            role=FleetRole.MEMBER
        )
        invitee_entity.add_component(membership)
        
        return True
    
    def leave_fleet(self, entity: Entity) -> bool:
        """Leave current fleet"""
        membership = entity.get_component(FleetMembership)
        if not membership:
            return False
        
        fleet = self.fleets.get(membership.fleet_id)
        if not fleet:
            return False
        
        # Remove from fleet
        if entity.id in fleet.members:
            del fleet.members[entity.id]
        
        # Remove component
        entity.remove_component(FleetMembership)
        
        # If fleet commander left, disband or promote
        if entity.id == fleet.commander_id:
            if fleet.members:
                # Promote first member to FC
                new_fc_id = next(iter(fleet.members))
                fleet.commander_id = new_fc_id
                fleet.members[new_fc_id].role = FleetRole.FLEET_COMMANDER
            else:
                # Disband empty fleet
                self.disband_fleet(fleet.fleet_id)
        
        return True
    
    def set_fleet_booster(
        self,
        fleet_id: str,
        booster_entity: Entity,
        booster_type: FleetBoosterType
    ) -> bool:
        """Activate a fleet booster"""
        fleet = self.fleets.get(fleet_id)
        if not fleet:
            return False
        
        # Check if entity is in fleet
        if booster_entity.id not in fleet.members:
            return False
        
        # Activate booster
        fleet.active_boosters[booster_type] = booster_entity.id
        
        # Apply bonuses to all fleet members
        self._apply_fleet_bonuses(fleet)
        
        return True
    
    def _apply_fleet_bonuses(self, fleet: Fleet):
        """Apply fleet bonuses to all members"""
        bonuses = {}
        
        # Calculate bonuses from active boosters
        for booster_type, booster_id in fleet.active_boosters.items():
            if booster_type == FleetBoosterType.ARMOR:
                bonuses["armor_hp"] = 0.10  # +10% armor HP
                bonuses["armor_resist"] = 0.05  # +5% resists
            elif booster_type == FleetBoosterType.SHIELD:
                bonuses["shield_hp"] = 0.10  # +10% shield HP
                bonuses["shield_resist"] = 0.05  # +5% resists
            elif booster_type == FleetBoosterType.SKIRMISH:
                bonuses["max_speed"] = 0.15  # +15% speed
                bonuses["agility"] = 0.10  # +10% agility
            elif booster_type == FleetBoosterType.INFORMATION:
                bonuses["targeting_range"] = 0.20  # +20% range
                bonuses["scan_resolution"] = 0.15  # +15% scan res
        
        # Apply to all members
        for member_id in fleet.members:
            entity = self.world.get_entity(member_id)
            if entity:
                membership = entity.get_component(FleetMembership)
                if membership:
                    membership.bonuses_active = bonuses.copy()
    
    def get_fleet_members(self, fleet_id: str) -> List[FleetMember]:
        """Get all members of a fleet"""
        fleet = self.fleets.get(fleet_id)
        if not fleet:
            return []
        
        return list(fleet.members.values())
    
    def get_fleet_by_entity(self, entity: Entity) -> Optional[Fleet]:
        """Get the fleet an entity is in"""
        membership = entity.get_component(FleetMembership)
        if not membership:
            return None
        
        return self.fleets.get(membership.fleet_id)
    
    def broadcast_target(
        self,
        fleet_id: str,
        broadcaster_entity: Entity,
        target_entity: Entity
    ) -> bool:
        """
        Broadcast a target to the fleet
        (In a real implementation, this would notify all fleet members)
        """
        fleet = self.fleets.get(fleet_id)
        if not fleet:
            return False
        
        # Check if broadcaster is in fleet
        if broadcaster_entity.id not in fleet.members:
            return False
        
        # In a full implementation, this would:
        # 1. Send network message to all fleet members
        # 2. Highlight target in their UI
        # 3. Allow quick lock-on
        
        return True
    
    def warp_fleet_to(
        self,
        fleet_id: str,
        commander_entity: Entity,
        destination: str
    ) -> bool:
        """
        Command entire fleet to warp to a destination
        """
        fleet = self.fleets.get(fleet_id)
        if not fleet:
            return False
        
        # Only FC can warp fleet
        if commander_entity.id != fleet.commander_id:
            return False
        
        # Warp all fleet members
        for member_id in fleet.members:
            entity = self.world.get_entity(member_id)
            if entity:
                # In a full implementation, this would trigger warp
                # For now, just return success
                pass
        
        return True
    
    def assign_squad(
        self,
        fleet_id: str,
        member_entity: Entity,
        squad_id: int
    ) -> bool:
        """Assign a member to a squad"""
        fleet = self.fleets.get(fleet_id)
        if not fleet:
            return False
        
        member = fleet.members.get(member_entity.id)
        if not member:
            return False
        
        # Remove from old squad
        if member.squad_id is not None:
            if member.squad_id in fleet.squads:
                fleet.squads[member.squad_id].remove(member_entity.id)
        
        # Add to new squad
        if squad_id not in fleet.squads:
            fleet.squads[squad_id] = []
        
        fleet.squads[squad_id].append(member_entity.id)
        member.squad_id = squad_id
        
        return True
    
    def promote_to_squad_commander(
        self,
        fleet_id: str,
        promoter_entity: Entity,
        member_entity: Entity,
        squad_id: int
    ) -> bool:
        """Promote a member to squad commander"""
        fleet = self.fleets.get(fleet_id)
        if not fleet:
            return False
        
        # Only FC can promote
        if promoter_entity.id != fleet.commander_id:
            return False
        
        member = fleet.members.get(member_entity.id)
        if not member:
            return False
        
        # Assign to squad and promote
        member.role = FleetRole.SQUAD_COMMANDER
        member.squad_id = squad_id
        
        # Update membership component
        membership = member_entity.get_component(FleetMembership)
        if membership:
            membership.role = FleetRole.SQUAD_COMMANDER
        
        return True
    
    def get_fleet_info(self, fleet_id: str) -> Optional[Dict]:
        """Get fleet information"""
        fleet = self.fleets.get(fleet_id)
        if not fleet:
            return None
        
        return {
            "fleet_id": fleet.fleet_id,
            "fleet_name": fleet.fleet_name,
            "commander_id": fleet.commander_id,
            "member_count": len(fleet.members),
            "max_members": fleet.max_members,
            "active_boosters": [bt.value for bt in fleet.active_boosters.keys()],
            "squad_count": len(fleet.squads),
            "wing_count": len(fleet.wings)
        }
    
    def update(self, delta_time: float):
        """Update fleet system"""
        # Update fleet bonuses, check for offline members, etc.
        for fleet in self.fleets.values():
            # Remove offline/disconnected members
            members_to_remove = []
            
            for member_id in fleet.members:
                entity = self.world.get_entity(member_id)
                if not entity:
                    members_to_remove.append(member_id)
            
            for member_id in members_to_remove:
                del fleet.members[member_id]
            
            # Disband empty fleets
            if not fleet.members:
                self.disband_fleet(fleet.fleet_id)
