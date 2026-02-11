#include "rendering/station_renderer.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <cmath>
#include <iostream>

namespace eve {

StationRenderer::StationRenderer() {
}

StationRenderer::~StationRenderer() {
    clearStations();
}

bool StationRenderer::initialize() {
    std::cout << "[StationRenderer] Initializing..." << std::endl;
    
    // Initialize faction visual properties
    initializeFactionVisuals();
    
    // Create procedural station meshes
    createStationMeshes();
    
    if (m_factionStationMeshes.empty() && m_upwellMeshes.empty()) {
        std::cerr << "[StationRenderer] Failed to create station meshes" << std::endl;
        return false;
    }
    
    std::cout << "[StationRenderer] Initialized with " 
              << m_factionStationMeshes.size() << " faction stations and "
              << m_upwellMeshes.size() << " Upwell structures" << std::endl;
    
    return true;
}

void StationRenderer::initializeFactionVisuals() {
    // Solari - Cathedral style with golden colors
    FactionVisuals solari;
    solari.primaryColor = glm::vec3(0.8f, 0.6f, 0.2f);
    solari.secondaryColor = glm::vec3(0.9f, 0.8f, 0.5f);
    solari.accentColor = glm::vec3(1.0f, 0.85f, 0.3f);
    solari.emissiveColor = glm::vec3(1.0f, 0.8f, 0.3f);
    solari.emissiveIntensity = 2.0f;
    solari.metallic = 0.7f;
    solari.roughness = 0.3f;
    solari.sizeMultiplier = 1.2f;
    m_factionVisuals[FactionStyle::SOLARI] = solari;
    
    // Veyren - Industrial blocky design
    FactionVisuals veyren;
    veyren.primaryColor = glm::vec3(0.4f, 0.45f, 0.5f);
    veyren.secondaryColor = glm::vec3(0.5f, 0.55f, 0.6f);
    veyren.accentColor = glm::vec3(0.2f, 0.4f, 0.6f);
    veyren.emissiveColor = glm::vec3(0.3f, 0.5f, 0.8f);
    veyren.emissiveIntensity = 1.5f;
    veyren.metallic = 0.8f;
    veyren.roughness = 0.5f;
    veyren.sizeMultiplier = 1.0f;
    m_factionVisuals[FactionStyle::VEYREN] = veyren;
    
    // Aurelian - Organic spherical design
    FactionVisuals aurelian;
    aurelian.primaryColor = glm::vec3(0.2f, 0.4f, 0.3f);
    aurelian.secondaryColor = glm::vec3(0.3f, 0.6f, 0.5f);
    aurelian.accentColor = glm::vec3(0.4f, 0.8f, 0.6f);
    aurelian.emissiveColor = glm::vec3(0.3f, 0.8f, 0.5f);
    aurelian.emissiveIntensity = 1.8f;
    aurelian.metallic = 0.5f;
    aurelian.roughness = 0.4f;
    aurelian.sizeMultiplier = 1.1f;
    m_factionVisuals[FactionStyle::AURELIAN] = aurelian;
    
    // Keldari - Rusty improvised design
    FactionVisuals keldari;
    keldari.primaryColor = glm::vec3(0.4f, 0.3f, 0.25f);
    keldari.secondaryColor = glm::vec3(0.5f, 0.4f, 0.3f);
    keldari.accentColor = glm::vec3(0.6f, 0.3f, 0.2f);
    keldari.emissiveColor = glm::vec3(0.8f, 0.4f, 0.2f);
    keldari.emissiveIntensity = 1.2f;
    keldari.metallic = 0.6f;
    keldari.roughness = 0.8f;
    keldari.sizeMultiplier = 0.9f;
    m_factionVisuals[FactionStyle::KELDARI] = keldari;
}

const StationRenderer::FactionVisuals& StationRenderer::getFactionVisuals(FactionStyle faction) const {
    auto it = m_factionVisuals.find(faction);
    if (it != m_factionVisuals.end()) {
        return it->second;
    }
    // Return Veyren as default
    static FactionVisuals defaultVisuals;
    return defaultVisuals;
}

void StationRenderer::createStationMeshes() {
    std::cout << "[StationRenderer] Creating procedural station meshes..." << std::endl;
    
    // Create faction stations
    m_factionStationMeshes[FactionStyle::SOLARI] = createSolariStation();
    m_factionStationMeshes[FactionStyle::VEYREN] = createVeyrenStation();
    m_factionStationMeshes[FactionStyle::AURELIAN] = createAurelianStation();
    m_factionStationMeshes[FactionStyle::KELDARI] = createKeldariStation();
    
    // Create Upwell structures
    m_upwellMeshes[UpwellType::ASTRAHUS] = createAstrahus();
    m_upwellMeshes[UpwellType::FORTIZAR] = createFortizar();
    m_upwellMeshes[UpwellType::KEEPSTAR] = createKeepstar();
    m_upwellMeshes[UpwellType::RAITARU] = createRaitaru();
    
    std::cout << "[StationRenderer] Created station meshes" << std::endl;
}

std::shared_ptr<Mesh> StationRenderer::createSolariStation() {
    // Solari: Cathedral style with golden spires
    // Main structure: central cylinder with spires on top
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    
    // Central cylinder (main body)
    const float bodyRadius = 500.0f;
    const float bodyHeight = 1000.0f;
    const int segments = 16;
    
    // Create cylinder vertices
    for (int i = 0; i <= segments; i++) {
        float theta = (float)i / segments * 2.0f * glm::pi<float>();
        float x = bodyRadius * cos(theta);
        float z = bodyRadius * sin(theta);
        
        // Bottom vertex
        Vertex vBottom;
        vBottom.position = glm::vec3(x, 0.0f, z);
        vBottom.normal = glm::normalize(glm::vec3(x, 0.0f, z));
        vBottom.texCoords = glm::vec2((float)i / segments, 0.0f);
        vBottom.color = glm::vec4(0.8f, 0.6f, 0.2f, 1.0f); // Golden color
        vertices.push_back(vBottom);
        
        // Top vertex
        Vertex vTop;
        vTop.position = glm::vec3(x, bodyHeight, z);
        vTop.normal = glm::normalize(glm::vec3(x, 0.0f, z));
        vTop.texCoords = glm::vec2((float)i / segments, 1.0f);
        vTop.color = glm::vec4(0.9f, 0.8f, 0.5f, 1.0f);
        vertices.push_back(vTop);
    }
    
    // Create cylinder indices
    for (int i = 0; i < segments; i++) {
        int base = i * 2;
        // Triangle 1
        indices.push_back(base);
        indices.push_back(base + 2);
        indices.push_back(base + 1);
        // Triangle 2
        indices.push_back(base + 1);
        indices.push_back(base + 2);
        indices.push_back(base + 3);
    }
    
    // Add spires on top (4 spires around the central axis)
    const float spireRadius = 100.0f;
    const float spireHeight = 600.0f;
    const float spireDistance = 300.0f;
    
    for (int s = 0; s < 4; s++) {
        float angle = s * glm::pi<float>() / 2.0f;
        float spireX = spireDistance * cos(angle);
        float spireZ = spireDistance * sin(angle);
        glm::vec3 spireBase(spireX, bodyHeight, spireZ);
        glm::vec3 spireTop(spireX, bodyHeight + spireHeight, spireZ);
        
        unsigned int baseIdx = vertices.size();
        
        // Cone for spire
        for (int i = 0; i <= 8; i++) {
            float theta = (float)i / 8 * 2.0f * glm::pi<float>();
            float x = spireRadius * cos(theta);
            float z = spireRadius * sin(theta);
            
            Vertex vBase;
            vBase.position = spireBase + glm::vec3(x, 0.0f, z);
            vBase.normal = glm::normalize(glm::vec3(x, spireHeight, z));
            vBase.color = glm::vec4(1.0f, 0.85f, 0.3f, 1.0f); // Bright gold for spires
            vertices.push_back(vBase);
        }
        
        // Add tip vertex
        Vertex vTip;
        vTip.position = spireTop;
        vTip.normal = glm::vec3(0.0f, 1.0f, 0.0f);
        vTip.color = glm::vec4(1.0f, 0.9f, 0.4f, 1.0f);
        unsigned int tipIdx = vertices.size();
        vertices.push_back(vTip);
        
        // Create cone triangles
        for (int i = 0; i < 8; i++) {
            indices.push_back(baseIdx + i);
            indices.push_back(baseIdx + i + 1);
            indices.push_back(tipIdx);
        }
    }
    
    return std::make_shared<Mesh>(vertices, indices);
}

std::shared_ptr<Mesh> StationRenderer::createVeyrenStation() {
    // Veyren: Industrial blocky design
    // Multiple connected boxes forming a city-block structure
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    
    // Central large box
    const float mainWidth = 800.0f;
    const float mainHeight = 600.0f;
    const float mainDepth = 800.0f;
    
    // Helper to add a box at position
    auto addBox = [&](glm::vec3 center, float w, float h, float d, glm::vec4 color) {
        unsigned int baseIdx = vertices.size();
        
        // 8 vertices of the box
        glm::vec3 corners[8] = {
            center + glm::vec3(-w/2, -h/2, -d/2),
            center + glm::vec3( w/2, -h/2, -d/2),
            center + glm::vec3( w/2,  h/2, -d/2),
            center + glm::vec3(-w/2,  h/2, -d/2),
            center + glm::vec3(-w/2, -h/2,  d/2),
            center + glm::vec3( w/2, -h/2,  d/2),
            center + glm::vec3( w/2,  h/2,  d/2),
            center + glm::vec3(-w/2,  h/2,  d/2)
        };
        
        // Add vertices for each face
        // Front face
        vertices.push_back({corners[0], glm::vec3(0, 0, -1), glm::vec2(0, 0), color});
        vertices.push_back({corners[1], glm::vec3(0, 0, -1), glm::vec2(1, 0), color});
        vertices.push_back({corners[2], glm::vec3(0, 0, -1), glm::vec2(1, 1), color});
        vertices.push_back({corners[3], glm::vec3(0, 0, -1), glm::vec2(0, 1), color});
        
        // Back face
        vertices.push_back({corners[5], glm::vec3(0, 0, 1), glm::vec2(0, 0), color});
        vertices.push_back({corners[4], glm::vec3(0, 0, 1), glm::vec2(1, 0), color});
        vertices.push_back({corners[7], glm::vec3(0, 0, 1), glm::vec2(1, 1), color});
        vertices.push_back({corners[6], glm::vec3(0, 0, 1), glm::vec2(0, 1), color});
        
        // Right face
        vertices.push_back({corners[1], glm::vec3(1, 0, 0), glm::vec2(0, 0), color});
        vertices.push_back({corners[5], glm::vec3(1, 0, 0), glm::vec2(1, 0), color});
        vertices.push_back({corners[6], glm::vec3(1, 0, 0), glm::vec2(1, 1), color});
        vertices.push_back({corners[2], glm::vec3(1, 0, 0), glm::vec2(0, 1), color});
        
        // Left face
        vertices.push_back({corners[4], glm::vec3(-1, 0, 0), glm::vec2(0, 0), color});
        vertices.push_back({corners[0], glm::vec3(-1, 0, 0), glm::vec2(1, 0), color});
        vertices.push_back({corners[3], glm::vec3(-1, 0, 0), glm::vec2(1, 1), color});
        vertices.push_back({corners[7], glm::vec3(-1, 0, 0), glm::vec2(0, 1), color});
        
        // Top face
        vertices.push_back({corners[3], glm::vec3(0, 1, 0), glm::vec2(0, 0), color});
        vertices.push_back({corners[2], glm::vec3(0, 1, 0), glm::vec2(1, 0), color});
        vertices.push_back({corners[6], glm::vec3(0, 1, 0), glm::vec2(1, 1), color});
        vertices.push_back({corners[7], glm::vec3(0, 1, 0), glm::vec2(0, 1), color});
        
        // Bottom face
        vertices.push_back({corners[4], glm::vec3(0, -1, 0), glm::vec2(0, 0), color});
        vertices.push_back({corners[5], glm::vec3(0, -1, 0), glm::vec2(1, 0), color});
        vertices.push_back({corners[1], glm::vec3(0, -1, 0), glm::vec2(1, 1), color});
        vertices.push_back({corners[0], glm::vec3(0, -1, 0), glm::vec2(0, 1), color});
        
        // Add indices for all 6 faces (2 triangles per face)
        for (int face = 0; face < 6; face++) {
            unsigned int faceBase = baseIdx + face * 4;
            indices.push_back(faceBase);
            indices.push_back(faceBase + 1);
            indices.push_back(faceBase + 2);
            indices.push_back(faceBase);
            indices.push_back(faceBase + 2);
            indices.push_back(faceBase + 3);
        }
    };
    
    // Main central box
    addBox(glm::vec3(0, 0, 0), mainWidth, mainHeight, mainDepth, 
           glm::vec4(0.4f, 0.45f, 0.5f, 1.0f));
    
    // Add smaller connected modules
    addBox(glm::vec3(500, 200, 0), 400, 300, 400, 
           glm::vec4(0.5f, 0.55f, 0.6f, 1.0f));
    addBox(glm::vec3(-500, 200, 0), 400, 300, 400, 
           glm::vec4(0.5f, 0.55f, 0.6f, 1.0f));
    addBox(glm::vec3(0, 200, 500), 400, 300, 400, 
           glm::vec4(0.5f, 0.55f, 0.6f, 1.0f));
    
    // Add antenna/tower on top
    addBox(glm::vec3(0, 500, 0), 100, 400, 100, 
           glm::vec4(0.2f, 0.4f, 0.6f, 1.0f));
    
    return std::make_shared<Mesh>(vertices, indices);
}

std::shared_ptr<Mesh> StationRenderer::createAurelianStation() {
    // Aurelian: Organic spherical design with green-blue colors
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    
    // Create a large central sphere
    const float mainRadius = 600.0f;
    const int segments = 20;
    const int rings = 16;
    
    for (int ring = 0; ring <= rings; ring++) {
        float phi = glm::pi<float>() * (float)ring / rings;
        float y = mainRadius * cos(phi);
        float ringRadius = mainRadius * sin(phi);
        
        for (int seg = 0; seg <= segments; seg++) {
            float theta = 2.0f * glm::pi<float>() * (float)seg / segments;
            float x = ringRadius * cos(theta);
            float z = ringRadius * sin(theta);
            
            Vertex v;
            v.position = glm::vec3(x, y, z);
            v.normal = glm::normalize(v.position);
            v.texCoords = glm::vec2((float)seg / segments, (float)ring / rings);
            v.color = glm::vec4(0.2f, 0.4f, 0.3f, 0.8f); // Semi-transparent green-blue
            vertices.push_back(v);
        }
    }
    
    // Create indices for sphere
    for (int ring = 0; ring < rings; ring++) {
        for (int seg = 0; seg < segments; seg++) {
            int current = ring * (segments + 1) + seg;
            int next = current + segments + 1;
            
            indices.push_back(current);
            indices.push_back(next);
            indices.push_back(current + 1);
            
            indices.push_back(current + 1);
            indices.push_back(next);
            indices.push_back(next + 1);
        }
    }
    
    // Add smaller spheres connected (organic design)
    for (int i = 0; i < 4; i++) {
        float angle = i * glm::pi<float>() / 2.0f;
        float distance = 500.0f;
        glm::vec3 offset(distance * cos(angle), 0.0f, distance * sin(angle));
        float smallRadius = 200.0f;
        
        unsigned int baseIdx = vertices.size();
        
        for (int ring = 0; ring <= 8; ring++) {
            float phi = glm::pi<float>() * (float)ring / 8;
            float y = smallRadius * cos(phi);
            float ringRadius = smallRadius * sin(phi);
            
            for (int seg = 0; seg <= 12; seg++) {
                float theta = 2.0f * glm::pi<float>() * (float)seg / 12;
                float x = ringRadius * cos(theta);
                float z = ringRadius * sin(theta);
                
                Vertex v;
                v.position = offset + glm::vec3(x, y, z);
                v.normal = glm::normalize(glm::vec3(x, y, z));
                v.color = glm::vec4(0.3f, 0.6f, 0.5f, 0.8f);
                vertices.push_back(v);
            }
        }
        
        for (int ring = 0; ring < 8; ring++) {
            for (int seg = 0; seg < 12; seg++) {
                int current = baseIdx + ring * 13 + seg;
                int next = current + 13;
                
                indices.push_back(current);
                indices.push_back(next);
                indices.push_back(current + 1);
                
                indices.push_back(current + 1);
                indices.push_back(next);
                indices.push_back(next + 1);
            }
        }
    }
    
    return std::make_shared<Mesh>(vertices, indices);
}

std::shared_ptr<Mesh> StationRenderer::createKeldariStation() {
    // Keldari: Rusty scaffolding, improvised, asymmetric
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    
    // Create irregular scaffolding structure using cylinders
    auto addCylinder = [&](glm::vec3 start, glm::vec3 end, float radius, glm::vec4 color) {
        unsigned int baseIdx = vertices.size();
        glm::vec3 dir = end - start;
        float length = glm::length(dir);
        glm::vec3 up = glm::normalize(dir);
        
        // Find perpendicular vector
        glm::vec3 right = glm::vec3(1, 0, 0);
        if (abs(glm::dot(up, right)) > 0.9f) {
            right = glm::vec3(0, 1, 0);
        }
        right = glm::normalize(glm::cross(up, right));
        glm::vec3 forward = glm::cross(right, up);
        
        const int segments = 8;
        for (int i = 0; i <= segments; i++) {
            float theta = (float)i / segments * 2.0f * glm::pi<float>();
            float x = radius * cos(theta);
            float z = radius * sin(theta);
            glm::vec3 offset = right * x + forward * z;
            
            Vertex vStart;
            vStart.position = start + offset;
            vStart.normal = glm::normalize(offset);
            vStart.color = color;
            vertices.push_back(vStart);
            
            Vertex vEnd;
            vEnd.position = end + offset;
            vEnd.normal = glm::normalize(offset);
            vEnd.color = color;
            vertices.push_back(vEnd);
        }
        
        for (int i = 0; i < segments; i++) {
            int base = baseIdx + i * 2;
            indices.push_back(base);
            indices.push_back(base + 2);
            indices.push_back(base + 1);
            
            indices.push_back(base + 1);
            indices.push_back(base + 2);
            indices.push_back(base + 3);
        }
    };
    
    // Main vertical support beams (irregular)
    glm::vec4 rustyColor(0.4f, 0.3f, 0.25f, 1.0f);
    glm::vec4 metalColor(0.5f, 0.4f, 0.3f, 1.0f);
    
    addCylinder(glm::vec3(-300, 0, -300), glm::vec3(-250, 800, -280), 50, rustyColor);
    addCylinder(glm::vec3(300, 0, -300), glm::vec3(280, 750, -320), 50, rustyColor);
    addCylinder(glm::vec3(-300, 0, 300), glm::vec3(-320, 780, 300), 50, rustyColor);
    addCylinder(glm::vec3(300, 0, 300), glm::vec3(290, 820, 280), 50, rustyColor);
    
    // Horizontal connecting beams
    addCylinder(glm::vec3(-250, 400, -280), glm::vec3(280, 400, -320), 30, metalColor);
    addCylinder(glm::vec3(-320, 400, 300), glm::vec3(290, 400, 280), 30, metalColor);
    addCylinder(glm::vec3(-250, 700, -280), glm::vec3(-320, 700, 300), 30, metalColor);
    addCylinder(glm::vec3(280, 700, -320), glm::vec3(290, 700, 280), 30, metalColor);
    
    // Central habitation module (box)
    unsigned int baseIdx = vertices.size();
    glm::vec3 center(0, 400, 0);
    float w = 400, h = 300, d = 400;
    glm::vec4 moduleColor(0.6f, 0.3f, 0.2f, 1.0f);
    
    glm::vec3 corners[8] = {
        center + glm::vec3(-w/2, -h/2, -d/2),
        center + glm::vec3( w/2, -h/2, -d/2),
        center + glm::vec3( w/2,  h/2, -d/2),
        center + glm::vec3(-w/2,  h/2, -d/2),
        center + glm::vec3(-w/2, -h/2,  d/2),
        center + glm::vec3( w/2, -h/2,  d/2),
        center + glm::vec3( w/2,  h/2,  d/2),
        center + glm::vec3(-w/2,  h/2,  d/2)
    };
    
    // Add box faces (simplified)
    for (int face = 0; face < 6; face++) {
        for (int i = 0; i < 4; i++) {
            Vertex v;
            v.color = moduleColor;
            vertices.push_back(v);
        }
    }
    
    return std::make_shared<Mesh>(vertices, indices);
}

std::shared_ptr<Mesh> StationRenderer::createAstrahus() {
    // Astrahus: Medium citadel with modular design
    // Simplified X-shaped structure with central core
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    
    // Central octagonal core
    const float coreRadius = 300.0f;
    const float coreHeight = 800.0f;
    const int segments = 8;
    glm::vec4 hullColor(0.5f, 0.5f, 0.55f, 1.0f);
    glm::vec4 glowColor(0.3f, 0.6f, 1.0f, 1.0f);
    
    for (int i = 0; i <= segments; i++) {
        float theta = (float)i / segments * 2.0f * glm::pi<float>();
        float x = coreRadius * cos(theta);
        float z = coreRadius * sin(theta);
        
        Vertex vBottom;
        vBottom.position = glm::vec3(x, -coreHeight/2, z);
        vBottom.normal = glm::normalize(glm::vec3(x, 0, z));
        vBottom.color = hullColor;
        vertices.push_back(vBottom);
        
        Vertex vTop;
        vTop.position = glm::vec3(x, coreHeight/2, z);
        vTop.normal = glm::normalize(glm::vec3(x, 0, z));
        vTop.color = hullColor;
        vertices.push_back(vTop);
    }
    
    for (int i = 0; i < segments; i++) {
        int base = i * 2;
        indices.push_back(base);
        indices.push_back(base + 2);
        indices.push_back(base + 1);
        
        indices.push_back(base + 1);
        indices.push_back(base + 2);
        indices.push_back(base + 3);
    }
    
    // Add 4 docking arms extending from center
    for (int arm = 0; arm < 4; arm++) {
        float angle = arm * glm::pi<float>() / 2.0f;
        glm::vec3 direction(cos(angle), 0, sin(angle));
        glm::vec3 armStart = direction * coreRadius;
        glm::vec3 armEnd = direction * (coreRadius + 500.0f);
        
        unsigned int baseIdx = vertices.size();
        const float armRadius = 80.0f;
        
        for (int i = 0; i <= 6; i++) {
            float theta = (float)i / 6 * 2.0f * glm::pi<float>();
            glm::vec3 perp1 = glm::normalize(glm::cross(direction, glm::vec3(0, 1, 0)));
            glm::vec3 perp2 = glm::cross(direction, perp1);
            glm::vec3 offset = (perp1 * cos(theta) + perp2 * sin(theta)) * armRadius;
            
            Vertex vStart;
            vStart.position = armStart + offset;
            vStart.normal = glm::normalize(offset);
            vStart.color = glowColor;
            vertices.push_back(vStart);
            
            Vertex vEnd;
            vEnd.position = armEnd + offset;
            vEnd.normal = glm::normalize(offset);
            vEnd.color = glowColor;
            vertices.push_back(vEnd);
        }
        
        for (int i = 0; i < 6; i++) {
            int base = baseIdx + i * 2;
            indices.push_back(base);
            indices.push_back(base + 2);
            indices.push_back(base + 1);
            
            indices.push_back(base + 1);
            indices.push_back(base + 2);
            indices.push_back(base + 3);
        }
    }
    
    return std::make_shared<Mesh>(vertices, indices);
}

std::shared_ptr<Mesh> StationRenderer::createFortizar() {
    // Fortizar: Larger version of Astrahus with more arms
    // Similar to Astrahus but scaled up with 8 arms instead of 4
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    
    const float coreRadius = 500.0f;
    const float coreHeight = 1200.0f;
    const int segments = 12;
    glm::vec4 hullColor(0.5f, 0.5f, 0.55f, 1.0f);
    
    // Central core (similar to Astrahus but larger)
    for (int i = 0; i <= segments; i++) {
        float theta = (float)i / segments * 2.0f * glm::pi<float>();
        float x = coreRadius * cos(theta);
        float z = coreRadius * sin(theta);
        
        Vertex vBottom;
        vBottom.position = glm::vec3(x, -coreHeight/2, z);
        vBottom.normal = glm::normalize(glm::vec3(x, 0, z));
        vBottom.color = hullColor;
        vertices.push_back(vBottom);
        
        Vertex vTop;
        vTop.position = glm::vec3(x, coreHeight/2, z);
        vTop.normal = glm::normalize(glm::vec3(x, 0, z));
        vTop.color = hullColor;
        vertices.push_back(vTop);
    }
    
    for (int i = 0; i < segments; i++) {
        int base = i * 2;
        indices.push_back(base);
        indices.push_back(base + 2);
        indices.push_back(base + 1);
        
        indices.push_back(base + 1);
        indices.push_back(base + 2);
        indices.push_back(base + 3);
    }
    
    return std::make_shared<Mesh>(vertices, indices);
}

std::shared_ptr<Mesh> StationRenderer::createKeepstar() {
    // Keepstar: Massive XL citadel
    // Ring structure with massive central sphere
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    
    const float sphereRadius = 1000.0f;
    const int segments = 24;
    const int rings = 20;
    glm::vec4 hullColor(0.5f, 0.5f, 0.55f, 1.0f);
    
    // Central massive sphere
    for (int ring = 0; ring <= rings; ring++) {
        float phi = glm::pi<float>() * (float)ring / rings;
        float y = sphereRadius * cos(phi);
        float ringRadius = sphereRadius * sin(phi);
        
        for (int seg = 0; seg <= segments; seg++) {
            float theta = 2.0f * glm::pi<float>() * (float)seg / segments;
            float x = ringRadius * cos(theta);
            float z = ringRadius * sin(theta);
            
            Vertex v;
            v.position = glm::vec3(x, y, z);
            v.normal = glm::normalize(v.position);
            v.color = hullColor;
            vertices.push_back(v);
        }
    }
    
    for (int ring = 0; ring < rings; ring++) {
        for (int seg = 0; seg < segments; seg++) {
            int current = ring * (segments + 1) + seg;
            int next = current + segments + 1;
            
            indices.push_back(current);
            indices.push_back(next);
            indices.push_back(current + 1);
            
            indices.push_back(current + 1);
            indices.push_back(next);
            indices.push_back(next + 1);
        }
    }
    
    return std::make_shared<Mesh>(vertices, indices);
}

std::shared_ptr<Mesh> StationRenderer::createRaitaru() {
    // Raitaru: Engineering complex with industrial design
    // Multiple connected cylinders forming a modular structure
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    
    const float mainRadius = 250.0f;
    const float mainHeight = 600.0f;
    const int segments = 12;
    glm::vec4 industrialColor(0.6f, 0.6f, 0.65f, 1.0f);
    
    // Main central cylinder
    for (int i = 0; i <= segments; i++) {
        float theta = (float)i / segments * 2.0f * glm::pi<float>();
        float x = mainRadius * cos(theta);
        float z = mainRadius * sin(theta);
        
        Vertex vBottom;
        vBottom.position = glm::vec3(x, 0, z);
        vBottom.normal = glm::normalize(glm::vec3(x, 0, z));
        vBottom.color = industrialColor;
        vertices.push_back(vBottom);
        
        Vertex vTop;
        vTop.position = glm::vec3(x, mainHeight, z);
        vTop.normal = glm::normalize(glm::vec3(x, 0, z));
        vTop.color = industrialColor;
        vertices.push_back(vTop);
    }
    
    for (int i = 0; i < segments; i++) {
        int base = i * 2;
        indices.push_back(base);
        indices.push_back(base + 2);
        indices.push_back(base + 1);
        
        indices.push_back(base + 1);
        indices.push_back(base + 2);
        indices.push_back(base + 3);
    }
    
    return std::make_shared<Mesh>(vertices, indices);
}

void StationRenderer::addStation(const StationInstance& station) {
    m_stations.push_back(station);
}

void StationRenderer::removeStation(const std::string& id) {
    m_stations.erase(
        std::remove_if(m_stations.begin(), m_stations.end(),
            [&id](const StationInstance& s) { return s.id == id; }),
        m_stations.end()
    );
}

void StationRenderer::clearStations() {
    m_stations.clear();
}

void StationRenderer::render(Shader* shader, const Camera& camera) {
    if (!shader) return;
    
    shader->use();
    shader->setMat4("view", camera.getViewMatrix());
    shader->setMat4("projection", camera.getProjectionMatrix());
    
    for (const auto& station : m_stations) {
        // Get the appropriate mesh
        std::shared_ptr<Mesh> mesh;
        if (station.isUpwell) {
            auto it = m_upwellMeshes.find(station.upwellType);
            if (it != m_upwellMeshes.end()) {
                mesh = it->second;
            }
        } else {
            auto it = m_factionStationMeshes.find(station.faction);
            if (it != m_factionStationMeshes.end()) {
                mesh = it->second;
            }
        }
        
        if (!mesh) continue;
        
        // Build model matrix
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, station.position);
        model = glm::rotate(model, station.rotation.y, glm::vec3(0, 1, 0));
        model = glm::rotate(model, station.rotation.x, glm::vec3(1, 0, 0));
        model = glm::rotate(model, station.rotation.z, glm::vec3(0, 0, 1));
        model = glm::scale(model, glm::vec3(station.scale));
        
        shader->setMat4("model", model);
        
        // Set material properties based on faction
        if (!station.isUpwell) {
            const FactionVisuals& visuals = getFactionVisuals(station.faction);
            shader->setVec3("material.albedo", visuals.primaryColor);
            shader->setFloat("material.metallic", visuals.metallic);
            shader->setFloat("material.roughness", visuals.roughness);
            shader->setVec3("material.emissive", visuals.emissiveColor);
            shader->setFloat("material.emissiveIntensity", visuals.emissiveIntensity);
        }
        
        // Render the mesh
        mesh->draw();
    }
}

} // namespace eve
