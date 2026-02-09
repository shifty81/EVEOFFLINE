# Modular Ship Generation System - Usage Guide

This guide explains how to use the new modular ship generation system with faction-specific rules and component assembly.

## Overview

The modular ship generation system consists of three main components:

1. **ShipPartLibrary**: Manages reusable ship components (hulls, engines, weapons)
2. **ShipGenerationRules**: Enforces faction and class-specific design constraints
3. **Model Integration**: Assembles parts into complete ship models

## Basic Usage

### 1. Initialize the System

```cpp
#include "rendering/ship_part_library.h"
#include "rendering/ship_generation_rules.h"
#include "rendering/model.h"

// Create and initialize the part library
eve::ShipPartLibrary partLibrary;
partLibrary.initialize();

// Create and initialize the rules engine
eve::ShipGenerationRules rules;
rules.initialize();
```

### 2. Generate a Ship Using Modular Parts

```cpp
// Create assembly configuration for a Minmatar Frigate ("Drifter")
eve::ShipAssemblyConfig config = partLibrary.createAssemblyConfig("Frigate", "Minmatar");

// The config now contains:
// - hullForwardId: "minmatar_forward_1"
// - hullMainId: "minmatar_main_1"  
// - hullRearId: "minmatar_rear_1"
// - overallScale: 3.5f
// - asymmetryFactor: 0.3f (Minmatar asymmetric design)

// Get faction and class rules
auto factionRules = rules.getFactionRules("Minmatar");
auto classRules = rules.getClassRules("Frigate");

std::cout << "Minmatar ships require asymmetry: " << factionRules.allowsAsymmetry << std::endl;
std::cout << "Frigates require " << classRules.minTurretHardpoints 
          << "-" << classRules.maxTurretHardpoints << " turrets" << std::endl;
```

### 3. Retrieve Ship Parts

```cpp
// Get specific parts by ID
const eve::ShipPart* forwardHull = partLibrary.getPart("minmatar_forward_1");
const eve::ShipPart* mainHull = partLibrary.getPart("minmatar_main_1");
const eve::ShipPart* rearHull = partLibrary.getPart("minmatar_rear_1");

// Get all engines for a faction
auto minmatarEngines = partLibrary.getPartsByType(eve::ShipPartType::ENGINE_MAIN, "Minmatar");

std::cout << "Found " << minmatarEngines.size() << " Minmatar engine designs" << std::endl;
```

### 4. Validate Ship Configuration

```cpp
// Create a part count map for validation
std::map<std::string, int> partCounts;
partCounts["hull_main"] = 1;
partCounts["engine"] = 2;
partCounts["turret"] = 2;

// Validate against rules
bool isValid = rules.validate("Minmatar", "Frigate", partCounts);

if (isValid) {
    std::cout << "Ship configuration is valid!" << std::endl;
} else {
    std::cout << "Ship configuration violates design rules" << std::endl;
}

// Get recommended part counts
auto recommended = rules.getRecommendedPartCounts("Minmatar", "Frigate");
std::cout << "Recommended turrets: " << recommended["turret"] << std::endl;
std::cout << "Recommended engines: " << recommended["engine"] << std::endl;
```

### 5. Check Placement Rules

```cpp
// Check if a weapon position is valid
glm::vec3 weaponPos(2.0f, 0.5f, 0.2f);  // Forward, slightly to the right
glm::vec3 shipSize(3.5f, 0.9f, 0.7f);   // Frigate dimensions

bool validWeapon = rules.isWeaponPlacementValid(weaponPos, shipSize);

// Check if an engine position is valid
glm::vec3 enginePos(-1.0f, 0.4f, 0.0f); // Rear of ship
bool validEngine = rules.isEnginePlacementValid(enginePos, shipSize);
```

## Faction-Specific Examples

### Minmatar Ships (Asymmetric, Rustic)

```cpp
// Minmatar "Drifter" (formerly Rifter) - Frigate
auto config = partLibrary.createAssemblyConfig("Frigate", "Minmatar");
auto factionRules = rules.getFactionRules("Minmatar");

// Characteristics:
// - allowsAsymmetry: true
// - asymmetryFactor: 0.3f
// - requiresVerticalElements: true (high vertical structures)
// - allowsExposedFramework: true
// - Color scheme: Rust brown, dark brown, light rust
```

### Caldari Ships (Blocky, Industrial)

```cpp
// Caldari "Brick" (formerly Drake) - Battlecruiser
auto config = partLibrary.createAssemblyConfig("Battlecruiser", "Caldari");
auto factionRules = rules.getFactionRules("Caldari");

// Characteristics:
// - requiresSymmetry: true
// - requiresAngularGeometry: true (blocky shapes)
// - asymmetryFactor: 0.0f (perfectly symmetric)
// - Color scheme: Steel blue, dark blue, light blue
```

### Gallente Ships (Organic, Smooth)

