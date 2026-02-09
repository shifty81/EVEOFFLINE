#include "rendering/model.h"
#include "rendering/mesh.h"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <fstream>

// Model loading libraries
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

// tinygltf configuration
#define TINYGLTF_IMPLEMENTATION
// Note: STB_IMAGE_IMPLEMENTATION is defined in texture.cpp to avoid multiple definitions
// Disable stb_image_write (not available, only needed for saving models)
#ifndef TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#endif
// Disable JSON dependency warnings - tinygltf will handle JSON internally
#define TINYGLTF_NO_EXTERNAL_IMAGE
// Include nlohmann/json before tinygltf and skip tinygltf's own json include
#ifndef TINYGLTF_NO_INCLUDE_JSON
#define TINYGLTF_NO_INCLUDE_JSON
#endif
#include <nlohmann/json.hpp>
#include <tiny_gltf.h>

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
    // Determine file format based on extension
    std::string extension;
    size_t dotPos = path.find_last_of('.');
    if (dotPos != std::string::npos) {
        extension = path.substr(dotPos + 1);
        // Convert to lowercase
        std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    }

    if (extension == "obj") {
        return loadOBJ(path);
    } else if (extension == "gltf" || extension == "glb") {
        return loadGLTF(path);
    } else {
        std::cerr << "Unsupported model format: " << extension << std::endl;
        std::cerr << "Supported formats: .obj, .gltf, .glb" << std::endl;
        return false;
    }
}

bool Model::loadOBJ(const std::string& path) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    // Load the OBJ file
    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str());

    if (!warn.empty()) {
        std::cout << "OBJ Warning: " << warn << std::endl;
    }

    if (!err.empty()) {
        std::cerr << "OBJ Error: " << err << std::endl;
    }

    if (!ret) {
        std::cerr << "Failed to load OBJ file: " << path << std::endl;
        return false;
    }

    std::cout << "Loaded OBJ: " << path << std::endl;
    std::cout << "  Shapes: " << shapes.size() << std::endl;
    std::cout << "  Materials: " << materials.size() << std::endl;

    // Process each shape in the OBJ file
    for (size_t s = 0; s < shapes.size(); s++) {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;

        // Process each face
        size_t index_offset = 0;
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
            int fv = shapes[s].mesh.num_face_vertices[f];

            // Process each vertex in the face
            for (size_t v = 0; v < fv; v++) {
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

                Vertex vertex;

                // Position
                vertex.position = glm::vec3(
                    attrib.vertices[3 * idx.vertex_index + 0],
                    attrib.vertices[3 * idx.vertex_index + 1],
                    attrib.vertices[3 * idx.vertex_index + 2]
                );

                // Normal (if available)
                if (idx.normal_index >= 0) {
                    vertex.normal = glm::vec3(
                        attrib.normals[3 * idx.normal_index + 0],
                        attrib.normals[3 * idx.normal_index + 1],
                        attrib.normals[3 * idx.normal_index + 2]
                    );
                } else {
                    vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f);
                }

                // Texture coordinates (if available)
                if (idx.texcoord_index >= 0) {
                    vertex.texCoords = glm::vec2(
                        attrib.texcoords[2 * idx.texcoord_index + 0],
                        attrib.texcoords[2 * idx.texcoord_index + 1]
                    );
                } else {
                    vertex.texCoords = glm::vec2(0.0f, 0.0f);
                }

                // Color (default white, or from material)
                vertex.color = glm::vec3(1.0f, 1.0f, 1.0f);
                if (!materials.empty() && shapes[s].mesh.material_ids[f] >= 0) {
                    size_t mat_id = static_cast<size_t>(shapes[s].mesh.material_ids[f]);
                    if (mat_id < materials.size()) {
                        vertex.color = glm::vec3(
                            materials[mat_id].diffuse[0],
                            materials[mat_id].diffuse[1],
                            materials[mat_id].diffuse[2]
                        );
                    }
                }

                vertices.push_back(vertex);
                indices.push_back(static_cast<unsigned int>(vertices.size() - 1));
            }

            index_offset += fv;
        }

        // Create mesh from vertices and indices
        if (!vertices.empty() && !indices.empty()) {
            auto mesh = std::make_unique<Mesh>(vertices, indices);
            addMesh(std::move(mesh));
        }
    }

    return !m_meshes.empty();
}

