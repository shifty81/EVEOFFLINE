#include "rendering/pbr_materials.h"
#include <iostream>

namespace eve {

PBRMaterialLibrary::PBRMaterialLibrary() {
}

PBRMaterialLibrary::~PBRMaterialLibrary() {
}

void PBRMaterialLibrary::initialize() {
    std::cout << "Initializing PBR material library..." << std::endl;
    
    createDefaultMaterials();
    createMetalMaterials();
    createPaintedMaterials();
    createShipHullMaterials();
    createEmissiveMaterials();
    createFactionMaterials();
    
    std::cout << "Loaded " << m_materials.size() << " PBR materials" << std::endl;
}

const PBRMaterial* PBRMaterialLibrary::getMaterial(const std::string& name) const {
    auto it = m_materials.find(name);
    if (it != m_materials.end()) {
        return &it->second;
    }
    
    // Return default material if not found
    static PBRMaterial defaultMaterial;
    return &defaultMaterial;
}

void PBRMaterialLibrary::addMaterial(const std::string& name, const PBRMaterial& material) {
    m_materials[name] = material;
}

PBRMaterial PBRMaterialLibrary::createFactionMaterial(const std::string& faction) {
    std::string materialName = faction + "_hull";
    const PBRMaterial* mat = getMaterial(materialName);
    if (mat) {
        return *mat;
    }
    
    // Return default if faction material doesn't exist
    return PBRMaterial();
}

std::vector<std::string> PBRMaterialLibrary::getMaterialNames() const {
    std::vector<std::string> names;
    names.reserve(m_materials.size());
    
    for (const auto& pair : m_materials) {
        names.push_back(pair.first);
    }
    
    return names;
}

void PBRMaterialLibrary::createDefaultMaterials() {
    // Default material
    PBRMaterial defaultMat;
    defaultMat.albedo = glm::vec3(0.8f, 0.8f, 0.8f);
    defaultMat.metallic = 0.0f;
    defaultMat.roughness = 0.5f;
    addMaterial("default", defaultMat);
}

void PBRMaterialLibrary::createMetalMaterials() {
    // Polished steel
    PBRMaterial steel;
    steel.albedo = glm::vec3(0.78f, 0.78f, 0.78f);
    steel.metallic = 1.0f;
    steel.roughness = 0.2f;
    addMaterial("steel_polished", steel);
    
    // Brushed steel
    PBRMaterial steelBrushed;
    steelBrushed.albedo = glm::vec3(0.7f, 0.7f, 0.7f);
    steelBrushed.metallic = 1.0f;
    steelBrushed.roughness = 0.4f;
    addMaterial("steel_brushed", steelBrushed);
    
    // Titanium
    PBRMaterial titanium;
    titanium.albedo = glm::vec3(0.65f, 0.65f, 0.7f);
    titanium.metallic = 1.0f;
    titanium.roughness = 0.3f;
    addMaterial("titanium", titanium);
    
    // Gold
    PBRMaterial gold;
    gold.albedo = glm::vec3(1.0f, 0.86f, 0.57f);
    gold.metallic = 1.0f;
    gold.roughness = 0.15f;
    addMaterial("gold", gold);
    
    // Copper
    PBRMaterial copper;
    copper.albedo = glm::vec3(0.95f, 0.64f, 0.54f);
    copper.metallic = 1.0f;
    copper.roughness = 0.25f;
    addMaterial("copper", copper);
}

void PBRMaterialLibrary::createPaintedMaterials() {
    // Glossy paint
    PBRMaterial glossyPaint;
    glossyPaint.albedo = glm::vec3(0.8f, 0.2f, 0.2f); // Red
    glossyPaint.metallic = 0.0f;
    glossyPaint.roughness = 0.1f;
    addMaterial("paint_glossy", glossyPaint);
    
    // Matte paint
    PBRMaterial mattePaint;
    mattePaint.albedo = glm::vec3(0.3f, 0.3f, 0.3f); // Gray
    mattePaint.metallic = 0.0f;
    mattePaint.roughness = 0.8f;
    addMaterial("paint_matte", mattePaint);
}

void PBRMaterialLibrary::createShipHullMaterials() {
    // Generic ship hull
    PBRMaterial hull;
    hull.albedo = glm::vec3(0.5f, 0.5f, 0.5f);
    hull.metallic = 0.8f;
    hull.roughness = 0.4f;
    addMaterial("ship_hull", hull);
    
    // Damaged hull
    PBRMaterial hullDamaged;
    hullDamaged.albedo = glm::vec3(0.4f, 0.35f, 0.3f);
    hullDamaged.metallic = 0.6f;
    hullDamaged.roughness = 0.7f;
    addMaterial("ship_hull_damaged", hullDamaged);
}

void PBRMaterialLibrary::createEmissiveMaterials() {
    // Engine glow
    PBRMaterial engineGlow;
    engineGlow.albedo = glm::vec3(0.1f, 0.1f, 0.1f);
    engineGlow.metallic = 0.0f;
    engineGlow.roughness = 0.5f;
    engineGlow.emissive = glm::vec3(1.0f, 0.6f, 0.2f); // Orange glow
    addMaterial("engine_glow", engineGlow);
    
    // Shield emitter
    PBRMaterial shieldEmitter;
    shieldEmitter.albedo = glm::vec3(0.1f, 0.1f, 0.1f);
    shieldEmitter.metallic = 0.8f;
    shieldEmitter.roughness = 0.2f;
    shieldEmitter.emissive = glm::vec3(0.2f, 0.6f, 1.0f); // Blue glow
    addMaterial("shield_emitter", shieldEmitter);
}

void PBRMaterialLibrary::createFactionMaterials() {
    // Minmatar - Rust and weathered metal
    PBRMaterial minmatar;
    minmatar.albedo = glm::vec3(0.5f, 0.35f, 0.25f);
    minmatar.metallic = 0.7f;
    minmatar.roughness = 0.6f;
    addMaterial("Minmatar_hull", minmatar);
    
    // Caldari - Polished steel and blue
    PBRMaterial caldari;
    caldari.albedo = glm::vec3(0.35f, 0.45f, 0.55f);
    caldari.metallic = 0.9f;
    caldari.roughness = 0.3f;
    addMaterial("Caldari_hull", caldari);
    
    // Gallente - Green-gray composite
    PBRMaterial gallente;
    gallente.albedo = glm::vec3(0.3f, 0.4f, 0.35f);
    gallente.metallic = 0.6f;
    gallente.roughness = 0.4f;
    addMaterial("Gallente_hull", gallente);
    
    // Amarr - Gold and brass
    PBRMaterial amarr;
    amarr.albedo = glm::vec3(0.6f, 0.55f, 0.45f);
    amarr.metallic = 0.95f;
    amarr.roughness = 0.2f;
    addMaterial("Amarr_hull", amarr);
    
    // Serpentis - Dark purple
    PBRMaterial serpentis;
    serpentis.albedo = glm::vec3(0.4f, 0.25f, 0.45f);
    serpentis.metallic = 0.7f;
    serpentis.roughness = 0.35f;
    addMaterial("Serpentis_hull", serpentis);
    
    // Guristas - Dark red
    PBRMaterial guristas;
    guristas.albedo = glm::vec3(0.5f, 0.2f, 0.2f);
    guristas.metallic = 0.8f;
    guristas.roughness = 0.4f;
    addMaterial("Guristas_hull", guristas);
    
    // Blood Raiders - Crimson
    PBRMaterial bloodRaiders;
    bloodRaiders.albedo = glm::vec3(0.4f, 0.15f, 0.15f);
    bloodRaiders.metallic = 0.75f;
    bloodRaiders.roughness = 0.3f;
    addMaterial("Blood Raiders_hull", bloodRaiders);
}

} // namespace eve
