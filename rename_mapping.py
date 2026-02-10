#!/usr/bin/env python3
"""
EVEOFFLINE Content Renaming Script
===================================
Renames all EVE Online content references to original names for the EVEOFFLINE project.
Uses a cosmic/stellar naming theme with race-specific naming conventions:
  - Solari (was Amarr): Latin/celestial names
  - Veyren (was Caldari): Sharp/angular Nordic-inspired names
  - Aurelian (was Gallente): French/elegant names
  - Keldari (was Minmatar): Rugged/nature-inspired names
"""

import json
import os
import re
import sys
from pathlib import Path

# Default data directory (relative to script location, fallback to env-specific path)
_SCRIPT_DIR = Path(__file__).resolve().parent
_DEFAULT_DATA_DIR = _SCRIPT_DIR / "data"

# ---------------------------------------------------------------------------
# RACE MAPPINGS
# ---------------------------------------------------------------------------
RACES = {
    "Amarr": "Solari",
    "Caldari": "Veyren",
    "Gallente": "Aurelian",
    "Minmatar": "Keldari",
    "ORE": "ORE",  # ORE stays as-is (it's a consortium, not EVE-specific)
}

RACE_ADJECTIVES = {
    "Amarrian": "Solarian",
    "Amarr Empire": "Solari Dominion",
    "Caldari State": "Veyren Combine",
    "Gallente Federation": "Aurelian Federation",
    "Minmatar Republic": "Keldari Republic",
}

# Bloodlines
BLOODLINES = {
    "Amarrian": "Solarian",
    "Ni-Kunni": "Zah-Khari",
    "Khanid": "Vorthane",
    "Deteis": "Thyren",
    "Civire": "Korvane",
    "Achura": "Ashiri",
    "Gallentean": "Aurelian",
    "Intaki": "Indari",
    "Jin-Mei": "Jin-Sol",
    "Brutor": "Tormund",
    "Sebiestor": "Kelvor",
    "Vherokior": "Varoshi",
}

# ---------------------------------------------------------------------------
# PIRATE FACTIONS
# ---------------------------------------------------------------------------
PIRATE_FACTIONS = {
    "Serpentis": "Venom Syndicate",
    "Guristas": "Iron Corsairs",
    "Blood Raiders": "Crimson Order",
    "Blood Raider": "Crimson Order",
    "Angel Cartel": "Shadow Pact",
    "Sansha's Nation": "Hollow Collective",
    "Mordu's Legion": "Stormwatch Legion",
    "Sisters of EVE": "Astral Sisterhood",
    "Rogue Drones": "Rogue Constructs",
    "Rogue Drone": "Rogue Construct",
}

# ---------------------------------------------------------------------------
# SECURITY / ORGANIZATIONS
# ---------------------------------------------------------------------------
ORGANIZATIONS = {
    "CONCORD": "AEGIS",
    "New Eden": "Astralis",
    "EVE OFFLINE": "EVEOFFLINE",  # project name stays
}

# NPC Corporations
NPC_CORPORATIONS = {
    "Caldari Navy": "Veyren Fleet Command",
    "Caldari Provisions": "Veyren Provisions",
    "Amarr Navy": "Solari Imperial Fleet",
    "Ministry of War": "Ministry of Order",
    "Federation Navy": "Aurelian Grand Navy",
    "Republic Fleet": "Keldari Tribal Fleet",
    "Core Complexion": "Core Synthesis",
    "Creodron": "Novadron",
    "Carthum Conglomerate": "Solvane Industries",
    "Ishukone Corporation": "Thyrkane Corporation",
}

# ---------------------------------------------------------------------------
# ORE MAPPINGS
# ---------------------------------------------------------------------------
ORES = {
    "Veldspar": "Dustite",
    "Scordite": "Ferrite",
    "Pyroxeres": "Ignaite",
    "Plagioclase": "Crystite",
    "Omber": "Shadite",
    "Kernite": "Corite",
    "Jaspet": "Lumine",
    "Hemorphite": "Sangite",
    "Hedbergite": "Glacite",
    "Gneiss": "Densite",
    "Dark Ochre": "Voidite",
    "Crokite": "Pyranite",
    "Bistot": "Stellite",
    "Arkonor": "Cosmite",
    "Mercoxit": "Nexorite",
}

