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
        bool requiresVerticalElements;  // Minmatar/Amarr verticality
        bool requiresOrganicCurves;     // Gallente smoothness
        bool requiresAngularGeometry;   // Caldari blockiness
        bool allowsExposedFramework;    // Minmatar industrial look
        bool requiresOrnateDetails;     // Amarr cathedral style
        
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
    
private:
    std::map<std::string, FactionRules> m_factionRules;
    std::map<std::string, ClassRules> m_classRules;
    std::vector<Rule> m_globalRules;
    
    // Initialize specific faction rules
    void initializeMinmatarRules();
    void initializeCaldariRules();
    void initializeGallenteRules();
    void initializeAmarrRules();
    
    // Initialize class rules
    void initializeFrigateRules();
    void initializeDestroyerRules();
    void initializeCruiserRules();
    void initializeBattlecruiserRules();
    void initializeBattleshipRules();
    void initializeCapitalRules();
};

} // namespace eve
