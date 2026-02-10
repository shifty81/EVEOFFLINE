#pragma once

#include <glm/glm.hpp>
#include <string>
#include <map>
#include <memory>
#include <vector>

namespace eve {

class Texture;

/**
 * PBR Material properties
 */
struct PBRMaterial {
    // Albedo color or texture
    glm::vec3 albedo;
    unsigned int albedoMap;
    
    // Metallic value (0 = dielectric, 1 = metal)
    float metallic;
    unsigned int metallicMap;
    
    // Roughness value (0 = smooth, 1 = rough)
    float roughness;
    unsigned int roughnessMap;
    
    // Normal map for surface detail
    unsigned int normalMap;
    
    // Ambient occlusion
    float ao;
    unsigned int aoMap;
    
    // Emissive color for glowing parts
    glm::vec3 emissive;
    unsigned int emissiveMap;
    
    // Material flags
    bool useAlbedoMap;
    bool useMetallicMap;
    bool useRoughnessMap;
    bool useNormalMap;
    bool useAOMap;
    bool useEmissiveMap;
    
    PBRMaterial()
        : albedo(1.0f, 1.0f, 1.0f)
        , albedoMap(0)
        , metallic(0.0f)
        , metallicMap(0)
        , roughness(0.5f)
        , roughnessMap(0)
        , normalMap(0)
        , ao(1.0f)
        , aoMap(0)
        , emissive(0.0f, 0.0f, 0.0f)
        , emissiveMap(0)
        , useAlbedoMap(false)
        , useMetallicMap(false)
        , useRoughnessMap(false)
        , useNormalMap(false)
        , useAOMap(false)
        , useEmissiveMap(false)
    {}
};

/**
 * PBR Material Library
 * Manages predefined PBR materials for ships and objects
 */
class PBRMaterialLibrary {
public:
    PBRMaterialLibrary();
    ~PBRMaterialLibrary();

    /**
     * Initialize the material library with default materials
     */
    void initialize();

    /**
     * Get a material by name
     */
    const PBRMaterial* getMaterial(const std::string& name) const;

    /**
     * Add a custom material
     */
    void addMaterial(const std::string& name, const PBRMaterial& material);

    /**
     * Create faction-specific ship material
     */
    PBRMaterial createFactionMaterial(const std::string& faction);

    /**
     * Get all material names
     */
    std::vector<std::string> getMaterialNames() const;

private:
    std::map<std::string, PBRMaterial> m_materials;
    
    // Helper methods to create default materials
    void createDefaultMaterials();
    void createMetalMaterials();
    void createPaintedMaterials();
    void createShipHullMaterials();
    void createEmissiveMaterials();
    void createFactionMaterials();
};

} // namespace eve