```cpp
// Gallente "Dominus" (formerly Dominix) - Battleship
auto config = partLibrary.createAssemblyConfig("Battleship", "Gallente");
auto factionRules = rules.getFactionRules("Gallente");

// Characteristics:
// - requiresSymmetry: true
// - requiresOrganicCurves: true (smooth, flowing forms)
// - mandatoryPartTypes: includes "drone_bay"
// - Color scheme: Dark green-gray, darker green, light green
```

### Amarr Ships (Golden, Ornate)

```cpp
// Amarr "Inquisitor" (formerly Punisher) - Frigate
auto config = partLibrary.createAssemblyConfig("Frigate", "Amarr");
auto factionRules = rules.getFactionRules("Amarr");

// Characteristics:
// - requiresSymmetry: true
// - requiresVerticalElements: true (spires)
// - requiresOrnateDetails: true (cathedral-like)
// - mandatoryPartTypes: includes "spire"
// - Color scheme: Gold-brass, dark gold, bright gold
```

## Ship Class Examples

### Frigate (3.5 units, fast and agile)

```cpp
auto classRules = rules.getClassRules("Frigate");

// Dimensions: 3.0-4.0 length, 0.7-1.2 width, 0.5-0.9 height
// Weapons: 2-3 turrets, 0-2 launchers, 0-1 drone bays
// Engines: 2-3
// Detail density: 1.0x
```

### Cruiser (6.0 units, versatile)

```cpp
auto classRules = rules.getClassRules("Cruiser");

// Dimensions: 5.5-6.5 length, 1.5-2.2 width, 1.0-1.5 height
// Weapons: 4-5 turrets, 2-4 launchers, 1-2 drone bays
// Engines: 3-4
// Detail density: 1.5x
```

### Battleship (12.0 units, heavy firepower)

```cpp
auto classRules = rules.getClassRules("Battleship");

// Dimensions: 11.0-13.0 length, 3.0-4.0 width, 2.0-3.0 height
// Weapons: 6-8 turrets, 4-6 launchers, 2-3 drone bays
// Engines: 6-8
// Detail density: 2.5x
```

### Titan (25.0 units, supercapital)

```cpp
auto classRules = rules.getClassRules("Titan");

// Dimensions: 23.0-27.0 length, 7.0-9.0 width, 5.0-7.0 height
// Weapons: 6-10 turrets
// Engines: 8-12
// Detail density: 4.0x
```

## Integration with Existing Model System

To integrate the modular system with the existing procedural generation:

```cpp
// In Model::createShipModel() or createShipModelWithRacialDesign()
std::unique_ptr<Model> Model::createShipModelWithRacialDesign(
    const std::string& shipType, const std::string& faction) {
    
    // Initialize libraries (do this once at startup)
    static ShipPartLibrary partLibrary;
    static ShipGenerationRules rules;
    static bool initialized = false;
    if (!initialized) {
        partLibrary.initialize();
        rules.initialize();
        initialized = true;
    }
    
    // Determine ship class
    std::string shipClass = determineShipClass(shipType);
    
    // Create assembly configuration
    auto config = partLibrary.createAssemblyConfig(shipClass, faction);
    
    // Get parts
    const ShipPart* forward = partLibrary.getPart(config.hullForwardId);
    const ShipPart* main = partLibrary.getPart(config.hullMainId);
    const ShipPart* rear = partLibrary.getPart(config.hullRearId);
    
    // Assemble ship model
    auto model = std::make_unique<Model>();
    
    // Apply scale and assembly (implementation needed)
    // This will combine the parts into a single mesh
    
    return model;
}
```

## Custom Part Creation

You can add custom parts to the library:

```cpp
eve::ShipPart customEngine;
customEngine.type = eve::ShipPartType::ENGINE_MAIN;
customEngine.name = "Custom High-Performance Engine";
customEngine.faction = "Minmatar";
customEngine.isSymmetric = false;

// Add vertices and indices for custom geometry
// ... (create vertices and indices)

// Add to library
partLibrary.addPart("custom_engine_1", customEngine);
```

## Reference Images

When creating ship designs, reference these sources:
- **eveonlineships.com**: Official ship screenshots
- **ArtStation (Elijah Sage)**: Custom faction concepts
- **EVE University Wiki**: Faction aesthetic descriptions

## Naming Convention

All ships use fan nicknames to avoid copyright:
- Minmatar "Drifter" (was Rifter)
- Caldari "Brick" (was Drake)
- Gallente "Dominus" (was Dominix)
- Amarr "Inquisitor" (was Punisher)

See `docs/SHIP_NAMING_GUIDE.md` for complete mapping.

## Performance Notes

- Part library initialization is done once at startup
- Parts are stored as vertex/index arrays
- Modular assembly allows for caching common components
- LOD (Level of Detail) can be applied per-part
- Detail density scales with ship class

---

*Last Updated: February 9, 2026*
