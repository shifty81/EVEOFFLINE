#pragma once

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <glm/glm.hpp>
#include "rendering/mesh.h"

namespace eve {

class Mesh;

/**
 * Faction color scheme
 */
struct FactionColors {
    glm::vec4 primary;
    glm::vec4 secondary;
    glm::vec4 accent;
};

/**
 * Ship design characteristics for enhanced procedural generation
 * Based on EVE Online faction design languages
 */
struct ShipDesignTraits {
    // Faction design language
    enum class DesignStyle {
        CALDARI_BLOCKY,      // Angular, industrial, city-block architecture
        AMARR_ORNATE,        // Golden spires, cathedral-like, vertical emphasis
        GALLENTE_ORGANIC,    // Smooth curves, flowing forms, drone aesthetics
        MINMATAR_ASYMMETRIC  // Irregular, exposed framework, welded-together look
    };
    
    DesignStyle style;
    
    // Visual characteristics
    bool hasSpires;          // Vertical spires (Amarr)
    bool isAsymmetric;       // Asymmetric design (Minmatar)
    bool hasExposedFramework;// Visible structure (Minmatar)
    bool isBlocky;           // Angular design (Caldari)
    bool isOrganic;          // Smooth curves (Gallente)
    
    // Weapon hardpoint configuration
    int turretHardpoints;    // Number of visible turret mounts
    int missileHardpoints;   // Number of missile launcher bays
    int droneHardpoints;     // Number of drone bay indicators
    
    // Engine configuration
    int engineCount;         // Number of engine exhausts
    bool hasLargeEngines;    // Massive engine banks (battleship+)
    
    // Scale modifiers for detail
    float detailScale;       // Scale factor for hull detail
    float asymmetryFactor;   // 0=symmetric, 1=highly asymmetric
};

/**
 * 3D Model class for rendering entities in EVE OFFLINE
 * 
 * Supports both file-based model loading and procedural generation of ship models.
 * The procedural generation system creates faction-specific ships with distinctive
 * visual characteristics based on EVE Online's design language.
 * 
 * Features:
 * - Procedural generation for all ship classes (frigates to titans)
 * - Faction-specific color schemes and design patterns for 7 factions
 * - Model caching to prevent duplicate geometry generation
 * - Support for stations and asteroids
 * - Tech I and Tech II ship variants with visual differentiation
 */
class Model {
public:
    Model();
    ~Model();

    /**
     * Load model from file
     */
    bool loadFromFile(const std::string& path);

    /**
     * Create procedural ship model with basic geometry
     * 
     * Generates a ship model based on ship type classification (frigate, cruiser, etc.)
     * and applies faction-specific colors. Ships are generated with appropriate scale
     * and complexity for their class.
     * 
     * @param shipType The type/class of ship (e.g., "Rifter", "Moa", "Raven")
     * @param faction The faction (e.g., "Caldari", "Gallente", "Minmatar", "Amarr")
     * @return Unique pointer to generated Model, or generic model if type unknown
     */
    static std::unique_ptr<Model> createShipModel(const std::string& shipType, const std::string& faction);
    
    /**
     * Create procedural ship model with faction-specific design patterns
     * 
     * Enhanced version of createShipModel that adds faction-specific design elements:
     * - Caldari: Angular, blocky designs with blue/grey colors
     * - Gallente: Smooth, curved hulls with green/gold accents
     * - Minmatar: Asymmetric, industrial look with rust/brown tones
     * - Amarr: Golden, ornate designs with religious aesthetics
     * - Jove: Organic, alien curves (rare faction)
     * - ORE: Utility-focused mining ship designs
     * - Pirate: Aggressive red/black color schemes
     * 
     * @param shipType The type/class of ship
     * @param faction The faction determining visual style
     * @return Unique pointer to generated Model with racial characteristics
     */
    static std::unique_ptr<Model> createShipModelWithRacialDesign(const std::string& shipType, const std::string& faction);

    /**
     * Draw the model
     */
    void draw() const;

    /**
     * Add a mesh to the model
     */
    void addMesh(std::unique_ptr<Mesh> mesh);

private:
    std::vector<std::unique_ptr<Mesh>> m_meshes;

