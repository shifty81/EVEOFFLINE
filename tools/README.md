# Atlas — Modding Tools

This directory contains utilities to help modders create and validate custom content.

## Available Tools

### validate_json.py

JSON validation tool for checking game data files.

**Usage:**
```bash
# Validate all JSON files in data/
python tools/validate_json.py

# Validate with verbose output
python tools/validate_json.py --verbose

# Validate a single file
python tools/validate_json.py --file data/ships/frigates.json

# Validate a specific directory
python tools/validate_json.py --data-dir custom_mod/data
```

**Features:**
- Syntax validation (checks for valid JSON)
- Structure validation (checks for required fields)
- Value range validation (warns about unusual values)
- Color-coded output for easy reading

**Note:** The validation rules are based on recommended structure. Some warnings may be acceptable depending on your mod's design goals.

### create_ship.py

Interactive ship creation tool that guides you through creating a new ship definition.

**Usage:**
```bash
python tools/create_ship.py
```

**Features:**
- Interactive prompts for all ship properties
- Ship class selection menu
- Default values for quick creation
- Optional bonus definition
- JSON output preview
- Auto-save to file with overwrite protection

**Example:**
The tool will prompt you for ship details and generate a complete JSON definition like:
```json
{
  "custom_frigate": {
    "id": "custom_frigate",
    "name": "Custom Frigate",
    "class": "Frigate",
    ...
  }
}
```

### BlenderSpaceshipGenerator/

Blender addon for procedurally generating spaceships, stations, and asteroid belts with full EVEOFFLINE integration.

**Requirements:** [Blender](https://www.blender.org/) 3.0+

**Installation:**
1. Open Blender → Edit → Preferences → Add-ons → Install
2. Navigate to `tools/BlenderSpaceshipGenerator/` and select `__init__.py`
3. Enable the addon in the Add-ons list

**Features:**
- Procedural ship generation for 18 ship classes (Shuttle → Titan)
- 4 faction styles matching EVEOFFLINE races (Solari, Veyren, Aurelian, Keldari)
- Modular brick-based construction with 18 brick types
- Interior generation with FPV-ready human-scale dimensions
- Station and asteroid belt generation (16 ore types)
- Import ships from EVEOFFLINE JSON data files
- Export OBJ models with correct Atlas engine axis orientation
- Ship DNA serialization for reproducible generation

**Documentation:**
- [README](BlenderSpaceshipGenerator/README.md) — Overview and quick start
- [Usage Guide](BlenderSpaceshipGenerator/USAGE.md) — Detailed usage instructions
- [EVEOFFLINE Guide](BlenderSpaceshipGenerator/EVEOFFLINE_GUIDE.md) — Integration with EVEOFFLINE
- [Engine Integration](BlenderSpaceshipGenerator/ENGINE_INTEGRATION.md) — Technical reference for engine developers
- [Examples](BlenderSpaceshipGenerator/EXAMPLES.md) — Usage examples

**Validation (no Blender required):**
```bash
python tools/BlenderSpaceshipGenerator/test_validation.py
```

## Future Tools

The following tools are planned for future releases:

### mission_editor.py (Planned)
GUI or CLI tool for creating and editing mission files with templates.

### create_module.py (Planned)
Interactive tool for creating module JSON definitions with guided prompts.

### balance_analyzer.py (Planned)
Analyzes ship and module stats to identify balance issues.

### mod_packager.py (Planned)
Packages mods into distributable archives with metadata.

## Contributing

Have an idea for a modding tool? Check out the [Contributing Guide](../docs/CONTRIBUTING.md) to get started!

---

*For more information on modding, see the [Modding Guide](../docs/MODDING_GUIDE.md)*