# ---------------------------------------------------------------------------
# MINERAL MAPPINGS
# ---------------------------------------------------------------------------
MINERALS = {
    "Tritanium": "Ferrium",
    "Pyerite": "Ignium",
    "Mexallon": "Allonium",
    "Isogen": "Isodium",
    "Nocxium": "Noctium",
    "Zydrine": "Zyrium",
    "Megacyte": "Megrium",
    "Morphite": "Morphium",
}

# ---------------------------------------------------------------------------
# SHIP MAPPINGS
# Complete mapping for every ship found in data/ships/*.json
# Naming conventions by race:
#   Solari (Amarr): Latin/celestial - dignified, imperial
#   Veyren (Caldari): Nordic/angular - sharp, precise
#   Aurelian (Gallente): French/elegant - graceful, refined
#   Keldari (Minmatar): Rugged/nature - raw, primal
# ---------------------------------------------------------------------------
SHIPS = {
    # === FRIGATES ===
    # Keldari (Minmatar)
    "Rifter": "Fang",
    # Veyren (Caldari)
    "Merlin": "Falk",
    # Aurelian (Gallente)
    "Tristan": "Revel",
    # Solari (Amarr)
    "Punisher": "Sentinel",

    # === DESTROYERS ===
    # Keldari (Minmatar)
    "Thrasher": "Cleaver",
    # Veyren (Caldari)
    "Cormorant": "Skarn",
    # Aurelian (Gallente)
    "Catalyst": "Vaillant",
    # Solari (Amarr)
    "Coercer": "Adjudicator",

    # === CRUISERS ===
    # Keldari (Minmatar)
    "Stabber": "Strider",
    "Rupture": "Ravine",
    # Veyren (Caldari)
    "Caracal": "Brynjar",
    "Moa": "Eidar",
    # Aurelian (Gallente)
    "Vexor": "Chevalier",
    # Solari (Amarr)
    "Maller": "Bastion",

    # === BATTLECRUISERS ===
    # Keldari (Minmatar)
    "Cyclone": "Galeforce",
    # Veyren (Caldari)
    "Ferox": "Fenvar",
    # Aurelian (Gallente)
    "Brutix": "Marquis",
    # Solari (Amarr)
    "Harbinger": "Herald",

    # === BATTLESHIPS ===
    # Keldari (Minmatar)
    "Tempest": "Gale",
    # Veyren (Caldari)
    "Raven": "Strix",
    # Aurelian (Gallente)
    "Dominix": "Sovereign",
    # Solari (Amarr)
    "Apocalypse": "Exarch",

    # === CARRIERS ===
    # Solari (Amarr)
    "Archon": "Solarius",
    # Aurelian (Gallente)
    "Thanatos": "Lumiere",
    # Veyren (Caldari)
    "Chimera": "Draknar",
    # Keldari (Minmatar)
    "Nidhoggur": "Ironprow",

    # === DREADNOUGHTS ===
    # Solari (Amarr)
    "Revelation": "Sanctum",
    # Aurelian (Gallente)
    "Moros": "Bastion Royal",
    # Veyren (Caldari)
    "Phoenix": "Valkyr",
    # Keldari (Minmatar)
    "Naglfar": "Thornwall",

    # === TITANS ===
    # Solari (Amarr)
    "Avatar": "Empyrean",
    # Aurelian (Gallente)
    "Erebus": "Grandeur",
    # Veyren (Caldari)
    "Leviathan": "Jormundur",
    # Keldari (Minmatar)
    "Ragnarok": "Worldbreaker",

    # === INDUSTRIALS ===
    # Keldari (Minmatar)
    "Hoarder": "Packrunner",
    # Veyren (Caldari)
    "Badger": "Drifthauler",
    # Aurelian (Gallente)
    "Iteron Mark V": "Marchand V",
    # Solari (Amarr)
    "Bestower": "Pilgrim Hauler",

    # === MINING BARGES ===
    "Procurer": "Ironbore",
    "Retriever": "Deepscoop",
    "Covetor": "Yieldmaster",

    # === EXHUMERS ===
    "Skiff": "Coreshield",
    "Mackinaw": "Vasthold",
    "Hulk": "Excavon",

    # === T2 FRIGATES - ASSAULT FRIGATES ===
    # Keldari (Minmatar)
    "Jaguar": "Lynx",
    "Wolf": "Direwolf",
    # Veyren (Caldari)
    "Hawk": "Gyrhawk",
    "Harpy": "Frostvane",
    # Aurelian (Gallente)
    "Enyo": "Duelliste",
    # Solari (Amarr)
    "Retribution": "Justicar",

    # === T2 FRIGATES - INTERCEPTORS ===
    # Keldari (Minmatar)
    "Claw": "Talon",
    "Stiletto": "Needlepoint",
    # Veyren (Caldari)
    "Crow": "Jaktfalk",
    "Raptor": "Svellhawk",
    # Aurelian (Gallente)
    "Taranis": "Eclair",
    "Ares": "Fleuret",
    # Solari (Amarr)
    "Crusader": "Templar",
    "Malediction": "Anathemist",

    # === T2 FRIGATES - COVERT OPS ===
    # Keldari (Minmatar)
    "Cheetah": "Duskrunner",
    # Veyren (Caldari)
    "Buzzard": "Spektor",
    # Aurelian (Gallente)
    "Helios": "Luminos",
    # Solari (Amarr)
    "Anathema": "Veiled Eye",

    # === T2 FRIGATES - STEALTH BOMBERS ===
    # Keldari (Minmatar)
    "Hound": "Shadowfang",
    # Veyren (Caldari)
    "Manticore": "Frostbane",
    # Aurelian (Gallente)
    "Nemesis": "Vengeresse",
    # Solari (Amarr)
    "Purifier": "Sanctifier",

    # === T2 DESTROYERS - INTERDICTORS ===
    # Keldari (Minmatar)
    "Sabre": "Gripshard",
    # Veyren (Caldari)
    "Flycatcher": "Nettvar",
    # Aurelian (Gallente)
    "Eris": "Barricade",
    # Solari (Amarr)
    "Heretic": "Denouncer",

    # === T2 CRUISERS - HEAVY ASSAULT CRUISERS ===
    # Keldari (Minmatar)
    "Vagabond": "Wanderer",
    "Muninn": "Gunnolf",
    # Veyren (Caldari)
    "Cerberus": "Hydralisk",
    "Eagle": "Valdris",
    # Aurelian (Gallente)
    "Ishtar": "Imperatrice",
    "Deimos": "Cavalier",
    # Solari (Amarr)
    "Zealot": "Ardent",
    "Sacrilege": "Inquisitor",

    # === T2 CRUISERS - HEAVY INTERDICTION CRUISERS ===
    # Keldari (Minmatar)
    "Broadsword": "Ironclamp",
    # Veyren (Caldari)
    "Onyx": "Frostlok",
    # Aurelian (Gallente)
    "Phobos": "Grappleur",
    # Solari (Amarr)
    "Devoter": "Warden",

    # === T2 CRUISERS - FORCE RECON SHIPS ===
    # Keldari (Minmatar)
    "Huginn": "Farseer",
    "Rapier": "Ghostblade",
    # Veyren (Caldari)
    "Falcon": "Skygaze",
    "Rook": "Watchkeep",
    # Aurelian (Gallente)
    "Arazu": "Surveillant",
    "Lachesis": "Arbitre",
    # Solari (Amarr)
    "Pilgrim": "Wayfarer",
    "Curse": "Maledictus",

    # === T2 CRUISERS - LOGISTICS CRUISERS ===
    # Keldari (Minmatar)
    "Scimitar": "Lifeblood",
    # Veyren (Caldari)
    "Basilisk": "Restorer",
    # Aurelian (Gallente)
    "Oneiros": "Soigneur",
    # Solari (Amarr)
    "Guardian": "Protector",

    # === T2 BATTLECRUISERS - COMMAND SHIPS ===
    # Keldari (Minmatar)
    "Claymore": "Warleader",
    # Veyren (Caldari)
    "Vulture": "Jarl",
    # Aurelian (Gallente)
    "Astarte": "Commandant",
    # Solari (Amarr)
    "Absolution": "Paragon",

    # === T2 BATTLESHIPS - MARAUDERS ===
    # Keldari (Minmatar)
    "Vargur": "Ironheart",
    # Veyren (Caldari)
    "Golem": "Monolith",
    # Aurelian (Gallente)
    "Kronos": "Majeste",
    # Solari (Amarr)
    "Paladin": "Archon Prime",
}

