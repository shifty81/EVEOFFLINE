"""
Corporation System
Handles corporation management, member roles, hangars, and wallets
Based on EVE Online's corporation system
"""

from typing import Optional, Dict, List, Set
from dataclasses import dataclass, field
from enum import Enum
from engine.core.ecs import World, Entity


class CorporationRole(Enum):
    """Corporation member roles"""
    MEMBER = "member"
    DIRECTOR = "director"
    CEO = "ceo"


@dataclass
class CorporationMember:
    """Individual corporation member"""
    entity_id: str
    character_name: str
    role: CorporationRole = CorporationRole.MEMBER
    joined_date: float = 0.0
    title: str = ""
    is_online: bool = True


@dataclass
class Corporation:
    """Corporation data structure"""
    corporation_id: str
    corporation_name: str
    ticker: str  # 3-5 character ticker (e.g., "TEST")
    ceo_id: str  # Entity ID of CEO
    description: str = ""
    tax_rate: float = 0.10  # Default 10% tax on member earnings
    
    # Members
    members: Dict[str, CorporationMember] = field(default_factory=dict)  # {entity_id: CorporationMember}
    max_members: int = 50  # Small corps for this game
    
    # Corporation wallet
    wallet_balance: float = 0.0  # Corporation ISK
    
    # Corporation hangars (shared storage)
    hangars: Dict[str, Dict[str, int]] = field(default_factory=lambda: {
        "Division 1": {},
        "Division 2": {},
        "Division 3": {},
    })  # {division_name: {item_id: quantity}}
    
    # Permissions
    hangar_access: Dict[str, List[str]] = field(default_factory=lambda: {
        "Division 1": ["member", "director", "ceo"],
        "Division 2": ["director", "ceo"],
        "Division 3": ["ceo"],
    })  # {division_name: [roles with access]}
    
    wallet_access: List[str] = field(default_factory=lambda: ["director", "ceo"])
    
    # Standings
    standings: Dict[str, float] = field(default_factory=dict)  # {other_corp_id: standing}
    
    # Creation date
    founded_date: float = 0.0


