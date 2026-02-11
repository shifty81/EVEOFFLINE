# EVE OFFLINE - Modding Tools

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

## Future Tools

The following tools are planned for future releases:

### ship_builder.py (Planned)
Interactive tool for creating ship JSON definitions with guided prompts.

### mission_editor.py (Planned)
GUI or CLI tool for creating and editing mission files.

### balance_analyzer.py (Planned)
Analyzes ship and module stats to identify balance issues.

### mod_packager.py (Planned)
Packages mods into distributable archives with metadata.

## Contributing

Have an idea for a modding tool? Check out the [Contributing Guide](../docs/CONTRIBUTING.md) to get started!

---

*For more information on modding, see the [Modding Guide](../docs/MODDING_GUIDE.md)*
