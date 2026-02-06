"""
NPC Bounty System
Handles bounty rewards when NPCs are destroyed.
In EVE Online, NPC pirate ships have bounties that are paid instantly
to the player (or split among fleet members) upon destruction.
"""

from typing import Optional, Dict, List, Tuple
from dataclasses import dataclass, field


@dataclass
class BountyPayment:
    """Record of a bounty payment"""
    payment_id: str
    npc_type: str
    npc_faction: str
    bounty_amount: float
    recipient_id: str
    fleet_split: bool = False
    fleet_members: int = 1
    individual_share: float = 0.0
    timestamp: float = 0.0


# NPC bounty values by type and size (ISK)
# Based on EVE Online bounty amounts
NPC_BOUNTIES = {
    # Serpentis
    "serpentis_scout": 15000,
    "serpentis_frigate": 20000,
    "serpentis_destroyer": 45000,
    "serpentis_cruiser": 150000,
    "serpentis_battlecruiser": 450000,
    "serpentis_battleship": 1250000,
    "serpentis_commander": 2500000,
    # Guristas
    "guristas_scout": 18000,
    "guristas_frigate": 22000,
    "guristas_destroyer": 50000,
    "guristas_cruiser": 175000,
    "guristas_battlecruiser": 500000,
    "guristas_battleship": 1400000,
    "guristas_commander": 2800000,
    # Blood Raiders
    "blood_raider_scout": 16000,
    "blood_raider_frigate": 21000,
    "blood_raider_destroyer": 48000,
    "blood_raider_cruiser": 160000,
    "blood_raider_battlecruiser": 480000,
    "blood_raider_battleship": 1350000,
    "blood_raider_commander": 2700000,
    # Sansha's Nation
    "sansha_scout": 17000,
    "sansha_frigate": 23000,
    "sansha_destroyer": 52000,
    "sansha_cruiser": 180000,
    "sansha_battlecruiser": 520000,
    "sansha_battleship": 1500000,
    "sansha_commander": 3000000,
    # Angel Cartel
    "angel_scout": 14000,
    "angel_frigate": 19000,
    "angel_destroyer": 42000,
    "angel_cruiser": 140000,
    "angel_battlecruiser": 420000,
    "angel_battleship": 1200000,
    "angel_commander": 2400000,
    # Rogue Drones
    "rogue_drone_small": 8000,
    "rogue_drone_frigate": 12000,
    "rogue_drone_medium": 25000,
    "rogue_drone_cruiser": 100000,
    "rogue_drone_large": 300000,
    "rogue_drone_battleship": 900000,
    "rogue_drone_queen": 5000000,
    # Generic pirate (fallback)
    "pirate_frigate": 15000,
    "pirate_cruiser": 125000,
    "pirate_battleship": 1000000,
}

# Security status multiplier for bounties
# In EVE, lower security space has higher bounty multiplier
SECURITY_BOUNTY_MULTIPLIER = {
    1.0: 1.0,
    0.9: 1.0,
    0.8: 1.0,
    0.7: 1.05,
    0.6: 1.1,
    0.5: 1.15,
    0.4: 1.2,
    0.3: 1.3,
    0.2: 1.4,
    0.1: 1.5,
    0.0: 1.6,
}


class BountySystem:
    """
    Manages NPC bounty rewards.
    When NPCs are destroyed, the killer receives ISK bounty payment.
    In fleet, bounty is split evenly among members on grid.
    """

    def __init__(self):
        # Payment history: {player_id: [BountyPayment]}
        self.payment_history: Dict[str, List[BountyPayment]] = {}
        # Total bounties earned: {player_id: total_isk}
        self.total_earned: Dict[str, float] = {}
        # Payment counter
        self._payment_counter = 0

    def get_bounty_value(
        self,
        npc_type: str,
        security_status: float = 1.0
    ) -> float:
        """
        Get the bounty value for an NPC type.

        Args:
            npc_type: The NPC type identifier
            security_status: System security status (0.0 to 1.0)

        Returns:
            Bounty amount in ISK
        """
        base_bounty = NPC_BOUNTIES.get(npc_type, 0)
        if base_bounty == 0:
            return 0.0

        # Apply security multiplier
        rounded_sec = round(security_status * 10) / 10
        multiplier = SECURITY_BOUNTY_MULTIPLIER.get(rounded_sec, 1.0)

        return base_bounty * multiplier

    def process_npc_kill(
        self,
        npc_type: str,
        npc_faction: str,
        killer_id: str,
        fleet_member_ids: Optional[List[str]] = None,
        security_status: float = 1.0,
        current_time: float = 0.0
    ) -> List[BountyPayment]:
        """
        Process an NPC kill and distribute bounty payments.

        Args:
            npc_type: Type of NPC killed
            npc_faction: Faction of the NPC
            killer_id: ID of the player who got the kill
            fleet_member_ids: Optional list of fleet member IDs on grid
            security_status: Current system security status
            current_time: Current game timestamp

        Returns:
            List of BountyPayment records (one per recipient)
        """
        total_bounty = self.get_bounty_value(npc_type, security_status)
        if total_bounty <= 0:
            return []

        payments = []

        if fleet_member_ids and len(fleet_member_ids) > 1:
            # Fleet split - divide evenly among members on grid
            share = total_bounty / len(fleet_member_ids)
            for member_id in fleet_member_ids:
                self._payment_counter += 1
                payment = BountyPayment(
                    payment_id=f"bounty_{self._payment_counter}",
                    npc_type=npc_type,
                    npc_faction=npc_faction,
                    bounty_amount=total_bounty,
                    recipient_id=member_id,
                    fleet_split=True,
                    fleet_members=len(fleet_member_ids),
                    individual_share=share,
                    timestamp=current_time,
                )
                payments.append(payment)
                self._record_payment(member_id, payment)
        else:
            # Solo kill - full bounty to killer
            self._payment_counter += 1
            payment = BountyPayment(
                payment_id=f"bounty_{self._payment_counter}",
                npc_type=npc_type,
                npc_faction=npc_faction,
                bounty_amount=total_bounty,
                recipient_id=killer_id,
                fleet_split=False,
                fleet_members=1,
                individual_share=total_bounty,
                timestamp=current_time,
            )
            payments.append(payment)
            self._record_payment(killer_id, payment)

        return payments

    def _record_payment(self, player_id: str, payment: BountyPayment):
        """Record a bounty payment in history"""
        if player_id not in self.payment_history:
            self.payment_history[player_id] = []
        self.payment_history[player_id].append(payment)

        if player_id not in self.total_earned:
            self.total_earned[player_id] = 0.0
        self.total_earned[player_id] += payment.individual_share

    def get_payment_history(
        self,
        player_id: str,
        limit: int = 50
    ) -> List[BountyPayment]:
        """Get recent bounty payments for a player"""
        history = self.payment_history.get(player_id, [])
        return history[-limit:]

    def get_total_earned(self, player_id: str) -> float:
        """Get total ISK earned from bounties"""
        return self.total_earned.get(player_id, 0.0)

    def get_session_earnings(
        self,
        player_id: str,
        session_start_time: float
    ) -> float:
        """Get ISK earned from bounties since session start"""
        history = self.payment_history.get(player_id, [])
        return sum(
            p.individual_share
            for p in history
            if p.timestamp >= session_start_time
        )
