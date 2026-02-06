"""
Contract System
Handles player-to-player contracts: item exchange, courier, and auction
Based on EVE Online's contract system
"""

from typing import Optional, Dict, List
from dataclasses import dataclass, field
from enum import Enum
from engine.core.ecs import World, Entity
import time


class ContractType(Enum):
    """Types of contracts"""
    ITEM_EXCHANGE = "item_exchange"  # Trade items for ISK
    COURIER = "courier"  # Transport items from A to B
    AUCTION = "auction"  # Auction items to highest bidder


class ContractStatus(Enum):
    """Contract status"""
    OUTSTANDING = "outstanding"  # Available to accept
    IN_PROGRESS = "in_progress"  # Accepted, being completed
    COMPLETED = "completed"  # Successfully completed
    FAILED = "failed"  # Failed (expired or courier lost items)
    CANCELLED = "cancelled"  # Cancelled by issuer


@dataclass
class ItemExchangeContract:
    """Item exchange contract (buy/sell items for ISK)"""
    contract_id: str
    issuer_id: str
    issuer_name: str
    contract_type: ContractType = ContractType.ITEM_EXCHANGE
    
    # Items offered
    items_offered: Dict[str, int] = field(default_factory=dict)  # {item_id: quantity}
    
    # Items requested (can be empty for pure sell contract)
    items_requested: Dict[str, int] = field(default_factory=dict)  # {item_id: quantity}
    
    # ISK exchange
    price: float = 0.0  # ISK price (positive = issuer wants ISK, negative = issuer pays ISK)
    collateral: float = 0.0  # Collateral required
    
    # Contract details
    status: ContractStatus = ContractStatus.OUTSTANDING
    acceptor_id: Optional[str] = None
    acceptor_name: Optional[str] = None
    
    # Timestamps
    issued_date: float = 0.0
    expiration_date: float = 0.0
    accepted_date: Optional[float] = None
    completed_date: Optional[float] = None
    
    # Location
    location: str = "Jita"  # Station/system where contract is available
    
    # Availability
    availability: str = "public"  # public, private (specific character), corporation


@dataclass
class CourierContract:
    """Courier contract (transport items from A to B)"""
    contract_id: str
    issuer_id: str
    issuer_name: str
    contract_type: ContractType = ContractType.COURIER
    
    # Items to transport
    items: Dict[str, int] = field(default_factory=dict)  # {item_id: quantity}
    volume: float = 0.0  # m3 of items
    
    # Locations
    start_location: str = "Jita"
    end_location: str = "Amarr"
    
    # Payment
    reward: float = 0.0  # ISK reward for completion
    collateral: float = 0.0  # ISK collateral (paid if items lost)
    
    # Contract details
    status: ContractStatus = ContractStatus.OUTSTANDING
    acceptor_id: Optional[str] = None
    acceptor_name: Optional[str] = None
    
    # Timestamps
    issued_date: float = 0.0
    expiration_date: float = 0.0
    accepted_date: Optional[float] = None
    completed_date: Optional[float] = None
    days_to_complete: int = 3  # Days after acceptance to complete
    
    # Availability
    availability: str = "public"


@dataclass
class AuctionContract:
    """Auction contract (items go to highest bidder)"""
    contract_id: str
    issuer_id: str
    issuer_name: str
    contract_type: ContractType = ContractType.AUCTION
    
    # Items being auctioned
    items: Dict[str, int] = field(default_factory=dict)  # {item_id: quantity}
    
    # Auction details
    starting_bid: float = 0.0
    buyout_price: Optional[float] = None  # Optional instant buyout
    current_bid: float = 0.0
    current_bidder_id: Optional[str] = None
    current_bidder_name: Optional[str] = None
    
    # Bid history
    bid_history: List[Dict[str, any]] = field(default_factory=list)
    
    # Contract details
    status: ContractStatus = ContractStatus.OUTSTANDING
    
    # Timestamps
    issued_date: float = 0.0
    expiration_date: float = 0.0
    completed_date: Optional[float] = None
    
    # Location
    location: str = "Jita"
    
    # Availability
    availability: str = "public"