# ---------------------------------------------------------------------------
# SYSTEM NAME MAPPINGS
# ---------------------------------------------------------------------------
SYSTEMS = {
    "Jita": "Thyrkstad",
    "Perimeter": "Rimward",
    "Rancer": "Duskfall",
    "Hek": "Kelheim",
    # "Amarr" system is handled by the RACES mapping → becomes "Solari"
    "Dodixie": "Aurendis",
    "Sobaseki": "Frostmere",
}

# Station name components mapped via faction names
STATION_NAME_PARTS = {
    "Caldari Navy Assembly Plant": "Veyren Fleet Assembly Yard",
    "Caldari Provisions Warehouse": "Veyren Provisions Depot",
}

# ---------------------------------------------------------------------------
# NPC / NAMED NPC MAPPINGS
# ---------------------------------------------------------------------------
NAMED_NPCS = {
    "Tobias Kruzhor": "Vargus Thornbane",
    "Draclira Merlonne": "Seraphine Blackveil",
    "Cormack Vaaja": "Theron Darkfang",
    "Hakim Stormare": "Korath Ironstorm",
}

# ---------------------------------------------------------------------------
# SKILL RACE REFERENCES (skill names that reference race names)
# ---------------------------------------------------------------------------
SKILL_RACE_REFS = {
    "amarr_frigate": "solari_frigate",
    "caldari_frigate": "veyren_frigate",
    "gallente_frigate": "aurelian_frigate",
    "minmatar_frigate": "keldari_frigate",
    "Amarr Frigate": "Solari Frigate",
    "Caldari Frigate": "Veyren Frigate",
    "Gallente Frigate": "Aurelian Frigate",
    "Minmatar Frigate": "Keldari Frigate",
}