class CorporationSystem:
    """
    Handles corporation operations and management
    """
    
    def __init__(self, world: World):
        self.world = world
        self.corp_counter = 0
        
        # Active corporations: {corp_id: Corporation}
        self.corporations: Dict[str, Corporation] = {}
        
        # Initialize NPC corps
        self._init_npc_corps()
    
    def _init_npc_corps(self):
        """Initialize default NPC corporations"""
        npc_corps = [
            ("npc_corp_caldari", "Caldari Navy", "CLDN", "NPC-owned Caldari State military corporation"),
            ("npc_corp_gallente", "Federal Navy", "FEDN", "NPC-owned Gallente Federation military corporation"),
            ("npc_corp_amarr", "Imperial Navy", "IMPN", "NPC-owned Amarr Empire military corporation"),
            ("npc_corp_minmatar", "Republic Fleet", "REPF", "NPC-owned Minmatar Republic military corporation"),
        ]
        
        for corp_id, name, ticker, desc in npc_corps:
            corp = Corporation(
                corporation_id=corp_id,
                corporation_name=name,
                ticker=ticker,
                ceo_id="npc",
                description=desc,
                tax_rate=0.11,  # NPC corps charge 11% tax
                max_members=999999
            )
            self.corporations[corp_id] = corp
    
    def create_corporation(
        self,
        ceo_entity: Entity,
        corporation_name: str,
        ticker: str,
        description: str = "",
        creation_cost: float = 1000000.0  # 1M ISK to create corp
    ) -> Optional[str]:
        """
        Create a new player corporation
        
        Args:
            ceo_entity: Entity of the CEO
            corporation_name: Name of the corporation
            ticker: 3-5 character ticker symbol
            description: Corporation description
            creation_cost: ISK cost to create corporation
            
        Returns:
            corporation_id if successful, None if failed
        """
        from engine.components.game_components import Player
        
        # Validate ticker
        if len(ticker) < 3 or len(ticker) > 5:
            return None
        
        # Check if CEO has enough ISK
        player_comp = ceo_entity.get_component(Player)
        if not player_comp or player_comp.isk < creation_cost:
            return None
        
        # Charge ISK
        player_comp.isk -= creation_cost
        
        # Generate corp ID
        self.corp_counter += 1
        corp_id = f"corp_{self.corp_counter}"
        
        # Create corporation
        corp = Corporation(
            corporation_id=corp_id,
            corporation_name=corporation_name,
            ticker=ticker,
            ceo_id=ceo_entity.id,
            description=description,
            founded_date=0.0,  # Would use time.time() in real implementation
            wallet_balance=0.0
        )
        
        # Add CEO as first member
        from engine.components.game_components import CorporationMembership
        
        member = CorporationMember(
            entity_id=ceo_entity.id,
            character_name=player_comp.character_name if player_comp else "Unknown",
            role=CorporationRole.CEO,
            joined_date=0.0,
            is_online=True
        )
        corp.members[ceo_entity.id] = member
        
        # Update player's corporation membership
        corp_membership = CorporationMembership(
            corporation_id=corp_id,
            corporation_name=corporation_name,
            role="ceo",
            joined_date=0.0
        )
        ceo_entity.add_component(corp_membership)
        
        # Update player component
        player_comp.corporation = corporation_name
        
        # Store corporation
        self.corporations[corp_id] = corp
        
        return corp_id
    
    def invite_member(
        self,
        corp_id: str,
        inviter_entity: Entity,
        invitee_entity: Entity
    ) -> bool:
        """
        Invite a player to join a corporation
        
        Args:
            corp_id: Corporation ID
            inviter_entity: Entity sending the invite (must be director or CEO)
            invitee_entity: Entity being invited
            
        Returns:
            True if invite sent successfully
        """
        corp = self.corporations.get(corp_id)
        if not corp:
            return False
        
        # Check if inviter has permission
        inviter_member = corp.members.get(inviter_entity.id)
        if not inviter_member or inviter_member.role not in [CorporationRole.CEO, CorporationRole.DIRECTOR]:
            return False
        
        # Check if corp is full
        if len(corp.members) >= corp.max_members:
            return False
        
        # In a real implementation, this would send an invite that the invitee must accept
        # For now, we'll auto-accept
        return self.add_member(corp_id, invitee_entity)
    
    def add_member(
        self,
        corp_id: str,
        entity: Entity
    ) -> bool:
        """
        Add a member to a corporation
        
        Args:
            corp_id: Corporation ID
            entity: Entity to add
            
        Returns:
            True if added successfully
        """
        from engine.components.game_components import Player, CorporationMembership
        
        corp = self.corporations.get(corp_id)
        if not corp:
            return False
        
        # Check if corp is full
        if len(corp.members) >= corp.max_members:
            return False
        
        # Check if already a member
        if entity.id in corp.members:
            return False
        
        player_comp = entity.get_component(Player)
        if not player_comp:
            return False
        
        # Add member
        member = CorporationMember(
            entity_id=entity.id,
            character_name=player_comp.character_name,
            role=CorporationRole.MEMBER,
            joined_date=0.0,
            is_online=True
        )
        corp.members[entity.id] = member
        
        # Update player's corporation membership
        corp_membership = CorporationMembership(
            corporation_id=corp_id,
            corporation_name=corp.corporation_name,
            role="member",
            joined_date=0.0
        )
        entity.add_component(corp_membership)
        
        # Update player component
        player_comp.corporation = corp.corporation_name
        
        return True
    
    def remove_member(
        self,
        corp_id: str,
        entity_id: str,
        remover_entity: Optional[Entity] = None
    ) -> bool:
        """
        Remove a member from a corporation
        
        Args:
            corp_id: Corporation ID
            entity_id: Entity ID to remove
            remover_entity: Entity performing the removal (must be director/CEO if kicking)
            
        Returns:
            True if removed successfully
        """
        corp = self.corporations.get(corp_id)
        if not corp:
            return False
        
        member = corp.members.get(entity_id)
        if not member:
            return False
        
        # Can't remove the CEO
        if member.role == CorporationRole.CEO:
            return False
        
        # Check permissions if someone else is removing
        if remover_entity and remover_entity.id != entity_id:
            remover_member = corp.members.get(remover_entity.id)
            if not remover_member or remover_member.role not in [CorporationRole.CEO, CorporationRole.DIRECTOR]:
                return False
        
        # Remove member
        del corp.members[entity_id]
        
        # Update player's corporation to NPC corp
        from engine.components.game_components import Player, CorporationMembership
        entity_obj = self.world.get_entity(entity_id)
        if entity_obj:
            player_comp = entity_obj.get_component(Player)
            if player_comp:
                player_comp.corporation = "NPC Corporation"
            
            corp_membership = CorporationMembership(
                corporation_id="npc_corp_caldari",
                corporation_name="NPC Corporation",
                role="member",
                joined_date=0.0
            )
            entity_obj.add_component(corp_membership)
        
        return True
    
    def set_member_role(
        self,
        corp_id: str,
        entity_id: str,
        new_role: CorporationRole,
        setter_entity: Entity
    ) -> bool:
        """
        Change a member's role
        
        Args:
            corp_id: Corporation ID
            entity_id: Entity ID to change role for
            new_role: New role
            setter_entity: Entity performing the change (must be CEO)
            
        Returns:
            True if role changed successfully
        """
        corp = self.corporations.get(corp_id)
        if not corp:
            return False
        
        # Only CEO can change roles
        setter_member = corp.members.get(setter_entity.id)
        if not setter_member or setter_member.role != CorporationRole.CEO:
            return False
        
        member = corp.members.get(entity_id)
        if not member:
            return False
        
        # Can't change CEO role (would need transfer of CEO)
        if member.role == CorporationRole.CEO or new_role == CorporationRole.CEO:
            return False
        
        member.role = new_role
        
        # Update component
        from engine.components.game_components import CorporationMembership
        entity_obj = self.world.get_entity(entity_id)
        if entity_obj:
            corp_membership = entity_obj.get_component(CorporationMembership)
            if corp_membership:
                corp_membership.role = new_role.value
        
        return True
    
    def deposit_to_wallet(
        self,
        corp_id: str,
        entity: Entity,
        amount: float
    ) -> bool:
        """
        Deposit ISK to corporation wallet
        
        Args:
            corp_id: Corporation ID
            entity: Entity making deposit
            amount: Amount to deposit
            
        Returns:
            True if deposited successfully
        """
        from engine.components.game_components import Player
        
        corp = self.corporations.get(corp_id)
        if not corp:
            return False
        
        # Check if member
        if entity.id not in corp.members:
            return False
        
        player_comp = entity.get_component(Player)
        if not player_comp or player_comp.isk < amount:
            return False
        
        # Transfer ISK
        player_comp.isk -= amount
        corp.wallet_balance += amount
        
        return True
    
    def withdraw_from_wallet(
        self,
        corp_id: str,
        entity: Entity,
        amount: float
    ) -> bool:
        """
        Withdraw ISK from corporation wallet
        
        Args:
            corp_id: Corporation ID
            entity: Entity making withdrawal (must have wallet access)
            amount: Amount to withdraw
            
        Returns:
            True if withdrawn successfully
        """
        from engine.components.game_components import Player
        
        corp = self.corporations.get(corp_id)
        if not corp:
            return False
        
        # Check permissions
        member = corp.members.get(entity.id)
        if not member or member.role.value not in corp.wallet_access:
            return False
        
        # Check balance
        if corp.wallet_balance < amount:
            return False
        
        player_comp = entity.get_component(Player)
        if not player_comp:
            return False
        
        # Transfer ISK
        corp.wallet_balance -= amount
        player_comp.isk += amount
        
        return True
    
    def add_to_hangar(
        self,
        corp_id: str,
        division: str,
        item_id: str,
        quantity: int
    ) -> bool:
        """
        Add items to corporation hangar
        
        Args:
            corp_id: Corporation ID
            division: Hangar division name
            item_id: Item ID
            quantity: Quantity to add
            
        Returns:
            True if added successfully
        """
        corp = self.corporations.get(corp_id)
        if not corp:
            return False
        
        if division not in corp.hangars:
            return False
        
        if item_id in corp.hangars[division]:
            corp.hangars[division][item_id] += quantity
        else:
            corp.hangars[division][item_id] = quantity
        
        return True
    
    def remove_from_hangar(
        self,
        corp_id: str,
        division: str,
        entity: Entity,
        item_id: str,
        quantity: int
    ) -> bool:
        """
        Remove items from corporation hangar
        
        Args:
            corp_id: Corporation ID
            division: Hangar division name
            entity: Entity removing items (must have access)
            item_id: Item ID
            quantity: Quantity to remove
            
        Returns:
            True if removed successfully
        """
        corp = self.corporations.get(corp_id)
        if not corp:
            return False
        
        if division not in corp.hangars:
            return False
        
        # Check permissions
        member = corp.members.get(entity.id)
        if not member or member.role.value not in corp.hangar_access.get(division, []):
            return False
        
        # Check if item exists and has enough quantity
        if item_id not in corp.hangars[division]:
            return False
        
        if corp.hangars[division][item_id] < quantity:
            return False
        
        # Remove items
        corp.hangars[division][item_id] -= quantity
        if corp.hangars[division][item_id] <= 0:
            del corp.hangars[division][item_id]
        
        return True
    
    def get_corporation(self, corp_id: str) -> Optional[Corporation]:
        """Get corporation by ID"""
        return self.corporations.get(corp_id)
    
    def get_corporation_by_name(self, name: str) -> Optional[Corporation]:
        """Get corporation by name"""
        for corp in self.corporations.values():
            if corp.corporation_name == name:
                return corp
        return None
    
    def apply_tax(
        self,
        entity: Entity,
        earnings: float
    ) -> float:
        """
        Apply corporation tax to earnings
        
        Args:
            entity: Entity earning ISK
            earnings: Amount earned
            
        Returns:
            Amount after tax
        """
        from engine.components.game_components import CorporationMembership
        
        corp_membership = entity.get_component(CorporationMembership)
        if not corp_membership:
            return earnings
        
        corp = self.corporations.get(corp_membership.corporation_id)
        if not corp:
            return earnings
        
        # Calculate tax
        tax_amount = earnings * corp.tax_rate
        after_tax = earnings - tax_amount
        
        # Add tax to corp wallet
        corp.wallet_balance += tax_amount
        
        return after_tax