class ContractSystem:
    """
    Handles player-to-player contracts
    """
    
    def __init__(self, world: World):
        self.world = world
        self.contract_counter = 0
        
        # All contracts: {contract_id: Contract}
        self.contracts: Dict[str, any] = {}
        
        # Broker fee for creating contracts (1% of contract value)
        self.broker_fee_rate = 0.01
    
    # ===== ITEM EXCHANGE CONTRACTS =====
    
    def create_item_exchange_contract(
        self,
        issuer_entity: Entity,
        items_offered: Dict[str, int],
        items_requested: Dict[str, int],
        price: float,
        collateral: float = 0.0,
        expiration_days: int = 7,
        availability: str = "public",
        location: str = "Jita"
    ) -> Optional[str]:
        """
        Create an item exchange contract
        
        Args:
            issuer_entity: Entity creating the contract
            items_offered: Items being offered {item_id: quantity}
            items_requested: Items being requested (can be empty)
            price: ISK price (positive = issuer wants ISK)
            collateral: ISK collateral required
            expiration_days: Days until contract expires
            availability: "public" or "private"
            location: Station/system where contract is
            
        Returns:
            contract_id if created successfully
        """
        from engine.components.game_components import Player, Inventory
        
        player_comp = issuer_entity.get_component(Player)
        if not player_comp:
            return None
        
        # Calculate broker fee
        contract_value = abs(price) + collateral
        broker_fee = contract_value * self.broker_fee_rate
        
        # Check if issuer has enough ISK for broker fee
        if player_comp.isk < broker_fee:
            return None
        
        # Check if issuer has the offered items
        inventory_comp = issuer_entity.get_component(Inventory)
        if not inventory_comp:
            return None
        
        for item_id, quantity in items_offered.items():
            if inventory_comp.items.get(item_id, 0) < quantity:
                return None
        
        # Charge broker fee
        player_comp.isk -= broker_fee
        
        # Remove offered items from inventory (held in contract)
        for item_id, quantity in items_offered.items():
            inventory_comp.items[item_id] -= quantity
            if inventory_comp.items[item_id] <= 0:
                del inventory_comp.items[item_id]
        
        # Generate contract ID
        self.contract_counter += 1
        contract_id = f"contract_{self.contract_counter}"
        
        # Create contract
        contract = ItemExchangeContract(
            contract_id=contract_id,
            issuer_id=issuer_entity.id,
            issuer_name=player_comp.character_name,
            items_offered=items_offered.copy(),
            items_requested=items_requested.copy(),
            price=price,
            collateral=collateral,
            issued_date=time.time(),
            expiration_date=time.time() + (expiration_days * 86400),
            location=location,
            availability=availability
        )
        
        self.contracts[contract_id] = contract
        
        # Add to issuer's contract list
        from engine.components.game_components import Contract as ContractComp
        contract_comp = issuer_entity.get_component(ContractComp)
        if not contract_comp:
            contract_comp = ContractComp()
            issuer_entity.add_component(contract_comp)
        
        contract_comp.active_contracts.append(contract_id)
        
        return contract_id
    
    def accept_item_exchange_contract(
        self,
        acceptor_entity: Entity,
        contract_id: str
    ) -> bool:
        """
        Accept an item exchange contract
        
        Args:
            acceptor_entity: Entity accepting the contract
            contract_id: Contract ID to accept
            
        Returns:
            True if accepted successfully
        """
        from engine.components.game_components import Player, Inventory, Contract as ContractComp
        
        contract = self.contracts.get(contract_id)
        if not contract or not isinstance(contract, ItemExchangeContract):
            return False
        
        # Check if contract is available
        if contract.status != ContractStatus.OUTSTANDING:
            return False
        
        # Check if expired
        if time.time() > contract.expiration_date:
            contract.status = ContractStatus.FAILED
            return False
        
        player_comp = acceptor_entity.get_component(Player)
        if not player_comp:
            return False
        
        # Check if acceptor has enough ISK
        total_isk_needed = max(0, contract.price) + contract.collateral
        if player_comp.isk < total_isk_needed:
            return False
        
        # Check if acceptor has requested items
        inventory_comp = acceptor_entity.get_component(Inventory)
        if not inventory_comp:
            inventory_comp = Inventory()
            acceptor_entity.add_component(inventory_comp)
        
        for item_id, quantity in contract.items_requested.items():
            if inventory_comp.items.get(item_id, 0) < quantity:
                return False
        
        # Execute contract
        # Transfer ISK
        if contract.price > 0:
            # Acceptor pays issuer
            player_comp.isk -= contract.price
            issuer_entity = self.world.get_entity(contract.issuer_id)
            if issuer_entity:
                issuer_player = issuer_entity.get_component(Player)
                if issuer_player:
                    issuer_player.isk += contract.price
        elif contract.price < 0:
            # Issuer pays acceptor
            player_comp.isk += abs(contract.price)
            issuer_entity = self.world.get_entity(contract.issuer_id)
            if issuer_entity:
                issuer_player = issuer_entity.get_component(Player)
                if issuer_player:
                    issuer_player.isk -= abs(contract.price)
        
        # Transfer offered items to acceptor
        for item_id, quantity in contract.items_offered.items():
            inventory_comp.items[item_id] = inventory_comp.items.get(item_id, 0) + quantity
        
        # Transfer requested items to issuer
        if contract.items_requested:
            issuer_entity = self.world.get_entity(contract.issuer_id)
            if issuer_entity:
                issuer_inventory = issuer_entity.get_component(Inventory)
                if not issuer_inventory:
                    issuer_inventory = Inventory()
                    issuer_entity.add_component(issuer_inventory)
                
                for item_id, quantity in contract.items_requested.items():
                    # Remove from acceptor
                    inventory_comp.items[item_id] -= quantity
                    if inventory_comp.items[item_id] <= 0:
                        del inventory_comp.items[item_id]
                    
                    # Add to issuer
                    issuer_inventory.items[item_id] = issuer_inventory.items.get(item_id, 0) + quantity
        
        # Update contract status
        contract.status = ContractStatus.COMPLETED
        contract.acceptor_id = acceptor_entity.id
        contract.acceptor_name = player_comp.character_name
        contract.accepted_date = time.time()
        contract.completed_date = time.time()
        
        # Update contract components
        contract_comp = acceptor_entity.get_component(ContractComp)
        if not contract_comp:
            contract_comp = ContractComp()
            acceptor_entity.add_component(contract_comp)
        
        contract_comp.completed_contracts.append(contract_id)
        
        # Update issuer's contract list
        issuer_entity = self.world.get_entity(contract.issuer_id)
        if issuer_entity:
            issuer_contract = issuer_entity.get_component(ContractComp)
            if issuer_contract and contract_id in issuer_contract.active_contracts:
                issuer_contract.active_contracts.remove(contract_id)
                issuer_contract.completed_contracts.append(contract_id)
        
        return True
    
    # ===== COURIER CONTRACTS =====
    
    def create_courier_contract(
        self,
        issuer_entity: Entity,
        items: Dict[str, int],
        start_location: str,
        end_location: str,
        reward: float,
        collateral: float,
        expiration_days: int = 7,
        days_to_complete: int = 3,
        availability: str = "public"
    ) -> Optional[str]:
        """
        Create a courier contract
        
        Args:
            issuer_entity: Entity creating the contract
            items: Items to transport {item_id: quantity}
            start_location: Starting station/system
            end_location: Destination station/system
            reward: ISK reward for completion
            collateral: ISK collateral (paid if items lost)
            expiration_days: Days until contract expires
            days_to_complete: Days to complete after acceptance
            availability: "public" or "private"
            
        Returns:
            contract_id if created successfully
        """
        from engine.components.game_components import Player, Inventory, Contract as ContractComp
        
        player_comp = issuer_entity.get_component(Player)
        if not player_comp:
            return None
        
        # Calculate broker fee
        broker_fee = (reward + collateral) * self.broker_fee_rate
        
        # Check if issuer has enough ISK
        if player_comp.isk < broker_fee:
            return None
        
        # Check if issuer has the items
        inventory_comp = issuer_entity.get_component(Inventory)
        if not inventory_comp:
            return None
        
        for item_id, quantity in items.items():
            if inventory_comp.items.get(item_id, 0) < quantity:
                return None
        
        # Calculate volume (simplified, assume 1m3 per item)
        volume = sum(items.values())
        
        # Charge broker fee
        player_comp.isk -= broker_fee
        
        # Remove items from inventory (held in contract)
        for item_id, quantity in items.items():
            inventory_comp.items[item_id] -= quantity
            if inventory_comp.items[item_id] <= 0:
                del inventory_comp.items[item_id]
        
        # Generate contract ID
        self.contract_counter += 1
        contract_id = f"contract_{self.contract_counter}"
        
        # Create contract
        contract = CourierContract(
            contract_id=contract_id,
            issuer_id=issuer_entity.id,
            issuer_name=player_comp.character_name,
            items=items.copy(),
            volume=volume,
            start_location=start_location,
            end_location=end_location,
            reward=reward,
            collateral=collateral,
            issued_date=time.time(),
            expiration_date=time.time() + (expiration_days * 86400),
            days_to_complete=days_to_complete,
            availability=availability
        )
        
        self.contracts[contract_id] = contract
        
        # Add to issuer's contract list
        contract_comp = issuer_entity.get_component(ContractComp)
        if not contract_comp:
            contract_comp = ContractComp()
            issuer_entity.add_component(contract_comp)
        
        contract_comp.active_contracts.append(contract_id)
        
        return contract_id
    
    def accept_courier_contract(
        self,
        acceptor_entity: Entity,
        contract_id: str
    ) -> bool:
        """
        Accept a courier contract
        
        Args:
            acceptor_entity: Entity accepting the contract
            contract_id: Contract ID to accept
            
        Returns:
            True if accepted successfully
        """
        from engine.components.game_components import Player, Inventory, Contract as ContractComp
        
        contract = self.contracts.get(contract_id)
        if not contract or not isinstance(contract, CourierContract):
            return False
        
        # Check if contract is available
        if contract.status != ContractStatus.OUTSTANDING:
            return False
        
        # Check if expired
        if time.time() > contract.expiration_date:
            contract.status = ContractStatus.FAILED
            return False
        
        player_comp = acceptor_entity.get_component(Player)
        if not player_comp:
            return False
        
        # Check if acceptor has enough ISK for collateral
        if player_comp.isk < contract.collateral:
            return False
        
        # Lock collateral
        player_comp.isk -= contract.collateral
        
        # Transfer items to acceptor's inventory
        inventory_comp = acceptor_entity.get_component(Inventory)
        if not inventory_comp:
            inventory_comp = Inventory()
            acceptor_entity.add_component(inventory_comp)
        
        for item_id, quantity in contract.items.items():
            inventory_comp.items[item_id] = inventory_comp.items.get(item_id, 0) + quantity
        
        # Update contract status
        contract.status = ContractStatus.IN_PROGRESS
        contract.acceptor_id = acceptor_entity.id
        contract.acceptor_name = player_comp.character_name
        contract.accepted_date = time.time()
        
        # Add to acceptor's active contracts
        contract_comp = acceptor_entity.get_component(ContractComp)
        if not contract_comp:
            contract_comp = ContractComp()
            acceptor_entity.add_component(contract_comp)
        
        contract_comp.active_contracts.append(contract_id)
        
        return True
    
    def complete_courier_contract(
        self,
        acceptor_entity: Entity,
        contract_id: str
    ) -> bool:
        """
        Complete a courier contract (when at destination)
        
        Args:
            acceptor_entity: Entity completing the contract
            contract_id: Contract ID to complete
            
        Returns:
            True if completed successfully
        """
        from engine.components.game_components import Player, Inventory, Contract as ContractComp
        
        contract = self.contracts.get(contract_id)
        if not contract or not isinstance(contract, CourierContract):
            return False
        
        # Check if contract is in progress
        if contract.status != ContractStatus.IN_PROGRESS:
            return False
        
        # Check if acceptor is the one who accepted
        if contract.acceptor_id != acceptor_entity.id:
            return False
        
        # In a real game, check if at destination
        # For now, we'll assume they are
        
        player_comp = acceptor_entity.get_component(Player)
        if not player_comp:
            return False
        
        # Remove items from acceptor's inventory
        inventory_comp = acceptor_entity.get_component(Inventory)
        if not inventory_comp:
            return False
        
        for item_id, quantity in contract.items.items():
            if inventory_comp.items.get(item_id, 0) < quantity:
                # Contract failed - items lost
                return self.fail_courier_contract(acceptor_entity, contract_id)
            
            inventory_comp.items[item_id] -= quantity
            if inventory_comp.items[item_id] <= 0:
                del inventory_comp.items[item_id]
        
        # Return items to issuer
        issuer_entity = self.world.get_entity(contract.issuer_id)
        if issuer_entity:
            issuer_inventory = issuer_entity.get_component(Inventory)
            if not issuer_inventory:
                issuer_inventory = Inventory()
                issuer_entity.add_component(issuer_inventory)
            
            for item_id, quantity in contract.items.items():
                issuer_inventory.items[item_id] = issuer_inventory.items.get(item_id, 0) + quantity
        
        # Pay reward and return collateral
        player_comp.isk += contract.reward + contract.collateral
        
        # Issuer pays reward
        if issuer_entity:
            issuer_player = issuer_entity.get_component(Player)
            if issuer_player:
                issuer_player.isk -= contract.reward
        
        # Update contract status
        contract.status = ContractStatus.COMPLETED
        contract.completed_date = time.time()
        
        # Update contract components
        contract_comp = acceptor_entity.get_component(ContractComp)
        if contract_comp and contract_id in contract_comp.active_contracts:
            contract_comp.active_contracts.remove(contract_id)
            contract_comp.completed_contracts.append(contract_id)
        
        # Update issuer's contract list
        if issuer_entity:
            issuer_contract = issuer_entity.get_component(ContractComp)
            if issuer_contract and contract_id in issuer_contract.active_contracts:
                issuer_contract.active_contracts.remove(contract_id)
                issuer_contract.completed_contracts.append(contract_id)
        
        return True
    
    def fail_courier_contract(
        self,
        acceptor_entity: Entity,
        contract_id: str
    ) -> bool:
        """
        Fail a courier contract (items lost)
        
        Args:
            acceptor_entity: Entity who failed the contract
            contract_id: Contract ID
            
        Returns:
            True if failed successfully
        """
        from engine.components.game_components import Player, Contract as ContractComp
        
        contract = self.contracts.get(contract_id)
        if not contract or not isinstance(contract, CourierContract):
            return False
        
        # Pay collateral to issuer
        issuer_entity = self.world.get_entity(contract.issuer_id)
        if issuer_entity:
            issuer_player = issuer_entity.get_component(Player)
            if issuer_player:
                issuer_player.isk += contract.collateral
        
        # Acceptor loses collateral (already deducted)
        
        # Update contract status
        contract.status = ContractStatus.FAILED
        contract.completed_date = time.time()
        
        # Update contract components
        player_comp = acceptor_entity.get_component(Player)
        if player_comp:
            contract_comp = acceptor_entity.get_component(ContractComp)
            if contract_comp and contract_id in contract_comp.active_contracts:
                contract_comp.active_contracts.remove(contract_id)
                contract_comp.failed_contracts.append(contract_id)
        
        return True
    
    # ===== GENERAL CONTRACT METHODS =====
    
    def get_contract(self, contract_id: str) -> Optional[any]:
        """Get contract by ID"""
        return self.contracts.get(contract_id)
    
    def list_available_contracts(
        self,
        entity: Entity,
        location: Optional[str] = None,
        contract_type: Optional[ContractType] = None
    ) -> List[any]:
        """
        List available contracts
        
        Args:
            entity: Entity viewing contracts
            location: Filter by location
            contract_type: Filter by contract type
            
        Returns:
            List of available contracts
        """
        available = []
        
        for contract in self.contracts.values():
            # Skip non-outstanding contracts
            if contract.status != ContractStatus.OUTSTANDING:
                continue
            
            # Skip expired contracts
            if time.time() > contract.expiration_date:
                contract.status = ContractStatus.FAILED
                continue
            
            # Filter by location
            if location and hasattr(contract, 'location') and contract.location != location:
                continue
            
            # Filter by type
            if contract_type and contract.contract_type != contract_type:
                continue
            
            # Check availability
            if contract.availability == "public":
                available.append(contract)
            elif contract.availability == "private":
                # Would check if contract is for this specific entity
                pass
        
        return available
    
    def cancel_contract(
        self,
        issuer_entity: Entity,
        contract_id: str
    ) -> bool:
        """
        Cancel a contract (only if not accepted)
        
        Args:
            issuer_entity: Entity cancelling (must be issuer)
            contract_id: Contract ID to cancel
            
        Returns:
            True if cancelled successfully
        """
        from engine.components.game_components import Inventory, Contract as ContractComp
        
        contract = self.contracts.get(contract_id)
        if not contract:
            return False
        
        # Check if issuer
        if contract.issuer_id != issuer_entity.id:
            return False
        
        # Can only cancel outstanding contracts
        if contract.status != ContractStatus.OUTSTANDING:
            return False
        
        # Return items if item exchange contract
        if isinstance(contract, ItemExchangeContract):
            inventory_comp = issuer_entity.get_component(Inventory)
            if inventory_comp:
                for item_id, quantity in contract.items_offered.items():
                    inventory_comp.items[item_id] = inventory_comp.items.get(item_id, 0) + quantity
        
        # Update contract status
        contract.status = ContractStatus.CANCELLED
        
        # Update contract component
        contract_comp = issuer_entity.get_component(ContractComp)
        if contract_comp and contract_id in contract_comp.active_contracts:
            contract_comp.active_contracts.remove(contract_id)
        
        return True
