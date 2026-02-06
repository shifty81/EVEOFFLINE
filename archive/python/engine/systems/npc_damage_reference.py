"""
NPC Damage Type Reference
Provides a lookup table for optimal damage types to deal against each NPC faction,
and what damage types each faction deals to players.
Based on EVE Online's NPC faction damage profiles.

This is a critical reference for PVE gameplay - knowing what to shoot and what to tank.
"""

from typing import Dict, List, Tuple
from dataclasses import dataclass


@dataclass
class FactionDamageProfile:
    """Damage profile for an NPC faction"""
    faction_id: str
    faction_name: str
    # What damage the NPCs deal to players (what to tank against)
    deals_primary: str      # Primary damage type dealt
    deals_secondary: str    # Secondary damage type dealt
    # What damage is most effective against the NPCs (what to shoot)
    weak_to_primary: str    # Primary resist hole
    weak_to_secondary: str  # Secondary resist hole
    # EWAR used by this faction
    ewar_types: List[str]
    # Regions where this faction is commonly found
    home_regions: List[str]
    # Notes about this faction
    notes: str = ""


# Complete NPC faction damage reference table
# Based on EVE Online Wiki data
NPC_FACTION_PROFILES: Dict[str, FactionDamageProfile] = {
    "serpentis": FactionDamageProfile(
        faction_id="serpentis",
        faction_name="Serpentis Corporation",
        deals_primary="thermal",
        deals_secondary="kinetic",
        weak_to_primary="kinetic",
        weak_to_secondary="thermal",
        ewar_types=["sensor_dampener"],
        home_regions=["Fountain", "Serpentis Prime"],
        notes="Use hybrid turrets. Sensor dampeners reduce targeting range.",
    ),
    "guristas": FactionDamageProfile(
        faction_id="guristas",
        faction_name="Guristas Pirates",
        deals_primary="kinetic",
        deals_secondary="thermal",
        weak_to_primary="kinetic",
        weak_to_secondary="thermal",
        ewar_types=["ecm"],
        home_regions=["Venal", "Tenal"],
        notes="Use missiles and hybrid turrets. ECM can break target locks.",
    ),
    "blood_raiders": FactionDamageProfile(
        faction_id="blood_raiders",
        faction_name="Blood Raiders",
        deals_primary="em",
        deals_secondary="thermal",
        weak_to_primary="em",
        weak_to_secondary="thermal",
        ewar_types=["energy_neutralizer", "tracking_disruptor"],
        home_regions=["Delve", "Period Basis", "Querious"],
        notes="Use energy turrets. Neutralizers drain capacitor. Keep cap stable.",
    ),
    "sansha": FactionDamageProfile(
        faction_id="sansha",
        faction_name="Sansha's Nation",
        deals_primary="em",
        deals_secondary="thermal",
        weak_to_primary="em",
        weak_to_secondary="thermal",
        ewar_types=["tracking_disruptor", "warp_scrambler"],
        home_regions=["Stain", "Esoteria", "Paragon Soul"],
        notes="Use energy turrets. Warp scramblers prevent escape. Tracking disruptors reduce turret accuracy.",
    ),
    "angel_cartel": FactionDamageProfile(
        faction_id="angel_cartel",
        faction_name="Angel Cartel",
        deals_primary="explosive",
        deals_secondary="kinetic",
        weak_to_primary="explosive",
        weak_to_secondary="kinetic",
        ewar_types=["stasis_webifier", "target_painter"],
        home_regions=["Curse", "Impass", "Great Wildlands"],
        notes="Use projectile turrets. Fast ships. Webifiers slow you down.",
    ),
    "rogue_drones": FactionDamageProfile(
        faction_id="rogue_drones",
        faction_name="Rogue Drones",
        deals_primary="em",
        deals_secondary="thermal",
        weak_to_primary="em",
        weak_to_secondary="thermal",
        ewar_types=["stasis_webifier"],
        home_regions=["Drone Regions", "Etherium Reach", "Kalevala Expanse"],
        notes="Mixed damage output. Can spawn reinforcement drones. No bounties on some types.",
    ),
    "mordus_legion": FactionDamageProfile(
        faction_id="mordus_legion",
        faction_name="Mordu's Legion",
        deals_primary="kinetic",
        deals_secondary="thermal",
        weak_to_primary="kinetic",
        weak_to_secondary="thermal",
        ewar_types=["warp_disruptor"],
        home_regions=["Pure Blind"],
        notes="Missile-focused. Long-range point can prevent escape at range.",
    ),
    "mercenaries": FactionDamageProfile(
        faction_id="mercenaries",
        faction_name="Mercenaries",
        deals_primary="thermal",
        deals_secondary="kinetic",
        weak_to_primary="thermal",
        weak_to_secondary="kinetic",
        ewar_types=[],
        home_regions=["Various"],
        notes="Found in various mission spaces. Mixed damage.",
    ),
    "eom": FactionDamageProfile(
        faction_id="eom",
        faction_name="Equilibrium of Mankind",
        deals_primary="em",
        deals_secondary="thermal",
        weak_to_primary="em",
        weak_to_secondary="thermal",
        ewar_types=["energy_neutralizer"],
        home_regions=["Various"],
        notes="Rare faction, similar to Blood Raiders.",
    ),
}

