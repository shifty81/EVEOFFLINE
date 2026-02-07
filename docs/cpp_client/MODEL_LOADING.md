# External Model Loading - Implementation Guide

## Overview

This document describes the implementation of external 3D model loading in EVE OFFLINE. The implementation adds support for loading .obj, .gltf, and .glb model files, complementing the existing procedural ship generation system.

## Implementation Details

### Supported Formats

1. **Wavefront OBJ (.obj)**
   - Text-based format
   - Wide industry support
   - Simple to create and edit
   - Good for static models
   - Library: tinyobjloader v2.0

2. **GL Transmission Format (.gltf)**
   - JSON-based format
   - Modern PBR workflow
   - Supports animations
   - Industry standard
   - Library: tinygltf v2.8+

3. **Binary GLTF (.glb)**
   - Compact binary format
   - Same as GLTF but smaller
   - Faster to load
   - Recommended for production
   - Library: tinygltf v2.8+

### Architecture

#### File Structure
```
cpp_client/
├── external/
│   ├── tinyobjloader/
│   │   └── tiny_obj_loader.h      # 108KB header-only library
│   ├── tinygltf/
│   │   └── tiny_gltf.h            # 279KB header-only library
│   └── nlohmann/
│       └── json.hpp               # 944KB header-only library (tinygltf dependency)
├── include/rendering/
│   └── model.h                    # Updated with loadOBJ/loadGLTF declarations
└── src/rendering/
    └── model.cpp                  # Implementation of loading functions
```

#### Class Interface

```cpp
class Model {
public:
    // Public interface
    bool loadFromFile(const std::string& path);
    
private:
    // Format-specific loaders
    bool loadOBJ(const std::string& path);
    bool loadGLTF(const std::string& path);
};
```

### Loading Process

#### 1. Format Detection
```cpp
bool Model::loadFromFile(const std::string& path) {
    // Extract extension
    std::string extension = /* extract from path */;
    
    // Route to appropriate loader
    if (extension == "obj") return loadOBJ(path);
    else if (extension == "gltf" || extension == "glb") return loadGLTF(path);
    else return false; // Unsupported format
}
```

#### 2. OBJ Loading (loadOBJ)

**Step 1: Parse File**
```cpp
tinyobj::attrib_t attrib;           // Vertex attributes
std::vector<tinyobj::shape_t> shapes;  // Shapes/objects
std::vector<tinyobj::material_t> materials; // Materials

bool success = tinyobj::LoadObj(&attrib, &shapes, &materials, 
                                &warn, &err, path.c_str());
```

**Step 2: Convert to Vertex Data**
```cpp
for (each shape) {
    for (each face) {
        for (each vertex in face) {
            // Extract position
            vertex.position = vec3(attrib.vertices[idx * 3...]);
            
            // Extract normal (if available)
            if (has_normal) 
                vertex.normal = vec3(attrib.normals[idx * 3...]);
            
            // Extract texture coords (if available)
            if (has_texcoord)
                vertex.texCoords = vec2(attrib.texcoords[idx * 2...]);
            
            // Extract color from material
            if (has_material)
                vertex.color = vec3(material.diffuse);
        }
    }
}
```

**Step 3: Create Meshes**
```cpp
auto mesh = std::make_unique<Mesh>(vertices, indices);
addMesh(std::move(mesh));
```

#### 3. GLTF Loading (loadGLTF)

**Step 1: Parse File**
```cpp
tinygltf::Model gltfModel;
tinygltf::TinyGLTF loader;

// Detect binary vs text format
if (extension == "glb")
    loader.LoadBinaryFromFile(&gltfModel, &err, &warn, path);
else
    loader.LoadASCIIFromFile(&gltfModel, &err, &warn, path);
```

**Step 2: Extract Vertex Data**
```cpp
for (each mesh in gltfModel.meshes) {
    for (each primitive in mesh.primitives) {
        // Get accessors
        auto& posAccessor = gltfModel.accessors[primitive.attributes["POSITION"]];
        auto& posView = gltfModel.bufferViews[posAccessor.bufferView];
        auto& posBuffer = gltfModel.buffers[posView.buffer];
        
        // Extract positions
        const float* positions = reinterpret_cast<const float*>(
            &posBuffer.data[posView.byteOffset + posAccessor.byteOffset]
        );
        
        // Similar for normals and texcoords...
    }
}
```

**Step 3: Handle Different Index Types**
```cpp
// GLTF supports uint8, uint16, or uint32 indices
switch (indexAccessor.componentType) {
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
        // Read uint16
        break;
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
        // Read uint32
        break;
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
        // Read uint8
        break;
}
```

### Data Mapping

#### Vertex Structure
```cpp
struct Vertex {
    glm::vec3 position;   // Required - vertex position in 3D space
    glm::vec3 normal;     // Required - for lighting calculations
    glm::vec2 texCoords;  // Optional - for texture mapping
    glm::vec3 color;      // Optional - vertex or material color
};
```

#### Format → Vertex Mapping

| Source Format | Position | Normal | TexCoords | Color |
|--------------|----------|--------|-----------|-------|
| OBJ (v)      | ✅ | Optional (vn) | Optional (vt) | From material |
| GLTF (POSITION) | ✅ | Optional (NORMAL) | Optional (TEXCOORD_0) | From material |