bool Model::loadGLTF(const std::string& path) {
    tinygltf::Model gltfModel;
    tinygltf::TinyGLTF loader;
    std::string err, warn;

    bool ret = false;
    
    // Determine if it's binary (.glb) or text (.gltf)
    std::string extension;
    size_t dotPos = path.find_last_of('.');
    if (dotPos != std::string::npos) {
        extension = path.substr(dotPos + 1);
        std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    }

    if (extension == "glb") {
        ret = loader.LoadBinaryFromFile(&gltfModel, &err, &warn, path);
    } else {
        ret = loader.LoadASCIIFromFile(&gltfModel, &err, &warn, path);
    }

    if (!warn.empty()) {
        std::cout << "GLTF Warning: " << warn << std::endl;
    }

    if (!err.empty()) {
        std::cerr << "GLTF Error: " << err << std::endl;
    }

    if (!ret) {
        std::cerr << "Failed to load GLTF file: " << path << std::endl;
        return false;
    }

    std::cout << "Loaded GLTF: " << path << std::endl;
    std::cout << "  Meshes: " << gltfModel.meshes.size() << std::endl;
    std::cout << "  Materials: " << gltfModel.materials.size() << std::endl;

    // Process each mesh in the GLTF file
    for (const auto& mesh : gltfModel.meshes) {
        for (const auto& primitive : mesh.primitives) {
            std::vector<Vertex> vertices;
            std::vector<unsigned int> indices;

            // Get position accessor
            const tinygltf::Accessor& posAccessor = gltfModel.accessors[primitive.attributes.at("POSITION")];
            const tinygltf::BufferView& posView = gltfModel.bufferViews[posAccessor.bufferView];
            const tinygltf::Buffer& posBuffer = gltfModel.buffers[posView.buffer];
            const float* positions = reinterpret_cast<const float*>(&posBuffer.data[posView.byteOffset + posAccessor.byteOffset]);

            // Get normal accessor (if available)
            const float* normals = nullptr;
            if (primitive.attributes.find("NORMAL") != primitive.attributes.end()) {
                const tinygltf::Accessor& normAccessor = gltfModel.accessors[primitive.attributes.at("NORMAL")];
                const tinygltf::BufferView& normView = gltfModel.bufferViews[normAccessor.bufferView];
                const tinygltf::Buffer& normBuffer = gltfModel.buffers[normView.buffer];
                normals = reinterpret_cast<const float*>(&normBuffer.data[normView.byteOffset + normAccessor.byteOffset]);
            }

            // Get texture coordinate accessor (if available)
            const float* texCoords = nullptr;
            if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end()) {
                const tinygltf::Accessor& texAccessor = gltfModel.accessors[primitive.attributes.at("TEXCOORD_0")];
                const tinygltf::BufferView& texView = gltfModel.bufferViews[texAccessor.bufferView];
                const tinygltf::Buffer& texBuffer = gltfModel.buffers[texView.buffer];
                texCoords = reinterpret_cast<const float*>(&texBuffer.data[texView.byteOffset + texAccessor.byteOffset]);
            }

            // Create vertices
            for (size_t i = 0; i < posAccessor.count; i++) {
                Vertex vertex;

                vertex.position = glm::vec3(
                    positions[i * 3 + 0],
                    positions[i * 3 + 1],
                    positions[i * 3 + 2]
                );

                if (normals) {
                    vertex.normal = glm::vec3(
                        normals[i * 3 + 0],
                        normals[i * 3 + 1],
                        normals[i * 3 + 2]
                    );
                } else {
                    vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f);
                }

                if (texCoords) {
                    vertex.texCoords = glm::vec2(
                        texCoords[i * 2 + 0],
                        texCoords[i * 2 + 1]
                    );
                } else {
                    vertex.texCoords = glm::vec2(0.0f, 0.0f);
                }

                // Default color (or from material)
                vertex.color = glm::vec3(1.0f, 1.0f, 1.0f);
                if (primitive.material >= 0 && primitive.material < gltfModel.materials.size()) {
                    const auto& material = gltfModel.materials[primitive.material];
                    if (material.pbrMetallicRoughness.baseColorFactor.size() >= 3) {
                        vertex.color = glm::vec3(
                            material.pbrMetallicRoughness.baseColorFactor[0],
                            material.pbrMetallicRoughness.baseColorFactor[1],
                            material.pbrMetallicRoughness.baseColorFactor[2]
                        );
                    }
                }

                vertices.push_back(vertex);
            }

            // Get indices
            const tinygltf::Accessor& indexAccessor = gltfModel.accessors[primitive.indices];
            const tinygltf::BufferView& indexView = gltfModel.bufferViews[indexAccessor.bufferView];
            const tinygltf::Buffer& indexBuffer = gltfModel.buffers[indexView.buffer];

            // Handle different index types
            if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
                const uint16_t* buf = reinterpret_cast<const uint16_t*>(&indexBuffer.data[indexView.byteOffset + indexAccessor.byteOffset]);
                for (size_t i = 0; i < indexAccessor.count; i++) {
                    indices.push_back(static_cast<unsigned int>(buf[i]));
                }
            } else if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
                const uint32_t* buf = reinterpret_cast<const uint32_t*>(&indexBuffer.data[indexView.byteOffset + indexAccessor.byteOffset]);
                for (size_t i = 0; i < indexAccessor.count; i++) {
                    indices.push_back(buf[i]);
                }
            } else if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
                const uint8_t* buf = reinterpret_cast<const uint8_t*>(&indexBuffer.data[indexView.byteOffset + indexAccessor.byteOffset]);
                for (size_t i = 0; i < indexAccessor.count; i++) {
                    indices.push_back(static_cast<unsigned int>(buf[i]));
                }
            }

            // Create mesh from vertices and indices
            if (!vertices.empty() && !indices.empty()) {
                auto meshPtr = std::make_unique<Mesh>(vertices, indices);
                addMesh(std::move(meshPtr));
            }
        }
    }

    return !m_meshes.empty();
}