    /**
     * Model loading helper methods
     * Internal methods for loading different file formats
     */
    bool loadOBJ(const std::string& path);
    bool loadGLTF(const std::string& path);

    /**
     * Find OBJ model file for a given ship type and faction
     * Searches the models/ships directory for matching OBJ files.
     * File naming convention: {faction}_{class}_{ShipName}.obj
     * @param shipType The ship type/name (e.g., "Rifter", "Raven")
     * @param faction The faction (e.g., "Minmatar", "Caldari")
     * @return Path to OBJ file if found, empty string otherwise
     */
    static std::string findOBJModelPath(const std::string& shipType, const std::string& faction);

    /**
     * Ship type classification helpers
     * These methods determine which procedural generation function to use
     * based on ship type string matching.
     */
    static bool isFrigate(const std::string& shipType);
    static bool isDestroyer(const std::string& shipType);
    static bool isCruiser(const std::string& shipType);
    static bool isTech2Cruiser(const std::string& shipType);
    static bool isBattlecruiser(const std::string& shipType);
    static bool isCommandShip(const std::string& shipType);
    static bool isBattleship(const std::string& shipType);
    static bool isMiningBarge(const std::string& shipType);
    static bool isCarrier(const std::string& shipType);
    static bool isDreadnought(const std::string& shipType);
    static bool isTitan(const std::string& shipType);
    static bool isStation(const std::string& shipType);
    static bool isAsteroid(const std::string& shipType);

    /**
     * Get faction-specific color scheme
     * @param faction Faction name
     * @return FactionColors struct with primary, secondary, and accent colors
     */
    static FactionColors getFactionColors(const std::string& faction);
    
    static std::unique_ptr<Model> createFrigateModel(const FactionColors& colors);
    static std::unique_ptr<Model> createDestroyerModel(const FactionColors& colors);
    static std::unique_ptr<Model> createCruiserModel(const FactionColors& colors);
    static std::unique_ptr<Model> createTech2CruiserModel(const FactionColors& colors);
    static std::unique_ptr<Model> createBattlecruiserModel(const FactionColors& colors);
    static std::unique_ptr<Model> createBattleshipModel(const FactionColors& colors);
    static std::unique_ptr<Model> createMiningBargeModel(const FactionColors& colors);
    static std::unique_ptr<Model> createCarrierModel(const FactionColors& colors);
    static std::unique_ptr<Model> createDreadnoughtModel(const FactionColors& colors);
    static std::unique_ptr<Model> createTitanModel(const FactionColors& colors);
    static std::unique_ptr<Model> createStationModel(const FactionColors& colors, const std::string& stationType);
    static std::unique_ptr<Model> createAsteroidModel(const std::string& oreType);
    static std::unique_ptr<Model> createGenericModel(const FactionColors& colors);

    /**
     * Helper functions for enhanced procedural ship detail generation
     */
    static ShipDesignTraits getDesignTraits(const std::string& faction, const std::string& shipClass);
    static void addWeaponHardpoints(std::vector<Vertex>& vertices, std::vector<unsigned int>& indices, 
                                    float posZ, float offsetX, float offsetY, int count, const glm::vec3& color);
    static void addEngineDetail(std::vector<Vertex>& vertices, std::vector<unsigned int>& indices,
                                float posZ, float width, float height, int count, const glm::vec3& color);
    static void addHullPanelLines(std::vector<Vertex>& vertices, std::vector<unsigned int>& indices,
                                  float startZ, float endZ, float width, const glm::vec3& color);
    static void addSpireDetail(std::vector<Vertex>& vertices, std::vector<unsigned int>& indices,
                               float posZ, float height, const glm::vec3& color);
    static void addAsymmetricDetail(std::vector<Vertex>& vertices, std::vector<unsigned int>& indices,
                                    float posZ, float offset, const glm::vec3& color);

    /**
     * Model cache for sharing geometry between instances
     * Key: "shipType_faction" string
     * Value: Shared pointer to Model for reuse
     */
    static std::map<std::string, std::shared_ptr<Model>> s_modelCache;
};

} // namespace eve
