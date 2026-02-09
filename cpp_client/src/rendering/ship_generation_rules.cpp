#include "rendering/ship_generation_rules.h"
#include <iostream>

namespace eve {

ShipGenerationRules::ShipGenerationRules() {
}

ShipGenerationRules::~ShipGenerationRules() {
}

void ShipGenerationRules::initialize() {
    std::cout << "Initializing Ship Generation Rules..." << std::endl;
    
    // Initialize faction-specific rules
    initializeMinmatarRules();
    initializeCaldariRules();
    initializeGallenteRules();
    initializeAmarrRules();
    
    // Initialize class-specific rules
    initializeFrigateRules();
    initializeDestroyerRules();
    initializeCruiserRules();
    initializeBattlecruiserRules();
    initializeBattleshipRules();
    initializeCapitalRules();
    
    std::cout << "Ship Generation Rules initialized" << std::endl;
}

const ShipGenerationRules::FactionRules& ShipGenerationRules::getFactionRules(const std::string& faction) const {
    auto it = m_factionRules.find(faction);
    if (it != m_factionRules.end()) {
        return it->second;
    }
    
    // Return default rules if faction not found
    static FactionRules defaultRules;
    return defaultRules;
}

const ShipGenerationRules::ClassRules& ShipGenerationRules::getClassRules(const std::string& shipClass) const {
    auto it = m_classRules.find(shipClass);
    if (it != m_classRules.end()) {
        return it->second;
    }
    
    // Return default rules if class not found
    static ClassRules defaultRules;
    return defaultRules;
}

std::vector<ShipGenerationRules::PlacementRule> ShipGenerationRules::getPlacementRules(
    const std::string& faction, const std::string& shipClass, const std::string& componentType) const {
    
    std::vector<PlacementRule> rules;
    
    if (componentType == "weapon") {
        PlacementRule weaponRule;
        weaponRule.componentType = "weapon";
        weaponRule.minPosition = glm::vec3(0.2f, -1.0f, -0.5f);  // Forward half of ship
        weaponRule.maxPosition = glm::vec3(1.0f, 1.0f, 0.5f);
        weaponRule.requiresLineOfSight = true;
        weaponRule.requiresRearPlacement = false;
        
        auto classRules = getClassRules(shipClass);
        weaponRule.minCount = classRules.minTurretHardpoints;
        weaponRule.maxCount = classRules.maxTurretHardpoints;
        
        rules.push_back(weaponRule);
    }
    else if (componentType == "engine") {
        PlacementRule engineRule;
        engineRule.componentType = "engine";
        engineRule.minPosition = glm::vec3(-1.0f, -1.0f, -0.5f);  // Rear of ship
        engineRule.maxPosition = glm::vec3(-0.2f, 1.0f, 0.5f);
        engineRule.requiresLineOfSight = false;
        engineRule.requiresRearPlacement = true;
        
        auto classRules = getClassRules(shipClass);
        engineRule.minCount = classRules.minEngines;
        engineRule.maxCount = classRules.maxEngines;
        
        rules.push_back(engineRule);
    }
    
    return rules;
}

bool ShipGenerationRules::validate(const std::string& faction, const std::string& shipClass,
                                    const std::map<std::string, int>& partCounts) const {
    auto factionRules = getFactionRules(faction);
    auto classRules = getClassRules(shipClass);
    
    // Check mandatory parts
    for (const auto& mandatoryPart : factionRules.mandatoryPartTypes) {
        auto it = partCounts.find(mandatoryPart);
        if (it == partCounts.end() || it->second == 0) {
            std::cerr << "Validation failed: Missing mandatory part " << mandatoryPart << std::endl;
            return false;
        }
    }
    
    // Check part count constraints
    for (const auto& pair : partCounts) {
        const std::string& partType = pair.first;
        int count = pair.second;
        
        auto minIt = factionRules.minPartCounts.find(partType);
        if (minIt != factionRules.minPartCounts.end() && count < minIt->second) {
            std::cerr << "Validation failed: Too few " << partType << " (min: " << minIt->second << ")" << std::endl;
            return false;
        }
        
        auto maxIt = factionRules.maxPartCounts.find(partType);
        if (maxIt != factionRules.maxPartCounts.end() && count > maxIt->second) {
            std::cerr << "Validation failed: Too many " << partType << " (max: " << maxIt->second << ")" << std::endl;
            return false;
        }
    }
    
    return true;
}

std::map<std::string, int> ShipGenerationRules::getRecommendedPartCounts(
    const std::string& faction, const std::string& shipClass) const {
    
    std::map<std::string, int> counts;
    auto classRules = getClassRules(shipClass);
    
    // Recommended weapon hardpoints (average of min and max)
    counts["turret"] = (classRules.minTurretHardpoints + classRules.maxTurretHardpoints) / 2;
    counts["launcher"] = (classRules.minLauncherHardpoints + classRules.maxLauncherHardpoints) / 2;
    counts["drone_bay"] = (classRules.minDroneBays + classRules.maxDroneBays) / 2;
    counts["engine"] = (classRules.minEngines + classRules.maxEngines) / 2;
    
    // Hull parts (always 1 of each)
    counts["hull_forward"] = 1;
    counts["hull_main"] = 1;
    counts["hull_rear"] = 1;
    
    // Detail parts based on detail density
    counts["panel_detail"] = static_cast<int>(5.0f * classRules.detailDensity);
    
    return counts;
}

bool ShipGenerationRules::isWeaponPlacementValid(const glm::vec3& position, const glm::vec3& shipSize) const {
    // Weapons should be in the forward 80% of the ship
    if (position.x < 0.2f * shipSize.x) {
        return false;
    }
    
    // Weapons should not be at extreme edges
    if (std::abs(position.y) > shipSize.y * 0.9f) {
        return false;
    }
    
    return true;
}

bool ShipGenerationRules::isEnginePlacementValid(const glm::vec3& position, const glm::vec3& shipSize) const {
    // Engines must be in the rear 30% of the ship
    if (position.x > -0.2f * shipSize.x) {
        return false;
    }
    
    return true;
}

// ==================== Faction-Specific Rules ====================

void ShipGenerationRules::initializeMinmatarRules() {
    FactionRules rules;
    rules.style = FactionStyle::MINMATAR;
    rules.requiresSymmetry = false;
    rules.allowsAsymmetry = true;
    rules.minAsymmetryFactor = 0.2f;
    rules.maxAsymmetryFactor = 0.5f;
    rules.requiresVerticalElements = true;  // High vertical structures
    rules.requiresOrganicCurves = false;
    rules.requiresAngularGeometry = false;
    rules.allowsExposedFramework = true;
    rules.requiresOrnateDetails = false;
    
    rules.mandatoryPartTypes = {"hull_main", "engine"};
    rules.minPartCounts["engine"] = 2;
    rules.maxPartCounts["panel_detail"] = 20;
    
    m_factionRules["Minmatar"] = rules;
}

void ShipGenerationRules::initializeCaldariRules() {
    FactionRules rules;
    rules.style = FactionStyle::CALDARI;
    rules.requiresSymmetry = true;
    rules.allowsAsymmetry = false;
    rules.minAsymmetryFactor = 0.0f;
    rules.maxAsymmetryFactor = 0.0f;
    rules.requiresVerticalElements = false;
    rules.requiresOrganicCurves = false;
    rules.requiresAngularGeometry = true;  // Blocky, angular
    rules.allowsExposedFramework = false;
    rules.requiresOrnateDetails = false;
    
    rules.mandatoryPartTypes = {"hull_main", "engine"};
    rules.minPartCounts["engine"] = 2;
    rules.maxPartCounts["panel_detail"] = 15;
    
    m_factionRules["Caldari"] = rules;
}

void ShipGenerationRules::initializeGallenteRules() {
    FactionRules rules;
    rules.style = FactionStyle::GALLENTE;
    rules.requiresSymmetry = true;
    rules.allowsAsymmetry = false;
    rules.minAsymmetryFactor = 0.0f;
    rules.maxAsymmetryFactor = 0.0f;
    rules.requiresVerticalElements = false;
    rules.requiresOrganicCurves = true;  // Smooth, flowing forms
    rules.requiresAngularGeometry = false;
    rules.allowsExposedFramework = false;
    rules.requiresOrnateDetails = false;
    
    rules.mandatoryPartTypes = {"hull_main", "engine", "drone_bay"};
    rules.minPartCounts["engine"] = 2;
    rules.minPartCounts["drone_bay"] = 1;
    rules.maxPartCounts["panel_detail"] = 12;
    
    m_factionRules["Gallente"] = rules;
}

void ShipGenerationRules::initializeAmarrRules() {
    FactionRules rules;
    rules.style = FactionStyle::AMARR;
    rules.requiresSymmetry = true;
    rules.allowsAsymmetry = false;
    rules.minAsymmetryFactor = 0.0f;
    rules.maxAsymmetryFactor = 0.0f;
    rules.requiresVerticalElements = true;  // Spires and vertical emphasis
    rules.requiresOrganicCurves = false;
    rules.requiresAngularGeometry = false;
    rules.allowsExposedFramework = false;
    rules.requiresOrnateDetails = true;  // Cathedral-like details
    
    rules.mandatoryPartTypes = {"hull_main", "engine", "spire"};
    rules.minPartCounts["engine"] = 2;
    rules.minPartCounts["spire"] = 1;
    rules.maxPartCounts["panel_detail"] = 25;
    
    m_factionRules["Amarr"] = rules;
}

// ==================== Class-Specific Rules ====================

void ShipGenerationRules::initializeFrigateRules() {
    ClassRules rules;
    rules.shipClass = "Frigate";
    rules.minLength = 3.0f;
    rules.maxLength = 4.0f;
    rules.minWidth = 0.7f;
    rules.maxWidth = 1.2f;
    rules.minHeight = 0.5f;
    rules.maxHeight = 0.9f;
    
    rules.minTurretHardpoints = 2;
    rules.maxTurretHardpoints = 3;
    rules.minLauncherHardpoints = 0;
    rules.maxLauncherHardpoints = 2;
    rules.minDroneBays = 0;
    rules.maxDroneBays = 1;
    rules.minEngines = 2;
    rules.maxEngines = 3;
    
    rules.detailDensity = 1.0f;
    
    m_classRules["Frigate"] = rules;
}

void ShipGenerationRules::initializeDestroyerRules() {
    ClassRules rules;
    rules.shipClass = "Destroyer";
    rules.minLength = 4.5f;
    rules.maxLength = 5.5f;
    rules.minWidth = 0.6f;
    rules.maxWidth = 0.9f;
    rules.minHeight = 0.5f;
    rules.maxHeight = 0.8f;
    
    rules.minTurretHardpoints = 6;
    rules.maxTurretHardpoints = 8;
    rules.minLauncherHardpoints = 0;
    rules.maxLauncherHardpoints = 0;
    rules.minDroneBays = 0;
    rules.maxDroneBays = 0;
    rules.minEngines = 2;
    rules.maxEngines = 4;
    
    rules.detailDensity = 1.2f;
    
    m_classRules["Destroyer"] = rules;
}

void ShipGenerationRules::initializeCruiserRules() {
    ClassRules rules;
    rules.shipClass = "Cruiser";
    rules.minLength = 5.5f;
    rules.maxLength = 6.5f;
    rules.minWidth = 1.5f;
    rules.maxWidth = 2.2f;
    rules.minHeight = 1.0f;
    rules.maxHeight = 1.5f;
    
    rules.minTurretHardpoints = 4;
    rules.maxTurretHardpoints = 5;
    rules.minLauncherHardpoints = 2;
    rules.maxLauncherHardpoints = 4;
    rules.minDroneBays = 1;
    rules.maxDroneBays = 2;
    rules.minEngines = 3;
    rules.maxEngines = 4;
    
    rules.detailDensity = 1.5f;
    
    m_classRules["Cruiser"] = rules;
}

void ShipGenerationRules::initializeBattlecruiserRules() {
    ClassRules rules;
    rules.shipClass = "Battlecruiser";
    rules.minLength = 8.0f;
    rules.maxLength = 9.0f;
    rules.minWidth = 2.2f;
    rules.maxWidth = 2.8f;
    rules.minHeight = 1.5f;
    rules.maxHeight = 2.0f;
    
    rules.minTurretHardpoints = 5;
    rules.maxTurretHardpoints = 7;
    rules.minLauncherHardpoints = 2;
    rules.maxLauncherHardpoints = 4;
    rules.minDroneBays = 1;
    rules.maxDroneBays = 2;
    rules.minEngines = 4;
    rules.maxEngines = 6;
    
    rules.detailDensity = 2.0f;
    
    m_classRules["Battlecruiser"] = rules;
}

void ShipGenerationRules::initializeBattleshipRules() {
    ClassRules rules;
    rules.shipClass = "Battleship";
    rules.minLength = 11.0f;
    rules.maxLength = 13.0f;
    rules.minWidth = 3.0f;
    rules.maxWidth = 4.0f;
    rules.minHeight = 2.0f;
    rules.maxHeight = 3.0f;
    
    rules.minTurretHardpoints = 6;
    rules.maxTurretHardpoints = 8;
    rules.minLauncherHardpoints = 4;
    rules.maxLauncherHardpoints = 6;
    rules.minDroneBays = 2;
    rules.maxDroneBays = 3;
    rules.minEngines = 6;
    rules.maxEngines = 8;
    
    rules.detailDensity = 2.5f;
    
    m_classRules["Battleship"] = rules;
}

void ShipGenerationRules::initializeCapitalRules() {
    ClassRules rules;
    
    // Carrier
    rules.shipClass = "Carrier";
    rules.minLength = 14.0f;
    rules.maxLength = 16.0f;
    rules.minWidth = 5.5f;
    rules.maxWidth = 6.5f;
    rules.minHeight = 2.5f;
    rules.maxHeight = 3.5f;
    rules.minTurretHardpoints = 2;
    rules.maxTurretHardpoints = 4;
    rules.minDroneBays = 5;
    rules.maxDroneBays = 10;
    rules.minEngines = 4;
    rules.maxEngines = 6;
    rules.detailDensity = 3.0f;
    m_classRules["Carrier"] = rules;
    
    // Dreadnought
    rules.shipClass = "Dreadnought";
    rules.minLength = 11.0f;
    rules.maxLength = 13.0f;
    rules.minWidth = 4.0f;
    rules.maxWidth = 5.0f;
    rules.minHeight = 3.0f;
    rules.maxHeight = 4.0f;
    rules.minTurretHardpoints = 4;
    rules.maxTurretHardpoints = 6;
    rules.minEngines = 4;
    rules.maxEngines = 6;
    rules.detailDensity = 3.0f;
    m_classRules["Dreadnought"] = rules;
    
    // Titan
    rules.shipClass = "Titan";
    rules.minLength = 23.0f;
    rules.maxLength = 27.0f;
    rules.minWidth = 7.0f;
    rules.maxWidth = 9.0f;
    rules.minHeight = 5.0f;
    rules.maxHeight = 7.0f;
    rules.minTurretHardpoints = 6;
    rules.maxTurretHardpoints = 10;
    rules.minEngines = 8;
    rules.maxEngines = 12;
    rules.detailDensity = 4.0f;
    m_classRules["Titan"] = rules;
}

} // namespace eve
