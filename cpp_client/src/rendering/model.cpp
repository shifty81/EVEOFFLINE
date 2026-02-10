#include "rendering/model.h"
#include "rendering/mesh.h"
#include "rendering/procedural_mesh_ops.h"
#include "rendering/procedural_ship_generator.h"
#include "rendering/ship_part_library.h"
#include "rendering/ship_generation_rules.h"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <filesystem>
#include <functional>

#ifdef _WIN32
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#include <climits>
#else
#include <unistd.h>
#include <climits>
#endif

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

// Returns the directory containing the running executable.
static std::string getExecutableDir() {
    std::filesystem::path exePath;
#ifdef _WIN32
    char buf[MAX_PATH];
    DWORD len = GetModuleFileNameA(nullptr, buf, MAX_PATH);
    if (len > 0 && len < MAX_PATH) {
        exePath = buf;
    }
#elif defined(__APPLE__)
    char buf[PATH_MAX];
    uint32_t size = sizeof(buf);
    if (_NSGetExecutablePath(buf, &size) == 0) {
        exePath = std::filesystem::canonical(buf);
    }
#else
    char buf[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (len != -1) {
        buf[len] = '\0';
        exePath = buf;
    }
#endif
    if (!exePath.empty()) {
        return exePath.parent_path().string();
    }
    return "";
}

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

std::string Model::findOBJModelPath(const std::string& shipType, const std::string& faction) {
    // Search directories relative to the current working directory
    std::vector<std::string> searchDirs = {
        "models/ships",
        "../models/ships",
        "bin/models/ships",
        "../bin/models/ships"
    };

    // Also search relative to the executable directory so that OBJ files are
    // found regardless of the current working directory (e.g. when the game is
    // launched from an IDE or a different shell location).
    std::string exeDir = getExecutableDir();
    if (!exeDir.empty()) {
        std::filesystem::path exeDirPath(exeDir);
        // Models next to the executable (e.g. bin/models/ships)
        searchDirs.push_back((exeDirPath / "models" / "ships").string());
        // Multi-config generators (Visual Studio/Xcode) place the binary in
        // a configuration subdirectory such as bin/Release/ or bin/Debug/.
        // The models directory lives one level above that.
        searchDirs.push_back((exeDirPath.parent_path() / "models" / "ships").string());
    }

    // Convert faction to lowercase for filename matching
    std::string factionLower = faction;
    std::transform(factionLower.begin(), factionLower.end(), factionLower.begin(), ::tolower);

    // Convert shipType to lowercase once before the directory loop
    std::string shipTypeLower = shipType;
    std::transform(shipTypeLower.begin(), shipTypeLower.end(), shipTypeLower.begin(), ::tolower);

    for (const auto& dir : searchDirs) {
        if (!std::filesystem::exists(dir)) continue;

        // Try to find an OBJ file whose name ends with the ship type
        // File convention: {faction}_{class}_{ShipName}.obj
        try {
            for (const auto& entry : std::filesystem::directory_iterator(dir)) {
                if (!entry.is_regular_file()) continue;
                std::string filename = entry.path().filename().string();
                if (filename.size() < 4 || filename.substr(filename.size() - 4) != ".obj") continue;

                // Check if filename starts with the faction (case-insensitive)
                std::string filenameLower = filename;
                std::transform(filenameLower.begin(), filenameLower.end(), filenameLower.begin(), ::tolower);

                if (filenameLower.find(factionLower + "_") != 0) continue;

                // Strip the .obj extension for matching
                std::string baseLower = filenameLower.substr(0, filenameLower.size() - 4);

                // Match by ship name at end of filename after last underscore
                size_t lastUnderscore = baseLower.rfind('_');
                if (lastUnderscore != std::string::npos) {
                    std::string modelShipName = baseLower.substr(lastUnderscore + 1);
                    if (modelShipName == shipTypeLower) {
                        std::cout << "Found OBJ model: " << entry.path().string() << " for " << shipType << std::endl;
                        return entry.path().string();
                    }
                }
            }
        } catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "Error scanning model directory " << dir << ": " << e.what() << std::endl;
        }
    }

    return "";
}

