#include "rendering/model.h"
#include "rendering/mesh.h"
#include <iostream>
#include <algorithm>
#include <cmath>

namespace eve {

// Mathematical constants
constexpr float PI = 3.14159265358979323846f;

// Initialize static cache
std::map<std::string, std::shared_ptr<Model>> Model::s_modelCache;

Model::Model() {
}

Model::~Model() {
}

bool Model::loadFromFile(const std::string& path) {
    // TODO: Implement model loading from file
    std::cout << "Model loading not yet implemented: " << path << std::endl;
    return false;
}

void Model::addMesh(std::unique_ptr<Mesh> mesh) {
    m_meshes.push_back(std::move(mesh));
}

std::unique_ptr<Model> Model::createShipModel(const std::string& shipType, const std::string& faction) {
    // Check cache
    std::string cacheKey = faction + "_" + shipType;
    auto it = s_modelCache.find(cacheKey);
    if (it != s_modelCache.end()) {
        // Return a copy of cached model
        auto copy = std::make_unique<Model>();
        // TODO: Deep copy meshes
        return copy;
    }

    // Get faction colors
    FactionColors colors = getFactionColors(faction);

    // Create appropriate model based on ship type
    std::unique_ptr<Model> model;
    if (isFrigate(shipType)) {
        model = createFrigateModel(colors);
    } else if (isDestroyer(shipType)) {
        model = createDestroyerModel(colors);
    } else if (isMiningBarge(shipType)) {
        model = createMiningBargeModel(colors);
    } else if (isTech2Cruiser(shipType)) {
        model = createTech2CruiserModel(colors);
    } else if (isCruiser(shipType)) {
        model = createCruiserModel(colors);
    } else if (isBattlecruiser(shipType)) {
        model = createBattlecruiserModel(colors);
    } else if (isBattleship(shipType)) {
        model = createBattleshipModel(colors);
    } else if (isCarrier(shipType)) {
        model = createCarrierModel(colors);
    } else if (isDreadnought(shipType)) {
        model = createDreadnoughtModel(colors);
    } else if (isTitan(shipType)) {
        model = createTitanModel(colors);
    } else if (isStation(shipType)) {
        model = createStationModel(colors, shipType);
    } else if (isAsteroid(shipType)) {
        model = createAsteroidModel(shipType);
    } else {
        model = createGenericModel(colors);
    }

    return model;
}

void Model::draw() const {
    for (const auto& mesh : m_meshes) {
        mesh->draw();
    }
}

// Ship type checking functions
bool Model::isFrigate(const std::string& shipType) {
    static const std::vector<std::string> frigateNames = {
        "Frigate", "Rifter", "Merlin", "Tristan", "Punisher",
        "Assault Frigate", "Jaguar", "Hawk", "Enyo", "Retribution", "Wolf", "Harpy"
    };
    return std::any_of(frigateNames.begin(), frigateNames.end(),
        [&shipType](const std::string& name) { return shipType.find(name) != std::string::npos; });
}

bool Model::isDestroyer(const std::string& shipType) {
    static const std::vector<std::string> destroyerNames = {
        "Destroyer", "Thrasher", "Cormorant", "Catalyst", "Coercer"
    };
    return std::any_of(destroyerNames.begin(), destroyerNames.end(),
        [&shipType](const std::string& name) { return shipType.find(name) != std::string::npos; });
}

bool Model::isCruiser(const std::string& shipType) {
    if (isTech2Cruiser(shipType)) return false;
    static const std::vector<std::string> cruiserNames = {
        "Cruiser", "Stabber", "Caracal", "Vexor", "Maller", "Rupture", "Moa"
    };
    return std::any_of(cruiserNames.begin(), cruiserNames.end(),
        [&shipType](const std::string& name) { return shipType.find(name) != std::string::npos; });
}

bool Model::isTech2Cruiser(const std::string& shipType) {
    static const std::vector<std::string> tech2Names = {
        "Heavy Assault Cruiser", "Vagabond", "Cerberus", "Ishtar", "Zealot",
        "Heavy Interdiction Cruiser", "Broadsword", "Onyx", "Phobos", "Devoter",
        "Force Recon Ship", "Huginn", "Rapier", "Falcon", "Arazu", "Pilgrim",
        "Combat Recon Ship", "Rook", "Lachesis", "Curse",
        "Logistics Cruiser", "Scimitar", "Basilisk", "Oneiros", "Guardian"
    };
    return std::any_of(tech2Names.begin(), tech2Names.end(),
        [&shipType](const std::string& name) { return shipType.find(name) != std::string::npos; });
}

bool Model::isBattlecruiser(const std::string& shipType) {
    static const std::vector<std::string> bcNames = {
        "Battlecruiser", "Cyclone", "Ferox", "Brutix", "Harbinger"
    };
    return std::any_of(bcNames.begin(), bcNames.end(),
        [&shipType](const std::string& name) { return shipType.find(name) != std::string::npos; });
}

bool Model::isBattleship(const std::string& shipType) {
    static const std::vector<std::string> bsNames = {
        "Battleship", "Tempest", "Raven", "Dominix", "Apocalypse"
    };
    return std::any_of(bsNames.begin(), bsNames.end(),
        [&shipType](const std::string& name) { return shipType.find(name) != std::string::npos; });
}

bool Model::isMiningBarge(const std::string& shipType) {
    static const std::vector<std::string> miningNames = {
        "Mining Barge", "Procurer", "Retriever", "Covetor", "Exhumer", "Hulk", "Mackinaw", "Skiff"
    };
    return std::any_of(miningNames.begin(), miningNames.end(),
        [&shipType](const std::string& name) { return shipType.find(name) != std::string::npos; });
}

bool Model::isCarrier(const std::string& shipType) {
    static const std::vector<std::string> carrierNames = {
        "Carrier", "Archon", "Thanatos", "Chimera", "Nidhoggur",
        "Supercarrier", "Hel", "Nyx", "Wyvern", "Aeon"
    };
    return std::any_of(carrierNames.begin(), carrierNames.end(),
        [&shipType](const std::string& name) { return shipType.find(name) != std::string::npos; });
}

bool Model::isDreadnought(const std::string& shipType) {
    static const std::vector<std::string> dreadNames = {
        "Dreadnought", "Revelation", "Moros", "Phoenix", "Naglfar"
    };
    return std::any_of(dreadNames.begin(), dreadNames.end(),
        [&shipType](const std::string& name) { return shipType.find(name) != std::string::npos; });
}

bool Model::isTitan(const std::string& shipType) {
    static const std::vector<std::string> titanNames = {
        "Titan", "Avatar", "Erebus", "Leviathan", "Ragnarok"
    };
    return std::any_of(titanNames.begin(), titanNames.end(),
        [&shipType](const std::string& name) { return shipType.find(name) != std::string::npos; });
}

bool Model::isStation(const std::string& shipType) {
    static const std::vector<std::string> stationNames = {
        "Station", "Citadel", "Keepstar", "Fortizar", "Astrahus",
        "Outpost", "Refinery", "Engineering Complex"
    };
    return std::any_of(stationNames.begin(), stationNames.end(),
        [&shipType](const std::string& name) { return shipType.find(name) != std::string::npos; });
}

bool Model::isAsteroid(const std::string& shipType) {
    static const std::vector<std::string> asteroidNames = {
        "Asteroid", "Veldspar", "Scordite", "Pyroxeres", "Plagioclase",
        "Omber", "Kernite", "Jaspet", "Hemorphite", "Hedbergite",
        "Gneiss", "Dark Ochre", "Crokite", "Bistot", "Arkonor", "Mercoxit"
    };
    return std::any_of(asteroidNames.begin(), asteroidNames.end(),
        [&shipType](const std::string& name) { return shipType.find(name) != std::string::npos; });
}

FactionColors Model::getFactionColors(const std::string& faction) {
    static const std::map<std::string, FactionColors> colorMap = {
        {"Minmatar", {
            glm::vec4(0.5f, 0.35f, 0.25f, 1.0f),  // Rust brown
            glm::vec4(0.3f, 0.2f, 0.15f, 1.0f),   // Dark brown
            glm::vec4(0.8f, 0.6f, 0.3f, 1.0f)     // Light rust
        }},
        {"Caldari", {
            glm::vec4(0.35f, 0.45f, 0.55f, 1.0f), // Steel blue
            glm::vec4(0.2f, 0.25f, 0.35f, 1.0f),  // Dark blue
            glm::vec4(0.5f, 0.7f, 0.9f, 1.0f)     // Light blue
        }},
        {"Gallente", {
            glm::vec4(0.3f, 0.4f, 0.35f, 1.0f),   // Dark green-gray
            glm::vec4(0.2f, 0.3f, 0.25f, 1.0f),   // Darker green
            glm::vec4(0.4f, 0.7f, 0.5f, 1.0f)     // Light green
        }},
        {"Amarr", {
            glm::vec4(0.6f, 0.55f, 0.45f, 1.0f),  // Gold-brass
            glm::vec4(0.4f, 0.35f, 0.25f, 1.0f),  // Dark gold
            glm::vec4(0.9f, 0.8f, 0.5f, 1.0f)     // Bright gold
        }},
        {"Serpentis", {
            glm::vec4(0.4f, 0.25f, 0.45f, 1.0f),  // Purple
            glm::vec4(0.2f, 0.15f, 0.25f, 1.0f),  // Dark purple
            glm::vec4(0.7f, 0.3f, 0.7f, 1.0f)     // Bright purple
        }},
        {"Guristas", {
            glm::vec4(0.5f, 0.2f, 0.2f, 1.0f),    // Dark red
            glm::vec4(0.3f, 0.1f, 0.1f, 1.0f),    // Very dark red
            glm::vec4(0.9f, 0.3f, 0.3f, 1.0f)     // Bright red
        }},
        {"Blood Raiders", {
            glm::vec4(0.4f, 0.15f, 0.15f, 1.0f),  // Blood red
            glm::vec4(0.2f, 0.05f, 0.05f, 1.0f),  // Almost black
            glm::vec4(0.8f, 0.2f, 0.2f, 1.0f)     // Crimson
        }}
    };

    auto it = colorMap.find(faction);
    if (it != colorMap.end()) {
        return it->second;
    }
    
    // Default colors
    return {
        glm::vec4(0.5f, 0.5f, 0.5f, 1.0f),  // Gray
        glm::vec4(0.3f, 0.3f, 0.3f, 1.0f),  // Dark gray
        glm::vec4(0.7f, 0.7f, 0.7f, 1.0f)   // Light gray
    };
}

// Ship model creation functions
std::unique_ptr<Model> Model::createFrigateModel(const FactionColors& colors) {
    auto model = std::make_unique<Model>();
    
    // Create a more detailed frigate hull with better geometry
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    float length = 3.5f;
    float width = 0.9f;
    float height = 0.7f;

    // Forward section - sleek nose
    vertices.push_back({{length, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {}, colors.primary});
    
    // Front-mid section - expanding
    for (int i = 0; i < 2; ++i) {
        float t = (i + 1) / 7.0f;
        float z = length * (1.0f - t * 0.3f);
        float scale = 0.3f + t * 1.2f;
        
        // Top, bottom, sides
        vertices.push_back({{z, width * scale, height * scale * 0.3f}, {0.0f, 1.0f, 0.3f}, {}, colors.primary});
        vertices.push_back({{z, -width * scale, height * scale * 0.3f}, {0.0f, -1.0f, 0.3f}, {}, colors.secondary});
        vertices.push_back({{z, width * scale * 0.5f, height * scale}, {0.3f, 0.5f, 1.0f}, {}, colors.primary});
        vertices.push_back({{z, -width * scale * 0.5f, height * scale}, {-0.3f, 0.5f, 1.0f}, {}, colors.primary});
        vertices.push_back({{z, width * scale * 0.5f, -height * scale * 0.7f}, {0.3f, 0.5f, -1.0f}, {}, colors.secondary});
        vertices.push_back({{z, -width * scale * 0.5f, -height * scale * 0.7f}, {-0.3f, 0.5f, -1.0f}, {}, colors.secondary});
    }
    
    // Mid section - main hull
    for (int i = 2; i < 4; ++i) {
        float t = (i + 1) / 7.0f;
        float z = length * (1.0f - t);
        float scale = 1.0f - (t - 0.3f) * 0.6f;
        
        vertices.push_back({{z, width * scale, height * scale * 0.3f}, {0.0f, 1.0f, 0.0f}, {}, colors.primary});
        vertices.push_back({{z, -width * scale, height * scale * 0.3f}, {0.0f, -1.0f, 0.0f}, {}, colors.secondary});
        vertices.push_back({{z, width * scale * 0.5f, height * scale}, {0.0f, 0.0f, 1.0f}, {}, colors.primary});
        vertices.push_back({{z, -width * scale * 0.5f, height * scale}, {0.0f, 0.0f, 1.0f}, {}, colors.primary});
        vertices.push_back({{z, width * scale * 0.5f, -height * scale * 0.7f}, {0.0f, 0.0f, -1.0f}, {}, colors.secondary});
        vertices.push_back({{z, -width * scale * 0.5f, -height * scale * 0.7f}, {0.0f, 0.0f, -1.0f}, {}, colors.secondary});
    }

    // Rear section - engine pods
    for (int i = 4; i < 6; ++i) {
        float t = (i + 1) / 7.0f;
        float z = length * (1.0f - t);
        float scale = 0.7f - (t - 0.6f) * 0.8f;
        
        vertices.push_back({{z, width * scale, 0.0f}, {0.0f, 1.0f, 0.0f}, {}, colors.accent});
        vertices.push_back({{z, -width * scale, 0.0f}, {0.0f, -1.0f, 0.0f}, {}, colors.accent});
        vertices.push_back({{z, 0.0f, height * scale}, {0.0f, 0.0f, 1.0f}, {}, colors.accent});
        vertices.push_back({{z, 0.0f, -height * scale * 0.7f}, {0.0f, 0.0f, -1.0f}, {}, colors.accent});
    }

    // Engine exhaust points
    vertices.push_back({{-length * 0.3f, width * 0.4f, 0.0f}, {-1.0f, 0.0f, 0.0f}, {}, colors.accent});
    vertices.push_back({{-length * 0.3f, -width * 0.4f, 0.0f}, {-1.0f, 0.0f, 0.0f}, {}, colors.accent});

    // Create triangles connecting vertices
    for (unsigned int i = 1; i < vertices.size() - 2; i += 2) {
        indices.push_back(0);
        indices.push_back(i);
        indices.push_back(i + 1);
        
        if (i + 2 < vertices.size()) {
            indices.push_back(i);
            indices.push_back(i + 1);
            indices.push_back(i + 2);
        }
    }

    auto mesh = std::make_unique<Mesh>(vertices, indices);
    model->addMesh(std::move(mesh));

    return model;
}

std::unique_ptr<Model> Model::createDestroyerModel(const FactionColors& colors) {
    auto model = std::make_unique<Model>();
    
    // Destroyers are longer, thinner, and more aggressive than frigates
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    float length = 5.0f;
    float width = 0.7f;
    float height = 0.6f;

    // Sharp aggressive nose
    vertices.push_back({{length, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {}, colors.primary});
    
    // Forward weapon section - narrow and sleek
    for (int i = 0; i < 3; ++i) {
        float t = (i + 1) / 10.0f;
        float z = length * (1.0f - t * 0.4f);
        float scale = 0.4f + t * 0.8f;
        
        vertices.push_back({{z, width * scale, height * scale * 0.4f}, {0.0f, 1.0f, 0.2f}, {}, colors.primary});
        vertices.push_back({{z, -width * scale, height * scale * 0.4f}, {0.0f, -1.0f, 0.2f}, {}, colors.secondary});
        vertices.push_back({{z, 0.0f, height * scale}, {0.0f, 0.0f, 1.0f}, {}, colors.primary});
        vertices.push_back({{z, 0.0f, -height * scale * 0.6f}, {0.0f, 0.0f, -1.0f}, {}, colors.secondary});
    }
    
    // Mid section - main destroyer spine
    for (int i = 3; i < 7; ++i) {
        float t = (i + 1) / 10.0f;
        float z = length * (1.0f - t);
        float scale = 1.0f - (t - 0.4f) * 0.5f;
        
        // Destroyer characteristic: dual-hull design with spine
        vertices.push_back({{z, width * scale * 1.2f, height * scale * 0.3f}, {0.0f, 1.0f, 0.0f}, {}, colors.primary});
        vertices.push_back({{z, -width * scale * 1.2f, height * scale * 0.3f}, {0.0f, -1.0f, 0.0f}, {}, colors.secondary});
        vertices.push_back({{z, 0.0f, height * scale * 1.1f}, {0.0f, 0.0f, 1.0f}, {}, colors.primary});
        vertices.push_back({{z, 0.0f, -height * scale * 0.5f}, {0.0f, 0.0f, -1.0f}, {}, colors.secondary});
    }

    // Rear engine section - split engines
    for (int i = 7; i < 9; ++i) {
        float t = (i + 1) / 10.0f;
        float z = length * (1.0f - t);
        float scale = 0.6f - (t - 0.7f) * 1.2f;
        
        vertices.push_back({{z, width * scale * 1.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {}, colors.accent});
        vertices.push_back({{z, -width * scale * 1.5f, 0.0f}, {0.0f, -1.0f, 0.0f}, {}, colors.accent});
        vertices.push_back({{z, 0.0f, height * scale * 0.8f}, {0.0f, 0.0f, 1.0f}, {}, colors.accent});
        vertices.push_back({{z, 0.0f, -height * scale * 0.5f}, {0.0f, 0.0f, -1.0f}, {}, colors.accent});
    }

    // Dual engine exhausts
    vertices.push_back({{-length * 0.25f, width * 0.8f, 0.0f}, {-1.0f, 0.0f, 0.0f}, {}, colors.accent});
    vertices.push_back({{-length * 0.25f, -width * 0.8f, 0.0f}, {-1.0f, 0.0f, 0.0f}, {}, colors.accent});

    // Create triangulated mesh
    for (unsigned int i = 1; i < vertices.size() - 2; i += 2) {
        indices.push_back(0);
        indices.push_back(i);
        indices.push_back(i + 1);
        
        if (i + 2 < vertices.size()) {
            indices.push_back(i);
            indices.push_back(i + 2);
            indices.push_back(i + 1);
        }
    }

    auto mesh = std::make_unique<Mesh>(vertices, indices);
    model->addMesh(std::move(mesh));

    return model;
}

std::unique_ptr<Model> Model::createCruiserModel(const FactionColors& colors) {
    auto model = std::make_unique<Model>();
    
    // Cruisers are larger, bulkier, with more presence
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    float length = 6.0f;
    float width = 1.8f;
    float height = 1.2f;

    // Command bridge section
    vertices.push_back({{length * 0.9f, 0.0f, height * 0.3f}, {1.0f, 0.0f, 0.3f}, {}, colors.primary});
    
    // Forward section - aggressive but substantial
    for (int i = 0; i < 2; ++i) {
        float t = (i + 1) / 12.0f;
        float z = length * (0.9f - t * 0.3f);
        float scale = 0.5f + t * 1.0f;
        
        vertices.push_back({{z, width * scale, height * scale * 0.4f}, {0.0f, 1.0f, 0.2f}, {}, colors.primary});
        vertices.push_back({{z, -width * scale, height * scale * 0.4f}, {0.0f, -1.0f, 0.2f}, {}, colors.secondary});
        vertices.push_back({{z, width * scale * 0.6f, height * scale}, {0.2f, 0.3f, 1.0f}, {}, colors.primary});
        vertices.push_back({{z, -width * scale * 0.6f, height * scale}, {-0.2f, 0.3f, 1.0f}, {}, colors.primary});
        vertices.push_back({{z, 0.0f, -height * scale * 0.5f}, {0.0f, 0.0f, -1.0f}, {}, colors.secondary});
    }
    
    // Main hull - wide and powerful
    for (int i = 2; i < 8; ++i) {
        float t = (i + 1) / 12.0f;
        float z = length * (0.9f - t);
        float scale = 1.0f + (i < 5 ? t * 0.3f : (1.0f - t) * 0.2f);
        
        vertices.push_back({{z, width * scale, height * scale * 0.3f}, {0.0f, 1.0f, 0.0f}, {}, colors.primary});
        vertices.push_back({{z, -width * scale, height * scale * 0.3f}, {0.0f, -1.0f, 0.0f}, {}, colors.secondary});
        vertices.push_back({{z, width * scale * 0.7f, height * scale}, {0.0f, 0.0f, 1.0f}, {}, colors.primary});
        vertices.push_back({{z, -width * scale * 0.7f, height * scale}, {0.0f, 0.0f, 1.0f}, {}, colors.primary});
        vertices.push_back({{z, 0.0f, -height * scale * 0.4f}, {0.0f, 0.0f, -1.0f}, {}, colors.secondary});
    }

    // Engine section - powerful propulsion
    for (int i = 8; i < 11; ++i) {
        float t = (i + 1) / 12.0f;
        float z = length * (0.9f - t);
        float scale = 1.2f - (t - 0.7f) * 1.5f;
        
        vertices.push_back({{z, width * scale, height * scale * 0.2f}, {0.0f, 1.0f, 0.0f}, {}, colors.accent});
        vertices.push_back({{z, -width * scale, height * scale * 0.2f}, {0.0f, -1.0f, 0.0f}, {}, colors.accent});
        vertices.push_back({{z, 0.0f, height * scale * 0.6f}, {0.0f, 0.0f, 1.0f}, {}, colors.accent});
        vertices.push_back({{z, 0.0f, -height * scale * 0.3f}, {0.0f, 0.0f, -1.0f}, {}, colors.accent});
    }

    // Main engine exhaust
    vertices.push_back({{-length * 0.15f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, {}, colors.accent});

    // Create mesh triangulation
    for (unsigned int i = 1; i < vertices.size() - 2; i += 2) {
        indices.push_back(0);
        indices.push_back(i);
        indices.push_back(i + 1);
        
        if (i + 2 < vertices.size()) {
            indices.push_back(i);
            indices.push_back(i + 2);
            indices.push_back(i + 3);
        }
    }

    auto mesh = std::make_unique<Mesh>(vertices, indices);
    model->addMesh(std::move(mesh));

    return model;
}

std::unique_ptr<Model> Model::createTech2CruiserModel(const FactionColors& colors) {
    // Tech 2 cruisers are similar to regular cruisers but with more detail
    auto model = createCruiserModel(colors);
    // TODO: Add additional details for Tech 2 ships
    return model;
}

std::unique_ptr<Model> Model::createBattlecruiserModel(const FactionColors& colors) {
    auto model = std::make_unique<Model>();
    
    // Battlecruisers are massive, intimidating warships
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    float length = 8.5f;
    float width = 2.5f;
    float height = 1.8f;

    // Forward command tower
    vertices.push_back({{length * 0.85f, 0.0f, height * 0.5f}, {1.0f, 0.0f, 0.5f}, {}, colors.primary});
    
    // Front weapon platforms - massive and menacing
    for (int i = 0; i < 3; ++i) {
        float t = (i + 1) / 15.0f;
        float z = length * (0.85f - t * 0.4f);
        float scale = 0.6f + t * 0.8f;
        
        vertices.push_back({{z, width * scale, height * scale * 0.3f}, {0.0f, 1.0f, 0.1f}, {}, colors.primary});
        vertices.push_back({{z, -width * scale, height * scale * 0.3f}, {0.0f, -1.0f, 0.1f}, {}, colors.secondary});
        vertices.push_back({{z, width * scale * 0.7f, height * scale}, {0.1f, 0.2f, 1.0f}, {}, colors.primary});
        vertices.push_back({{z, -width * scale * 0.7f, height * scale}, {-0.1f, 0.2f, 1.0f}, {}, colors.primary});
        vertices.push_back({{z, 0.0f, -height * scale * 0.4f}, {0.0f, 0.0f, -1.0f}, {}, colors.secondary});
    }
    
    // Main hull - broad and imposing
    for (int i = 3; i < 10; ++i) {
        float t = (i + 1) / 15.0f;
        float z = length * (0.85f - t);
        float widthScale = 1.0f + (i < 7 ? (t - 0.2f) * 0.4f : (1.0f - t) * 0.3f);
        float heightScale = 1.0f - t * 0.15f;
        
        vertices.push_back({{z, width * widthScale, height * heightScale * 0.25f}, {0.0f, 1.0f, 0.0f}, {}, colors.primary});
        vertices.push_back({{z, -width * widthScale, height * heightScale * 0.25f}, {0.0f, -1.0f, 0.0f}, {}, colors.secondary});
        vertices.push_back({{z, width * widthScale * 0.6f, height * heightScale}, {0.0f, 0.0f, 1.0f}, {}, colors.primary});
        vertices.push_back({{z, -width * widthScale * 0.6f, height * heightScale}, {0.0f, 0.0f, 1.0f}, {}, colors.primary});
        vertices.push_back({{z, 0.0f, -height * heightScale * 0.35f}, {0.0f, 0.0f, -1.0f}, {}, colors.secondary});
    }

    // Engine array - multiple powerful engines
    for (int i = 10; i < 14; ++i) {
        float t = (i + 1) / 15.0f;
        float z = length * (0.85f - t);
        float scale = 1.3f - (t - 0.7f) * 1.8f;
        
        vertices.push_back({{z, width * scale * 0.9f, height * scale * 0.15f}, {0.0f, 1.0f, 0.0f}, {}, colors.accent});
        vertices.push_back({{z, -width * scale * 0.9f, height * scale * 0.15f}, {0.0f, -1.0f, 0.0f}, {}, colors.accent});
        vertices.push_back({{z, width * scale * 0.4f, height * scale * 0.5f}, {0.0f, 0.0f, 1.0f}, {}, colors.accent});
        vertices.push_back({{z, -width * scale * 0.4f, height * scale * 0.5f}, {0.0f, 0.0f, 1.0f}, {}, colors.accent});
        vertices.push_back({{z, 0.0f, -height * scale * 0.25f}, {0.0f, 0.0f, -1.0f}, {}, colors.accent});
    }

    // Massive engine cluster exhausts
    vertices.push_back({{-length * 0.18f, width * 0.6f, 0.0f}, {-1.0f, 0.0f, 0.0f}, {}, colors.accent});
    vertices.push_back({{-length * 0.18f, -width * 0.6f, 0.0f}, {-1.0f, 0.0f, 0.0f}, {}, colors.accent});
    vertices.push_back({{-length * 0.18f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, {}, colors.accent});

    // Triangulation
    for (unsigned int i = 1; i < vertices.size() - 3; i += 2) {
        indices.push_back(0);
        indices.push_back(i);
        indices.push_back(i + 1);
        
        if (i + 3 < vertices.size()) {
            indices.push_back(i);
            indices.push_back(i + 2);
            indices.push_back(i + 3);
        }
    }

    auto mesh = std::make_unique<Mesh>(vertices, indices);
    model->addMesh(std::move(mesh));

    return model;
}

std::unique_ptr<Model> Model::createBattleshipModel(const FactionColors& colors) {
    auto model = std::make_unique<Model>();
    
    // Battleships are the largest subcapital ships - absolutely massive and powerful
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    float length = 12.0f;
    float width = 3.5f;
    float height = 2.5f;

    // Command citadel
    vertices.push_back({{length * 0.8f, 0.0f, height * 0.6f}, {1.0f, 0.0f, 0.6f}, {}, colors.primary});
    
    // Forward battle section with weapon batteries
    for (int i = 0; i < 4; ++i) {
        float t = (i + 1) / 20.0f;
        float z = length * (0.8f - t * 0.5f);
        float scale = 0.5f + t * 1.2f;
        
        vertices.push_back({{z, width * scale, height * scale * 0.25f}, {0.0f, 1.0f, 0.1f}, {}, colors.primary});
        vertices.push_back({{z, -width * scale, height * scale * 0.25f}, {0.0f, -1.0f, 0.1f}, {}, colors.secondary});
        vertices.push_back({{z, width * scale * 0.65f, height * scale * 0.9f}, {0.1f, 0.15f, 1.0f}, {}, colors.primary});
        vertices.push_back({{z, -width * scale * 0.65f, height * scale * 0.9f}, {-0.1f, 0.15f, 1.0f}, {}, colors.primary});
        vertices.push_back({{z, width * scale * 0.5f, -height * scale * 0.3f}, {0.1f, 0.0f, -1.0f}, {}, colors.secondary});
        vertices.push_back({{z, -width * scale * 0.5f, -height * scale * 0.3f}, {-0.1f, 0.0f, -1.0f}, {}, colors.secondary});
    }
    
    // Main battleship superstructure - extremely wide and imposing
    for (int i = 4; i < 14; ++i) {
        float t = (i + 1) / 20.0f;
        float z = length * (0.8f - t);
        float widthScale = 1.0f + (i < 10 ? (t - 0.25f) * 0.5f : (1.0f - t) * 0.4f);
        float heightScale = 1.0f - t * 0.12f;
        
        vertices.push_back({{z, width * widthScale, height * heightScale * 0.2f}, {0.0f, 1.0f, 0.0f}, {}, colors.primary});
        vertices.push_back({{z, -width * widthScale, height * heightScale * 0.2f}, {0.0f, -1.0f, 0.0f}, {}, colors.secondary});
        vertices.push_back({{z, width * widthScale * 0.55f, height * heightScale * 0.85f}, {0.0f, 0.0f, 1.0f}, {}, colors.primary});
        vertices.push_back({{z, -width * widthScale * 0.55f, height * heightScale * 0.85f}, {0.0f, 0.0f, 1.0f}, {}, colors.primary});
        vertices.push_back({{z, width * widthScale * 0.45f, -height * heightScale * 0.25f}, {0.0f, 0.0f, -1.0f}, {}, colors.secondary});
        vertices.push_back({{z, -width * widthScale * 0.45f, -height * heightScale * 0.25f}, {0.0f, 0.0f, -1.0f}, {}, colors.secondary});
    }

    // Massive engine banks
    for (int i = 14; i < 19; ++i) {
        float t = (i + 1) / 20.0f;
        float z = length * (0.8f - t);
        float scale = 1.5f - (t - 0.7f) * 2.2f;
        
        vertices.push_back({{z, width * scale * 0.8f, height * scale * 0.12f}, {0.0f, 1.0f, 0.0f}, {}, colors.accent});
        vertices.push_back({{z, -width * scale * 0.8f, height * scale * 0.12f}, {0.0f, -1.0f, 0.0f}, {}, colors.accent});
        vertices.push_back({{z, width * scale * 0.35f, height * scale * 0.4f}, {0.0f, 0.0f, 1.0f}, {}, colors.accent});
        vertices.push_back({{z, -width * scale * 0.35f, height * scale * 0.4f}, {0.0f, 0.0f, 1.0f}, {}, colors.accent});
        vertices.push_back({{z, width * scale * 0.3f, -height * scale * 0.2f}, {0.0f, 0.0f, -1.0f}, {}, colors.accent});
        vertices.push_back({{z, -width * scale * 0.3f, -height * scale * 0.2f}, {0.0f, 0.0f, -1.0f}, {}, colors.accent});
    }

    // Multiple massive engine exhaust ports
    vertices.push_back({{-length * 0.22f, width * 0.9f, height * 0.1f}, {-1.0f, 0.0f, 0.0f}, {}, colors.accent});
    vertices.push_back({{-length * 0.22f, -width * 0.9f, height * 0.1f}, {-1.0f, 0.0f, 0.0f}, {}, colors.accent});
    vertices.push_back({{-length * 0.22f, width * 0.4f, -height * 0.1f}, {-1.0f, 0.0f, 0.0f}, {}, colors.accent});
    vertices.push_back({{-length * 0.22f, -width * 0.4f, -height * 0.1f}, {-1.0f, 0.0f, 0.0f}, {}, colors.accent});

    // Triangulation with proper connectivity
    for (unsigned int i = 1; i < vertices.size() - 4; i += 3) {
        indices.push_back(0);
        indices.push_back(i);
        indices.push_back(i + 1);
        
        if (i + 3 < vertices.size()) {
            indices.push_back(i);
            indices.push_back(i + 2);
            indices.push_back(i + 3);
        }
    }

    auto mesh = std::make_unique<Mesh>(vertices, indices);
    model->addMesh(std::move(mesh));

    return model;
}

std::unique_ptr<Model> Model::createMiningBargeModel(const FactionColors& colors) {
    auto model = std::make_unique<Model>();
    
    // Mining barges are bulky and wide
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    float length = 6.0f;
    float width = 3.0f;
    float height = 2.0f;

    vertices.push_back({{length * 0.5f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {}, colors.primary});
    
    for (int i = 0; i < 4; ++i) {
        float t = (i + 1) / 5.0f;
        float z = -length * t;
        float scale = 1.0f;
        
        vertices.push_back({{z, width * scale, 0.0f}, {0.0f, 1.0f, 0.0f}, {}, colors.primary});
        vertices.push_back({{z, -width * scale, 0.0f}, {0.0f, -1.0f, 0.0f}, {}, colors.secondary});
        vertices.push_back({{z, 0.0f, height * scale}, {0.0f, 0.0f, 1.0f}, {}, colors.primary});
        vertices.push_back({{z, 0.0f, -height * scale}, {0.0f, 0.0f, -1.0f}, {}, colors.secondary});
    }

    vertices.push_back({{-length, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, {}, colors.accent});

    for (unsigned int i = 0; i < vertices.size() - 1; ++i) {
        indices.push_back(0);
        indices.push_back(i);
        indices.push_back(i + 1);
    }

    auto mesh = std::make_unique<Mesh>(vertices, indices);
    model->addMesh(std::move(mesh));

    return model;
}

std::unique_ptr<Model> Model::createGenericModel(const FactionColors& colors) {
    // Default to frigate model for unknown ship types
    return createFrigateModel(colors);
}

std::unique_ptr<Model> Model::createCarrierModel(const FactionColors& colors) {
    auto model = std::make_unique<Model>();
    
    // Carriers are massive capital ships with a distinctive carrier deck
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    float length = 15.0f;
    float width = 6.0f;
    float height = 4.0f;

    // Front command section (smaller)
    vertices.push_back({{length * 0.8f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {}, colors.primary});
    
    // Main carrier deck sections - flat and wide
    for (int i = 0; i < 10; ++i) {
        float t = (i + 1) / 11.0f;
        float z = length * 0.8f - length * t;
        // Carriers are wider in the middle
        float widthScale = (i < 5) ? (1.0f + t * 0.5f) : (1.5f - (t - 0.5f) * 0.8f);
        float heightScale = 1.0f - t * 0.2f;
        
        vertices.push_back({{z, width * widthScale, 0.0f}, {0.0f, 1.0f, 0.0f}, {}, colors.primary});
        vertices.push_back({{z, -width * widthScale, 0.0f}, {0.0f, -1.0f, 0.0f}, {}, colors.secondary});
        vertices.push_back({{z, 0.0f, height * heightScale}, {0.0f, 0.0f, 1.0f}, {}, colors.primary});
        vertices.push_back({{z, 0.0f, -height * heightScale * 0.3f}, {0.0f, 0.0f, -1.0f}, {}, colors.secondary});
    }

    // Rear engine section
    vertices.push_back({{-length * 0.3f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, {}, colors.accent});

    for (unsigned int i = 0; i < vertices.size() - 1; ++i) {
        indices.push_back(0);
        indices.push_back(i);
        indices.push_back(i + 1);
    }

    auto mesh = std::make_unique<Mesh>(vertices, indices);
    model->addMesh(std::move(mesh));

    return model;
}

std::unique_ptr<Model> Model::createDreadnoughtModel(const FactionColors& colors) {
    auto model = std::make_unique<Model>();
    
    // Dreadnoughts are compact but heavily armored with massive gun platforms
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    float length = 12.0f;
    float width = 4.5f;
    float height = 5.0f;

    // Front weapon platform
    vertices.push_back({{length * 0.7f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {}, colors.primary});
    
    // Main hull - bulky and thick
    for (int i = 0; i < 8; ++i) {
        float t = (i + 1) / 9.0f;
        float z = length * 0.7f - length * t;
        // Dreadnoughts are thick and compact
        float scale = 1.0f + (t < 0.5f ? t * 0.4f : (1.0f - t) * 0.4f);
        
        vertices.push_back({{z, width * scale, 0.0f}, {0.0f, 1.0f, 0.0f}, {}, colors.primary});
        vertices.push_back({{z, -width * scale, 0.0f}, {0.0f, -1.0f, 0.0f}, {}, colors.secondary});
        vertices.push_back({{z, 0.0f, height * scale}, {0.0f, 0.0f, 1.0f}, {}, colors.primary});
        vertices.push_back({{z, 0.0f, -height * scale}, {0.0f, 0.0f, -1.0f}, {}, colors.secondary});
    }

    // Rear engine array
    vertices.push_back({{-length * 0.35f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, {}, colors.accent});

    for (unsigned int i = 0; i < vertices.size() - 1; ++i) {
        indices.push_back(0);
        indices.push_back(i);
        indices.push_back(i + 1);
    }

    auto mesh = std::make_unique<Mesh>(vertices, indices);
    model->addMesh(std::move(mesh));

    return model;
}

std::unique_ptr<Model> Model::createTitanModel(const FactionColors& colors) {
    auto model = std::make_unique<Model>();
    
    // Titans are absolutely massive, the largest ships in the game
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    float length = 25.0f;
    float width = 8.0f;
    float height = 7.0f;

    // Massive front section
    vertices.push_back({{length * 0.6f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {}, colors.primary});
    
    // Command tower and main hull
    for (int i = 0; i < 15; ++i) {
        float t = (i + 1) / 16.0f;
        float z = length * 0.6f - length * t;
        
        // Titans have a distinctive shape - wider in forward sections
        float widthScale = 1.0f + (t < 0.4f ? t * 0.8f : (1.0f - t) * 0.5f);
        float heightScale = 1.0f + (t < 0.3f ? t * 0.5f : (1.0f - t) * 0.3f);
        
        vertices.push_back({{z, width * widthScale, 0.0f}, {0.0f, 1.0f, 0.0f}, {}, colors.primary});
        vertices.push_back({{z, -width * widthScale, 0.0f}, {0.0f, -1.0f, 0.0f}, {}, colors.secondary});
        vertices.push_back({{z, 0.0f, height * heightScale}, {0.0f, 0.0f, 1.0f}, {}, colors.primary});
        vertices.push_back({{z, 0.0f, -height * heightScale * 0.6f}, {0.0f, 0.0f, -1.0f}, {}, colors.secondary});
    }

    // Massive engine clusters
    vertices.push_back({{-length * 0.45f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, {}, colors.accent});

    for (unsigned int i = 0; i < vertices.size() - 1; ++i) {
        indices.push_back(0);
        indices.push_back(i);
        indices.push_back(i + 1);
    }

    auto mesh = std::make_unique<Mesh>(vertices, indices);
    model->addMesh(std::move(mesh));

    return model;
}

std::unique_ptr<Model> Model::createStationModel(const FactionColors& colors, const std::string& stationType) {
    auto model = std::make_unique<Model>();
    
    // Stations are large stationary structures
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    float size = 20.0f;
    float radius = size * 0.5f;
    
    // Create a basic station structure with a central hub and spokes
    // Central core
    int segments = 8;
    for (int i = 0; i < segments; ++i) {
        float angle = (i * 2.0f * PI) / segments;
        float x = radius * 0.3f * std::cos(angle);
        float y = radius * 0.3f * std::sin(angle);
        
        vertices.push_back({{x, y, radius * 0.5f}, {0.0f, 0.0f, 1.0f}, {}, colors.primary});
        vertices.push_back({{x, y, -radius * 0.5f}, {0.0f, 0.0f, -1.0f}, {}, colors.secondary});
    }
    
    // Add docking spokes
    for (int i = 0; i < 4; ++i) {
        float angle = (i * PI * 0.5f);
        float x = radius * std::cos(angle);
        float y = radius * std::sin(angle);
        
        vertices.push_back({{x, y, 0.0f}, {std::cos(angle), std::sin(angle), 0.0f}, {}, colors.accent});
    }

    // Create triangles for the station structure
    for (unsigned int i = 0; i < vertices.size() / 2 - 1; ++i) {
        indices.push_back(i);
        indices.push_back(i + 1);
        indices.push_back(i + 2);
    }

    auto mesh = std::make_unique<Mesh>(vertices, indices);
    model->addMesh(std::move(mesh));

    return model;
}

std::unique_ptr<Model> Model::createAsteroidModel(const std::string& oreType) {
    auto model = std::make_unique<Model>();
    
    // Asteroids are irregular rock formations
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    // Determine color based on ore type
    glm::vec3 asteroidColor = glm::vec3(0.5f, 0.5f, 0.5f); // Default gray
    
    if (oreType.find("Veldspar") != std::string::npos) {
        asteroidColor = glm::vec3(0.6f, 0.4f, 0.2f); // Brown-orange
    } else if (oreType.find("Scordite") != std::string::npos) {
        asteroidColor = glm::vec3(0.5f, 0.5f, 0.55f); // Gray metallic
    } else if (oreType.find("Pyroxeres") != std::string::npos) {
        asteroidColor = glm::vec3(0.7f, 0.3f, 0.2f); // Red-brown
    } else if (oreType.find("Plagioclase") != std::string::npos) {
        asteroidColor = glm::vec3(0.3f, 0.5f, 0.4f); // Green-gray
    } else if (oreType.find("Omber") != std::string::npos) {
        asteroidColor = glm::vec3(0.8f, 0.6f, 0.3f); // Golden-brown
    } else if (oreType.find("Kernite") != std::string::npos) {
        asteroidColor = glm::vec3(0.3f, 0.6f, 0.7f); // Blue-cyan
    } else if (oreType.find("Jaspet") != std::string::npos) {
        asteroidColor = glm::vec3(0.6f, 0.2f, 0.3f); // Dark red
    } else if (oreType.find("Hemorphite") != std::string::npos) {
        asteroidColor = glm::vec3(0.9f, 0.3f, 0.2f); // Bright red-orange
    }

    float size = 2.5f;
    
    // Create an irregular shape using multiple vertices
    int points = 12;
    for (int i = 0; i < points; ++i) {
        float theta = (i * 2.0f * PI) / points;
        float phi = (i * PI) / points;
        
        // Add randomness to make it look irregular
        float r = size * (0.7f + (i % 3) * 0.15f);
        
        float x = r * std::sin(phi) * std::cos(theta);
        float y = r * std::sin(phi) * std::sin(theta);
        float z = r * std::cos(phi);
        
        glm::vec3 normal = glm::normalize(glm::vec3(x, y, z));
        
        vertices.push_back({{x, y, z}, normal, {}, asteroidColor});
    }

    // Create triangles
    for (unsigned int i = 0; i < vertices.size(); ++i) {
        indices.push_back(0);
        indices.push_back(i);
        indices.push_back((i + 1) % vertices.size());
    }

    auto mesh = std::make_unique<Mesh>(vertices, indices);
    model->addMesh(std::move(mesh));

    return model;
}

} // namespace eve