# ---------------------------------------------------------------------------
# MISSION NAME MAPPINGS
# ---------------------------------------------------------------------------
MISSIONS = {
    "Right to Rule": "Mandate of Light",
    "Penumbra": "Edge of Frost",
    "Syndication": "Accord of Liberty",
    "Wildfire": "Ashstorm",
}

# ---------------------------------------------------------------------------
# LORE / WORLD TERMS
# ---------------------------------------------------------------------------
LORE_TERMS = {
    "New Eden": "Astralis",
    "capsuleer": "starhelm",
    "capsuleers": "starhelms",
    "Capsuleer": "Starhelm",
    "Capsuleers": "Starhelms",
}

# ---------------------------------------------------------------------------
# ICE TYPES (mapped if present)
# ---------------------------------------------------------------------------
ICE_TYPES = {
    "Clear Icicle": "Glacial Shard",
    "Glacial Mass": "Frost Mass",
    "Blue Ice": "Azure Ice",
    "White Glaze": "Pale Glaze",
    "Glare Crust": "Bright Crust",
    "Dark Glitter": "Shadow Glitter",
    "Gelidus": "Cryolis",
    "Krystallos": "Stellacite",
}

# ---------------------------------------------------------------------------
# GAS TYPES (mapped if present)
# ---------------------------------------------------------------------------
GAS_TYPES = {
    "Fullerite": "Nebuline",
    "Cytoserocin": "Vaporin",
    "Mykoserocin": "Mistocin",
}

# ---------------------------------------------------------------------------
# MODULE CATEGORIES (these are generic enough to not need renaming, but
# faction-prefixed module names do)
# ---------------------------------------------------------------------------
FACTION_MODULE_PREFIXES = {
    "Caldari Navy": "Veyren Fleet",
    "Amarr Navy": "Solari Imperial",
    "Federation Navy": "Aurelian Grand",
    "Republic Fleet": "Keldari Tribal",
    "Serpentis": "Venom Syndicate",
    "Guristas": "Iron Corsairs",
    "Blood Raider": "Crimson Order",
    "Angel Cartel": "Shadow Pact",
    "Sansha": "Hollow",
    "True Sansha": "True Hollow",
    "Shadow Serpentis": "Deep Venom",
    "Dread Guristas": "Dread Corsairs",
    "Dark Blood": "Dark Crimson",
    "Domination": "Pact Ascendant",
}

# ---------------------------------------------------------------------------
# SLEEPER / WORMHOLE TERMS
# ---------------------------------------------------------------------------
SLEEPER_TERMS = {
    "Sleeper": "Dormant",
    "sleeper": "dormant",
    "Sleepers": "Dormants",
}

# ---------------------------------------------------------------------------
# BUILD THE MASTER MAPPING
# Combine all mappings into one, ordered from longest key to shortest
# to ensure longer matches take priority (e.g., "Blood Raiders" before "Blood")
# ---------------------------------------------------------------------------

