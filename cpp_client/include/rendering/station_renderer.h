#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <string>
#include <map>
#include "mesh.h"
#include "shader.h"
#include "camera.h"

namespace atlas {

/**
 * Station Renderer
 * Renders procedural space stations using faction-specific designs
 * Based on data/universe/station_visual_data.json specifications
 */
class StationRenderer {
public:
    /**
     * Faction types for station styles
     */
    enum class FactionStyle {
        SOLARI,      // Cathedral style with golden spires
        VEYREN,      // Industrial blocky design
        AURELIAN,    // Organic spherical with green-blue glass
        KELDARI      // Rusty scaffolding, improvised
    };
    
    /**
     * Upwell structure types (player-owned)
     */
    enum class UpwellType {
        ASTRAHUS,    // Medium citadel
        FORTIZAR,    // Large citadel
        KEEPSTAR,    // XL citadel
        RAITARU      // Engineering complex
    };
    
    /**
     * Station instance data
     */
    struct StationInstance {
        std::string id;
        glm::vec3 position;
        glm::vec3 rotation;
        FactionStyle faction;
        float scale;
        bool isUpwell;
        UpwellType upwellType;
    };
    
    /**
     * Visual properties for faction stations
     */
    struct FactionVisuals {
        glm::vec3 primaryColor;
        glm::vec3 secondaryColor;
        glm::vec3 accentColor;
        glm::vec3 emissiveColor;
        float emissiveIntensity;
        float metallic;
        float roughness;
        float sizeMultiplier;
    };
    
    StationRenderer();
    ~StationRenderer();
    
    /**
     * Initialize station renderer
     * Creates procedural station meshes for all factions
     */
    bool initialize();
    
    /**
     * Add a station to be rendered
     */
    void addStation(const StationInstance& station);
    
    /**
     * Remove a station by ID
     */
    void removeStation(const std::string& id);
    
    /**
     * Clear all stations
     */
    void clearStations();
    
    /**
     * Render all stations
     */
    void render(Shader* shader, const Camera& camera);
    
    /**
     * Get number of stations
     */
    size_t getStationCount() const { return m_stations.size(); }

private:
    // Station meshes by faction
    std::map<FactionStyle, std::shared_ptr<Mesh>> m_factionStationMeshes;
    
    // Upwell structure meshes
    std::map<UpwellType, std::shared_ptr<Mesh>> m_upwellMeshes;
    
    // Active station instances
    std::vector<StationInstance> m_stations;
    
    // Visual properties for each faction
    std::map<FactionStyle, FactionVisuals> m_factionVisuals;
    
    /**
     * Create all station meshes
     */
    void createStationMeshes();
    
    /**
     * Create Solari cathedral-style station
     */
    std::shared_ptr<Mesh> createSolariStation();
    
    /**
     * Create Veyren industrial station
     */
    std::shared_ptr<Mesh> createVeyrenStation();
    
    /**
     * Create Aurelian organic station
     */
    std::shared_ptr<Mesh> createAurelianStation();
    
    /**
     * Create Keldari rusty station
     */
    std::shared_ptr<Mesh> createKeldariStation();
    
    /**
     * Create Astrahus medium citadel
     */
    std::shared_ptr<Mesh> createAstrahus();
    
    /**
     * Create Fortizar large citadel
     */
    std::shared_ptr<Mesh> createFortizar();
    
    /**
     * Create Keepstar XL citadel
     */
    std::shared_ptr<Mesh> createKeepstar();
    
    /**
     * Create Raitaru engineering complex
     */
    std::shared_ptr<Mesh> createRaitaru();
    
    /**
     * Initialize faction visual properties from JSON data
     */
    void initializeFactionVisuals();
    
    /**
     * Get faction visuals
     */
    const FactionVisuals& getFactionVisuals(FactionStyle faction) const;
};

} // namespace atlas