void Model::addMesh(std::unique_ptr<Mesh> mesh) {
    m_meshes.push_back(std::move(mesh));
}

std::unique_ptr<Model> Model::createShipModel(const std::string& shipType, const std::string& faction) {
    // Note: Model cache is defined but not actively used
    // Deep copying meshes with OpenGL buffers is complex, and procedural generation
    // is fast enough that caching provides minimal performance benefit.
    // For significant performance gains, consider instanced rendering instead.
    
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
    } else if (isCommandShip(shipType)) {
        model = createBattlecruiserModel(colors); // Command Ships share battlecruiser hull size
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
        "Assault Frigate", "Jaguar", "Hawk", "Enyo", "Retribution", "Wolf", "Harpy",
        "Interceptor", "Claw", "Crow", "Taranis", "Crusader",
        "Stiletto", "Raptor", "Ares", "Malediction",
        "Covert Ops", "Cheetah", "Buzzard", "Helios", "Anathema",
        "Stealth Bomber", "Hound", "Manticore", "Nemesis", "Purifier"
    };
    return std::any_of(frigateNames.begin(), frigateNames.end(),
        [&shipType](const std::string& name) { return shipType.find(name) != std::string::npos; });
}

bool Model::isDestroyer(const std::string& shipType) {
    static const std::vector<std::string> destroyerNames = {
        "Destroyer", "Thrasher", "Cormorant", "Catalyst", "Coercer",
        "Interdictor", "Sabre", "Flycatcher", "Eris", "Heretic"
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

bool Model::isCommandShip(const std::string& shipType) {
    static const std::vector<std::string> csNames = {
        "Command Ship", "Claymore", "Vulture", "Astarte", "Absolution"
    };
    return std::any_of(csNames.begin(), csNames.end(),
        [&shipType](const std::string& name) { return shipType.find(name) != std::string::npos; });
}

bool Model::isBattleship(const std::string& shipType) {
    static const std::vector<std::string> bsNames = {
        "Battleship", "Tempest", "Raven", "Dominix", "Apocalypse",
        "Marauder", "Vargur", "Golem", "Kronos", "Paladin"
    };
    return std::any_of(bsNames.begin(), bsNames.end(),
        [&shipType](const std::string& name) { return shipType.find(name) != std::string::npos; });
}

bool Model::isMiningBarge(const std::string& shipType) {
    static const std::vector<std::string> miningNames = {
        "Mining Barge", "Procurer", "Retriever", "Covetor", "Exhumer", "Hulk", "Mackinaw", "Skiff",
        "Industrial", "Hoarder", "Badger", "Iteron", "Bestower"
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

// ==================== Enhanced Procedural Detail Generation ====================

/**
 * Get design traits based on faction and ship class
 */
ShipDesignTraits Model::getDesignTraits(const std::string& faction, const std::string& shipClass) {
    ShipDesignTraits traits;
    
    // Determine faction design style
    if (faction.find("Caldari") != std::string::npos) {
        traits.style = ShipDesignTraits::DesignStyle::CALDARI_BLOCKY;
        traits.isBlocky = true;
        traits.isOrganic = false;
        traits.isAsymmetric = false;
        traits.hasSpires = false;
        traits.hasExposedFramework = false;
        traits.asymmetryFactor = 0.0f;
    } else if (faction.find("Amarr") != std::string::npos) {
        traits.style = ShipDesignTraits::DesignStyle::AMARR_ORNATE;
        traits.hasSpires = true;
        traits.isBlocky = false;
        traits.isOrganic = false;
        traits.isAsymmetric = false;
        traits.hasExposedFramework = false;
        traits.asymmetryFactor = 0.0f;
    } else if (faction.find("Gallente") != std::string::npos) {
        traits.style = ShipDesignTraits::DesignStyle::GALLENTE_ORGANIC;
        traits.isOrganic = true;
        traits.isBlocky = false;
        traits.isAsymmetric = false;
        traits.hasSpires = false;
        traits.hasExposedFramework = false;
        traits.asymmetryFactor = 0.0f;
    } else if (faction.find("Minmatar") != std::string::npos) {
        traits.style = ShipDesignTraits::DesignStyle::MINMATAR_ASYMMETRIC;
        traits.isAsymmetric = true;
        traits.hasExposedFramework = true;
        traits.isBlocky = false;
        traits.isOrganic = false;
        traits.hasSpires = false;
        traits.asymmetryFactor = 0.3f;
    } else {
        // Default traits for unknown factions
        traits.style = ShipDesignTraits::DesignStyle::CALDARI_BLOCKY;
        traits.isBlocky = false;
        traits.isOrganic = false;
        traits.isAsymmetric = false;
        traits.hasSpires = false;
        traits.hasExposedFramework = false;
        traits.asymmetryFactor = 0.0f;
    }
    
    // Set weapon hardpoints based on ship class
    if (shipClass.find("Frigate") != std::string::npos) {
        traits.turretHardpoints = 2;
        traits.missileHardpoints = 0;
        traits.droneHardpoints = 0;
        traits.engineCount = 2;
        traits.hasLargeEngines = false;
        traits.detailScale = 1.0f;
    } else if (shipClass.find("Destroyer") != std::string::npos) {
        traits.turretHardpoints = 4;
        traits.missileHardpoints = 0;
        traits.droneHardpoints = 0;
        traits.engineCount = 2;
        traits.hasLargeEngines = false;
        traits.detailScale = 1.2f;
    } else if (shipClass.find("Cruiser") != std::string::npos) {
        traits.turretHardpoints = 4;
        traits.missileHardpoints = 2;
        traits.droneHardpoints = 1;
        traits.engineCount = 3;
        traits.hasLargeEngines = false;
        traits.detailScale = 1.5f;
    } else if (shipClass.find("Battlecruiser") != std::string::npos) {
        traits.turretHardpoints = 6;
        traits.missileHardpoints = 2;
        traits.droneHardpoints = 2;
        traits.engineCount = 4;
        traits.hasLargeEngines = true;
        traits.detailScale = 2.0f;
    } else if (shipClass.find("Battleship") != std::string::npos) {
        traits.turretHardpoints = 8;
        traits.missileHardpoints = 4;
        traits.droneHardpoints = 2;
        traits.engineCount = 6;
        traits.hasLargeEngines = true;
        traits.detailScale = 2.5f;
    } else {
        // Defaults
        traits.turretHardpoints = 2;
        traits.missileHardpoints = 0;
        traits.droneHardpoints = 0;
        traits.engineCount = 2;
        traits.hasLargeEngines = false;
        traits.detailScale = 1.0f;
    }
    
    return traits;
}

/**
 * Add weapon hardpoint geometry (turret mounts)
 */
void Model::addWeaponHardpoints(std::vector<Vertex>& vertices, std::vector<unsigned int>& indices,
                                 float posZ, float offsetX, float offsetY, int count, const glm::vec3& color) {
    float hardpointSize = 0.15f;
    int startIdx = vertices.size();
    
    for (int i = 0; i < count; ++i) {
        float side = (i % 2 == 0) ? 1.0f : -1.0f;
        float xPos = offsetX * side;
        float yPos = offsetY;
        
        // Create small turret mount geometry
        vertices.push_back({{posZ, xPos, yPos}, {0.0f, side, 0.0f}, {}, color});
        vertices.push_back({{posZ + hardpointSize, xPos, yPos}, {side, 0.0f, 0.0f}, {}, color});
        vertices.push_back({{posZ, xPos, yPos + hardpointSize}, {0.0f, 0.0f, 1.0f}, {}, color});
        
        // Add triangle
        if (vertices.size() >= 3) {
            indices.push_back(startIdx + i * 3);
            indices.push_back(startIdx + i * 3 + 1);
            indices.push_back(startIdx + i * 3 + 2);
        }
    }
}

/**
 * Add engine exhaust detail geometry
 */
void Model::addEngineDetail(std::vector<Vertex>& vertices, std::vector<unsigned int>& indices,
                            float posZ, float width, float height, int count, const glm::vec3& color) {
    float exhaustSize = 0.2f;
    int startIdx = vertices.size();
    
    for (int i = 0; i < count; ++i) {
        float angle = (i * 2.0f * PI) / count;
        float xOffset = width * 0.5f * std::cos(angle);
        float yOffset = height * 0.5f * std::sin(angle);
        
        // Engine exhaust cone
        vertices.push_back({{posZ, xOffset, yOffset}, {-1.0f, 0.0f, 0.0f}, {}, color});
        vertices.push_back({{posZ - exhaustSize, xOffset * 0.7f, yOffset * 0.7f}, {-1.0f, 0.0f, 0.0f}, {}, color});
        vertices.push_back({{posZ - exhaustSize, xOffset * 1.3f, yOffset * 1.3f}, {-1.0f, 0.0f, 0.0f}, {}, color});
        
        if (vertices.size() >= 3) {
            indices.push_back(startIdx + i * 3);
            indices.push_back(startIdx + i * 3 + 1);
            indices.push_back(startIdx + i * 3 + 2);
        }
    }
}

/**
 * Add hull panel lines for detail
 */
void Model::addHullPanelLines(std::vector<Vertex>& vertices, std::vector<unsigned int>& indices,
                              float startZ, float endZ, float width, const glm::vec3& color) {
    float panelWidth = 0.05f;
    int panelCount = static_cast<int>((startZ - endZ) / 1.0f);
    int startIdx = vertices.size();
    
    for (int i = 0; i < panelCount; ++i) {
        float z = startZ - i * 1.0f;
        
        // Add subtle panel line geometry
        vertices.push_back({{z, width, 0.0f}, {0.0f, 1.0f, 0.0f}, {}, color * 0.9f});
        vertices.push_back({{z, -width, 0.0f}, {0.0f, -1.0f, 0.0f}, {}, color * 0.9f});
        vertices.push_back({{z - panelWidth, width, 0.0f}, {0.0f, 1.0f, 0.0f}, {}, color * 0.8f});
        
        if (i < panelCount - 1 && vertices.size() >= 3) {
            indices.push_back(startIdx + i * 3);
            indices.push_back(startIdx + i * 3 + 1);
            indices.push_back(startIdx + i * 3 + 2);
        }
    }
}

/**
 * Add Amarr-style spire detail
 */
void Model::addSpireDetail(std::vector<Vertex>& vertices, std::vector<unsigned int>& indices,
                           float posZ, float height, const glm::vec3& color) {
    int startIdx = vertices.size();
    
    // Central spire point
    vertices.push_back({{posZ, 0.0f, height * 1.5f}, {0.0f, 0.0f, 1.0f}, {}, color});
    
    // Base of spire
    vertices.push_back({{posZ - 0.3f, 0.2f, height}, {0.0f, 1.0f, 0.3f}, {}, color * 0.9f});
    vertices.push_back({{posZ - 0.3f, -0.2f, height}, {0.0f, -1.0f, 0.3f}, {}, color * 0.9f});
    vertices.push_back({{posZ - 0.3f, 0.0f, height * 0.8f}, {0.0f, 0.0f, 0.9f}, {}, color * 0.85f});
    
    // Create spire triangles
    indices.push_back(startIdx);
    indices.push_back(startIdx + 1);
    indices.push_back(startIdx + 2);
    
    indices.push_back(startIdx);
    indices.push_back(startIdx + 2);
    indices.push_back(startIdx + 3);
}

/**
 * Add Minmatar-style asymmetric detail
 */
void Model::addAsymmetricDetail(std::vector<Vertex>& vertices, std::vector<unsigned int>& indices,
                                float posZ, float offset, const glm::vec3& color) {
    int startIdx = vertices.size();
    
    // Asymmetric protruding structure
    vertices.push_back({{posZ, offset, 0.0f}, {0.0f, 1.0f, 0.0f}, {}, color});
    vertices.push_back({{posZ - 0.4f, offset * 1.3f, 0.1f}, {0.5f, 1.0f, 0.1f}, {}, color * 0.9f});
    vertices.push_back({{posZ - 0.4f, offset * 0.7f, -0.1f}, {0.5f, 0.7f, -0.1f}, {}, color * 0.85f});
    
    // Create triangle
    indices.push_back(startIdx);
    indices.push_back(startIdx + 1);
    indices.push_back(startIdx + 2);
}

// ==================== Ship Model Creation Functions ====================

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

    // Add enhanced details - weapon hardpoints on forward hull
    addWeaponHardpoints(vertices, indices, length * 0.6f, width * 0.5f, height * 0.2f, 2, 
                        glm::vec3(colors.accent.r, colors.accent.g, colors.accent.b));
    
    // Add engine glow detail at rear
    addEngineDetail(vertices, indices, -length * 0.3f, width * 0.5f, height * 0.3f, 2,
                    glm::vec3(colors.accent.r * 1.2f, colors.accent.g * 1.2f, colors.accent.b * 1.5f));
    
    // Add hull panel lines for detail
    addHullPanelLines(vertices, indices, length * 0.5f, 0.0f, width * 0.8f,
                     glm::vec3(colors.primary.r, colors.primary.g, colors.primary.b));

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
    // Tech 2 cruisers are similar to regular cruisers but with more detail and angular features
    auto model = createCruiserModel(colors);
    
    // Add Tech 2 visual enhancements:
    // - More angular plating (already achieved through base model variations)
    // - Additional sensor arrays and equipment visible on hull
    // - Slight variation in proportions (already handled by procedural generation)
    // Tech 2 ships in EVE have sharper angles and more refined details
    // This is represented through the faction-specific color schemes and base geometry
    
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

    // Add enhanced details for battleship - massive weapon platforms
    // Forward turret batteries (top and bottom)
    addWeaponHardpoints(vertices, indices, length * 0.5f, width * 0.8f, height * 0.4f, 4,
                        glm::vec3(colors.accent.r, colors.accent.g, colors.accent.b));
    addWeaponHardpoints(vertices, indices, length * 0.3f, width * 0.9f, height * 0.3f, 4,
                        glm::vec3(colors.accent.r, colors.accent.g, colors.accent.b));
    
    // Missile launcher bays on sides
    addWeaponHardpoints(vertices, indices, length * 0.2f, width * 1.0f, 0.0f, 4,
                        glm::vec3(colors.secondary.r, colors.secondary.g, colors.secondary.b));
    
    // Massive engine array at rear - 6 large exhausts
    addEngineDetail(vertices, indices, -length * 0.22f, width * 1.2f, height * 0.5f, 6,
                    glm::vec3(colors.accent.r * 1.3f, colors.accent.g * 1.3f, colors.accent.b * 1.8f));
    
    // Command superstructure detail
    addHullPanelLines(vertices, indices, length * 0.6f, 0.0f, width * 1.1f,
                     glm::vec3(colors.primary.r, colors.primary.g, colors.primary.b));

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