def build_master_mapping():
    """Combine all sub-mappings into a single master dictionary."""
    master = {}
    for d in [
        RACES,
        RACE_ADJECTIVES,
        BLOODLINES,
        PIRATE_FACTIONS,
        ORGANIZATIONS,
        NPC_CORPORATIONS,
        ORES,
        MINERALS,
        SHIPS,
        SYSTEMS,
        STATION_NAME_PARTS,
        NAMED_NPCS,
        SKILL_RACE_REFS,
        MISSIONS,
        LORE_TERMS,
        ICE_TYPES,
        GAS_TYPES,
        FACTION_MODULE_PREFIXES,
        SLEEPER_TERMS,
    ]:
        master.update(d)
    return master


MASTER_MAPPING = build_master_mapping()


def _build_lowercase_mapping():
    """Build a lowercase-keyed version for case-insensitive matching."""
    return {k.lower(): (k, v) for k, v in MASTER_MAPPING.items()}


LOWERCASE_MAPPING = _build_lowercase_mapping()


def _build_key_mapping():
    """Build a mapping for JSON keys (typically snake_case / lowercase)."""
    km = {}
    for orig, replacement in MASTER_MAPPING.items():
        lower_orig = orig.lower().replace(" ", "_").replace("-", "_")
        lower_repl = replacement.lower().replace(" ", "_").replace("-", "_")
        if lower_orig != lower_repl:
            km[lower_orig] = lower_repl
    return km


KEY_MAPPING = _build_key_mapping()

# Sort keys by length descending so longer matches are tried first
SORTED_KEYS = sorted(MASTER_MAPPING.keys(), key=len, reverse=True)


def _match_case(original_text, replacement):
    """
    Attempt to preserve the case pattern of original_text in replacement.
    Handles: ALL UPPER, all lower, Title Case, and mixed (returns replacement as-is).
    """
    if original_text.isupper():
        return replacement.upper()
    if original_text.islower():
        return replacement.lower()
    if original_text.istitle():
        return replacement.title()
    # For mixed case or single-char, return as defined
    return replacement


def rename_string(text):
    """
    Apply all renamings to a string value.
    Handles case-insensitive matching while preserving the case pattern.
    Also handles snake_case identifiers embedded in values.
    """
    if not isinstance(text, str):
        return text

    # First pass: apply snake_case key mappings for id-like values
    # (values that look like identifiers: lowercase with underscores, no spaces)
    if re.match(r'^[a-z0-9_]+$', text):
        renamed = text
        sorted_km = sorted(KEY_MAPPING.items(), key=lambda x: len(x[0]), reverse=True)
        for old_k, new_k in sorted_km:
            if old_k in renamed:
                renamed = renamed.replace(old_k, new_k)
        if renamed != text:
            return renamed

    # Second pass: apply display-name replacements (case-insensitive)
    for key in SORTED_KEYS:
        if key.lower() not in text.lower():
            continue
        pattern = re.compile(re.escape(key), re.IGNORECASE)
        def replacer(match, _repl=MASTER_MAPPING[key]):
            return _match_case(match.group(0), _repl)
        text = pattern.sub(replacer, text)

    return text


def rename_key(key):
    """
    Rename a JSON key (typically snake_case identifiers).
    Applies KEY_MAPPING for exact matches and substring replacements.
    """
    if not isinstance(key, str):
        return key

    result = key
    # Sort KEY_MAPPING by length descending for correct replacement order
    sorted_km = sorted(KEY_MAPPING.items(), key=lambda x: len(x[0]), reverse=True)
    for old_k, new_k in sorted_km:
        if old_k in result:
            result = result.replace(old_k, new_k)
    return result


def rename_value(value):
    """Recursively rename string values in a JSON structure."""
    if isinstance(value, str):
        return rename_string(value)
    if isinstance(value, list):
        return [rename_value(item) for item in value]
    if isinstance(value, dict):
        return rename_dict(value)
    return value


def rename_dict(d):
    """Rename both keys and values in a dictionary, recursively."""
    new_dict = {}
    for key, value in d.items():
        new_key = rename_key(key)
        new_value = rename_value(value)
        new_dict[new_key] = new_value
    return new_dict


