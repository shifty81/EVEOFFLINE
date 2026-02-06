"""
Random Name Generator
Generates EVE-like names for various game elements
"""

import random
from typing import List, Optional


class NameGenerator:
    """
    Generates random names for ships, characters, systems, etc.
    Similar to EVE Online but with unique naming conventions
    """
    
    # Character name components (EVE-style)
    FIRST_NAMES_MALE = [
        "Marcus", "Drake", "Orion", "Zephyr", "Atlas", "Phoenix", "Kaiden", "Ryker",
        "Sirius", "Talon", "Vex", "Nero", "Axel", "Cyrus", "Magnus", "Raven",
        "Kane", "Jax", "Kael", "Zane", "Darius", "Thorne", "Blaze", "Storm"
    ]
    
    FIRST_NAMES_FEMALE = [
        "Nova", "Aria", "Luna", "Lyra", "Aurora", "Vega", "Stellar", "Cassandra",
        "Seraph", "Echo", "Nyx", "Astrid", "Celeste", "Electra", "Iris", "Stella",
        "Zara", "Kira", "Sable", "Rogue", "Tempest", "Vesper", "Ember", "Skye"
    ]
    
    LAST_NAMES = [
        "Stormbreaker", "Darkstar", "Voidwalker", "Starfire", "Ironheart", "Swiftblade",
        "Shadowborn", "Lightbringer", "Frostwind", "Thunderstrike", "Ashborn", "Steelwind",
        "Moonshadow", "Sunforge", "Nightfall", "Dawnbringer", "Voidseeker", "Starborn",
        "Skyhammer", "Stormchaser", "Blacksun", "Silvermoon", "Redshard", "Blueflame"
    ]
    
    # Ship name prefixes and suffixes (EVE-style)
    SHIP_PREFIXES = [
        "INS", "USS", "RSS", "CSS", "GSS", "ASS", "HSS", "ESS"  # Imperial, United, Republic, etc.
    ]
    
    SHIP_NAMES_HEROIC = [
        "Valor", "Defiance", "Vengeance", "Resolute", "Indomitable", "Relentless",
        "Invincible", "Unconquered", "Dauntless", "Intrepid", "Fearless", "Vigilant"
    ]
    
    SHIP_NAMES_CELESTIAL = [
        "Andromeda", "Orion", "Cassiopeia", "Polaris", "Sirius", "Vega",
        "Arcturus", "Betelgeuse", "Rigel", "Aldebaran", "Antares", "Altair"
    ]
    
    SHIP_NAMES_MYTHIC = [
        "Hyperion", "Kronos", "Atlas", "Prometheus", "Heracles", "Perseus",
        "Achilles", "Odysseus", "Aegis", "Nemesis", "Phoenix", "Titan"
    ]
    
    SHIP_NAMES_DESCRIPTIVE = [
        "Thunderbolt", "Stormfront", "Wildfire", "Avalanche", "Hurricane", "Typhoon",
        "Cyclone", "Tempest", "Maelstrom", "Cataclysm", "Eclipse", "Nebula"
    ]
    
    # Corporation names
    CORP_PREFIXES = [
        "Stellar", "Galactic", "Cosmic", "Void", "Star", "Nova", "Quantum",
        "Nebula", "Celestial", "Interstellar", "Deep Space", "Dark Matter"
    ]
    
    CORP_TYPES = [
        "Industries", "Corporation", "Enterprises", "Consortium", "Alliance",
        "Holdings", "Syndicate", "Collective", "Federation", "Conglomerate",
        "Trading Company", "Logistics", "Security", "Technologies"
    ]
    
    # System name components
    SYSTEM_PREFIXES = [
        "Alpha", "Beta", "Gamma", "Delta", "Epsilon", "Zeta", "Theta", "Omega",
        "Nova", "Stellar", "Void", "Dark", "Deep", "High", "Low", "Outer"
    ]
    
    SYSTEM_CORES = [
        "Centauri", "Crucis", "Draconis", "Aquilae", "Cygni", "Aurigae",
        "Persei", "Orionis", "Tauri", "Virginis", "Leonis", "Scorpii"
    ]
    
    SYSTEM_SUFFIXES = [
        "Prime", "Secundus", "Tertius", "Major", "Minor", "Proxima",
        "Alpha", "Beta", "One", "Two", "Three", "Central"
    ]
    
    # Station names
    STATION_PREFIXES = [
        "Citadel", "Bastion", "Fortress", "Outpost", "Station", "Haven",
        "Sanctuary", "Terminal", "Hub", "Gateway", "Nexus", "Trade Post"
    ]
    
    STATION_DESCRIPTORS = [
        "Prime", "Central", "Trade", "Military", "Research", "Mining",
        "Industrial", "Commercial", "Federal", "Imperial", "Orbital", "Deep Space"
    ]
    
    # Asteroid names
    ASTEROID_TYPES = [
        "Veldspar", "Scordite", "Pyroxeres", "Plagioclase", "Omber", "Kernite",
        "Jaspet", "Hemorphite", "Hedbergite", "Gneiss", "Dark Ochre", "Crokite",
        "Bistot", "Arkonor", "Mercoxit", "Spodumain"
    ]
    
    # Mission names
    MISSION_ADJECTIVES = [
        "Deadly", "Silent", "Swift", "Hidden", "Ancient", "Forgotten", "Lost",
        "Desperate", "Critical", "Urgent", "Secret", "Classified", "Dangerous"
    ]
    
    MISSION_NOUNS = [
        "Convoy", "Shipment", "Threat", "Menace", "Artifact", "Data", "Cargo",
        "Signal", "Transmission", "Asset", "Target", "Objective", "Operation"
    ]
    
    @classmethod
    def generate_character_name(cls, gender: Optional[str] = None) -> str:
        """
        Generate a random character name
        
        Args:
            gender: 'male', 'female', or None for random
            
        Returns:
            Full character name (First Last)
        """
        if gender == 'male' or (gender is None and random.random() < 0.5):
            first = random.choice(cls.FIRST_NAMES_MALE)
        else:
            first = random.choice(cls.FIRST_NAMES_FEMALE)
        
        last = random.choice(cls.LAST_NAMES)
        return f"{first} {last}"
    
    @classmethod
    def generate_ship_name(cls, style: Optional[str] = None) -> str:
        """
        Generate a random ship name
        
        Args:
            style: 'heroic', 'celestial', 'mythic', 'descriptive', or None for random
            
        Returns:
            Ship name (e.g., "INS Thunderbolt")
        """
        prefix = random.choice(cls.SHIP_PREFIXES)
        
        if style == 'heroic':
            name = random.choice(cls.SHIP_NAMES_HEROIC)
        elif style == 'celestial':
            name = random.choice(cls.SHIP_NAMES_CELESTIAL)
        elif style == 'mythic':
            name = random.choice(cls.SHIP_NAMES_MYTHIC)
        elif style == 'descriptive':
            name = random.choice(cls.SHIP_NAMES_DESCRIPTIVE)
        else:
            # Random from all categories
            all_names = (cls.SHIP_NAMES_HEROIC + cls.SHIP_NAMES_CELESTIAL + 
                        cls.SHIP_NAMES_MYTHIC + cls.SHIP_NAMES_DESCRIPTIVE)
            name = random.choice(all_names)
        
        return f"{prefix} {name}"
    
    @classmethod
    def generate_corporation_name(cls) -> str:
        """
        Generate a random corporation name
        
        Returns:
            Corporation name (e.g., "Stellar Industries")
        """
        prefix = random.choice(cls.CORP_PREFIXES)
        corp_type = random.choice(cls.CORP_TYPES)
        return f"{prefix} {corp_type}"
    
    @classmethod
    def generate_system_name(cls) -> str:
        """
        Generate a random solar system name
        
        Returns:
            System name (e.g., "Alpha Centauri Prime")
        """
        prefix = random.choice(cls.SYSTEM_PREFIXES)
        core = random.choice(cls.SYSTEM_CORES)
        
        # 50% chance to add suffix
        if random.random() < 0.5:
            suffix = random.choice(cls.SYSTEM_SUFFIXES)
            return f"{prefix} {core} {suffix}"
        else:
            return f"{prefix} {core}"
    
    @classmethod
    def generate_station_name(cls, system_name: Optional[str] = None) -> str:
        """
        Generate a random station name
        
        Args:
            system_name: Optional system name to incorporate
            
        Returns:
            Station name (e.g., "Trade Hub Prime")
        """
        prefix = random.choice(cls.STATION_PREFIXES)
        descriptor = random.choice(cls.STATION_DESCRIPTORS)
        
        if system_name and random.random() < 0.3:
            # Use system name
            return f"{system_name} {prefix}"
        else:
            return f"{descriptor} {prefix}"
    
    @classmethod
    def generate_asteroid_designation(cls) -> str:
        """
        Generate a random asteroid designation
        
        Returns:
            Asteroid name (e.g., "Veldspar-4472")
        """
        ore_type = random.choice(cls.ASTEROID_TYPES)
        number = random.randint(1000, 9999)
        return f"{ore_type}-{number}"
    
    @classmethod
    def generate_mission_name(cls) -> str:
        """
        Generate a random mission name
        
        Returns:
            Mission name (e.g., "Deadly Convoy")
        """
        adjective = random.choice(cls.MISSION_ADJECTIVES)
        noun = random.choice(cls.MISSION_NOUNS)
        return f"{adjective} {noun}"
    
    @classmethod
    def generate_pilot_callsign(cls) -> str:
        """
        Generate a random pilot callsign (short nickname)
        
        Returns:
            Callsign (e.g., "Viper-7")
        """
        callsigns = [
            "Viper", "Hawk", "Falcon", "Eagle", "Phoenix", "Dragon",
            "Ghost", "Shadow", "Reaper", "Rogue", "Blade", "Storm",
            "Thunder", "Lightning", "Frost", "Flame", "Steel", "Iron"
        ]
        
        callsign = random.choice(callsigns)
        number = random.randint(1, 99)
        return f"{callsign}-{number}"
    
    @classmethod
    def generate_pirate_name(cls, faction: Optional[str] = None) -> str:
        """
        Generate a random pirate/NPC name
        
        Args:
            faction: Pirate faction (affects naming style)
            
        Returns:
            Pirate name
        """
        pirate_prefixes = ["Captain", "Commander", "Warlord", "Raider", "Corsair"]
        pirate_names = [
            "Blackheart", "Ironjaw", "Bloodfang", "Darkblade", "Redscar",
            "Voidreaver", "Starcrusher", "Skullbreaker", "Doomhammer", "Deathbringer"
        ]
        
        prefix = random.choice(pirate_prefixes)
        name = random.choice(pirate_names)
        return f"{prefix} {name}"
    
    @classmethod
    def generate_exploration_site_name(cls) -> str:
        """
        Generate a random exploration site name
        
        Returns:
            Site name (e.g., "Ancient Hideout")
        """
        adjectives = [
            "Ancient", "Forgotten", "Hidden", "Lost", "Abandoned", "Ruined",
            "Derelict", "Mysterious", "Secret", "Dangerous", "Unstable"
        ]
        
        sites = [
            "Hideout", "Base", "Outpost", "Installation", "Complex", "Structure",
            "Facility", "Station", "Colony", "Settlement", "Ruins", "Wreckage"
        ]
        
        adjective = random.choice(adjectives)
        site = random.choice(sites)
        return f"{adjective} {site}"