# Quick lookup functions
def get_optimal_damage(faction_id: str) -> Tuple[str, str]:
    """
    Get the optimal damage types to use against an NPC faction.

    Returns:
        Tuple of (primary_damage_type, secondary_damage_type)
    """
    profile = NPC_FACTION_PROFILES.get(faction_id.lower())
    if profile:
        return (profile.weak_to_primary, profile.weak_to_secondary)
    return ("kinetic", "thermal")  # Default safe choice


def get_tank_against(faction_id: str) -> Tuple[str, str]:
    """
    Get the damage types an NPC faction deals (what you should tank against).

    Returns:
        Tuple of (primary_damage_type, secondary_damage_type)
    """
    profile = NPC_FACTION_PROFILES.get(faction_id.lower())
    if profile:
        return (profile.deals_primary, profile.deals_secondary)
    return ("kinetic", "thermal")  # Default safe choice


def get_faction_ewar(faction_id: str) -> List[str]:
    """Get the EWAR types used by a faction."""
    profile = NPC_FACTION_PROFILES.get(faction_id.lower())
    if profile:
        return profile.ewar_types
    return []


def get_all_factions() -> List[str]:
    """Get list of all known NPC factions."""
    return list(NPC_FACTION_PROFILES.keys())


def get_faction_profile(faction_id: str) -> FactionDamageProfile:
    """Get the full damage profile for a faction."""
    return NPC_FACTION_PROFILES.get(faction_id.lower())


# Compact damage cheat sheet for UI display
DAMAGE_CHEAT_SHEET = {
    "Serpentis":       {"deal": "Kin/Therm", "tank": "Therm/Kin", "ewar": "Damp"},
    "Guristas":        {"deal": "Kin/Therm", "tank": "Kin/Therm", "ewar": "ECM"},
    "Blood Raiders":   {"deal": "EM/Therm",  "tank": "EM/Therm",  "ewar": "Neut/TD"},
    "Sansha's Nation": {"deal": "EM/Therm",  "tank": "EM/Therm",  "ewar": "TD/Scram"},
    "Angel Cartel":    {"deal": "Exp/Kin",   "tank": "Exp/Kin",   "ewar": "Web/TP"},
    "Rogue Drones":    {"deal": "EM/Therm",  "tank": "EM/Therm",  "ewar": "Web"},
    "Mordu's Legion":  {"deal": "Kin/Therm", "tank": "Kin/Therm", "ewar": "Point"},
}