def apply_renaming(filepath):
    """
    Read a JSON file, apply all renamings to both keys and string values,
    and write the result back to the same file.
    """
    filepath = Path(filepath)
    if not filepath.exists():
        print(f"  SKIP (not found): {filepath}")
        return False
    if filepath.suffix != ".json":
        print(f"  SKIP (not JSON): {filepath}")
        return False

    try:
        with open(filepath, "r", encoding="utf-8") as f:
            data = json.load(f)
    except (json.JSONDecodeError, UnicodeDecodeError) as e:
        print(f"  ERROR reading {filepath}: {e}")
        return False

    renamed_data = rename_value(data)

    with open(filepath, "w", encoding="utf-8") as f:
        json.dump(renamed_data, f, indent=2, ensure_ascii=False)
        f.write("\n")

    print(f"  OK: {filepath}")
    return True


def rename_all(data_dir=None):
    """
    Process all JSON files under the data/ directory of the EVEOFFLINE project.
    Optionally specify a custom data_dir path.
    """
    if data_dir is None:
        # Default to the project data directory
        data_dir = _DEFAULT_DATA_DIR

    data_dir = Path(data_dir)
    if not data_dir.is_dir():
        print(f"ERROR: Data directory not found: {data_dir}")
        sys.exit(1)

    json_files = sorted(data_dir.rglob("*.json"))
    print(f"Found {len(json_files)} JSON files in {data_dir}")
    print("=" * 60)

    success = 0
    failed = 0
    for jf in json_files:
        if apply_renaming(jf):
            success += 1
        else:
            failed += 1

    print("=" * 60)
    print(f"Done. {success} files renamed, {failed} skipped/failed.")


def print_mapping_summary():
    """Print a human-readable summary of all mappings."""
    categories = [
        ("Races", RACES),
        ("Race Adjectives", RACE_ADJECTIVES),
        ("Bloodlines", BLOODLINES),
        ("Pirate Factions", PIRATE_FACTIONS),
        ("Organizations", ORGANIZATIONS),
        ("NPC Corporations", NPC_CORPORATIONS),
        ("Ores", ORES),
        ("Minerals", MINERALS),
        ("Ships", SHIPS),
        ("Systems", SYSTEMS),
        ("Station Names", STATION_NAME_PARTS),
        ("Named NPCs", NAMED_NPCS),
        ("Skills (race refs)", SKILL_RACE_REFS),
        ("Missions", MISSIONS),
        ("Lore Terms", LORE_TERMS),
        ("Ice Types", ICE_TYPES),
        ("Gas Types", GAS_TYPES),
        ("Faction Module Prefixes", FACTION_MODULE_PREFIXES),
        ("Sleeper Terms", SLEEPER_TERMS),
    ]
    total = 0
    for name, mapping in categories:
        print(f"\n{'─' * 50}")
        print(f"  {name} ({len(mapping)} entries)")
        print(f"{'─' * 50}")
        for old, new in mapping.items():
            print(f"    {old:30s} → {new}")
        total += len(mapping)
    print(f"\n{'═' * 50}")
    print(f"  TOTAL MAPPINGS: {total}")
    print(f"{'═' * 50}")


# ---------------------------------------------------------------------------
# CLI ENTRY POINT
# ---------------------------------------------------------------------------
if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser(
        description="EVEOFFLINE Content Renaming Tool"
    )
    parser.add_argument(
        "--apply", action="store_true",
        help="Apply renaming to all JSON files in data/"
    )
    parser.add_argument(
        "--file", type=str,
        help="Apply renaming to a single JSON file"
    )
    parser.add_argument(
        "--summary", action="store_true",
        help="Print a summary of all mappings"
    )
    parser.add_argument(
        "--data-dir", type=str, default=None,
        help="Override the data directory path"
    )
    parser.add_argument(
        "--dry-run", action="store_true",
        help="Show what would be renamed without modifying files"
    )

    args = parser.parse_args()

    if args.summary:
        print_mapping_summary()
    elif args.file:
        if args.dry_run:
            with open(args.file, "r") as f:
                data = json.load(f)
            renamed = rename_value(data)
            print(json.dumps(renamed, indent=2, ensure_ascii=False))
        else:
            apply_renaming(args.file)
    elif args.apply:
        if args.dry_run:
            data_dir = Path(args.data_dir) if args.data_dir else _DEFAULT_DATA_DIR
            for jf in sorted(data_dir.rglob("*.json")):
                with open(jf, "r") as f:
                    data = json.load(f)
                renamed = rename_value(data)
                print(f"\n{'='*60}")
                print(f"FILE: {jf}")
                print(f"{'='*60}")
                print(json.dumps(renamed, indent=2, ensure_ascii=False)[:2000])
                print("...")
        else:
            rename_all(args.data_dir)
    else:
        parser.print_help()
