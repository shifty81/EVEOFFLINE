#include "rendering/asteroid_field_renderer.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <cmath>
#include <iostream>
#include <algorithm>

namespace atlas {

AsteroidFieldRenderer::AsteroidFieldRenderer()
    : m_fieldCenter(0.0f, 0.0f, 0.0f)
{
}

AsteroidFieldRenderer::~AsteroidFieldRenderer() {
    clearField();
}

bool AsteroidFieldRenderer::initialize() {
    std::cout << "[AsteroidFieldRenderer] Initializing..." << std::endl;
    
    // Create instanced renderer
    m_renderer = std::make_unique<InstancedRenderer>();
    
    // Create procedural asteroid meshes
    createAsteroidMeshes();
    
    // Register meshes with instanced renderer
    if (m_asteroidMeshes.empty()) {
        std::cerr << "[AsteroidFieldRenderer] Failed to create asteroid meshes" << std::endl;
        return false;
    }
    
    // Register each mesh type (max 5000 instances per mesh)
    for (size_t i = 0; i < m_asteroidMeshes.size(); i++) {
        std::string meshName = "asteroid_" + std::to_string(i);
        m_renderer->registerMesh(meshName, m_asteroidMeshes[i], 5000);
    }
    
    std::cout << "[AsteroidFieldRenderer] Initialized with " 
              << m_asteroidMeshes.size() << " asteroid mesh types" << std::endl;
    
    return true;
}

void AsteroidFieldRenderer::createAsteroidMeshes() {
    std::cout << "[AsteroidFieldRenderer] Creating procedural asteroid meshes..." << std::endl;
    
    // Create 3 different asteroid mesh types with varying detail
    // Type 0: Low detail (for distant/small asteroids)
    m_asteroidMeshes.push_back(createAsteroidMesh(1, 0.2f, 100));
    
    // Type 1: Medium detail (for medium asteroids)
    m_asteroidMeshes.push_back(createAsteroidMesh(2, 0.3f, 200));
    
    // Type 2: High detail (for large/huge asteroids)
    m_asteroidMeshes.push_back(createAsteroidMesh(2, 0.35f, 300));
    
    std::cout << "[AsteroidFieldRenderer] Created " << m_asteroidMeshes.size() 
              << " asteroid mesh types" << std::endl;
}

std::shared_ptr<Mesh> AsteroidFieldRenderer::createAsteroidMesh(
    int subdivisions,
    float displacement,
    int seed
) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    
    // Create icosphere as base shape
    createIcosphere(vertices, indices, subdivisions);
    
    // Apply random displacement for rocky appearance
    displaceVertices(vertices, displacement, seed);
    
    // Recalculate normals after displacement
    for (size_t i = 0; i < indices.size(); i += 3) {
        Vertex& v0 = vertices[indices[i]];
        Vertex& v1 = vertices[indices[i + 1]];
        Vertex& v2 = vertices[indices[i + 2]];
        
        glm::vec3 edge1 = v1.position - v0.position;
        glm::vec3 edge2 = v2.position - v0.position;
        glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));
        
        v0.normal = normal;
        v1.normal = normal;
        v2.normal = normal;
    }
    
    return std::make_shared<Mesh>(vertices, indices);
}

void AsteroidFieldRenderer::createIcosphere(
    std::vector<Vertex>& vertices,
    std::vector<unsigned int>& indices,
    int subdivisions
) {
    // Create icosahedron base
    const float t = (1.0f + std::sqrt(5.0f)) / 2.0f;
    
    // 12 vertices of icosahedron
    std::vector<glm::vec3> positions = {
        {-1, t, 0}, {1, t, 0}, {-1, -t, 0}, {1, -t, 0},
        {0, -1, t}, {0, 1, t}, {0, -1, -t}, {0, 1, -t},
        {t, 0, -1}, {t, 0, 1}, {-t, 0, -1}, {-t, 0, 1}
    };
    
    // Normalize positions to unit sphere
    for (auto& pos : positions) {
        pos = glm::normalize(pos);
    }
    
    // 20 faces of icosahedron
    std::vector<glm::uvec3> faces = {
        {0, 11, 5}, {0, 5, 1}, {0, 1, 7}, {0, 7, 10}, {0, 10, 11},
        {1, 5, 9}, {5, 11, 4}, {11, 10, 2}, {10, 7, 6}, {7, 1, 8},
        {3, 9, 4}, {3, 4, 2}, {3, 2, 6}, {3, 6, 8}, {3, 8, 9},
        {4, 9, 5}, {2, 4, 11}, {6, 2, 10}, {8, 6, 7}, {9, 8, 1}
    };
    
    // Subdivide faces (simple - no subdivision for now, can add later)
    // For now, just use the icosahedron directly
    
    // Convert to vertices
    for (const auto& pos : positions) {
        Vertex v;
        v.position = pos;
        v.normal = glm::normalize(pos); // For sphere, normal = position
        v.texCoords = glm::vec2(0.5f, 0.5f);
        v.color = glm::vec3(0.6f, 0.5f, 0.4f); // Gray/brown asteroid color
        vertices.push_back(v);
    }
    
    // Convert faces to indices
    for (const auto& face : faces) {
        indices.push_back(face.x);
        indices.push_back(face.y);
        indices.push_back(face.z);
    }
}

