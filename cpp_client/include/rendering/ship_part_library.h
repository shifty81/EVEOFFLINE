#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <glm/glm.hpp>
#include "rendering/mesh.h"
#include "rendering/procedural_mesh_ops.h"
#include "rendering/reference_model_analyzer.h"

namespace eve {

/**
 * Types of ship parts for modular assembly
 */
enum class ShipPartType {
    HULL_FORWARD,      // Forward hull section (nose, command bridge)
    HULL_MAIN,         // Main hull body
    HULL_REAR,         // Rear hull section
    WING_LEFT,         // Left wing/strut
    WING_RIGHT,        // Right wing/strut
    ENGINE_MAIN,       // Primary engine cluster
    ENGINE_AUXILIARY,  // Secondary engines
    WEAPON_TURRET,     // Turret hardpoint
    WEAPON_LAUNCHER,   // Missile/torpedo launcher
    WEAPON_DRONE_BAY,  // Drone bay
    PANEL_DETAIL,      // Hull panel greeble
    ANTENNA_ARRAY,     // Communication arrays
    SPIRE_ORNAMENT,    // Amarr-style spires
    FRAMEWORK_EXPOSED  // Minmatar-style exposed framework
};

/**
 * Represents a single modular ship part with geometry and metadata
 */
struct ShipPart {
    ShipPartType type;
    std::string name;
    std::string faction;           // Which faction this part belongs to
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    glm::vec3 attachmentPoint;     // Where this part connects to others
    glm::vec3 scale;               // Default scale
    bool isSymmetric;              // Whether to mirror this part
    float detailLevel;             // LOD hint (0.0 = low detail, 1.0 = high detail)
    
    ShipPart() 
        : type(ShipPartType::HULL_MAIN)
        , name("")
        , faction("")
        , attachmentPoint(0.0f)
        , scale(1.0f)
        , isSymmetric(true)
        , detailLevel(1.0f)
    {}
};

/**
 * Configuration for assembling a complete ship from parts
 */
struct ShipAssemblyConfig {
    std::string shipClass;         // "Frigate", "Cruiser", "Battleship", etc.
    std::string faction;           // "Minmatar", "Caldari", "Gallente", "Amarr"
    
    // Part selection
    std::string hullForwardId;
    std::string hullMainId;
    std::string hullRearId;
    std::vector<std::string> wingIds;
    std::vector<std::string> engineIds;
    std::vector<std::string> weaponIds;
    std::vector<std::string> detailIds;
    
    // Scale modifiers
    float overallScale;
    glm::vec3 proportions;         // Length, width, height multipliers
    
    // Assembly rules
    bool enforceSymmetry;          // Amarr/Caldari symmetry requirement
    bool allowAsymmetry;           // Minmatar asymmetry allowance
    float asymmetryFactor;         // 0.0 = perfect symmetry, 1.0 = maximum asymmetry
    
    ShipAssemblyConfig()
        : overallScale(1.0f)
        , proportions(1.0f)
        , enforceSymmetry(true)
        , allowAsymmetry(false)
        , asymmetryFactor(0.0f)
    {}
};

/**
 * Parameters controlling procedural variation when generating ship variants.
 * Values derived from measured ranges in the reference OBJ models
 * (see docs/research/OBJ_MODEL_ANALYSIS.md).
 */
struct ShipVariationParams {
    // Hull proportion jitter (0.0 = exact reference, 1.0 = full measured range)
    float proportionJitter;
    
    // Scale jitter factor applied to overall size (e.g., 0.1 = ±10%)
    float scaleJitter;
    
    // Surface greeble density multiplier (0.5 = half detail, 2.0 = double)
    float detailMultiplier;
    
    // Seed for deterministic variation (0 = random)
    unsigned int seed;
    
    ShipVariationParams()
        : proportionJitter(0.3f)
        , scaleJitter(0.1f)
        , detailMultiplier(1.0f)
        , seed(0)
    {}
};

/**
 * Library of modular ship parts organized by faction and type
 * Manages the creation and storage of reusable ship components
 */
class ShipPartLibrary {
public:
    ShipPartLibrary();
    ~ShipPartLibrary();
    
    /**
     * Initialize the library with predefined parts for all factions
     */
    void initialize();
    
    /**
     * Get a ship part by ID
     */
    const ShipPart* getPart(const std::string& partId) const;
    
    /**
     * Get all parts of a specific type for a faction
     */
    std::vector<const ShipPart*> getPartsByType(ShipPartType type, const std::string& faction) const;
    
    /**
     * Add a custom part to the library
     */
    void addPart(const std::string& id, const ShipPart& part);
    
    /**
     * Create a ship assembly configuration for a given ship class and faction
     */
    ShipAssemblyConfig createAssemblyConfig(const std::string& shipClass, const std::string& faction) const;
    
    /**
     * Create a varied assembly configuration using reference model traits.
     * Applies controlled randomness within the measured OBJ model ranges
     * so that each generated ship is unique but faction-appropriate.
     */
    ShipAssemblyConfig createVariedAssemblyConfig(const std::string& shipClass,
                                                   const std::string& faction,
                                                   const ShipVariationParams& variation) const;
    
    /**
     * Create hull parts from learned reference model parameters.
     * Uses the cross-section profiles and radius multipliers extracted by
     * the ReferenceModelAnalyzer to generate ship parts that mimic the
     * proportions and silhouettes of the analyzed models.
     *
     * @param analyzer  Analyzer that has already processed reference models
     * @param faction   Faction name (determines color scheme and polygon sides)
     * @param partIdPrefix  Prefix for part IDs (e.g. "learned_")
     */
    void createPartsFromLearnedModels(const ReferenceModelAnalyzer& analyzer,
                                       const std::string& faction,
                                       const std::string& partIdPrefix = "learned_");

private:
    // Storage for all parts, keyed by unique ID
    std::map<std::string, ShipPart> m_parts;
    
    // Helper methods to create faction-specific parts
    void createMinmatarParts(const glm::vec4& primary, const glm::vec4& secondary, const glm::vec4& accent);
    void createCaldariParts(const glm::vec4& primary, const glm::vec4& secondary, const glm::vec4& accent);
    void createGallenteParts(const glm::vec4& primary, const glm::vec4& secondary, const glm::vec4& accent);
    void createAmarrParts(const glm::vec4& primary, const glm::vec4& secondary, const glm::vec4& accent);
    
    // Helper to create basic geometric primitives
    ShipPart createBoxPart(const glm::vec3& size, const glm::vec4& color, ShipPartType type);
    ShipPart createCylinderPart(float radius, float length, int segments, const glm::vec4& color, ShipPartType type);
    ShipPart createConePart(float radius, float length, int segments, const glm::vec4& color, ShipPartType type);

    // Extrusion-based part creation (procedural_mesh_ops integration)
    ShipPart createExtrudedHullPart(int sides, int segments, float segmentLength,
                                    float baseRadius, const std::vector<float>& radiusMultipliers,
                                    float scaleX, float scaleZ,
                                    const glm::vec4& color, ShipPartType type);
    ShipPart createBeveledPanelPart(int sides, float radius, float borderSize,
                                    float depth, const glm::vec4& color, ShipPartType type);
    ShipPart createPyramidDetailPart(int sides, float radius, float height,
                                     const glm::vec4& color, ShipPartType type);
};

} // namespace eve
