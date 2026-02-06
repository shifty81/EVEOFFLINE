# Shadow Mapping Implementation - Phase 3

## Overview

Shadow mapping is a technique for rendering realistic shadows in 3D scenes. This implementation provides high-quality shadows for directional lights (sun/star lighting) with soft edges using Percentage Closer Filtering (PCF).

## Architecture

### Core Components

#### 1. ShadowMap Class (`shadow_map.h`/`shadow_map.cpp`)

The main class that manages shadow map generation and rendering.

**Key Features:**
- Framebuffer with depth texture (configurable resolution, default 2048x2048)
- Shadow pass rendering (depth-only pass from light's perspective)
- Light space matrix calculation for orthographic projection
- Border clamping to white (no shadow outside frustum)

**API:**
```cpp
// Create shadow map
Rendering::ShadowMap shadowMap(2048, 2048);

// Begin shadow pass (render scene to depth texture)
shadowMap.beginShadowPass();
// ... render scene geometry ...
shadowMap.endShadowPass();

// Bind for use in lighting pass
shadowMap.bindShadowTexture(0); // Texture unit 0

// Get light space matrix
glm::mat4 lightSpace = shadowMap.getLightSpaceMatrix(
    lightDir,           // Light direction
    sceneCenter,        // Center of scene
    sceneRadius         // Bounding sphere radius
);
```

#### 2. Shadow Mapping Shaders

**Shadow Map Shaders** (`shadow_map.vert`/`shadow_map.frag`):
- Depth-only rendering from light's perspective
- Minimal fragment shader (depth written automatically)
- Uses light space matrix for projection

**Enhanced Lighting Shaders** (`multi_light_shadow.vert`/`multi_light_shadow.frag`):
- Extended version of multi_light shaders with shadow support
- Vertex shader computes fragment position in light space
- Fragment shader performs shadow lookup and PCF filtering

#### 3. Lighting System Integration

The existing `Lighting::LightManager` already has `castsShadows` flag support in the `Light` structure, making integration seamless.

## Technical Details

### Shadow Map Resolution

- **Default:** 2048x2048 pixels
- **Configurable:** Pass width/height to `ShadowMap` constructor
- **Trade-off:** Higher resolution = better quality but more memory/GPU time

### Shadow Bias

Prevents "shadow acne" (self-shadowing artifacts) by adding a small depth offset.

```cpp
// In shader
float bias = max(shadowBias * (1.0 - dot(normal, lightDir)), shadowBias * 0.1);
```

- **Adaptive:** Larger bias for surfaces parallel to light
- **Configurable:** `shadowBias` uniform (default 0.005)

### Percentage Closer Filtering (PCF)

Soft shadows using 3x3 kernel sampling:

```glsl
// Sample 9 neighboring texels
for(int x = -1; x <= 1; ++x) {
    for(int y = -1; y <= 1; ++y) {
        float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
        shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
    }
}
shadow /= 9.0;
```

**Result:** Smoother shadow edges instead of hard aliasing.

### Polygon Offset

Prevents shadow acne during depth testing:

```cpp
glEnable(GL_POLYGON_OFFSET_FILL);
glPolygonOffset(2.0f, 4.0f);
```

Applied during shadow pass rendering.

## Usage Example

```cpp
// 1. Setup
Rendering::ShadowMap shadowMap(2048, 2048);
Lighting::LightManager lightManager;

// Create directional light with shadows
Lighting::Light sun = Lighting::LightManager::createDirectionalLight(
    glm::vec3(0.5f, -1.0f, -0.3f),  // Direction
    glm::vec3(1.0f, 0.95f, 0.9f),    // Color
    1.0f                              // Intensity
);
sun.castsShadows = true;
lightManager.addLight(sun);

// 2. Render Loop

// Calculate light space matrix
glm::mat4 lightSpaceMatrix = shadowMap.getLightSpaceMatrix(
    sunLight->direction,
    glm::vec3(0.0f, 0.0f, 0.0f),  // Scene center
    150.0f                          // Scene radius
);

// === SHADOW PASS ===
shadowShader.use();
shadowShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

shadowMap.beginShadowPass();
// Render all shadow-casting geometry
renderScene(shadowShader);
shadowMap.endShadowPass();

// === LIGHTING PASS ===
glViewport(0, 0, screenWidth, screenHeight);
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

lightingShader.use();
lightingShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
lightingShader.setBool("useShadows", true);
lightingShader.setFloat("shadowBias", 0.005f);

// Bind shadow map
shadowMap.bindShadowTexture(0);
lightingShader.setInt("shadowMap", 0);

// Upload lights
lightManager.uploadToShader(&lightingShader);

// Render scene with lighting and shadows
renderScene(lightingShader);
```

## Performance Considerations

### Memory Usage

- **2048x2048 shadow map:** ~16 MB (GL_DEPTH_COMPONENT32F)
- **4096x4096 shadow map:** ~64 MB

### GPU Time

- **Shadow pass:** Renders entire scene from light perspective
- **Lighting pass:** Additional texture lookups (9 samples for PCF)

**Optimization Tips:**
- Use lower resolution for distant/less important shadows
- Consider cascaded shadow maps for large scenes
- Limit number of shadow-casting lights (currently: first directional light only)

## Limitations

### Current Implementation

1. **Single Shadow Map:** Only the first directional light casts shadows
2. **No Point/Spot Shadows:** Only directional lights supported
3. **Fixed PCF Kernel:** 3x3 kernel (could be made configurable)
4. **No Soft Shadow Penumbra:** PCF provides edge softening but not realistic penumbra

### Future Enhancements

- **Cascaded Shadow Maps (CSM):** Better shadow quality across large distances
- **Point Light Shadows:** Cube map shadow mapping
- **Spot Light Shadows:** Similar to directional but with perspective projection
- **Variable Penumbra:** Contact-hardening shadows
- **Shadow Map Pooling:** Reuse shadow maps for multiple lights

## Shader Uniforms

### Shadow Map Vertex Shader

```glsl
uniform mat4 lightSpaceMatrix;  // Light projection * view
uniform mat4 model;              // Model matrix
```

### Shadow-Enhanced Lighting Shader

**Vertex Shader:**
```glsl
uniform mat4 lightSpaceMatrix;  // For calculating FragPosLightSpace
```

**Fragment Shader:**
```glsl
uniform sampler2D shadowMap;    // Shadow depth texture
uniform bool useShadows;        // Enable/disable shadows
uniform float shadowBias;       // Shadow acne prevention
```

## Debugging Tips

### Shadow Acne

**Symptoms:** Self-shadowing patterns on surfaces
**Solutions:**
- Increase `shadowBias` (careful: too high causes "peter panning")
- Use `glPolygonOffset` (already implemented)
- Increase shadow map resolution

### Peter Panning

**Symptoms:** Shadows detached from objects
**Solutions:**
- Decrease `shadowBias`
- Use normal offset bias instead of constant bias

### Aliasing/Jagged Edges

**Symptoms:** Pixelated shadow edges
**Solutions:**
- Increase shadow map resolution
- Increase PCF kernel size
- Use better filtering (VSM, ESM)

### No Shadows Visible

**Checklist:**
1. `useShadows` uniform set to true?
2. Light has `castsShadows = true`?
3. Shadow map bound to correct texture unit?
4. Light space matrix calculated correctly?
5. Objects rendered in shadow pass?

## References

- [LearnOpenGL - Shadow Mapping](https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping)
- [GPU Gems - Chapter 11](https://developer.nvidia.com/gpugems/gpugems/part-ii-lighting-and-shadows/chapter-11-shadow-map-antialiasing)
- [Cascaded Shadow Maps](https://docs.microsoft.com/en-us/windows/win32/dxtecharts/cascaded-shadow-maps)

## File Locations

```
cpp_client/
├── include/rendering/shadow_map.h          # Shadow map class
├── src/rendering/shadow_map.cpp            # Shadow map implementation
├── shaders/
│   ├── shadow_map.vert                     # Shadow pass vertex shader
│   ├── shadow_map.frag                     # Shadow pass fragment shader
│   ├── multi_light_shadow.vert             # Lighting with shadows vertex shader
│   └── multi_light_shadow.frag             # Lighting with shadows fragment shader
└── test_shadow_mapping.cpp                 # Test program (WIP)
```

---

**Author:** GitHub Copilot Workspace  
**Date:** February 4, 2026  
**Phase:** 3 (Shadow Mapping & Advanced Rendering)  
**Status:** Core implementation complete, test program pending GL header fix