void AsteroidFieldRenderer::displaceVertices(
    std::vector<Vertex>& vertices,
    float amount,
    int seed
) {
    for (size_t i = 0; i < vertices.size(); i++) {
        // Use hash for deterministic randomness
        float r1 = hash(seed, i * 3);
        float r2 = hash(seed, i * 3 + 1);
        float r3 = hash(seed, i * 3 + 2);
        
        // Displace along normal direction
        glm::vec3 displacement = vertices[i].normal * amount * (r1 - 0.5f) * 2.0f;
        
        // Add some tangential displacement for irregularity
        glm::vec3 tangent = glm::normalize(glm::cross(vertices[i].normal, glm::vec3(0, 1, 0)));
        if (glm::length(tangent) < 0.01f) {
            tangent = glm::normalize(glm::cross(vertices[i].normal, glm::vec3(1, 0, 0)));
        }
        glm::vec3 bitangent = glm::cross(vertices[i].normal, tangent);
        
        displacement += tangent * amount * 0.3f * (r2 - 0.5f) * 2.0f;
        displacement += bitangent * amount * 0.3f * (r3 - 0.5f) * 2.0f;
        
        vertices[i].position += displacement;
    }
}

float AsteroidFieldRenderer::hash(int seed, int index) const {
    // Simple hash function for deterministic randomness
    unsigned int h = seed ^ (index * 2654435761u);
    h ^= (h >> 16);
    h *= 0x85ebca6bu;
    h ^= (h >> 13);
    h *= 0xc2b2ae35u;
    h ^= (h >> 16);
    
    return (h & 0xFFFFFF) / float(0xFFFFFF);
}

void AsteroidFieldRenderer::generateField(
    const glm::vec3& center,
    float radius,
    const std::vector<int>& asteroidCounts,
    BeltLayout layout,
    int seed
) {
    std::cout << "[AsteroidFieldRenderer] Generating asteroid field..." << std::endl;
    std::cout << "[AsteroidFieldRenderer] Center: (" << center.x << ", " 
              << center.y << ", " << center.z << ")" << std::endl;
    std::cout << "[AsteroidFieldRenderer] Radius: " << radius << "m" << std::endl;
    
    // Clear existing field
    clearField();
    
    m_fieldCenter = center;
    
    // Generate asteroids for each size category
    const AsteroidSize sizes[] = {
        AsteroidSize::SMALL,
        AsteroidSize::MEDIUM,
        AsteroidSize::LARGE,
        AsteroidSize::HUGE
    };
    
    int asteroidIndex = 0;
    for (size_t sizeIdx = 0; sizeIdx < std::min(asteroidCounts.size(), size_t(4)); sizeIdx++) {
        AsteroidSize size = sizes[sizeIdx];
        int count = asteroidCounts[sizeIdx];
        
        std::cout << "[AsteroidFieldRenderer] Generating " << count 
                  << " asteroids of size " << static_cast<int>(size) << std::endl;
        
        for (int i = 0; i < count; i++) {
            // Generate position
            glm::vec3 position = generatePosition(center, radius, layout, seed, asteroidIndex);
            
            // Choose mesh type based on size
            int meshType;
            if (size == AsteroidSize::SMALL) {
                meshType = 0; // Low detail
            } else if (size == AsteroidSize::MEDIUM) {
                meshType = 1; // Medium detail
            } else {
                meshType = 2; // High detail
            }
            
            std::string meshName = "asteroid_" + std::to_string(meshType);
            
            // Get scale for size
            float scale = getSizeScale(size);
            
            // Add random scale variation (±20%)
            float scaleVar = hash(seed, asteroidIndex * 7) * 0.4f + 0.8f;
            scale *= scaleVar;
            
            // Random rotation
            float rotation = hash(seed, asteroidIndex * 11) * 360.0f;
            
            // Get color
            glm::vec4 color = getAsteroidColor(meshType, seed + asteroidIndex);
            
            // Create transform matrix
            glm::mat4 transform = glm::translate(glm::mat4(1.0f), position);
            transform = glm::rotate(transform, glm::radians(rotation), glm::vec3(0, 1, 0));
            transform = glm::scale(transform, glm::vec3(scale));
            
            // Create instance data
            InstanceData instanceData;
            instanceData.transform = transform;
            instanceData.color = color;
            
            // Add to instanced renderer
            int instanceId = m_renderer->addInstance(meshName, instanceData);
            
            // Store asteroid data
            AsteroidInstance asteroid;
            asteroid.instanceId = instanceId;
            asteroid.position = position;
            asteroid.size = size;
            asteroid.meshType = meshName;
            asteroid.scale = scale;
            asteroid.rotation = rotation;
            
            m_asteroids.push_back(asteroid);
            asteroidIndex++;
        }
    }
    
    // Update renderer buffers
    m_renderer->updateBuffers();
    
    std::cout << "[AsteroidFieldRenderer] Generated " << m_asteroids.size() 
              << " asteroids" << std::endl;
}

