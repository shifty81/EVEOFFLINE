#include "rendering/ship_part_library.h"
#include "rendering/model.h"
#include <cmath>
#include <iostream>

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

// ==================== Faction-Specific Part Creation ====================

void ShipPartLibrary::createMinmatarParts(const glm::vec4& primary, const glm::vec4& secondary, const glm::vec4& accent) {
    // Minmatar: Asymmetric, rustic, exposed framework
    
    // Forward hull - angular nose
    ShipPart forward = createConePart(0.3f, 1.0f, 6, primary, ShipPartType::HULL_FORWARD);
    forward.name = "Minmatar Angular Nose";
    forward.faction = "Minmatar";
    forward.isSymmetric = false;
    forward.attachmentPoint = glm::vec3(-1.0f, 0.0f, 0.0f);
    addPart("minmatar_forward_1", forward);
    
    // Main hull - boxy with exposed framework
    ShipPart main = createBoxPart(glm::vec3(2.0f, 0.8f, 0.6f), primary, ShipPartType::HULL_MAIN);
    main.name = "Minmatar Industrial Hull";
    main.faction = "Minmatar";
    main.isSymmetric = false;
    main.attachmentPoint = glm::vec3(0.0f, 0.0f, 0.0f);
    addPart("minmatar_main_1", main);
    
    // Rear hull - engine mount
    ShipPart rear = createBoxPart(glm::vec3(0.8f, 0.6f, 0.5f), secondary, ShipPartType::HULL_REAR);
    rear.name = "Minmatar Engine Mount";
    rear.faction = "Minmatar";
    rear.isSymmetric = false;
    rear.attachmentPoint = glm::vec3(1.0f, 0.0f, 0.0f);
    addPart("minmatar_rear_1", rear);
    
    // Engine - cylindrical exhausts
    ShipPart engine = createCylinderPart(0.2f, 0.5f, 8, accent, ShipPartType::ENGINE_MAIN);
    engine.name = "Minmatar Engine Exhaust";
    engine.faction = "Minmatar";
    engine.isSymmetric = true;
    addPart("minmatar_engine_1", engine);
}

void ShipPartLibrary::createCaldariParts(const glm::vec4& primary, const glm::vec4& secondary, const glm::vec4& accent) {
    // Caldari: Blocky, angular, industrial
    
    // Forward hull - blocky nose
    ShipPart forward = createBoxPart(glm::vec3(1.2f, 0.6f, 0.5f), primary, ShipPartType::HULL_FORWARD);
    forward.name = "Caldari Blocky Nose";
    forward.faction = "Caldari";
    forward.isSymmetric = true;
    forward.attachmentPoint = glm::vec3(-1.2f, 0.0f, 0.0f);
    addPart("caldari_forward_1", forward);
    
    // Main hull - rectangular city-block style
    ShipPart main = createBoxPart(glm::vec3(2.5f, 1.0f, 0.8f), primary, ShipPartType::HULL_MAIN);
    main.name = "Caldari Industrial Hull";
    main.faction = "Caldari";
    main.isSymmetric = true;
    main.attachmentPoint = glm::vec3(0.0f, 0.0f, 0.0f);
    addPart("caldari_main_1", main);
    
    // Rear hull - squared engine section
    ShipPart rear = createBoxPart(glm::vec3(1.0f, 0.8f, 0.6f), secondary, ShipPartType::HULL_REAR);
    rear.name = "Caldari Engine Section";
    rear.faction = "Caldari";
    rear.isSymmetric = true;
    rear.attachmentPoint = glm::vec3(1.25f, 0.0f, 0.0f);
    addPart("caldari_rear_1", rear);
    
    // Engine - square exhausts
    ShipPart engine = createBoxPart(glm::vec3(0.4f, 0.25f, 0.25f), accent, ShipPartType::ENGINE_MAIN);
    engine.name = "Caldari Square Engine";
    engine.faction = "Caldari";
    engine.isSymmetric = true;
    addPart("caldari_engine_1", engine);
}

