#include "rendering/model.h"
#include "rendering/mesh.h"
#include <iostream>
#include <algorithm>
#include <cmath>

namespace eve {

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
        "Mining Barge", "Procurer", "Retriever", "Covetor"
    };
    return std::any_of(miningNames.begin(), miningNames.end(),
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
    
    // Create a simple frigate hull (elongated diamond shape)
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    // Main hull (elongated)
    float length = 3.0f;
    float width = 0.8f;
    float height = 0.6f;

    // Front nose
    vertices.push_back({{length, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {}, colors.primary});
    
    // Mid sections
    for (int i = 0; i < 3; ++i) {
        float t = (i + 1) / 4.0f;
        float z = -length * t;
        float scale = 1.0f - t * 0.5f;
        
        vertices.push_back({{z, width * scale, 0.0f}, {0.0f, 1.0f, 0.0f}, {}, colors.primary});
        vertices.push_back({{z, -width * scale, 0.0f}, {0.0f, -1.0f, 0.0f}, {}, colors.secondary});
        vertices.push_back({{z, 0.0f, height * scale}, {0.0f, 0.0f, 1.0f}, {}, colors.primary});
        vertices.push_back({{z, 0.0f, -height * scale}, {0.0f, 0.0f, -1.0f}, {}, colors.secondary});
    }

    // Back end
    vertices.push_back({{-length, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, {}, colors.accent});

    // Create triangles to form hull
    // Simple indexing for basic shape
    for (unsigned int i = 0; i < vertices.size() - 1; ++i) {
        indices.push_back(0);
        indices.push_back(i);
        indices.push_back(i + 1);
    }

    auto mesh = std::make_unique<Mesh>(vertices, indices);
    model->addMesh(std::move(mesh));

    return model;
}

std::unique_ptr<Model> Model::createDestroyerModel(const FactionColors& colors) {
    auto model = std::make_unique<Model>();
    
    // Destroyers are longer and thinner than frigates
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    float length = 4.5f;
    float width = 0.6f;
    float height = 0.5f;

    // Similar to frigate but more elongated
    vertices.push_back({{length, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {}, colors.primary});
    
    for (int i = 0; i < 4; ++i) {
        float t = (i + 1) / 5.0f;
        float z = -length * t;
        float scale = 1.0f - t * 0.3f;
        
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

std::unique_ptr<Model> Model::createCruiserModel(const FactionColors& colors) {
    auto model = std::make_unique<Model>();
    
    // Cruisers are larger and bulkier
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    float length = 5.0f;
    float width = 1.5f;
    float height = 1.0f;

    vertices.push_back({{length, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {}, colors.primary});
    
    for (int i = 0; i < 5; ++i) {
        float t = (i + 1) / 6.0f;
        float z = -length * t;
        float scale = 1.0f - t * 0.4f;
        
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

std::unique_ptr<Model> Model::createTech2CruiserModel(const FactionColors& colors) {
    // Tech 2 cruisers are similar to regular cruisers but with more detail
    auto model = createCruiserModel(colors);
    // TODO: Add additional details for Tech 2 ships
    return model;
}

std::unique_ptr<Model> Model::createBattlecruiserModel(const FactionColors& colors) {
    auto model = std::make_unique<Model>();
    
    // Battlecruisers are massive
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    float length = 7.0f;
    float width = 2.0f;
    float height = 1.5f;

    vertices.push_back({{length, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {}, colors.primary});
    
    for (int i = 0; i < 6; ++i) {
        float t = (i + 1) / 7.0f;
        float z = -length * t;
        float scale = 1.0f - t * 0.3f;
        
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

std::unique_ptr<Model> Model::createBattleshipModel(const FactionColors& colors) {
    auto model = std::make_unique<Model>();
    
    // Battleships are the largest
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    float length = 10.0f;
    float width = 3.0f;
    float height = 2.0f;

    vertices.push_back({{length, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {}, colors.primary});
    
    for (int i = 0; i < 8; ++i) {
        float t = (i + 1) / 9.0f;
        float z = -length * t;
        float scale = 1.0f - t * 0.25f;
        
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

} // namespace eve