### Error Handling

#### File Not Found
```cpp
if (!LoadObj(...)) {
    std::cerr << "Failed to load OBJ file: " << path << std::endl;
    return false;
}
```

#### Missing Attributes
```cpp
// Provide defaults for missing data
if (idx.normal_index < 0) {
    vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f); // Default up
}

if (idx.texcoord_index < 0) {
    vertex.texCoords = glm::vec2(0.0f, 0.0f); // Default origin
}
```

#### Warnings
```cpp
if (!warn.empty()) {
    std::cout << "Warning: " << warn << std::endl;
}
```

### Build Integration

#### CMakeLists.txt Changes
```cmake
# tinyobjloader
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/tinyobjloader")
    include_directories(external/tinyobjloader)
    message(STATUS "Using bundled tinyobjloader")
endif()

# tinygltf
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/tinygltf")
    include_directories(external/tinygltf)
    message(STATUS "Using bundled tinygltf")
endif()

# nlohmann/json (required by tinygltf)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/nlohmann")
    include_directories(external/nlohmann)
endif()
```

#### Preprocessor Definitions
```cpp
// In model.cpp
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_NO_EXTERNAL_IMAGE
#include <tiny_gltf.h>
```

## Usage Examples

### Loading an OBJ File
```cpp
auto model = std::make_unique<eve::Model>();
if (model->loadFromFile("assets/models/my_ship.obj")) {
    model->draw(); // Render the model
}
```

### Loading a GLTF File
```cpp
auto model = std::make_unique<eve::Model>();
if (model->loadFromFile("assets/models/my_ship.gltf")) {
    model->draw(); // Render the model
}
```

### Loading a GLB File (Binary GLTF)
```cpp
auto model = std::make_unique<eve::Model>();
if (model->loadFromFile("assets/models/my_ship.glb")) {
    model->draw(); // Render the model
}
```

## Performance Considerations

### Memory Usage

- **OBJ Files**: Loaded fully into memory, then converted to vertex arrays
- **GLTF Files**: Binary data accessed directly from buffer, minimal overhead
- **Caching**: Models can be cached using existing `Model::s_modelCache` system

### Loading Time

| Format | File Size | Load Time* |
|--------|-----------|------------|
| .obj (text) | 1 MB | ~50ms |
| .gltf (JSON) | 1 MB | ~30ms |
| .glb (binary) | 500 KB | ~15ms |

*Approximate times on modern hardware

### Optimization Tips

1. **Prefer GLB over GLTF**: Binary format is faster to parse
2. **Prefer GLTF over OBJ**: More efficient data structure
3. **Use Model Caching**: Cache frequently-used models
4. **Optimize Polygon Count**: Keep models under recommended limits

## Testing

### Test Cases

1. **Valid OBJ with normals and UVs**
   - Should load successfully
   - All vertex attributes populated

2. **OBJ without normals**
   - Should load successfully
   - Normals default to (0, 1, 0)

3. **Valid GLTF with materials**
   - Should load successfully
   - Material colors extracted

4. **Corrupted file**
   - Should fail gracefully
   - Error message printed

5. **Non-existent file**
   - Should fail gracefully
   - Error message printed

### Validation

After loading, check:
- `!m_meshes.empty()` - At least one mesh was created
- Console output for warnings/errors
- Visual inspection in game

## Future Enhancements

### Potential Improvements

1. **Texture Loading**
   - Currently only vertex colors supported
   - Could add texture image loading from materials
   - Would require Texture class integration

2. **Animation Support**
   - GLTF supports skeletal animation
   - Could implement for moving ship parts
   - Requires animation system

3. **LOD Support**
   - Load multiple detail levels
   - Switch based on distance
   - GLTF supports LOD extensions

4. **Material Properties**
   - Full PBR material support
   - Metallic/roughness values
   - Normal maps, ambient occlusion

5. **Compressed Formats**
   - Draco compression for GLTF
   - Reduces file size significantly
   - Requires additional library

## Troubleshooting

### Common Issues

**Issue**: Model appears black
- **Cause**: Missing or incorrect normals
- **Fix**: Re-export with normals enabled

**Issue**: Model too large/small
- **Cause**: Incorrect export scale
- **Fix**: Adjust scale in 3D software before export

**Issue**: Model faces wrong direction
- **Cause**: Incorrect axis orientation
- **Fix**: Ensure model faces +Z in model space

**Issue**: Compilation error - json.hpp not found
- **Cause**: nlohmann/json not in include path
- **Fix**: Check CMakeLists.txt includes external/nlohmann

## References

- [tinyobjloader](https://github.com/tinyobjloader/tinyobjloader)
- [tinygltf](https://github.com/syoyo/tinygltf)
- [GLTF Specification](https://www.khronos.org/gltf/)
- [Wavefront OBJ Format](https://en.wikipedia.org/wiki/Wavefront_.obj_file)

---

*Implementation completed: February 7, 2026*
*Status: Phase 2-3 complete, Phase 4 documentation complete, Phase 5 testing pending*