glm::vec3 AsteroidFieldRenderer::generatePosition(
    const glm::vec3& center,
    float radius,
    BeltLayout layout,
    int seed,
    int index
) {
    glm::vec3 position = center;
    
    if (layout == BeltLayout::SEMICIRCLE) {
        // Semicircle layout (arc) - matching Python implementation
        float angle = hash(seed, index * 2) * glm::pi<float>(); // 0 to 180 degrees
        float distance = (hash(seed, index * 3) * 0.5f + 0.5f) * radius; // 0.5 to 1.0 * radius
        
        position.x += distance * std::cos(angle);
        position.y += distance * std::sin(angle);
        position.z += (hash(seed, index * 5) - 0.5f) * radius * 0.4f; // -0.2 to 0.2 * radius
        
    } else { // SPHERICAL
        // Spherical layout - matching Python implementation
        float theta = hash(seed, index * 2) * 2.0f * glm::pi<float>();
        float phi = (hash(seed, index * 3) - 0.5f) * glm::pi<float>();
        float distance = (hash(seed, index * 5) * 0.7f + 0.3f) * radius; // 0.3 to 1.0 * radius
        
        position.x += distance * std::cos(phi) * std::cos(theta);
        position.y += distance * std::cos(phi) * std::sin(theta);
        position.z += distance * std::sin(phi);
    }
    
    return position;
}

float AsteroidFieldRenderer::getSizeScale(AsteroidSize size) const {
    // Base scales matching asteroid sizes
    // These are in game units (meters in the game world)
    switch (size) {
        case AsteroidSize::SMALL:
            return 50.0f;   // ~50m diameter
        case AsteroidSize::MEDIUM:
            return 150.0f;  // ~150m diameter
        case AsteroidSize::LARGE:
            return 400.0f;  // ~400m diameter
        case AsteroidSize::HUGE:
            return 1000.0f; // ~1000m diameter
        default:
            return 100.0f;
    }
}

glm::vec4 AsteroidFieldRenderer::getAsteroidColor(int meshType, int seed) const {
    // Generate color variations for different asteroid types
    float r1 = hash(seed, 1);
    float r2 = hash(seed, 2);
    
    // Base colors for different ore types (matching EVE)
    std::vector<glm::vec3> baseColors = {
        {0.6f, 0.5f, 0.4f},  // Gray/brown (common ores)
        {0.5f, 0.6f, 0.5f},  // Greenish (some ores)
        {0.4f, 0.45f, 0.55f}, // Bluish (ice/some ores)
        {0.55f, 0.5f, 0.45f}, // Tan (some ores)
    };
    
    int colorIndex = static_cast<int>(r1 * baseColors.size());
    if (colorIndex >= baseColors.size()) colorIndex = baseColors.size() - 1;
    
    glm::vec3 color = baseColors[colorIndex];
    
    // Add some variation
    color.r += (r2 - 0.5f) * 0.1f;
    color.g += (hash(seed, 3) - 0.5f) * 0.1f;
    color.b += (hash(seed, 4) - 0.5f) * 0.1f;
    
    // Clamp to valid range
    color = glm::clamp(color, glm::vec3(0.0f), glm::vec3(1.0f));
    
    return glm::vec4(color, 1.0f);
}

void AsteroidFieldRenderer::clearField() {
    if (!m_renderer) return;
    
    // Remove all instances
    for (const auto& asteroid : m_asteroids) {
        m_renderer->removeInstance(asteroid.instanceId);
    }
    
    bool hadAsteroids = !m_asteroids.empty();
    m_asteroids.clear();
    
    // Update buffers if we removed asteroids
    if (hadAsteroids) {
        m_renderer->updateBuffers();
    }
}

void AsteroidFieldRenderer::render(Shader* shader, const Camera& camera) {
    if (!m_renderer || m_asteroids.empty()) {
        return;
    }
    
    // Render all asteroids using instanced rendering
    m_renderer->renderAll(shader);
}

} // namespace atlas
