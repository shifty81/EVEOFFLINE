# Procedural Ship Generation - Next Steps & Recommendations

## Current Implementation Status

✅ **COMPLETE**:
- Modular architecture (ShipPartLibrary)
- Ruleset engine (ShipGenerationRules)
- Faction-specific design rules
- Class-based constraints
- Placement validation logic
- Enhanced detail functions
- Ship naming guide (copyright-safe)
- Comprehensive documentation

⏳ **IN PROGRESS**:
- Model class integration
- Ship JSON data updates

🔜 **PLANNED**:
- Procedural texturing
- Complete testing suite

## Recommended Integration Steps

### Step 1: Integrate Modular Assembly into Model Class

**File**: `cpp_client/src/rendering/model.cpp`

**Location**: Modify `Model::createShipModelWithRacialDesign()` function

**Implementation**:
```cpp
std::unique_ptr<Model> Model::createShipModelWithRacialDesign(
    const std::string& shipType, const std::string& faction) {
    
    // Initialize libraries (singleton pattern)
    static ShipPartLibrary partLibrary;
    static ShipGenerationRules rules;
    static bool initialized = false;
    if (!initialized) {
        partLibrary.initialize();
        rules.initialize();
        initialized = true;
    }
    
    // Determine ship class from ship type
    std::string shipClass;
    if (isFrigate(shipType)) shipClass = "Frigate";
    else if (isDestroyer(shipType)) shipClass = "Destroyer";
    else if (isCruiser(shipType)) shipClass = "Cruiser";
    // ... etc for all classes
    
    // Get assembly configuration
    auto config = partLibrary.createAssemblyConfig(shipClass, faction);
    
    // Retrieve parts
    const ShipPart* forward = partLibrary.getPart(config.hullForwardId);
    const ShipPart* main = partLibrary.getPart(config.hullMainId);
    const ShipPart* rear = partLibrary.getPart(config.hullRearId);
    
    // Create model
    auto model = std::make_unique<Model>();
    
    // Assemble parts into unified mesh
    std::vector<Vertex> allVertices;
    std::vector<unsigned int> allIndices;
    
    // Add forward hull
    if (forward) {
        glm::mat4 forwardTransform = glm::translate(glm::mat4(1.0f), 
            config.overallScale * forward->attachmentPoint);
        addPartToMesh(forward, forwardTransform, allVertices, allIndices);
    }
    
    // Add main hull
    if (main) {
        glm::mat4 mainTransform = glm::scale(glm::mat4(1.0f), 
            glm::vec3(config.overallScale));
        addPartToMesh(main, mainTransform, allVertices, allIndices);
    }
    
    // Add rear hull
    if (rear) {
        glm::mat4 rearTransform = glm::translate(glm::mat4(1.0f), 
            config.overallScale * rear->attachmentPoint);
        addPartToMesh(rear, rearTransform, allVertices, allIndices);
    }
    
    // Add engines based on class rules
    auto classRules = rules.getClassRules(shipClass);
    auto engines = partLibrary.getPartsByType(ShipPartType::ENGINE_MAIN, faction);
    if (!engines.empty()) {
        int engineCount = (classRules.minEngines + classRules.maxEngines) / 2;
        for (int i = 0; i < engineCount && i < engines.size(); ++i) {
            // Calculate engine position (rear, distributed)
            float yOffset = (i % 2 == 0 ? 1.0f : -1.0f) * 0.3f;
            glm::vec3 enginePos(-config.overallScale * 0.3f, 
                               config.overallScale * yOffset, 0.0f);
            glm::mat4 engineTransform = glm::translate(glm::mat4(1.0f), enginePos);
            addPartToMesh(engines[i % engines.size()], engineTransform, 
                         allVertices, allIndices);
        }
    }
    
    // Add weapon hardpoints
    int turretCount = (classRules.minTurretHardpoints + classRules.maxTurretHardpoints) / 2;
    addWeaponHardpoints(allVertices, allIndices, 
                       config.overallScale * 0.5f, 
                       config.proportions.y * 0.5f, 
                       config.proportions.z * 0.3f,
                       turretCount,
                       glm::vec3(0.8f, 0.7f, 0.6f));
    
    // Add hull panel details
    int panelCount = static_cast<int>(classRules.detailDensity * 5.0f);
    addHullPanelLines(allVertices, allIndices,
                     config.overallScale * 0.6f,
                     -config.overallScale * 0.2f,
                     config.proportions.y * 0.8f,
                     glm::vec3(0.5f, 0.5f, 0.5f));
    
    // Add faction-specific details
    auto factionRules = rules.getFactionRules(faction);
    if (factionRules.requiresOrnateDetails && factionRules.hasSpires) {
        // Add Amarr spires
        addSpireDetail(allVertices, allIndices,
                      config.overallScale * 0.3f,
                      config.proportions.z * 2.0f,
                      glm::vec3(0.9f, 0.8f, 0.5f));
    }
    
    if (factionRules.allowsAsymmetry) {
        // Add Minmatar asymmetric elements
        addAsymmetricDetail(allVertices, allIndices,
                           config.overallScale * 0.2f,
                           config.asymmetryFactor * config.proportions.y,
                           glm::vec3(0.5f, 0.35f, 0.25f));
    }
    
    // Create final mesh
    auto mesh = std::make_unique<Mesh>(allVertices, allIndices);
    model->addMesh(std::move(mesh));
    
    return model;
}

// Helper function to add part to mesh with transform
void Model::addPartToMesh(const ShipPart* part, const glm::mat4& transform,
                          std::vector<Vertex>& allVertices, 
                          std::vector<unsigned int>& allIndices) {
    unsigned int baseIndex = allVertices.size();
    
    for (const auto& vertex : part->vertices) {
        Vertex transformed = vertex;
        glm::vec4 pos = transform * glm::vec4(vertex.position, 1.0f);
        transformed.position = glm::vec3(pos);
        glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(transform)));
        transformed.normal = glm::normalize(normalMatrix * vertex.normal);
        allVertices.push_back(transformed);
    }
    
    for (unsigned int index : part->indices) {
        allIndices.push_back(baseIndex + index);
    }
}
```

