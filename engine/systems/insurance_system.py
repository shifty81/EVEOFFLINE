"""
Ship Insurance System
Handles ship insurance policies, payouts on ship destruction, and insurance management.
Based on EVE Online's insurance mechanic where players can insure ship hulls
to recover partial ISK when their ship is destroyed.
"""

from typing import Optional, Dict, List
from dataclasses import dataclass, field
from enum import Enum
import time


class InsuranceLevel(Enum):
    """Insurance tiers matching EVE Online's system"""
    NONE = "none"           # Free default insurance
    BASIC = "basic"
    STANDARD = "standard"
    BRONZE = "bronze"
    SILVER = "silver"
    GOLD = "gold"
    PLATINUM = "platinum"


# Payout percentage and cost percentage of base hull value per tier
INSURANCE_TIERS = {
    InsuranceLevel.NONE:     {"payout_pct": 0.40, "cost_pct": 0.00},
    InsuranceLevel.BASIC:    {"payout_pct": 0.50, "cost_pct": 0.05},
    InsuranceLevel.STANDARD: {"payout_pct": 0.60, "cost_pct": 0.10},
    InsuranceLevel.BRONZE:   {"payout_pct": 0.70, "cost_pct": 0.18},
    InsuranceLevel.SILVER:   {"payout_pct": 0.80, "cost_pct": 0.25},
    InsuranceLevel.GOLD:     {"payout_pct": 0.90, "cost_pct": 0.35},
    InsuranceLevel.PLATINUM: {"payout_pct": 1.00, "cost_pct": 0.50},
}

# Insurance duration in seconds (12 weeks = 7,257,600 seconds)
INSURANCE_DURATION = 12 * 7 * 24 * 3600  # 12 weeks


@dataclass
class InsurancePolicy:
    """An active insurance policy on a ship"""
    policy_id: str
    ship_entity_id: str
    ship_type: str
    owner_id: str
    level: InsuranceLevel
    base_hull_value: float      # The base value of the ship hull in ISK
    payout_amount: float        # ISK paid out on destruction
    premium_paid: float         # ISK paid for the policy
    issued_time: float          # Timestamp when policy was purchased
    expiry_time: float          # Timestamp when policy expires
    is_active: bool = True
    is_void: bool = False       # Voided if ship is repackaged/traded


@dataclass
class InsuranceRecord:
    """Historical record of an insurance claim"""
    record_id: str
    policy_id: str
    ship_type: str
    owner_id: str
    payout_amount: float
    claim_time: float
    cause: str = "destruction"  # destruction, self_destruct


# Base hull values for ship classes (ISK)
# These represent the material cost to build the hull
BASE_HULL_VALUES = {
    # Frigates
    "Frigate": 500000,
    # Destroyers
    "Destroyer": 1200000,
    # Cruisers
    "Cruiser": 10000000,
    # Battlecruisers
    "Battlecruiser": 35000000,
    # Battleships
    "Battleship": 85000000,
    # Mining Barges
    "Mining Barge": 25000000,
    # Exhumers
    "Exhumer": 150000000,
    # Industrial
    "Industrial": 5000000,
}

# Specific ship hull values override class defaults
SHIP_HULL_VALUES = {
    # Frigates
    "rifter": 450000,
    "punisher": 500000,
    "merlin": 480000,
    "tristan": 520000,
    "slasher": 400000,
    "executioner": 470000,
    "kestrel": 430000,
    "incursus": 490000,
    "breacher": 420000,
    "tormentor": 460000,
    "condor": 380000,
    "atron": 410000,
    # Destroyers
    "thrasher": 1100000,
    "coercer": 1200000,
    "cormorant": 1150000,
    "catalyst": 1250000,
    "talwar": 1300000,
    "dragoon": 1350000,
    "corax": 1280000,
    "algos": 1400000,
    # Cruisers
    "stabber": 9000000,
    "maller": 10000000,
    "caracal": 9500000,
    "vexor": 10500000,
    "rupture": 9800000,
    "omen": 10200000,
    "moa": 9300000,
    "thorax": 10800000,
    # Battlecruisers
    "hurricane": 33000000,
    "harbinger": 35000000,
    "drake": 34000000,
    "myrmidon": 36000000,
    "cyclone": 32000000,
    "prophecy": 37000000,
    "ferox": 33500000,
    "brutix": 38000000,
    # Battleships
    "tempest": 80000000,
    "apocalypse": 85000000,
    "raven": 82000000,
    "megathron": 88000000,
    "maelstrom": 83000000,
    "abaddon": 90000000,
    "rokh": 84000000,
    "hyperion": 92000000,
    # Mining
    "procurer": 22000000,
    "retriever": 24000000,
    "covetor": 28000000,
    "skiff": 180000000,
    "mackinaw": 200000000,
    "hulk": 250000000,
}


