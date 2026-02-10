#include "utils/name_generator.h"
#include <sstream>
#include <chrono>

namespace eve {
namespace utils {

NameGenerator::NameGenerator() {
    // Seed RNG with current time
    auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    rng_.seed(static_cast<unsigned int>(seed));
    
    initializeNameData();
}

void NameGenerator::initializeNameData() {
    // Character names
    first_names_male_ = {
        "Marcus", "Drake", "Orion", "Zephyr", "Atlas", "Phoenix", "Kaiden", "Ryker",
        "Sirius", "Talon", "Vex", "Nero", "Axel", "Cyrus", "Magnus", "Raven",
        "Kane", "Jax", "Kael", "Zane", "Darius", "Thorne", "Blaze", "Storm"
    };
    
    first_names_female_ = {
        "Nova", "Aria", "Luna", "Lyra", "Aurora", "Vega", "Stellar", "Cassandra",
        "Seraph", "Echo", "Nyx", "Astrid", "Celeste", "Electra", "Iris", "Stella",
        "Zara", "Kira", "Sable", "Rogue", "Tempest", "Vesper", "Ember", "Skye"
    };
    
    last_names_ = {
        "Stormbreaker", "Darkstar", "Voidwalker", "Starfire", "Ironheart", "Swiftblade",
        "Shadowborn", "Lightbringer", "Frostwind", "Thunderstrike", "Ashborn", "Steelwind",
        "Moonshadow", "Sunforge", "Nightfall", "Dawnbringer", "Voidseeker", "Starborn",
        "Skyhammer", "Stormchaser", "Blacksun", "Silvermoon", "Redshard", "Blueflame"
    };
    
    // Ship names
    ship_prefixes_ = {"INS", "USS", "RSS", "CSS", "GSS", "ASS", "HSS", "ESS"};
    
    ship_names_heroic_ = {
        "Valor", "Defiance", "Vengeance", "Resolute", "Indomitable", "Relentless",
        "Invincible", "Unconquered", "Dauntless", "Intrepid", "Fearless", "Vigilant"
    };
    
    ship_names_celestial_ = {
        "Andromeda", "Orion", "Cassiopeia", "Polaris", "Sirius", "Vega",
        "Arcturus", "Betelgeuse", "Rigel", "Aldebaran", "Antares", "Altair"
    };
    
    ship_names_mythic_ = {
        "Hyperion", "Kronos", "Atlas", "Prometheus", "Heracles", "Perseus",
        "Achilles", "Odysseus", "Aegis", "Nemesis", "Phoenix", "Titan"
    };
    
    ship_names_descriptive_ = {
        "Thunderbolt", "Stormfront", "Wildfire", "Avalanche", "Hurricane", "Typhoon",
        "Cyclone", "Tempest", "Maelstrom", "Cataclysm", "Eclipse", "Nebula"
    };
    
    // Corporation names
    corp_prefixes_ = {
        "Stellar", "Galactic", "Cosmic", "Void", "Star", "Nova", "Quantum",
        "Nebula", "Celestial", "Interstellar", "Deep Space", "Dark Matter"
    };
    
    corp_types_ = {
        "Industries", "Corporation", "Enterprises", "Consortium", "Alliance",
        "Holdings", "Syndicate", "Collective", "Federation", "Conglomerate",
        "Trading Company", "Logistics", "Security", "Technologies"
    };
    
    // System names
    system_prefixes_ = {
        "Alpha", "Beta", "Gamma", "Delta", "Epsilon", "Zeta", "Theta", "Omega",
        "Nova", "Stellar", "Void", "Dark", "Deep", "High", "Low", "Outer"
    };
    
    system_cores_ = {
        "Centauri", "Crucis", "Draconis", "Aquilae", "Cygni", "Aurigae",
        "Persei", "Orionis", "Tauri", "Virginis", "Leonis", "Scorpii"
    };
    
    system_suffixes_ = {
        "Prime", "Secundus", "Tertius", "Major", "Minor", "Proxima",
        "Alpha", "Beta", "One", "Two", "Three", "Central"
    };
    
    // Station names
    station_prefixes_ = {
        "Citadel", "Bastion", "Fortress", "Outpost", "Station", "Haven",
        "Sanctuary", "Terminal", "Hub", "Gateway", "Nexus", "Trade Post"
    };
    
    station_descriptors_ = {
        "Prime", "Central", "Trade", "Military", "Research", "Mining",
        "Industrial", "Commercial", "Federal", "Imperial", "Orbital", "Deep Space"
    };
    
    // Mission names
    mission_adjectives_ = {
        "Deadly", "Silent", "Swift", "Hidden", "Ancient", "Forgotten", "Lost",
        "Desperate", "Critical", "Urgent", "Secret", "Classified", "Dangerous"
    };
    
    mission_nouns_ = {
        "Convoy", "Shipment", "Threat", "Menace", "Artifact", "Data", "Cargo",
        "Signal", "Transmission", "Asset", "Target", "Objective", "Operation"
    };
    
    // Asteroid types
    asteroid_types_ = {
        "Dustite", "Ferrite", "Ignaite", "Crystite", "Shadite", "Corite",
        "Lumine", "Sangite", "Glacite", "Densite", "Voidite", "Pyranite",
        "Stellite", "Cosmite", "Nexorite", "Spodumain"
    };
}

std::string NameGenerator::generateCharacterName(bool male) {
    const auto& first_names = male ? first_names_male_ : first_names_female_;
    std::string first = randomChoice(first_names);
    std::string last = randomChoice(last_names_);
    return first + " " + last;
}

std::string NameGenerator::generateShipName(ShipStyle style) {
    std::string prefix = randomChoice(ship_prefixes_);
    std::string name;
    
    if (style == ShipStyle::Random) {
        // Pick random style
        int random_style = randomInt(0, 3);
        style = static_cast<ShipStyle>(random_style);
    }
    
    switch (style) {
        case ShipStyle::Heroic:
            name = randomChoice(ship_names_heroic_);
            break;
        case ShipStyle::Celestial:
            name = randomChoice(ship_names_celestial_);
            break;
        case ShipStyle::Mythic:
            name = randomChoice(ship_names_mythic_);
            break;
        case ShipStyle::Descriptive:
            name = randomChoice(ship_names_descriptive_);
            break;
        default:
            name = randomChoice(ship_names_heroic_);
    }
    
    return prefix + " " + name;
}

std::string NameGenerator::generateCorporationName() {
    std::string prefix = randomChoice(corp_prefixes_);
    std::string type = randomChoice(corp_types_);
    return prefix + " " + type;
}

std::string NameGenerator::generateSystemName() {
    std::string prefix = randomChoice(system_prefixes_);
    std::string core = randomChoice(system_cores_);
    
    if (randomBool()) {
        std::string suffix = randomChoice(system_suffixes_);
        return prefix + " " + core + " " + suffix;
    } else {
        return prefix + " " + core;
    }
}

std::string NameGenerator::generateStationName(const std::string& system_name) {
    std::string prefix = randomChoice(station_prefixes_);
    std::string descriptor = randomChoice(station_descriptors_);
    
    if (!system_name.empty() && randomBool(0.3f)) {
        return system_name + " " + prefix;
    } else {
        return descriptor + " " + prefix;
    }
}

std::string NameGenerator::generateMissionName() {
    std::string adjective = randomChoice(mission_adjectives_);
    std::string noun = randomChoice(mission_nouns_);
    return adjective + " " + noun;
}

std::string NameGenerator::generateExplorationSiteName() {
    std::vector<std::string> adjectives = {
        "Ancient", "Forgotten", "Hidden", "Lost", "Abandoned", "Ruined",
        "Derelict", "Mysterious", "Secret", "Dangerous", "Unstable"
    };
    
    std::vector<std::string> sites = {
        "Hideout", "Base", "Outpost", "Installation", "Complex", "Structure",
        "Facility", "Station", "Colony", "Settlement", "Ruins", "Wreckage"
    };
    
    std::string adjective = randomChoice(adjectives);
    std::string site = randomChoice(sites);
    return adjective + " " + site;
}

std::string NameGenerator::generatePirateName() {
    std::vector<std::string> prefixes = {
        "Captain", "Commander", "Warlord", "Raider", "Corsair"
    };
    
    std::vector<std::string> names = {
        "Blackheart", "Ironjaw", "Bloodfang", "Darkblade", "Redscar",
        "Voidreaver", "Starcrusher", "Skullbreaker", "Doomhammer", "Deathbringer"
    };
    
    std::string prefix = randomChoice(prefixes);
    std::string name = randomChoice(names);
    return prefix + " " + name;
}

std::string NameGenerator::generatePilotCallsign() {
    std::vector<std::string> callsigns = {
        "Viper", "Hawk", "Falcon", "Eagle", "Phoenix", "Dragon",
        "Ghost", "Shadow", "Reaper", "Rogue", "Blade", "Storm",
        "Thunder", "Lightning", "Frost", "Flame", "Steel", "Iron"
    };
    
    std::string callsign = randomChoice(callsigns);
    int number = randomInt(1, 99);
    std::ostringstream oss;
    oss << callsign << "-" << number;
    return oss.str();
}

std::string NameGenerator::generateAsteroidDesignation() {
    std::string ore_type = randomChoice(asteroid_types_);
    int number = randomInt(1000, 9999);
    std::ostringstream oss;
    oss << ore_type << "-" << number;
    return oss.str();
}

template<typename T>
const T& NameGenerator::randomChoice(const std::vector<T>& vec) {
    std::uniform_int_distribution<size_t> dist(0, vec.size() - 1);
    return vec[dist(rng_)];
}

int NameGenerator::randomInt(int min, int max) {
    std::uniform_int_distribution<int> dist(min, max);
    return dist(rng_);
}

bool NameGenerator::randomBool(float probability) {
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    return dist(rng_) < probability;
}

} // namespace utils
} // namespace eve