### Step 2: Update Ship JSON Files

**Files**: `data/ships/*.json`

**Example** (`data/ships/frigates.json`):
```json
{
  "drifter": {
    "id": "drifter",
    "name": "Drifter",
    "original_name": "Rifter",
    "class": "Frigate",
    "race": "Minmatar",
    "description": "The Drifter is a versatile frigate combining speed with firepower. Known as the 'rustbucket' by pilots, it's a classic choice for new capsuleers.",
    "hull_hp": 450,
    ...
  },
  "wizard": {
    "id": "wizard",
    "name": "Wizard",
    "original_name": "Merlin",
    "class": "Frigate",
    "race": "Caldari",
    ...
  }
}
```

**Mass Update Strategy**:
1. Create a Python script to update all JSON files
2. Use SHIP_NAMING_GUIDE.md as mapping source
3. Keep `original_name` field for reference
4. Update all references in code

### Step 3: Update Model Ship Type Checking

**File**: `cpp_client/src/rendering/model.cpp`

**Functions to Update**:
```cpp
bool Model::isFrigate(const std::string& shipType) {
    static const std::vector<std::string> frigateNames = {
        "Frigate", 
        // Old names (for backward compatibility)
        "Rifter", "Merlin", "Tristan", "Punisher",
        // New fan nicknames
        "Drifter", "Wizard", "Isolde", "Inquisitor",
        "Assault Frigate", "Jaguar", "Hawk", "Enyo", "Retribution", "Wolf", "Harpy"
    };
    return std::any_of(frigateNames.begin(), frigateNames.end(),
        [&shipType](const std::string& name) { 
            return shipType.find(name) != std::string::npos; 
        });
}

// Repeat for all ship classes...
```

### Step 4: Procedural Texturing (Phase 4)

**New File**: `cpp_client/include/rendering/procedural_texture_generator.h`

**Proposed Interface**:
```cpp
class ProceduralTextureGenerator {
public:
    // Generate albedo texture based on faction colors
    std::unique_ptr<Texture> generateAlbedo(
        int width, int height,
        const FactionColors& colors,
        const std::string& faction
    );
    
    // Generate normal map for hull details
    std::unique_ptr<Texture> generateNormalMap(
        int width, int height,
        float detailStrength = 0.5f
    );
    
    // Generate lightmap for engines and windows
    std::unique_ptr<Texture> generateLightmap(
        int width, int height,
        const std::vector<glm::vec2>& glowPoints,
        const glm::vec3& glowColor
    );
    
    // Automatic UV unwrapping
    void generateUVCoordinates(
        std::vector<Vertex>& vertices,
        const std::vector<unsigned int>& indices
    );
};
```