# Convenience functions
def random_character_name(gender: Optional[str] = None) -> str:
    """Generate random character name"""
    return NameGenerator.generate_character_name(gender)


def random_ship_name(style: Optional[str] = None) -> str:
    """Generate random ship name"""
    return NameGenerator.generate_ship_name(style)


def random_corporation_name() -> str:
    """Generate random corporation name"""
    return NameGenerator.generate_corporation_name()


def random_system_name() -> str:
    """Generate random system name"""
    return NameGenerator.generate_system_name()


def random_station_name(system_name: Optional[str] = None) -> str:
    """Generate random station name"""
    return NameGenerator.generate_station_name(system_name)


def random_mission_name() -> str:
    """Generate random mission name"""
    return NameGenerator.generate_mission_name()


if __name__ == "__main__":
    # Demo the name generator
    print("EVE OFFLINE Name Generator Demo")
    print("=" * 50)
    print()
    
    print("Character Names:")
    for _ in range(5):
        print(f"  {NameGenerator.generate_character_name()}")
    print()
    
    print("Ship Names:")
    for style in ['heroic', 'celestial', 'mythic', 'descriptive']:
        print(f"  {NameGenerator.generate_ship_name(style)}")
    print()
    
    print("Corporation Names:")
    for _ in range(3):
        print(f"  {NameGenerator.generate_corporation_name()}")
    print()
    
    print("System Names:")
    for _ in range(3):
        system = NameGenerator.generate_system_name()
        print(f"  {system}")
        print(f"    Station: {NameGenerator.generate_station_name(system)}")
    print()
    
    print("Mission Names:")
    for _ in range(5):
        print(f"  {NameGenerator.generate_mission_name()}")
    print()
    
    print("Exploration Sites:")
    for _ in range(3):
        print(f"  {NameGenerator.generate_exploration_site_name()}")
    print()
    
    print("Pilot Callsigns:")
    for _ in range(5):
        print(f"  {NameGenerator.generate_pilot_callsign()}")
