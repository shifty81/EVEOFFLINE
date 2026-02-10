#include "rendering/ship_part_library.h"
#include "rendering/model.h"
#include "rendering/ship_generation_rules.h"
#include "rendering/procedural_mesh_ops.h"
#include "rendering/reference_model_analyzer.h"
#include <cmath>
#include <iostream>
#include <random>

namespace eve {

// Mathematical constants
constexpr float PI = 3.14159265358979323846f;

ShipPartLibrary::ShipPartLibrary() {
}

ShipPartLibrary::~ShipPartLibrary() {
}

void ShipPartLibrary::initialize() {
    std::cout << "Initializing Ship Part Library..." << std::endl;
    
    // Get faction colors (reuse from Model class)
    // Minmatar
    glm::vec4 minPrimary(0.5f, 0.35f, 0.25f, 1.0f);
    glm::vec4 minSecondary(0.3f, 0.2f, 0.15f, 1.0f);
    glm::vec4 minAccent(0.8f, 0.6f, 0.3f, 1.0f);
    
    // Caldari
    glm::vec4 calPrimary(0.35f, 0.45f, 0.55f, 1.0f);
    glm::vec4 calSecondary(0.2f, 0.25f, 0.35f, 1.0f);
    glm::vec4 calAccent(0.5f, 0.7f, 0.9f, 1.0f);
    
    // Gallente
    glm::vec4 galPrimary(0.3f, 0.4f, 0.35f, 1.0f);
    glm::vec4 galSecondary(0.2f, 0.3f, 0.25f, 1.0f);
    glm::vec4 galAccent(0.4f, 0.7f, 0.5f, 1.0f);
    
    // Amarr
    glm::vec4 amaPrimary(0.6f, 0.55f, 0.45f, 1.0f);
    glm::vec4 amaSecondary(0.4f, 0.35f, 0.25f, 1.0f);
    glm::vec4 amaAccent(0.9f, 0.8f, 0.5f, 1.0f);
    
    // Create parts for each faction
    createMinmatarParts(minPrimary, minSecondary, minAccent);
    createCaldariParts(calPrimary, calSecondary, calAccent);
    createGallenteParts(galPrimary, galSecondary, galAccent);
    createAmarrParts(amaPrimary, amaSecondary, amaAccent);
    
    std::cout << "Ship Part Library initialized with " << m_parts.size() << " parts" << std::endl;
}

const ShipPart* ShipPartLibrary::getPart(const std::string& partId) const {
    auto it = m_parts.find(partId);
    if (it != m_parts.end()) {
        return &it->second;
    }
    return nullptr;
}

std::vector<const ShipPart*> ShipPartLibrary::getPartsByType(ShipPartType type, const std::string& faction) const {
    std::vector<const ShipPart*> result;
    for (const auto& pair : m_parts) {
        if (pair.second.type == type && pair.second.faction == faction) {
            result.push_back(&pair.second);
        }
    }
    return result;
}

void ShipPartLibrary::addPart(const std::string& id, const ShipPart& part) {
    m_parts[id] = part;
}

ShipAssemblyConfig ShipPartLibrary::createAssemblyConfig(const std::string& shipClass, const std::string& faction) const {
    ShipAssemblyConfig config;
    config.shipClass = shipClass;
    config.faction = faction;
    
    // Set up assembly based on faction and class
    if (faction == "Minmatar") {
        config.enforceSymmetry = false;
        config.allowAsymmetry = true;
        config.asymmetryFactor = 0.3f;
        config.hullForwardId = "minmatar_forward_1";
        config.hullMainId = "minmatar_main_1";
        config.hullRearId = "minmatar_rear_1";
    } else if (faction == "Caldari") {
        config.enforceSymmetry = true;
        config.allowAsymmetry = false;
        config.asymmetryFactor = 0.0f;
        config.hullForwardId = "caldari_forward_1";
        config.hullMainId = "caldari_main_1";
        config.hullRearId = "caldari_rear_1";
    } else if (faction == "Gallente") {
        config.enforceSymmetry = true;
        config.allowAsymmetry = false;
        config.asymmetryFactor = 0.0f;
        config.hullForwardId = "gallente_forward_1";
        config.hullMainId = "gallente_main_1";
        config.hullRearId = "gallente_rear_1";
    } else if (faction == "Amarr") {
        config.enforceSymmetry = true;
        config.allowAsymmetry = false;
        config.asymmetryFactor = 0.0f;
        config.hullForwardId = "amarr_forward_1";
        config.hullMainId = "amarr_main_1";
        config.hullRearId = "amarr_rear_1";
    }
    
    // Set scale based on ship class
    if (shipClass == "Frigate") {
        config.overallScale = 3.5f;
        config.proportions = glm::vec3(1.0f, 0.25f, 0.2f);
    } else if (shipClass == "Destroyer") {
        config.overallScale = 5.0f;
        config.proportions = glm::vec3(1.0f, 0.14f, 0.12f);
    } else if (shipClass == "Cruiser") {
        config.overallScale = 6.0f;
        config.proportions = glm::vec3(1.0f, 0.3f, 0.2f);
    } else if (shipClass == "Battlecruiser") {
        config.overallScale = 8.5f;
        config.proportions = glm::vec3(1.0f, 0.29f, 0.24f);
    } else if (shipClass == "Battleship") {
        config.overallScale = 12.0f;
        config.proportions = glm::vec3(1.0f, 0.29f, 0.25f);
    } else if (shipClass == "Carrier") {
        config.overallScale = 15.0f;
        config.proportions = glm::vec3(1.0f, 0.4f, 0.2f);
    } else if (shipClass == "Dreadnought") {
        config.overallScale = 12.0f;
        config.proportions = glm::vec3(1.0f, 0.375f, 0.3f);
    } else if (shipClass == "Titan") {
        config.overallScale = 25.0f;
        config.proportions = glm::vec3(1.0f, 0.32f, 0.28f);
    }
    
    return config;
}

ShipAssemblyConfig ShipPartLibrary::createVariedAssemblyConfig(
    const std::string& shipClass, const std::string& faction,
    const ShipVariationParams& variation) const {
    
    // Start from the base configuration
    ShipAssemblyConfig config = createAssemblyConfig(shipClass, faction);
    
    // Set up deterministic RNG
    std::mt19937 rng(variation.seed != 0 ? variation.seed 
                     : static_cast<unsigned int>(std::hash<std::string>{}(shipClass + "|" + faction)));
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
    
    // Look up reference model traits for this faction and class
    ShipGenerationRules rules;
    rules.initialize();
    ReferenceModelTraits factionRef = rules.getFactionReferenceTraits(faction);
    ReferenceModelTraits classRef = rules.getClassReferenceTraits(shipClass);
    
    // Apply proportion jitter within the measured aspect ratio range
    float jitter = variation.proportionJitter;
    if (jitter > 0.0f) {
        // Vary width proportion using the faction's measured L:W range
        float lwRange = factionRef.maxAspectLW - factionRef.minAspectLW;
        float lwOffset = dist(rng) * jitter * lwRange * 0.5f;
        float targetLW = factionRef.avgAspectLW + lwOffset;
        // Adjust width proportion: higher L:W -> narrower width
        float widthFactor = factionRef.avgAspectLW / std::max(targetLW, 1.0f);
        config.proportions.y *= widthFactor;
        
        // Vary height proportion using the faction's L:H ratio
        float lhOffset = dist(rng) * jitter * 0.3f;
        config.proportions.z *= (1.0f + lhOffset);
    }
    
    // Apply scale jitter
    if (variation.scaleJitter > 0.0f) {
        float scaleFactor = 1.0f + dist(rng) * variation.scaleJitter;
        config.overallScale *= scaleFactor;
    }
    
    // Adjust asymmetry factor within faction-appropriate bounds
    if (config.allowAsymmetry && jitter > 0.0f) {
        float asymJitter = std::abs(dist(rng)) * jitter * 0.2f;
        config.asymmetryFactor = std::min(config.asymmetryFactor + asymJitter, 1.0f);
    }
    
    return config;
}

// ==================== Faction-Specific Part Creation ====================

void ShipPartLibrary::createMinmatarParts(const glm::vec4& primary, const glm::vec4& secondary, const glm::vec4& accent) {
    // Minmatar: Asymmetric, rustic, exposed framework
    // Uses extrusion-based hull generation matching the reference project
    
    // Forward hull - angular extruded nose (6-sided, 3 segments, narrow)
    auto fwdMults = generateRadiusMultipliers(3, 0.6f, 101u);
    ShipPart forward = createExtrudedHullPart(6, 3, 0.35f, 0.6f, fwdMults,
                                               0.8f, 1.0f, primary,
                                               ShipPartType::HULL_FORWARD);
    forward.name = "Minmatar Angular Nose";
    forward.faction = "Minmatar";
    forward.isSymmetric = false;
    forward.attachmentPoint = glm::vec3(-1.0f, 0.0f, 0.0f);
    addPart("minmatar_forward_1", forward);
    
    // Main hull - wider industrial body (6-sided, 5 segments)
    auto mainMults = generateRadiusMultipliers(5, 1.0f, 102u);
    ShipPart main = createExtrudedHullPart(6, 5, 0.4f, 1.0f, mainMults,
                                            1.2f, 0.7f, primary,
                                            ShipPartType::HULL_MAIN);
    main.name = "Minmatar Industrial Hull";
    main.faction = "Minmatar";
    main.isSymmetric = false;
    main.attachmentPoint = glm::vec3(0.0f, 0.0f, 0.0f);
    addPart("minmatar_main_1", main);
    
    // Rear hull - engine mount (6-sided, 2 segments)
    auto rearMults = generateRadiusMultipliers(2, 0.8f, 103u);
    ShipPart rear = createExtrudedHullPart(6, 2, 0.3f, 0.8f, rearMults,
                                            0.9f, 0.7f, secondary,
                                            ShipPartType::HULL_REAR);
    rear.name = "Minmatar Engine Mount";
    rear.faction = "Minmatar";
    rear.isSymmetric = false;
    rear.attachmentPoint = glm::vec3(1.0f, 0.0f, 0.0f);
    addPart("minmatar_rear_1", rear);
    
    // Engine - cylindrical exhausts (keep simple primitive for accent)
    ShipPart engine = createCylinderPart(0.2f, 0.5f, 8, accent, ShipPartType::ENGINE_MAIN);
    engine.name = "Minmatar Engine Exhaust";
    engine.faction = "Minmatar";
    engine.isSymmetric = true;
    addPart("minmatar_engine_1", engine);
    
    // Exposed framework panel (beveled detail)
    ShipPart framework = createBeveledPanelPart(4, 0.4f, 0.3f, -0.15f,
                                                 accent, ShipPartType::FRAMEWORK_EXPOSED);
    framework.name = "Minmatar Exposed Framework";
    framework.faction = "Minmatar";
    framework.isSymmetric = false;
    addPart("minmatar_framework_1", framework);
}

void ShipPartLibrary::createCaldariParts(const glm::vec4& primary, const glm::vec4& secondary, const glm::vec4& accent) {
    // Caldari: Blocky, angular, industrial
    // Uses 4-sided extrusion for angular silhouettes
    
    // Forward hull - blocky extruded nose (4-sided, 3 segments)
    auto fwdMults = generateRadiusMultipliers(3, 0.7f, 201u);
    ShipPart forward = createExtrudedHullPart(4, 3, 0.4f, 0.7f, fwdMults,
                                               1.0f, 0.8f, primary,
                                               ShipPartType::HULL_FORWARD);
    forward.name = "Caldari Blocky Nose";
    forward.faction = "Caldari";
    forward.isSymmetric = true;
    forward.attachmentPoint = glm::vec3(-1.2f, 0.0f, 0.0f);
    addPart("caldari_forward_1", forward);
    
    // Main hull - rectangular body (4-sided, 6 segments)
    auto mainMults = generateRadiusMultipliers(6, 1.0f, 202u);
    ShipPart main = createExtrudedHullPart(4, 6, 0.45f, 1.0f, mainMults,
                                            1.3f, 0.9f, primary,
                                            ShipPartType::HULL_MAIN);
    main.name = "Caldari Industrial Hull";
    main.faction = "Caldari";
    main.isSymmetric = true;
    main.attachmentPoint = glm::vec3(0.0f, 0.0f, 0.0f);
    addPart("caldari_main_1", main);
    
    // Rear hull - squared engine section (4-sided, 2 segments)
    auto rearMults = generateRadiusMultipliers(2, 0.9f, 203u);
    ShipPart rear = createExtrudedHullPart(4, 2, 0.35f, 0.9f, rearMults,
                                            1.1f, 0.8f, secondary,
                                            ShipPartType::HULL_REAR);
    rear.name = "Caldari Engine Section";
    rear.faction = "Caldari";
    rear.isSymmetric = true;
    rear.attachmentPoint = glm::vec3(1.25f, 0.0f, 0.0f);
    addPart("caldari_rear_1", rear);
    
    // Engine - square exhausts (keep primitive for accent)
    ShipPart engine = createBoxPart(glm::vec3(0.4f, 0.25f, 0.25f), accent, ShipPartType::ENGINE_MAIN);
    engine.name = "Caldari Square Engine";
    engine.faction = "Caldari";
    engine.isSymmetric = true;
    addPart("caldari_engine_1", engine);
    
    // Panel detail (beveled angular panel)
    ShipPart panel = createBeveledPanelPart(4, 0.3f, 0.25f, -0.1f,
                                             accent, ShipPartType::PANEL_DETAIL);
    panel.name = "Caldari Angular Panel";
    panel.faction = "Caldari";
    panel.isSymmetric = true;
    addPart("caldari_panel_1", panel);
}

void ShipPartLibrary::createGallenteParts(const glm::vec4& primary, const glm::vec4& secondary, const glm::vec4& accent) {
    // Gallente: Organic, smooth curves
    // Uses high-sided extrusion (12-sided) for smooth organic shapes
    
    // Forward hull - smooth extruded nose (12-sided, 3 segments)
    auto fwdMults = generateRadiusMultipliers(3, 0.6f, 301u);
    ShipPart forward = createExtrudedHullPart(12, 3, 0.4f, 0.6f, fwdMults,
                                               1.0f, 1.0f, primary,
                                               ShipPartType::HULL_FORWARD);
    forward.name = "Gallente Smooth Nose";
    forward.faction = "Gallente";
    forward.isSymmetric = true;
    forward.attachmentPoint = glm::vec3(-1.2f, 0.0f, 0.0f);
    addPart("gallente_forward_1", forward);
    
    // Main hull - organic ellipsoidal body (12-sided, 6 segments)
    auto mainMults = generateRadiusMultipliers(6, 1.0f, 302u);
    ShipPart main = createExtrudedHullPart(12, 6, 0.45f, 1.0f, mainMults,
                                            1.1f, 0.9f, primary,
                                            ShipPartType::HULL_MAIN);
    main.name = "Gallente Organic Hull";
    main.faction = "Gallente";
    main.isSymmetric = true;
    main.attachmentPoint = glm::vec3(0.0f, 0.0f, 0.0f);
    addPart("gallente_main_1", main);
    
    // Rear hull - curved engine housing (12-sided, 2 segments)
    auto rearMults = generateRadiusMultipliers(2, 0.8f, 303u);
    ShipPart rear = createExtrudedHullPart(12, 2, 0.35f, 0.8f, rearMults,
                                            1.0f, 0.9f, secondary,
                                            ShipPartType::HULL_REAR);
    rear.name = "Gallente Engine Housing";
    rear.faction = "Gallente";
    rear.isSymmetric = true;
    rear.attachmentPoint = glm::vec3(1.25f, 0.0f, 0.0f);
    addPart("gallente_rear_1", rear);
    
    // Engine - rounded exhausts (keep primitive for accent)
    ShipPart engine = createCylinderPart(0.15f, 0.4f, 12, accent, ShipPartType::ENGINE_MAIN);
    engine.name = "Gallente Rounded Engine";
    engine.faction = "Gallente";
    engine.isSymmetric = true;
    addPart("gallente_engine_1", engine);
}

void ShipPartLibrary::createAmarrParts(const glm::vec4& primary, const glm::vec4& secondary, const glm::vec4& accent) {
    // Amarr: Golden, ornate, with spires
    // Uses 8-sided extrusion for balanced angular-but-refined shapes
    
    // Forward hull - cathedral extruded nose (8-sided, 4 segments)
    auto fwdMults = generateRadiusMultipliers(4, 0.5f, 401u);
    ShipPart forward = createExtrudedHullPart(8, 4, 0.4f, 0.5f, fwdMults,
                                               0.9f, 1.0f, primary,
                                               ShipPartType::HULL_FORWARD);
    forward.name = "Amarr Cathedral Nose";
    forward.faction = "Amarr";
    forward.isSymmetric = true;
    forward.attachmentPoint = glm::vec3(-1.5f, 0.0f, 0.0f);
    addPart("amarr_forward_1", forward);
    
    // Main hull - ornate plated body (8-sided, 5 segments)
    auto mainMults = generateRadiusMultipliers(5, 1.0f, 402u);
    ShipPart main = createExtrudedHullPart(8, 5, 0.45f, 1.0f, mainMults,
                                            1.1f, 0.8f, primary,
                                            ShipPartType::HULL_MAIN);
    main.name = "Amarr Ornate Hull";
    main.faction = "Amarr";
    main.isSymmetric = true;
    main.attachmentPoint = glm::vec3(0.0f, 0.0f, 0.0f);
    addPart("amarr_main_1", main);
    
    // Rear hull - golden engine section (8-sided, 2 segments)
    auto rearMults = generateRadiusMultipliers(2, 0.8f, 403u);
    ShipPart rear = createExtrudedHullPart(8, 2, 0.35f, 0.8f, rearMults,
                                            1.0f, 0.85f, secondary,
                                            ShipPartType::HULL_REAR);
    rear.name = "Amarr Engine Section";
    rear.faction = "Amarr";
    rear.isSymmetric = true;
    rear.attachmentPoint = glm::vec3(1.1f, 0.0f, 0.0f);
    addPart("amarr_rear_1", rear);
    
    // Engine - golden exhausts (keep primitive for accent)
    ShipPart engine = createCylinderPart(0.18f, 0.45f, 8, accent, ShipPartType::ENGINE_MAIN);
    engine.name = "Amarr Golden Engine";
    engine.faction = "Amarr";
    engine.isSymmetric = true;
    addPart("amarr_engine_1", engine);
    
    // Spire ornament - vertical pyramid emphasis
    ShipPart spire = createPyramidDetailPart(6, 0.15f, 0.8f, accent,
                                              ShipPartType::SPIRE_ORNAMENT);
    spire.name = "Amarr Decorative Spire";
    spire.faction = "Amarr";
    spire.isSymmetric = true;
    addPart("amarr_spire_1", spire);
}

// ==================== Geometric Primitive Helpers ====================

ShipPart ShipPartLibrary::createBoxPart(const glm::vec3& size, const glm::vec4& color, ShipPartType type) {
    ShipPart part;
    part.type = type;
    
    float hx = size.x * 0.5f;
    float hy = size.y * 0.5f;
    float hz = size.z * 0.5f;
    
    glm::vec3 col3(color.r, color.g, color.b);
    
    // Create box vertices (8 corners)
    part.vertices = {
        // Front face (Z+)
        {{-hx, -hy, hz}, {0, 0, 1}, {}, col3},
        {{hx, -hy, hz}, {0, 0, 1}, {}, col3},
        {{hx, hy, hz}, {0, 0, 1}, {}, col3},
        {{-hx, hy, hz}, {0, 0, 1}, {}, col3},
        // Back face (Z-)
        {{-hx, -hy, -hz}, {0, 0, -1}, {}, col3},
        {{hx, -hy, -hz}, {0, 0, -1}, {}, col3},
        {{hx, hy, -hz}, {0, 0, -1}, {}, col3},
        {{-hx, hy, -hz}, {0, 0, -1}, {}, col3},
    };
    
    // Create box indices (12 triangles)
    part.indices = {
        // Front
        0, 1, 2, 0, 2, 3,
        // Back
        5, 4, 7, 5, 7, 6,
        // Top
        3, 2, 6, 3, 6, 7,
        // Bottom
        4, 5, 1, 4, 1, 0,
        // Right
        1, 5, 6, 1, 6, 2,
        // Left
        4, 0, 3, 4, 3, 7
    };
    
    return part;
}

ShipPart ShipPartLibrary::createCylinderPart(float radius, float length, int segments, const glm::vec4& color, ShipPartType type) {
    ShipPart part;
    part.type = type;
    
    glm::vec3 col3(color.r, color.g, color.b);
    
    // Create cylinder vertices
    for (int i = 0; i <= segments; ++i) {
        float angle = (2.0f * PI * i) / segments;
        float x = radius * cos(angle);
        float z = radius * sin(angle);
        glm::vec3 normal = glm::normalize(glm::vec3(0, x, z));
        
        // Front cap
        part.vertices.push_back({{-length * 0.5f, x, z}, normal, {}, col3});
        // Back cap
        part.vertices.push_back({{length * 0.5f, x, z}, normal, {}, col3});
    }
    
    // Create cylinder indices
    for (int i = 0; i < segments; ++i) {
        int base = i * 2;
        part.indices.push_back(base);
        part.indices.push_back(base + 2);
        part.indices.push_back(base + 1);
        
        part.indices.push_back(base + 1);
        part.indices.push_back(base + 2);
        part.indices.push_back(base + 3);
    }
    
    return part;
}

ShipPart ShipPartLibrary::createConePart(float radius, float length, int segments, const glm::vec4& color, ShipPartType type) {
    ShipPart part;
    part.type = type;
    
    glm::vec3 col3(color.r, color.g, color.b);
    
    // Tip of cone
    part.vertices.push_back({{length, 0, 0}, {1, 0, 0}, {}, col3});
    
    // Base circle
    for (int i = 0; i <= segments; ++i) {
        float angle = (2.0f * PI * i) / segments;
        float y = radius * cos(angle);
        float z = radius * sin(angle);
        glm::vec3 normal = glm::normalize(glm::vec3(0.5f, y, z));
        part.vertices.push_back({{0, y, z}, normal, {}, col3});
    }
    
    // Create cone indices
    for (int i = 1; i <= segments; ++i) {
        part.indices.push_back(0);
        part.indices.push_back(i);
        part.indices.push_back(i + 1);
    }
    
    return part;
}

// ==================== Extrusion-Based Part Creation ====================

ShipPart ShipPartLibrary::createExtrudedHullPart(int sides, int segments,
                                                  float segmentLength, float baseRadius,
                                                  const std::vector<float>& radiusMultipliers,
                                                  float scaleX, float scaleZ,
                                                  const glm::vec4& color, ShipPartType type) {
    ShipPart part;
    part.type = type;

    glm::vec3 col3(color.r, color.g, color.b);
    TriangulatedMesh mesh = buildSegmentedHull(sides, segments, segmentLength,
                                               baseRadius, radiusMultipliers,
                                               scaleX, scaleZ, col3);
    part.vertices = std::move(mesh.vertices);
    part.indices  = std::move(mesh.indices);
    return part;
}

ShipPart ShipPartLibrary::createBeveledPanelPart(int sides, float radius,
                                                  float borderSize, float depth,
                                                  const glm::vec4& color,
                                                  ShipPartType type) {
    ShipPart part;
    part.type = type;

    glm::vec3 col3(color.r, color.g, color.b);

    PolyFace face = generatePolygonFace(sides, radius);
    auto beveledFaces = bevelCutFace(face, borderSize, depth);
    TriangulatedMesh mesh = triangulateFaces(beveledFaces, col3);

    part.vertices = std::move(mesh.vertices);
    part.indices  = std::move(mesh.indices);
    return part;
}

ShipPart ShipPartLibrary::createPyramidDetailPart(int sides, float radius,
                                                   float height,
                                                   const glm::vec4& color,
                                                   ShipPartType type) {
    ShipPart part;
    part.type = type;

    glm::vec3 col3(color.r, color.g, color.b);

    PolyFace face = generatePolygonFace(sides, radius);
    auto pyramidFaces = pyramidizeFace(face, height);
    TriangulatedMesh mesh = triangulateFaces(pyramidFaces, col3);

    part.vertices = std::move(mesh.vertices);
    part.indices  = std::move(mesh.indices);
    return part;
}

// ==================== Learned Model Part Creation ====================

void ShipPartLibrary::createPartsFromLearnedModels(const ReferenceModelAnalyzer& analyzer,
                                                    const std::string& faction,
                                                    const std::string& partIdPrefix) {
    if (analyzer.getModelCount() == 0) {
        std::cerr << "[ShipPartLibrary] No analyzed models to learn from" << std::endl;
        return;
    }

    // Determine faction color scheme
    glm::vec4 primary, secondary, accent;
    if (faction == "Minmatar") {
        primary   = glm::vec4(0.5f, 0.35f, 0.25f, 1.0f);
        secondary = glm::vec4(0.3f, 0.2f, 0.15f, 1.0f);
        accent    = glm::vec4(0.8f, 0.6f, 0.3f, 1.0f);
    } else if (faction == "Caldari") {
        primary   = glm::vec4(0.35f, 0.45f, 0.55f, 1.0f);
        secondary = glm::vec4(0.2f, 0.25f, 0.35f, 1.0f);
        accent    = glm::vec4(0.5f, 0.7f, 0.9f, 1.0f);
    } else if (faction == "Gallente") {
        primary   = glm::vec4(0.3f, 0.4f, 0.35f, 1.0f);
        secondary = glm::vec4(0.2f, 0.3f, 0.25f, 1.0f);
        accent    = glm::vec4(0.4f, 0.7f, 0.5f, 1.0f);
    } else if (faction == "Amarr") {
        primary   = glm::vec4(0.6f, 0.55f, 0.45f, 1.0f);
        secondary = glm::vec4(0.4f, 0.35f, 0.25f, 1.0f);
        accent    = glm::vec4(0.9f, 0.8f, 0.5f, 1.0f);
    } else {
        // Default neutral grey for unknown factions
        primary   = glm::vec4(0.4f, 0.4f, 0.45f, 1.0f);
        secondary = glm::vec4(0.3f, 0.3f, 0.35f, 1.0f);
        accent    = glm::vec4(0.6f, 0.6f, 0.7f, 1.0f);
    }

    // Determine polygon sides from faction style
    int factionSides = 6;
    if (faction == "Caldari")  factionSides = 4;
    if (faction == "Amarr")    factionSides = 8;
    if (faction == "Gallente") factionSides = 12;

    auto params = analyzer.computeLearnedParams();

    std::cout << "[ShipPartLibrary] Creating parts from " << params.modelCount
              << " learned models (faction=" << faction << ")" << std::endl;

    // --- Forward hull (learned from first third of cross-section) ---
    {
        int fwdSegments = 3;
        auto fwdMults = analyzer.generateLearnedRadiusMultipliers(fwdSegments, 101u);
        float fwdScale = (params.blendedProfile.size() > 1) ? params.blendedProfile[1] : 0.6f;

        ShipPart forward = createExtrudedHullPart(
            factionSides, fwdSegments, 0.35f,
            params.avgBaseRadius * fwdScale, fwdMults,
            params.avgAspectLW / 2.0f, 1.0f,
            primary, ShipPartType::HULL_FORWARD);
        forward.name = partIdPrefix + faction + " Learned Forward Hull";
        forward.faction = faction;
        forward.isSymmetric = (faction != "Minmatar");
        forward.attachmentPoint = glm::vec3(-1.0f, 0.0f, 0.0f);
        addPart(partIdPrefix + faction + "_forward_1", forward);
    }

    // --- Main hull (learned from middle section of cross-section) ---
    {
        int mainSegments = 5;
        auto mainMults = analyzer.generateLearnedRadiusMultipliers(mainSegments, 102u);

        ShipPart main = createExtrudedHullPart(
            factionSides, mainSegments, 0.4f,
            params.avgBaseRadius, mainMults,
            params.avgAspectLW / 2.0f, 0.8f,
            primary, ShipPartType::HULL_MAIN);
        main.name = partIdPrefix + faction + " Learned Main Hull";
        main.faction = faction;
        main.isSymmetric = (faction != "Minmatar");
        main.attachmentPoint = glm::vec3(0.0f, 0.0f, 0.0f);
        addPart(partIdPrefix + faction + "_main_1", main);
    }

    // --- Rear hull (learned from last third of cross-section) ---
    {
        int rearSegments = 2;
        auto rearMults = analyzer.generateLearnedRadiusMultipliers(rearSegments, 103u);
        float rearScale = (params.blendedProfile.size() > 2)
            ? params.blendedProfile[params.blendedProfile.size() - 2] : 0.8f;

        ShipPart rear = createExtrudedHullPart(
            factionSides, rearSegments, 0.3f,
            params.avgBaseRadius * rearScale, rearMults,
            params.avgAspectLW / 2.5f, 0.7f,
            secondary, ShipPartType::HULL_REAR);
        rear.name = partIdPrefix + faction + " Learned Rear Hull";
        rear.faction = faction;
        rear.isSymmetric = (faction != "Minmatar");
        rear.attachmentPoint = glm::vec3(1.0f, 0.0f, 0.0f);
        addPart(partIdPrefix + faction + "_rear_1", rear);
    }

    // --- Engine (keep simple geometry, accent color) ---
    {
        ShipPart engine = createCylinderPart(
            0.18f, 0.45f, factionSides, accent, ShipPartType::ENGINE_MAIN);
        engine.name = partIdPrefix + faction + " Learned Engine";
        engine.faction = faction;
        engine.isSymmetric = true;
        addPart(partIdPrefix + faction + "_engine_1", engine);
    }

    // --- Panel detail (beveled, derived from learned detail density) ---
    {
        float detailScale = params.avgBaseRadius * 0.4f;
        ShipPart panel = createBeveledPanelPart(
            factionSides, detailScale, 0.3f, -0.12f,
            accent, ShipPartType::PANEL_DETAIL);
        panel.name = partIdPrefix + faction + " Learned Panel";
        panel.faction = faction;
        panel.isSymmetric = (faction != "Minmatar");
        addPart(partIdPrefix + faction + "_panel_1", panel);
    }

    std::cout << "[ShipPartLibrary] Created learned parts with prefix '"
              << partIdPrefix << "'" << std::endl;
}

} // namespace eve