void ShipPartLibrary::createGallenteParts(const glm::vec4& primary, const glm::vec4& secondary, const glm::vec4& accent) {
    // Gallente: Organic, smooth curves
    
    // Forward hull - rounded nose
    ShipPart forward = createConePart(0.4f, 1.2f, 12, primary, ShipPartType::HULL_FORWARD);
    forward.name = "Gallente Smooth Nose";
    forward.faction = "Gallente";
    forward.isSymmetric = true;
    forward.attachmentPoint = glm::vec3(-1.2f, 0.0f, 0.0f);
    addPart("gallente_forward_1", forward);
    
    // Main hull - organic ellipsoid
    ShipPart main = createCylinderPart(0.5f, 2.5f, 16, primary, ShipPartType::HULL_MAIN);
    main.name = "Gallente Organic Hull";
    main.faction = "Gallente";
    main.isSymmetric = true;
    main.attachmentPoint = glm::vec3(0.0f, 0.0f, 0.0f);
    addPart("gallente_main_1", main);
    
    // Rear hull - curved engine housing
    ShipPart rear = createCylinderPart(0.4f, 1.0f, 12, secondary, ShipPartType::HULL_REAR);
    rear.name = "Gallente Engine Housing";
    rear.faction = "Gallente";
    rear.isSymmetric = true;
    rear.attachmentPoint = glm::vec3(1.25f, 0.0f, 0.0f);
    addPart("gallente_rear_1", rear);
    
    // Engine - rounded exhausts
    ShipPart engine = createCylinderPart(0.15f, 0.4f, 12, accent, ShipPartType::ENGINE_MAIN);
    engine.name = "Gallente Rounded Engine";
    engine.faction = "Gallente";
    engine.isSymmetric = true;
    addPart("gallente_engine_1", engine);
}

void ShipPartLibrary::createAmarrParts(const glm::vec4& primary, const glm::vec4& secondary, const glm::vec4& accent) {
    // Amarr: Golden, ornate, with spires
    
    // Forward hull - cathedral nose with spire
    ShipPart forward = createConePart(0.35f, 1.5f, 8, primary, ShipPartType::HULL_FORWARD);
    forward.name = "Amarr Cathedral Nose";
    forward.faction = "Amarr";
    forward.isSymmetric = true;
    forward.attachmentPoint = glm::vec3(-1.5f, 0.0f, 0.0f);
    addPart("amarr_forward_1", forward);
    
    // Main hull - ornate plated hull
    ShipPart main = createBoxPart(glm::vec3(2.2f, 0.9f, 0.7f), primary, ShipPartType::HULL_MAIN);
    main.name = "Amarr Ornate Hull";
    main.faction = "Amarr";
    main.isSymmetric = true;
    main.attachmentPoint = glm::vec3(0.0f, 0.0f, 0.0f);
    addPart("amarr_main_1", main);
    
    // Rear hull - golden engine section
    ShipPart rear = createBoxPart(glm::vec3(0.9f, 0.75f, 0.6f), secondary, ShipPartType::HULL_REAR);
    rear.name = "Amarr Engine Section";
    rear.faction = "Amarr";
    rear.isSymmetric = true;
    rear.attachmentPoint = glm::vec3(1.1f, 0.0f, 0.0f);
    addPart("amarr_rear_1", rear);
    
    // Engine - golden exhausts
    ShipPart engine = createCylinderPart(0.18f, 0.45f, 8, accent, ShipPartType::ENGINE_MAIN);
    engine.name = "Amarr Golden Engine";
    engine.faction = "Amarr";
    engine.isSymmetric = true;
    addPart("amarr_engine_1", engine);
    
    // Spire ornament - vertical emphasis
    ShipPart spire = createConePart(0.1f, 0.8f, 6, accent, ShipPartType::SPIRE_ORNAMENT);
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

} // namespace eve
