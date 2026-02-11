# Station Renderer - Implementation Guide

## Overview

The `StationRenderer` class provides procedural rendering of space stations with faction-specific designs and Upwell player structures. It creates geometric station models on-the-fly based on the visual data specifications in `data/universe/station_visual_data.json`.

## Features

- **4 Faction Styles**: Solari (cathedral), Veyren (industrial), Aurelian (organic), Keldari (rusty)
- **4 Upwell Structures**: Astrahus, Fortizar, Keepstar, Raitaru
- **Procedural Generation**: All meshes created algorithmically, no external models needed
- **Material Properties**: PBR materials with faction-specific colors, metallic/roughness values
- **Efficient Rendering**: Reuses meshes for multiple station instances

## Usage Example

### Basic Setup

```cpp
#include "rendering/station_renderer.h"
#include "rendering/shader.h"
#include "rendering/camera.h"

// Initialize the renderer
eve::StationRenderer stationRenderer;
if (!stationRenderer.initialize()) {
    std::cerr << "Failed to initialize StationRenderer" << std::endl;
    return false;
}
```

### Adding Faction Stations

```cpp
// Add a Solari (golden cathedral style) station
eve::StationRenderer::StationInstance solariStation;
solariStation.id = "station_jita_4-4";
solariStation.position = glm::vec3(1000000.0f, 0.0f, 5000000.0f);
solariStation.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
solariStation.faction = eve::StationRenderer::FactionStyle::SOLARI;
solariStation.scale = 1.0f;
solariStation.isUpwell = false;
stationRenderer.addStation(solariStation);

// Add a Veyren (industrial blocky) station
eve::StationRenderer::StationInstance veyrenStation;
veyrenStation.id = "station_caldari_navy";
veyrenStation.position = glm::vec3(-2000000.0f, 100000.0f, 3000000.0f);
veyrenStation.rotation = glm::vec3(0.0f, glm::radians(45.0f), 0.0f);
veyrenStation.faction = eve::StationRenderer::FactionStyle::VEYREN;
veyrenStation.scale = 1.2f;
veyrenStation.isUpwell = false;
stationRenderer.addStation(veyrenStation);
```

### Adding Upwell Structures

```cpp
// Add an Astrahus medium citadel
eve::StationRenderer::StationInstance astrahus;
astrahus.id = "citadel_player_01";
astrahus.position = glm::vec3(0.0f, 500000.0f, 8000000.0f);
astrahus.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
astrahus.scale = 1.0f;
astrahus.isUpwell = true;
astrahus.upwellType = eve::StationRenderer::UpwellType::ASTRAHUS;
stationRenderer.addStation(astrahus);

// Add a Keepstar XL citadel
eve::StationRenderer::StationInstance keepstar;
keepstar.id = "citadel_keepstar_home";
keepstar.position = glm::vec3(10000000.0f, 0.0f, 10000000.0f);
keepstar.rotation = glm::vec3(0.0f, glm::radians(90.0f), 0.0f);
keepstar.scale = 2.0f;
keepstar.isUpwell = true;
keepstar.upwellType = eve::StationRenderer::UpwellType::KEEPSTAR;
stationRenderer.addStation(keepstar);
```

### Rendering

```cpp
// In your render loop
void render() {
    // Set up your shader with required uniforms
    shader.use();
    
    // Render all stations
    stationRenderer.render(&shader, camera);
}
```

### Required Shader Uniforms

Your shader must support the following uniforms:

```glsl
// Vertex shader
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// Fragment shader (for PBR materials)
struct Material {
    vec3 albedo;
    float metallic;
    float roughness;
    vec3 emissive;
    float emissiveIntensity;
};
uniform Material material;
```

## Integration with SolarSystemScene

