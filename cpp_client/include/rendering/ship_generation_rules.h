#pragma once

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <glm/glm.hpp>

namespace eve {

/**
 * Faction design styles for ship generation rules
 */
enum class FactionStyle {
    MINMATAR,    // Asymmetric, rustic, exposed framework, vertical emphasis
    CALDARI,     // Blocky, angular, industrial, functional
    GALLENTE,    // Organic, smooth curves, drone-focused
    AMARR,       // Symmetric, ornate, golden spires, cathedral-like
    PIRATE,      // Aggressive, modified designs
    ORE          // Utility, mining-focused
};

/**
 * Reference model traits measured from the 311 OBJ ship models in data/ships/obj_models.
 * These values drive procedural generation to produce ships with realistic proportions
 * and faction-distinctive silhouettes, while allowing controlled variation.
 * See docs/research/OBJ_MODEL_ANALYSIS.md for the full analysis.
 */
struct ReferenceModelTraits {
    // Aspect ratios measured from reference OBJ models
    float avgAspectLW;      // Average length-to-width ratio
    float minAspectLW;      // Minimum L:W observed
    float maxAspectLW;      // Maximum L:W observed
    float avgAspectLH;      // Average length-to-height ratio

    // Complexity metrics from reference models
    int avgVertexCount;     // Average vertex count for this faction/class
    int avgFaceCount;       // Average face count

    // Detail density multiplier relative to frigate baseline (1.0)
    float detailDensityMultiplier;

    ReferenceModelTraits()
        : avgAspectLW(2.0f), minAspectLW(1.0f), maxAspectLW(5.0f)
        , avgAspectLH(3.5f)
        , avgVertexCount(8000), avgFaceCount(8000)
        , detailDensityMultiplier(1.0f)
    {}
};

/**
 * Ship generation rules that constrain and guide the procedural generation
 * Based on faction design language and ship class requirements
 */
class ShipGenerationRules {
public:
    struct Rule {
        std::string name;
        std::string description;
        std::function<bool(const std::string&, const std::string&)> validator;
        bool isMandatory;  // If true, generation fails if rule violated
        
        Rule() : isMandatory(false) {}
    };
    
    struct PlacementRule {
        std::string componentType;  // "weapon", "engine", "shield"
        glm::vec3 minPosition;      // Minimum allowed position (relative)
        glm::vec3 maxPosition;      // Maximum allowed position (relative)
        bool requiresLineOfSight;   // For weapons
        bool requiresRearPlacement; // For engines
        int minCount;
        int maxCount;
    };
    
    struct FactionRules {
        FactionStyle style;
        bool requiresSymmetry;
        bool allowsAsymmetry;
        float minAsymmetryFactor;
        float maxAsymmetryFactor;
        bool requiresVerticalElements;  // Keldari/Solari verticality
        bool requiresOrganicCurves;     // Aurelian smoothness
        bool requiresAngularGeometry;   // Veyren blockiness
        bool allowsExposedFramework;    // Keldari industrial look
        bool requiresOrnateDetails;     // Solari cathedral style

        // Reference traits from analyzed OBJ models
        ReferenceModelTraits referenceTraits;
        
        std::vector<std::string> mandatoryPartTypes;  // Parts that must be present
        std::map<std::string, int> minPartCounts;     // Minimum count for each part type
        std::map<std::string, int> maxPartCounts;     // Maximum count for each part type
        
        FactionRules()
            : style(FactionStyle::CALDARI)
            , requiresSymmetry(true)
            , allowsAsymmetry(false)
            , minAsymmetryFactor(0.0f)
            , maxAsymmetryFactor(0.0f)
            , requiresVerticalElements(false)
            , requiresOrganicCurves(false)
            , requiresAngularGeometry(false)
            , allowsExposedFramework(false)
            , requiresOrnateDetails(false)
        {}
    };
    
    struct ClassRules {
        std::string shipClass;     // "Frigate", "Cruiser", "Battleship", etc.
        float minLength;
        float maxLength;
        float minWidth;
        float maxWidth;
        float minHeight;
        float maxHeight;
        
        int minTurretHardpoints;
        int maxTurretHardpoints;
        int minLauncherHardpoints;
        int maxLauncherHardpoints;
        int minDroneBays;
        int maxDroneBays;
        int minEngines;
        int maxEngines;
        
        float detailDensity;  // How many greebles/details to add

        // Reference traits from analyzed OBJ models
        ReferenceModelTraits referenceTraits;
        
        ClassRules()
            : minLength(1.0f), maxLength(10.0f)
            , minWidth(0.5f), maxWidth(5.0f)
            , minHeight(0.5f), maxHeight(3.0f)
            , minTurretHardpoints(0), maxTurretHardpoints(8)
            , minLauncherHardpoints(0), maxLauncherHardpoints(6)
            , minDroneBays(0), maxDroneBays(5)
            , minEngines(1), maxEngines(8)
            , detailDensity(1.0f)
        {}
    };
    
public:
    ShipGenerationRules();
    ~ShipGenerationRules();
    
    /**
     * Initialize rules for all factions and ship classes
     */
    void initialize();
    
    /**
     * Get faction-specific rules
     */
    const FactionRules& getFactionRules(const std::string& faction) const;
    
    /**
     * Get class-specific rules
     */
    const ClassRules& getClassRules(const std::string& shipClass) const;
    
    /**
     * Get placement rules for a component type
     */
    std::vector<PlacementRule> getPlacementRules(const std::string& faction, 
                                                  const std::string& shipClass,
                                                  const std::string& componentType) const;
    
    /**
     * Validate if a ship configuration meets all mandatory rules
     */
    bool validate(const std::string& faction, const std::string& shipClass,
                  const std::map<std::string, int>& partCounts) const;
    
    /**
     * Get recommended part counts for a ship configuration
     */
    std::map<std::string, int> getRecommendedPartCounts(const std::string& faction,
                                                         const std::string& shipClass) const;
    
    /**
     * Check if a weapon placement is valid (line of sight, positioning)
     */
    bool isWeaponPlacementValid(const glm::vec3& position, const glm::vec3& shipSize) const;
    
    /**
     * Check if an engine placement is valid (rear positioning)
     */
    bool isEnginePlacementValid(const glm::vec3& position, const glm::vec3& shipSize) const;
    
    /**
     * Get reference model traits for a faction (measured from OBJ models)
     */
    ReferenceModelTraits getFactionReferenceTraits(const std::string& faction) const;
    
    /**
     * Get reference model traits for a ship class (measured from OBJ models)
     */
    ReferenceModelTraits getClassReferenceTraits(const std::string& shipClass) const;
    
private:
    std::map<std::string, FactionRules> m_factionRules;
    std::map<std::string, ClassRules> m_classRules;
    std::vector<Rule> m_globalRules;
    
    // Initialize specific faction rules
    void initializeKeldariRules();
    void initializeVeyrenRules();
    void initializeAurelianRules();
    void initializeSolariRules();
    
    // Initialize class rules
    void initializeFrigateRules();
    void initializeDestroyerRules();
    void initializeCruiserRules();
    void initializeBattlecruiserRules();
    void initializeBattleshipRules();
    void initializeCapitalRules();
};

} // namespace eve
