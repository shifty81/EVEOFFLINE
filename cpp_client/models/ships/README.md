# OBJ Ship Models

This directory is used by CMake as a fallback location for OBJ ship models.

The build system automatically extracts models from `data/ships/obj_models/obj_models.7z`
into the build output directory. You only need `7z` (p7zip-full on Linux, 7-Zip on Windows)
installed and the extraction happens during `cmake` configuration.

## File Naming Convention

Files follow the pattern: `{faction}_{class}_{ShipName}.obj`

Examples:
- `minmatar_frigate_Rifter.obj`
- `caldari_battleship_Raven.obj`
- `amarr_cruiser_Omen.obj`
- `gallente_frigate_Tristan.obj`

## How It Works

When the game starts, `Model::createShipModel()` checks the `models/ships/` directory
(next to the executable) for a matching OBJ file before falling back to procedural
generation. The match is based on the faction name and ship name from the filename.