```cpp
#include "core/solar_system_scene.h"
#include "rendering/station_renderer.h"

class GameRenderer {
private:
    eve::SolarSystemScene m_scene;
    eve::StationRenderer m_stationRenderer;
    
public:
    void initialize() {
        m_stationRenderer.initialize();
        
        // Load system
        m_scene.loadTestSystem();
        
        // Convert celestials to station instances
        for (const auto& celestial : m_scene.getCelestials()) {
            if (celestial.type == eve::Celestial::Type::STATION) {
                eve::StationRenderer::StationInstance station;
                station.id = celestial.id;
                station.position = celestial.position;
                station.rotation = glm::vec3(0.0f);
                station.scale = celestial.radius / 5000.0f; // Scale based on radius
                station.isUpwell = false;
                
                // Determine faction from station name or data
                // For now, use a simple mapping
                if (celestial.name.find("Solari") != std::string::npos) {
                    station.faction = eve::StationRenderer::FactionStyle::SOLARI;
                } else if (celestial.name.find("Veyren") != std::string::npos) {
                    station.faction = eve::StationRenderer::FactionStyle::VEYREN;
                } else if (celestial.name.find("Aurelian") != std::string::npos) {
                    station.faction = eve::StationRenderer::FactionStyle::AURELIAN;
                } else {
                    station.faction = eve::StationRenderer::FactionStyle::KELDARI;
                }
                
                m_stationRenderer.addStation(station);
            }
        }
    }
    
    void render(eve::Shader* shader, const eve::Camera& camera) {
        m_stationRenderer.render(shader, camera);
    }
};
```

## Faction Visual Properties

The renderer uses the following visual properties from `station_visual_data.json`:

### Solari (Cathedral Style)
- **Primary Color**: Golden (0.8, 0.6, 0.2)
- **Features**: Central cylinder with 4 golden spires
- **Style**: Grand, ornate, cathedral-like
- **Size Multiplier**: 1.2x

### Veyren (Industrial)
- **Primary Color**: Steel blue (0.4, 0.45, 0.5)
- **Features**: Multiple connected boxes, city-block structure
- **Style**: Blocky, utilitarian, industrial
- **Size Multiplier**: 1.0x

### Aurelian (Organic)
- **Primary Color**: Green-blue (0.2, 0.4, 0.3)
- **Features**: Large central sphere with connected smaller spheres
- **Style**: Flowing curves, organic, sleek
- **Size Multiplier**: 1.1x

### Keldari (Rusty)
- **Primary Color**: Rusty brown (0.4, 0.3, 0.25)
- **Features**: Asymmetric scaffolding with central habitation module
- **Style**: Improvised, patchwork, exposed machinery
- **Size Multiplier**: 0.9x

## Performance Considerations

- Station meshes are generated once during initialization and reused for all instances
- Each faction station type has one mesh (4 total)
- Each Upwell structure type has one mesh (4 total)
- Total of 8 meshes in memory regardless of number of stations
- Rendering is done per-station with individual model matrices

## Future Enhancements

Possible improvements for future versions:

1. **LOD System**: Implement level-of-detail for distant stations
2. **Animated Elements**: Add rotating sections, extending docking arms
3. **Lights**: Add emissive lights for docking bays and windows
4. **Textures**: Apply procedural or image-based textures
5. **Damage States**: Show visual damage when station HP is reduced
6. **Tethering Visual**: Display tethering range for Upwell structures
7. **Custom Models**: Support loading .obj/.gltf models for unique stations

## Testing

The renderer has been integrated with the build system in `cpp_client/CMakeLists.txt`. To test:

```bash
cd cpp_client
mkdir build && cd build
cmake ..
make
./eve_client
```

## Related Files

- **Header**: `cpp_client/include/rendering/station_renderer.h`
- **Implementation**: `cpp_client/src/rendering/station_renderer.cpp`
- **Data**: `data/universe/station_visual_data.json`
- **CMake**: `cpp_client/CMakeLists.txt`

## Notes

- The renderer creates simplified geometric representations of stations
- All geometry is procedurally generated, no external model files required
- Vertex colors are baked into the mesh for faction-specific appearance
- Compatible with standard PBR shader pipelines

---

**Status**: Implemented and ready for integration (Phase 7 complete)
**Next Steps**: Network integration (Phase 8) to sync stations with server data