std::unique_ptr<Model> Model::createShipModel(const std::string& shipType, const std::string& faction) {
    // Try to load from OBJ file first
    std::string objPath = findOBJModelPath(shipType, faction);
    if (!objPath.empty()) {
        auto model = std::make_unique<Model>();
        if (model->loadFromFile(objPath)) {
            std::cout << "Using OBJ model for " << shipType << " (" << faction << ")" << std::endl;
            return model;
        }
        std::cerr << "Failed to load OBJ model, falling back to procedural generation" << std::endl;
    }

    // Try procedural generation from a seed OBJ (reference assets)
    {
        ProceduralShipGenerator generator;
        // Configure reference assets (99-intergalactic_spaceship-obj & 24-textures)
        ReferenceAssetConfig assetCfg;
        assetCfg.objArchivePath      = "testing/99-intergalactic_spaceship-obj.rar";
        assetCfg.textureArchivePath  = "testing/24-textures.zip";
        assetCfg.extractedObjDir     = "models/ships";
        assetCfg.extractedTextureDir = "textures";
        generator.setReferenceAssets(assetCfg);

        // Determine ship class for seed lookup
        std::string shipClass = "generic";
        if (isFrigate(shipType))         shipClass = "frigate";
        else if (isDestroyer(shipType))  shipClass = "destroyer";
        else if (isCruiser(shipType))    shipClass = "cruiser";
        else if (isBattlecruiser(shipType)) shipClass = "battlecruiser";
        else if (isBattleship(shipType)) shipClass = "battleship";
        else if (isCarrier(shipType))    shipClass = "carrier";
        else if (isDreadnought(shipType)) shipClass = "dreadnought";
        else if (isTitan(shipType))      shipClass = "titan";

        std::string seedPath = generator.findSeedOBJ(faction, shipClass);
        if (!seedPath.empty()) {
            ProceduralShipParams params;
            // Generate a deterministic seed from ship type + faction
            params.seed = static_cast<unsigned int>(
                std::hash<std::string>{}(shipType + "_" + faction));
            params.enforceSymmetry = true;

            // Scale procedural detail by ship class — small ships get less
            // noise and fewer extrusions for cleaner silhouettes
            if (shipClass == "frigate") {
                params.extrusionCount = 2;          // Minimal greebles
                params.extrusionDepth = 0.08f;      // Shallow extrusions
                params.noiseAmplitude = 0.0f;       // No noise for clean look
                params.engineCount = 2;
                params.weaponCount = 1;
                params.antennaCount = 0;
            } else if (shipClass == "destroyer") {
                params.extrusionCount = 3;
                params.extrusionDepth = 0.10f;
                params.noiseAmplitude = 0.0f;
                params.engineCount = 2;
                params.weaponCount = 2;
                params.antennaCount = 0;
            } else {
                params.extrusionCount = 5;
                params.extrusionDepth = 0.15f;
                params.engineCount = 2;
                params.weaponCount = 2;
                params.antennaCount = 0;
            }

            // Faction-specific colour
            FactionColors fc = getFactionColors(faction);
            params.primaryColor = glm::vec3(fc.primary);

            auto model = generator.generateFromFile(seedPath, params);
            if (model) {
                std::cout << "Using procedural seed OBJ for " << shipType
                          << " (" << faction << ")" << std::endl;
                return model;
            }
        }
    }

    // Fall back to procedural generation
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

// ==================== Procedural Hull Generation via buildSegmentedHull ====================

/**
 * Parameters controlling the procedural hull builder for each ship class.
 * These replace the old ad-hoc vertex/index generation that produced broken
 * triangles (the "squiggly lines" problem).
 */
struct HullParams {
    int sides;            // Cross-section polygon sides (4=blocky, 6=angular, 8=refined, 12=smooth)
    int segments;         // Number of extrusion steps along hull length
    float segmentLength;  // Length of each segment
    float baseRadius;     // Starting cross-section radius
    float scaleX;         // Width scale on cross-section
    float scaleZ;         // Height scale on cross-section
    unsigned int seed;    // Deterministic seed for radius variation
};

/**
 * Get faction-appropriate cross-section sides.
 * More sides = smoother silhouette.
 */
static int getFactionSides(const std::string& faction) {
    // Original EVE factions
    if (faction.find("Veyren") != std::string::npos) return 4;   // Blocky/angular
    if (faction.find("Keldari") != std::string::npos) return 6;  // Industrial/angular
    if (faction.find("Solari") != std::string::npos) return 8;     // Refined/ornate
    if (faction.find("Aurelian") != std::string::npos) return 12; // Smooth/organic
    // New PVE factions (matching their analog's design language)
    if (faction.find("Core Nexus") != std::string::npos) return 4;           // Veyren analog — blocky
    if (faction.find("Rust-Scrap") != std::string::npos) return 6;           // Keldari analog — industrial
    if (faction.find("Sanctum Hegemony") != std::string::npos) return 8;     // Solari analog — ornate
    if (faction.find("Vanguard Republic") != std::string::npos) return 12;   // Aurelian analog — smooth
    return 6; // Default
}

// Forward declaration for buildShipFromParams (defined later in this file)
static std::unique_ptr<Model> buildShipFromParams(
    const HullParams& params,
    const FactionColors& colors);

std::unique_ptr<Model> Model::createShipModelWithRacialDesign(const std::string& shipType, const std::string& faction) {
    // Try OBJ model first
    std::string objPath = findOBJModelPath(shipType, faction);
    if (!objPath.empty()) {
        auto model = std::make_unique<Model>();
        if (model->loadFromFile(objPath)) {
            return model;
        }
    }

    FactionColors colors = getFactionColors(faction);

    // Determine ship class for hull parameters
    std::string shipClass;
    if (isFrigate(shipType))           shipClass = "Frigate";
    else if (isDestroyer(shipType))    shipClass = "Destroyer";
    else if (isTech2Cruiser(shipType)) shipClass = "Cruiser";
    else if (isCruiser(shipType))      shipClass = "Cruiser";
    else if (isCommandShip(shipType))  shipClass = "Battlecruiser";
    else if (isBattlecruiser(shipType))shipClass = "Battlecruiser";
    else if (isBattleship(shipType))   shipClass = "Battleship";
    else if (isCarrier(shipType))      shipClass = "Carrier";
    else if (isDreadnought(shipType))  shipClass = "Dreadnought";
    else if (isTitan(shipType))        shipClass = "Titan";
    else if (isMiningBarge(shipType))  shipClass = "Frigate";
    else if (isStation(shipType))      return createStationModel(colors, shipType);
    else if (isAsteroid(shipType))     return createAsteroidModel(shipType);
    else                               shipClass = "Frigate";

    // Use ShipPartLibrary to get faction/class-appropriate assembly config
    // with per-ship-type seed for unique but deterministic variation
    ShipVariationParams variation;
    variation.seed = static_cast<unsigned int>(std::hash<std::string>{}(shipType + "|" + faction));
    variation.proportionJitter = 0.3f;
    variation.scaleJitter = 0.1f;

    ShipPartLibrary library;
    library.initialize();
    ShipAssemblyConfig config = library.createVariedAssemblyConfig(shipClass, faction, variation);

    // Map assembly config to hull parameters using faction-appropriate sides
    HullParams p;
    p.sides = getFactionSides(faction);
    p.seed = variation.seed;
    p.scaleX = config.proportions.y / config.proportions.x;
    p.scaleZ = config.proportions.z / config.proportions.x;

    // Map ship class to segment count and dimensions — small ships tuned for clean silhouettes
    if (shipClass == "Frigate") {
        p.segments = 5;
        p.segmentLength = config.overallScale / 5.5f;   // Tighter segments
        p.baseRadius = config.overallScale * 0.09f;      // Sleeker radius
    } else if (shipClass == "Destroyer") {
        p.segments = 6;
        p.segmentLength = config.overallScale / 6.0f;
        p.baseRadius = config.overallScale * 0.065f;     // Thinner, more elongated
    } else if (shipClass == "Cruiser") {
        p.segments = 6;
        p.segmentLength = config.overallScale / 6.0f;
        p.baseRadius = config.overallScale * 0.12f;
    } else if (shipClass == "Battlecruiser") {
        p.segments = 7;
        p.segmentLength = config.overallScale / 7.0f;
        p.baseRadius = config.overallScale * 0.10f;
    } else if (shipClass == "Battleship") {
        p.segments = 8;
        p.segmentLength = config.overallScale / 8.0f;
        p.baseRadius = config.overallScale * 0.09f;
    } else if (shipClass == "Carrier") {
        p.segments = 10;
        p.segmentLength = config.overallScale / 10.0f;
        p.baseRadius = config.overallScale * 0.08f;
    } else if (shipClass == "Dreadnought") {
        p.segments = 8;
        p.segmentLength = config.overallScale / 8.0f;
        p.baseRadius = config.overallScale * 0.11f;
    } else if (shipClass == "Titan") {
        p.segments = 12;
        p.segmentLength = config.overallScale / 12.0f;
        p.baseRadius = config.overallScale * 0.07f;
    } else {
        p.segments = 4;
        p.segmentLength = config.overallScale / 4.0f;
        p.baseRadius = config.overallScale * 0.13f;
    }

    return buildShipFromParams(p, colors);
}

void Model::draw() const {
    for (const auto& mesh : m_meshes) {
        mesh->draw();
    }
}

// Ship type checking functions
bool Model::isFrigate(const std::string& shipType) {
    static const std::vector<std::string> frigateNames = {
        "Frigate", "Fang", "Falk", "Revel", "Sentinel",
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
        "Destroyer", "Thrasher", "Cormorant", "Vipere", "Coercer",
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
        "Heavy Assault Cruiser", "Wanderer", "Hydralisk", "Imperatrice", "Ardent",
        "Gunnolf", "Valdris", "Cavalier", "Inquisitor",
        "Heavy Interdiction Cruiser", "Ironclamp", "Frostlok", "Grappleur", "Warden",
        "Force Recon Ship", "Farseer", "Ghostblade", "Skygaze", "Surveillant", "Wayfarer",
        "Combat Recon Ship", "Watchkeep", "Arbitre", "Maledictus",
        "Logistics Cruiser", "Lifeblood", "Restorer", "Soigneur", "Protector"
    };
    return std::any_of(tech2Names.begin(), tech2Names.end(),
        [&shipType](const std::string& name) { return shipType.find(name) != std::string::npos; });
}

bool Model::isBattlecruiser(const std::string& shipType) {
    static const std::vector<std::string> bcNames = {
        "Battlecruiser", "Galeforce", "Fenvar", "Marquis", "Herald"
    };
    return std::any_of(bcNames.begin(), bcNames.end(),
        [&shipType](const std::string& name) { return shipType.find(name) != std::string::npos; });
}

bool Model::isCommandShip(const std::string& shipType) {
    static const std::vector<std::string> csNames = {
        "Command Ship", "Warleader", "Jarl", "Commandant", "Paragon"
    };
    return std::any_of(csNames.begin(), csNames.end(),
        [&shipType](const std::string& name) { return shipType.find(name) != std::string::npos; });
}

bool Model::isBattleship(const std::string& shipType) {
    static const std::vector<std::string> bsNames = {
        "Battleship", "Gale", "Strix", "Sovereign", "Exarch",
        "Marauder", "Ironheart", "Monolith", "Majeste", "Solarius Prime"
    };
    return std::any_of(bsNames.begin(), bsNames.end(),
        [&shipType](const std::string& name) { return shipType.find(name) != std::string::npos; });
}

bool Model::isMiningBarge(const std::string& shipType) {
    static const std::vector<std::string> miningNames = {
        "Mining Barge", "Ironbore", "Deepscoop", "Yieldmaster", "Exhumer", "Excavon", "Vasthold", "Coreshield",
        "Industrial", "Packrunner", "Drifthauler", "Marchand", "Wayfarer Hauler"
    };
    return std::any_of(miningNames.begin(), miningNames.end(),
        [&shipType](const std::string& name) { return shipType.find(name) != std::string::npos; });
}

bool Model::isCarrier(const std::string& shipType) {
    static const std::vector<std::string> carrierNames = {
        "Carrier", "Solarius", "Lumiere", "Draknar", "Ironprow",
        "Supercarrier"
    };
    return std::any_of(carrierNames.begin(), carrierNames.end(),
        [&shipType](const std::string& name) { return shipType.find(name) != std::string::npos; });
}

bool Model::isDreadnought(const std::string& shipType) {
    static const std::vector<std::string> dreadNames = {
        "Dreadnought", "Sanctum", "Bastion Royal", "Valkyr", "Thornwall"
    };
    return std::any_of(dreadNames.begin(), dreadNames.end(),
        [&shipType](const std::string& name) { return shipType.find(name) != std::string::npos; });
}

bool Model::isTitan(const std::string& shipType) {
    static const std::vector<std::string> titanNames = {
        "Titan", "Empyrean", "Grandeur", "Jormundur", "Worldbreaker"
    };
    return std::any_of(titanNames.begin(), titanNames.end(),
        [&shipType](const std::string& name) { return shipType.find(name) != std::string::npos; });
}

bool Model::isStation(const std::string& shipType) {
    static const std::vector<std::string> stationNames = {
        "Station", "Citadel", "Bastion", "Fortress", "Beacon",
        "Outpost", "Refinery", "Engineering Complex"
    };
    return std::any_of(stationNames.begin(), stationNames.end(),
        [&shipType](const std::string& name) { return shipType.find(name) != std::string::npos; });
}

bool Model::isAsteroid(const std::string& shipType) {
    static const std::vector<std::string> asteroidNames = {
        "Asteroid", "Dustite", "Ferrite", "Ignaite", "Crystite",
        "Shadite", "Corite", "Lumine", "Sangite", "Glacite",
        "Densite", "Voidite", "Pyranite", "Stellite", "Cosmite", "Nexorite"
    };
    return std::any_of(asteroidNames.begin(), asteroidNames.end(),
        [&shipType](const std::string& name) { return shipType.find(name) != std::string::npos; });
}

FactionColors Model::getFactionColors(const std::string& faction) {
    static const std::map<std::string, FactionColors> colorMap = {
        // === Original EVE Online factions ===
        {"Keldari", {
            glm::vec4(0.5f, 0.35f, 0.25f, 1.0f),  // Rust brown
            glm::vec4(0.3f, 0.2f, 0.15f, 1.0f),   // Dark brown
            glm::vec4(0.8f, 0.6f, 0.3f, 1.0f)     // Light rust
        }},
        {"Veyren", {
            glm::vec4(0.35f, 0.45f, 0.55f, 1.0f), // Steel blue
            glm::vec4(0.2f, 0.25f, 0.35f, 1.0f),  // Dark blue
            glm::vec4(0.5f, 0.7f, 0.9f, 1.0f)     // Light blue
        }},
        {"Aurelian", {
            glm::vec4(0.3f, 0.4f, 0.35f, 1.0f),   // Dark green-gray
            glm::vec4(0.2f, 0.3f, 0.25f, 1.0f),   // Darker green
            glm::vec4(0.4f, 0.7f, 0.5f, 1.0f)     // Light green
        }},
        {"Solari", {
            glm::vec4(0.6f, 0.55f, 0.45f, 1.0f),  // Gold-brass
            glm::vec4(0.4f, 0.35f, 0.25f, 1.0f),  // Dark gold
            glm::vec4(0.9f, 0.8f, 0.5f, 1.0f)     // Bright gold
        }},
        // === New PVE factions (from game design doc) ===
        {"Sanctum Hegemony", {                      // Solari analog
            glm::vec4(0.7f, 0.6f, 0.35f, 1.0f),   // Polished gold
            glm::vec4(0.45f, 0.38f, 0.2f, 1.0f),   // Dark gold
            glm::vec4(0.95f, 0.85f, 0.45f, 1.0f)  // Bright gold shine
        }},
        {"Core Nexus", {                            // Veyren analog
            glm::vec4(0.3f, 0.35f, 0.42f, 1.0f),  // Dark steel grey
            glm::vec4(0.15f, 0.18f, 0.25f, 1.0f),  // Near-black blue
            glm::vec4(0.45f, 0.55f, 0.7f, 1.0f)   // Muted blue accent
        }},
        {"Vanguard Republic", {                     // Aurelian analog
            glm::vec4(0.25f, 0.4f, 0.38f, 1.0f),  // Teal-green
            glm::vec4(0.15f, 0.28f, 0.25f, 1.0f),  // Dark teal
            glm::vec4(0.35f, 0.65f, 0.55f, 1.0f)  // Light sea-green
        }},
        {"Rust-Scrap Coalition", {                  // Keldari analog
            glm::vec4(0.55f, 0.25f, 0.2f, 1.0f),  // Rusty red
            glm::vec4(0.2f, 0.12f, 0.1f, 1.0f),   // Dark rust/black
            glm::vec4(0.75f, 0.4f, 0.25f, 1.0f)   // Orange-rust accent
        }},
        // === Pirate/NPC factions ===
        {"Venom Syndicate", {
            glm::vec4(0.4f, 0.25f, 0.45f, 1.0f),  // Purple
            glm::vec4(0.2f, 0.15f, 0.25f, 1.0f),  // Dark purple
            glm::vec4(0.7f, 0.3f, 0.7f, 1.0f)     // Bright purple
        }},
        {"Iron Corsairs", {
            glm::vec4(0.5f, 0.2f, 0.2f, 1.0f),    // Dark red
            glm::vec4(0.3f, 0.1f, 0.1f, 1.0f),    // Very dark red
            glm::vec4(0.9f, 0.3f, 0.3f, 1.0f)     // Bright red
        }},
        {"Crimson Order", {
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

/**
 * Build a complete ship model from hull parameters using the procedural
 * mesh system.  Generates a TriangulatedMesh with correct triangles,
 * proper winding order, and valid indices (always multiples of 3).
 *
 * Replaces the old manual vertex/index code that produced degenerate
 * triangles and squiggly rendering artifacts.
 */
static std::unique_ptr<Model> buildShipFromParams(
    const HullParams& params,
    const FactionColors& colors) {

    auto model = std::make_unique<Model>();

    glm::vec3 primaryColor(colors.primary.r, colors.primary.g, colors.primary.b);

    auto mults = generateRadiusMultipliers(params.segments, params.baseRadius, params.seed);

    TriangulatedMesh hull = buildSegmentedHull(
        params.sides, params.segments,
        params.segmentLength, params.baseRadius,
        mults, params.scaleX, params.scaleZ,
        primaryColor);

    if (!hull.vertices.empty() && !hull.indices.empty()) {
        auto mesh = std::make_unique<Mesh>(hull.vertices, hull.indices);
        model->addMesh(std::move(mesh));
    }

    return model;
}

// ==================== Enhanced Procedural Detail Generation ====================

/**
 * Get design traits based on faction and ship class
 */
ShipDesignTraits Model::getDesignTraits(const std::string& faction, const std::string& shipClass) {
    ShipDesignTraits traits;
    
    // Determine faction design style
    if (faction.find("Veyren") != std::string::npos ||
        faction.find("Core Nexus") != std::string::npos) {
        traits.style = ShipDesignTraits::DesignStyle::CALDARI_BLOCKY;
        traits.isBlocky = true;
        traits.isOrganic = false;
        traits.isAsymmetric = false;
        traits.hasSpires = false;
        traits.hasExposedFramework = false;
        traits.asymmetryFactor = 0.0f;
    } else if (faction.find("Solari") != std::string::npos ||
               faction.find("Sanctum Hegemony") != std::string::npos) {
        traits.style = ShipDesignTraits::DesignStyle::AMARR_ORNATE;
        traits.hasSpires = true;
        traits.isBlocky = false;
        traits.isOrganic = false;
        traits.isAsymmetric = false;
        traits.hasExposedFramework = false;
        traits.asymmetryFactor = 0.0f;
    } else if (faction.find("Aurelian") != std::string::npos ||
               faction.find("Vanguard Republic") != std::string::npos) {
        traits.style = ShipDesignTraits::DesignStyle::GALLENTE_ORGANIC;
        traits.isOrganic = true;
        traits.isBlocky = false;
        traits.isAsymmetric = false;
        traits.hasSpires = false;
        traits.hasExposedFramework = false;
        traits.asymmetryFactor = 0.0f;
    } else if (faction.find("Keldari") != std::string::npos ||
               faction.find("Rust-Scrap") != std::string::npos) {
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
 * Add Solari-style spire detail
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
 * Add Keldari-style asymmetric detail
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

// Ship model creation functions — small class ships tuned for clean, tight silhouettes
std::unique_ptr<Model> Model::createFrigateModel(const FactionColors& colors) {
    HullParams p;
    p.sides = 6;
    p.segments = 5;           // One more segment for better shape definition
    p.segmentLength = 0.7f;   // Shorter segments for tighter proportions
    p.baseRadius = 0.3f;      // Smaller radius for sleeker frigate silhouette
    p.scaleX = 1.1f;          // Slightly wider than tall
    p.scaleZ = 0.65f;         // Flatter profile — more spaceship-like
    p.seed = 100u;
    return buildShipFromParams(p, colors);
}

std::unique_ptr<Model> Model::createDestroyerModel(const FactionColors& colors) {
    HullParams p;
    p.sides = 6;
    p.segments = 6;           // More segments for elongated destroyer look
    p.segmentLength = 0.8f;   // Slightly longer segments
    p.baseRadius = 0.25f;     // Thinner than frigate — more elongated
    p.scaleX = 0.9f;          // Slightly narrower
    p.scaleZ = 0.6f;          // Flat profile
    p.seed = 200u;
    return buildShipFromParams(p, colors);
}

std::unique_ptr<Model> Model::createCruiserModel(const FactionColors& colors) {
    HullParams p;
    p.sides = 6;
    p.segments = 6;
    p.segmentLength = 1.0f;
    p.baseRadius = 0.65f;
    p.scaleX = 1.2f;
    p.scaleZ = 0.8f;
    p.seed = 300u;
    return buildShipFromParams(p, colors);
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
    HullParams p;
    p.sides = 6;
    p.segments = 7;
    p.segmentLength = 1.2f;
    p.baseRadius = 0.8f;
    p.scaleX = 1.1f;
    p.scaleZ = 0.9f;
    p.seed = 500u;
    return buildShipFromParams(p, colors);
}

std::unique_ptr<Model> Model::createBattleshipModel(const FactionColors& colors) {
    HullParams p;
    p.sides = 8;
    p.segments = 8;
    p.segmentLength = 1.5f;
    p.baseRadius = 1.0f;
    p.scaleX = 1.2f;
    p.scaleZ = 0.85f;
    p.seed = 600u;
    return buildShipFromParams(p, colors);
}

std::unique_ptr<Model> Model::createMiningBargeModel(const FactionColors& colors) {
    HullParams p;
    p.sides = 4;
    p.segments = 5;
    p.segmentLength = 1.2f;
    p.baseRadius = 0.9f;
    p.scaleX = 1.5f;
    p.scaleZ = 0.7f;
    p.seed = 700u;
    return buildShipFromParams(p, colors);
}

std::unique_ptr<Model> Model::createGenericModel(const FactionColors& colors) {
    // Default to frigate model for unknown ship types
    return createFrigateModel(colors);
}

std::unique_ptr<Model> Model::createCarrierModel(const FactionColors& colors) {
    HullParams p;
    p.sides = 8;
    p.segments = 10;
    p.segmentLength = 1.5f;
    p.baseRadius = 1.2f;
    p.scaleX = 1.6f;
    p.scaleZ = 0.6f;
    p.seed = 800u;
    return buildShipFromParams(p, colors);
}

std::unique_ptr<Model> Model::createDreadnoughtModel(const FactionColors& colors) {
    HullParams p;
    p.sides = 6;
    p.segments = 8;
    p.segmentLength = 1.5f;
    p.baseRadius = 1.3f;
    p.scaleX = 1.0f;
    p.scaleZ = 1.1f;
    p.seed = 900u;
    return buildShipFromParams(p, colors);
}

std::unique_ptr<Model> Model::createTitanModel(const FactionColors& colors) {
    HullParams p;
    p.sides = 8;
    p.segments = 12;
    p.segmentLength = 2.0f;
    p.baseRadius = 1.8f;
    p.scaleX = 1.1f;
    p.scaleZ = 0.9f;
    p.seed = 1000u;
    return buildShipFromParams(p, colors);
}

std::unique_ptr<Model> Model::createStationModel(const FactionColors& colors, const std::string& stationType) {
    // Stations use a high-sided cylindrical hull for a ring/hub shape
    HullParams p;
    p.sides = 12;
    p.segments = 6;
    p.segmentLength = 3.0f;
    p.baseRadius = 3.0f;
    p.scaleX = 1.0f;
    p.scaleZ = 1.0f;
    p.seed = static_cast<unsigned int>(std::hash<std::string>{}(stationType));
    return buildShipFromParams(p, colors);
}

std::unique_ptr<Model> Model::createAsteroidModel(const std::string& oreType) {
    auto model = std::make_unique<Model>();

    // Determine color based on ore type
    glm::vec3 asteroidColor = glm::vec3(0.5f, 0.5f, 0.5f); // Default gray

    if (oreType.find("Dustite") != std::string::npos) {
        asteroidColor = glm::vec3(0.6f, 0.4f, 0.2f);
    } else if (oreType.find("Ferrite") != std::string::npos) {
        asteroidColor = glm::vec3(0.5f, 0.5f, 0.55f);
    } else if (oreType.find("Ignaite") != std::string::npos) {
        asteroidColor = glm::vec3(0.7f, 0.3f, 0.2f);
    } else if (oreType.find("Crystite") != std::string::npos) {
        asteroidColor = glm::vec3(0.3f, 0.5f, 0.4f);
    } else if (oreType.find("Shadite") != std::string::npos) {
        asteroidColor = glm::vec3(0.8f, 0.6f, 0.3f);
    } else if (oreType.find("Corite") != std::string::npos) {
        asteroidColor = glm::vec3(0.3f, 0.6f, 0.7f);
    } else if (oreType.find("Lumine") != std::string::npos) {
        asteroidColor = glm::vec3(0.6f, 0.2f, 0.3f);
    } else if (oreType.find("Sangite") != std::string::npos) {
        asteroidColor = glm::vec3(0.9f, 0.3f, 0.2f);
    }

    // Use irregular polygon extrusion for a rocky, lumpy shape
    unsigned int seed = static_cast<unsigned int>(std::hash<std::string>{}(oreType));
    auto mults = generateRadiusMultipliers(3, 2.0f, seed);
    TriangulatedMesh hull = buildSegmentedHull(
        5, 3, 1.5f, 2.0f, mults, 1.0f, 1.0f, asteroidColor);

    if (!hull.vertices.empty() && !hull.indices.empty()) {
        auto mesh = std::make_unique<Mesh>(hull.vertices, hull.indices);
        model->addMesh(std::move(mesh));
    }

    return model;
}

} // namespace eve