### Step 5: Testing Checklist

**Unit Tests** (when OpenGL available):
- [ ] ShipPartLibrary initialization
- [ ] Part retrieval by ID and type
- [ ] Assembly configuration creation
- [ ] Rule validation
- [ ] Placement validation
- [ ] Faction rules enforcement
- [ ] Class constraints checking

**Integration Tests**:
- [ ] Complete ship generation pipeline
- [ ] All factions produce valid geometry
- [ ] All classes have correct dimensions
- [ ] Weapon/engine placement validation
- [ ] Asymmetry factor applied correctly (Minmatar)
- [ ] Spires added for Amarr ships
- [ ] Detail density scales with class

**Visual Tests** (manual inspection):
- [ ] Frigate looks appropriately small and nimble
- [ ] Battleship looks massive and imposing
- [ ] Titan dwarfs all other ships
- [ ] Minmatar ships appear asymmetric
- [ ] Caldari ships appear blocky
- [ ] Gallente ships appear smooth
- [ ] Amarr ships have vertical spires
- [ ] Weapon hardpoints are visible
- [ ] Engines are clearly at the rear

## Quick Reference Commands

### Build the Project
```bash
cd /home/runner/work/EVEOFFLINE/EVEOFFLINE
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release -j4
```

### Run Tests (when available)
```bash
cd build
ctest -C Release --output-on-failure
```

### Generate Example Ships
```cpp
// In main.cpp or test file
eve::ShipPartLibrary partLibrary;
eve::ShipGenerationRules rules;
partLibrary.initialize();
rules.initialize();

// Generate one of each faction
auto minmatarFrigate = Model::createShipModelWithRacialDesign("Drifter", "Minmatar");
auto caldariFrigate = Model::createShipModelWithRacialDesign("Wizard", "Caldari");
auto gallenteFrigate = Model::createShipModelWithRacialDesign("Isolde", "Gallente");
auto amarrFrigate = Model::createShipModelWithRacialDesign("Inquisitor", "Amarr");

// Generate one of each class (Minmatar)
auto frigate = Model::createShipModelWithRacialDesign("Drifter", "Minmatar");
auto destroyer = Model::createShipModelWithRacialDesign("Thrasher", "Minmatar");
auto cruiser = Model::createShipModelWithRacialDesign("Stabby", "Minmatar");
auto battlecruiser = Model::createShipModelWithRacialDesign("Cyclone", "Minmatar");
auto battleship = Model::createShipModelWithRacialDesign("Tempest", "Minmatar");
```

## Performance Optimization Tips

1. **Part Caching**: Parts are stored once, reused many times
2. **Lazy Initialization**: Initialize libraries only when first ship is created
3. **LOD System**: Use `detailLevel` field to skip details at distance
4. **Instanced Rendering**: Use for multiple ships of same type
5. **Mesh Merging**: Combine multiple parts into single draw call

## Known Limitations

1. **No Dynamic Assembly Yet**: Parts defined but not yet assembled in runtime
2. **Fixed Part Library**: No runtime part addition (design choice for performance)
3. **Simple Geometry**: Parts use basic primitives (box, cylinder, cone)
4. **No Texture Baking**: Procedural textures not yet implemented
5. **No Animation**: Static models only (engines don't glow dynamically)

## Future Expansion Possibilities

1. **Player-Customizable Ships**: Allow players to mix faction parts
2. **Damaged States**: Generate damage decals procedurally
3. **Tech Variants**: Tech II/III visual differentiation
4. **Faction Hybrids**: Pirate factions mixing design elements
5. **Dynamic LOD**: Automatic detail reduction based on distance
6. **Editor Tool**: Visual part assembly tool for designers

## Contact & Support

For questions or issues:
- Check `docs/MODULAR_SHIP_GENERATION_USAGE.md` for usage examples
- Review `docs/PROCEDURAL_SHIP_GENERATION_SUMMARY.md` for architecture
- See `docs/SHIP_NAMING_GUIDE.md` for ship name mappings

---

**Document Version**: 1.0  
**Last Updated**: February 9, 2026  
**Status**: Core implementation complete, integration in progress