class InsuranceSystem:
    """
    Manages ship insurance policies and claims.
    Based on EVE Online's insurance system.
    """

    def __init__(self):
        # Active policies: {policy_id: InsurancePolicy}
        self.policies: Dict[str, InsurancePolicy] = {}
        # Ship to policy mapping: {ship_entity_id: policy_id}
        self.ship_policies: Dict[str, str] = {}
        # Claim history: {owner_id: [InsuranceRecord]}
        self.claim_history: Dict[str, List[InsuranceRecord]] = {}
        # Policy counter for generating IDs
        self._policy_counter = 0
        self._record_counter = 0

    def get_hull_value(self, ship_type: str, ship_class: str = "Frigate") -> float:
        """
        Get the base hull value for insurance purposes.
        Uses specific ship values first, then falls back to class defaults.
        """
        ship_key = ship_type.lower().replace(" ", "_")
        if ship_key in SHIP_HULL_VALUES:
            return float(SHIP_HULL_VALUES[ship_key])
        return float(BASE_HULL_VALUES.get(ship_class, 500000))

    def get_insurance_quote(
        self,
        ship_type: str,
        ship_class: str = "Frigate",
        level: InsuranceLevel = InsuranceLevel.PLATINUM
    ) -> Dict[str, float]:
        """
        Get an insurance quote for a ship.

        Returns:
            Dict with 'premium' (cost) and 'payout' (amount on destruction)
        """
        hull_value = self.get_hull_value(ship_type, ship_class)
        tier = INSURANCE_TIERS[level]

        premium = hull_value * tier["cost_pct"]
        payout = hull_value * tier["payout_pct"]

        return {
            "premium": premium,
            "payout": payout,
            "hull_value": hull_value,
            "level": level.value,
        }

    def purchase_insurance(
        self,
        ship_entity_id: str,
        ship_type: str,
        ship_class: str,
        owner_id: str,
        level: InsuranceLevel,
        current_time: float,
        wallet_isk: float
    ) -> Optional[InsurancePolicy]:
        """
        Purchase an insurance policy for a ship.

        Args:
            ship_entity_id: The entity ID of the ship
            ship_type: Ship type name (e.g., 'rifter')
            ship_class: Ship class (e.g., 'Frigate')
            owner_id: Owner player ID
            level: Insurance tier to purchase
            current_time: Current game timestamp
            wallet_isk: Current ISK in player's wallet

        Returns:
            InsurancePolicy if successful, None if failed
        """
        # Check if ship already has active insurance
        if ship_entity_id in self.ship_policies:
            existing_policy_id = self.ship_policies[ship_entity_id]
            existing = self.policies.get(existing_policy_id)
            if existing and existing.is_active and not existing.is_void:
                # Cancel existing policy (no refund, like EVE)
                existing.is_active = False

        quote = self.get_insurance_quote(ship_type, ship_class, level)
        premium = quote["premium"]

        # Free default insurance doesn't need ISK check
        if level != InsuranceLevel.NONE and wallet_isk < premium:
            return None

        self._policy_counter += 1
        policy_id = f"ins_{self._policy_counter}"

        policy = InsurancePolicy(
            policy_id=policy_id,
            ship_entity_id=ship_entity_id,
            ship_type=ship_type,
            owner_id=owner_id,
            level=level,
            base_hull_value=quote["hull_value"],
            payout_amount=quote["payout"],
            premium_paid=premium,
            issued_time=current_time,
            expiry_time=current_time + INSURANCE_DURATION,
        )

        self.policies[policy_id] = policy
        self.ship_policies[ship_entity_id] = policy_id

        return policy

    def claim_insurance(
        self,
        ship_entity_id: str,
        current_time: float,
        cause: str = "destruction"
    ) -> Optional[InsuranceRecord]:
        """
        File an insurance claim when a ship is destroyed.

        Args:
            ship_entity_id: The entity ID of the destroyed ship
            current_time: Current game timestamp
            cause: Cause of destruction

        Returns:
            InsuranceRecord with payout info, or None if no valid policy
        """
        policy_id = self.ship_policies.get(ship_entity_id)
        if not policy_id:
            # No policy - provide default 40% payout
            return None

        policy = self.policies.get(policy_id)
        if not policy or not policy.is_active or policy.is_void:
            return None

        # Check expiry
        if current_time > policy.expiry_time:
            policy.is_active = False
            return None

        # Process the claim
        self._record_counter += 1
        record = InsuranceRecord(
            record_id=f"claim_{self._record_counter}",
            policy_id=policy_id,
            ship_type=policy.ship_type,
            owner_id=policy.owner_id,
            payout_amount=policy.payout_amount,
            claim_time=current_time,
            cause=cause,
        )

        # Deactivate the policy
        policy.is_active = False

        # Remove ship mapping
        del self.ship_policies[ship_entity_id]

        # Add to history
        if policy.owner_id not in self.claim_history:
            self.claim_history[policy.owner_id] = []
        self.claim_history[policy.owner_id].append(record)

        return record

    def void_policy(self, ship_entity_id: str) -> bool:
        """
        Void an insurance policy (e.g., when ship is repackaged, traded, or contracted).

        Returns:
            True if a policy was voided
        """
        policy_id = self.ship_policies.get(ship_entity_id)
        if not policy_id:
            return False

        policy = self.policies.get(policy_id)
        if not policy:
            return False

        policy.is_void = True
        policy.is_active = False
        del self.ship_policies[ship_entity_id]

        return True

    def get_active_policy(self, ship_entity_id: str) -> Optional[InsurancePolicy]:
        """Get the active insurance policy for a ship"""
        policy_id = self.ship_policies.get(ship_entity_id)
        if not policy_id:
            return None

        policy = self.policies.get(policy_id)
        if policy and policy.is_active and not policy.is_void:
            return policy
        return None

    def get_claim_history(self, owner_id: str) -> List[InsuranceRecord]:
        """Get all insurance claim records for a player"""
        return self.claim_history.get(owner_id, [])

    def update(self, current_time: float):
        """Update insurance system - expire old policies"""
        expired_ships = []
        for ship_id, policy_id in self.ship_policies.items():
            policy = self.policies.get(policy_id)
            if policy and current_time > policy.expiry_time:
                policy.is_active = False
                expired_ships.append(ship_id)

        for ship_id in expired_ships:
            del self.ship_policies[ship_id]
