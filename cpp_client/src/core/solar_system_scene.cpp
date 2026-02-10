#include "core/solar_system_scene.h"
#include "core/ship_physics.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <random>
#include <sstream>

namespace eve {

SolarSystemScene::SolarSystemScene()
    : m_securityLevel(1.0f)
{
    m_engineTrail.emitting = false;
    m_engineTrail.intensity = 0.0f;
    m_engineTrail.position = glm::vec3(0.0f);
    m_engineTrail.velocity = glm::vec3(0.0f);

    m_warpVisual.active = false;
    m_warpVisual.progress = 0.0f;
    m_warpVisual.phase = 0;
    m_warpVisual.direction = glm::vec3(0.0f, 0.0f, 1.0f);
    m_warpVisual.speedAU = 0.0f;
}

void SolarSystemScene::initialize(const std::string& systemId, const std::string& systemName,
                                   float securityLevel) {
    m_systemId = systemId;
    m_systemName = systemName;
    m_securityLevel = securityLevel;
    m_celestials.clear();
    std::cout << "[SolarSystem] Initialized: " << systemName
              << " (sec: " << securityLevel << ")" << std::endl;
}

void SolarSystemScene::loadTestSystem() {
    initialize("test_system", "Asakai", 0.4f);

    // Sun at the origin — every system has one
    Celestial sun;
    sun.id = "sun";
    sun.name = "Asakai - Star";
    sun.type = Celestial::Type::SUN;
    sun.position = glm::vec3(0.0f, 0.0f, 0.0f);
    sun.radius = 500000.0f;  // 500km radius (500,000 meters)
    sun.distanceFromSun_AU = 0.0f;
    sun.lightColor = glm::vec3(1.0f, 0.95f, 0.85f);  // Warm yellow-white
    sun.lightIntensity = 1.5f;
    addCelestial(sun);

    // Planet I — inner rocky planet
    Celestial planet1;
    planet1.id = "planet_1";
    planet1.name = "Asakai I";
    planet1.type = Celestial::Type::PLANET;
    planet1.position = glm::vec3(5.2f * AU_IN_METERS, 0.0f, 0.0f);
    planet1.radius = 6000.0f;
    planet1.distanceFromSun_AU = 5.2f;
    addCelestial(planet1);

    // Planet II — gas giant
    Celestial planet2;
    planet2.id = "planet_2";
    planet2.name = "Asakai II";
    planet2.type = Celestial::Type::PLANET;
    planet2.position = glm::vec3(0.0f, 0.0f, 12.8f * AU_IN_METERS);
    planet2.radius = 40000.0f;
    planet2.distanceFromSun_AU = 12.8f;
    addCelestial(planet2);

    // Planet III — outer ice world
    Celestial planet3;
    planet3.id = "planet_3";
    planet3.name = "Asakai III";
    planet3.type = Celestial::Type::PLANET;
    planet3.position = glm::vec3(-28.4f * AU_IN_METERS, 0.0f, 5.0f * AU_IN_METERS);
    planet3.radius = 8000.0f;
    planet3.distanceFromSun_AU = 28.4f;
    addCelestial(planet3);

    // Asteroid Belt I
    Celestial belt1;
    belt1.id = "belt_1";
    belt1.name = "Asakai - Asteroid Belt I";
    belt1.type = Celestial::Type::ASTEROID_BELT;
    belt1.position = glm::vec3(8.5f * AU_IN_METERS, 0.0f, 2.0f * AU_IN_METERS);
    belt1.radius = 50000.0f;
    belt1.distanceFromSun_AU = 8.5f;
    addCelestial(belt1);

    // Asteroid Belt II
    Celestial belt2;
    belt2.id = "belt_2";
    belt2.name = "Asakai - Asteroid Belt II";
    belt2.type = Celestial::Type::ASTEROID_BELT;
    belt2.position = glm::vec3(-3.0f * AU_IN_METERS, 0.0f, 18.3f * AU_IN_METERS);
    belt2.radius = 30000.0f;
    belt2.distanceFromSun_AU = 18.3f;
    addCelestial(belt2);

    // Station
    Celestial station;
    station.id = "station_1";
    station.name = "Asakai III - Blood Raider Assembly Plant";
    station.type = Celestial::Type::STATION;
    station.position = glm::vec3(-28.0f * AU_IN_METERS, 500.0f, 5.2f * AU_IN_METERS);
    station.radius = 5000.0f;
    station.distanceFromSun_AU = 28.0f;
    station.services = {"repair", "fitting", "market"};
    addCelestial(station);

    // Stargate to neighboring system
    Celestial gate;
    gate.id = "gate_perimeter";
    gate.name = "Stargate (Perimeter)";
    gate.type = Celestial::Type::STARGATE;
    gate.position = glm::vec3(15.0f * AU_IN_METERS, -1000.0f, -32.1f * AU_IN_METERS);
    gate.radius = 2500.0f;
    gate.distanceFromSun_AU = 32.1f;
    gate.linkedSystem = "perimeter";
    addCelestial(gate);

    std::cout << "[SolarSystem] Test system loaded with " << m_celestials.size()
              << " celestials" << std::endl;
}

void SolarSystemScene::addCelestial(const Celestial& celestial) {
    m_celestials.push_back(celestial);
}

const Celestial* SolarSystemScene::findCelestial(const std::string& id) const {
    for (const auto& c : m_celestials) {
        if (c.id == id) return &c;
    }
    return nullptr;
}

const Celestial* SolarSystemScene::getSun() const {
    for (const auto& c : m_celestials) {
        if (c.type == Celestial::Type::SUN) return &c;
    }
    return nullptr;
}

glm::vec3 SolarSystemScene::getSunLightDirection(const glm::vec3& objectPosition) const {
    const Celestial* sun = getSun();
    if (!sun) {
        // Default directional light if no sun
        return glm::normalize(glm::vec3(-0.5f, -1.0f, -0.3f));
    }
    glm::vec3 toSun = sun->position - objectPosition;
    float dist = glm::length(toSun);
    if (dist < 0.001f) {
        return glm::normalize(glm::vec3(0.0f, -1.0f, 0.0f));
    }
    return glm::normalize(toSun);
}

glm::vec3 SolarSystemScene::getSunLightColor() const {
    const Celestial* sun = getSun();
    if (!sun) return glm::vec3(1.0f, 0.95f, 0.9f);
    return sun->lightColor;
}

float SolarSystemScene::getSunLightIntensity() const {
    const Celestial* sun = getSun();
    if (!sun) return 1.0f;
    return sun->lightIntensity;
}

std::vector<const Celestial*> SolarSystemScene::getWarpDestinations(
    const glm::vec3& shipPosition, float minWarpDistance) const {

    std::vector<const Celestial*> destinations;
    for (const auto& c : m_celestials) {
        float dist = glm::length(c.position - shipPosition);
        if (dist >= minWarpDistance) {
            destinations.push_back(&c);
        }
    }

    // Sort by distance
    std::sort(destinations.begin(), destinations.end(),
        [&shipPosition](const Celestial* a, const Celestial* b) {
            float da = glm::length(a->position - shipPosition);
            float db = glm::length(b->position - shipPosition);
            return da < db;
        });

    return destinations;
}

void SolarSystemScene::update(float deltaTime, ShipPhysics* shipPhysics) {
    if (!shipPhysics) {
        m_engineTrail.emitting = false;
        m_warpVisual.active = false;
        return;
    }

    // Resolve celestial collisions during sub-warp flight
    if (!shipPhysics->isWarping()) {
        auto zones = getCollisionZones();
        shipPhysics->resolveCollision(zones);
    }

    // Update engine trail state based on ship throttle
    float throttle = shipPhysics->getEngineThrottle();
    m_engineTrail.emitting = (throttle > 0.01f);
    m_engineTrail.intensity = throttle;
    m_engineTrail.position = shipPhysics->getPosition();
    m_engineTrail.velocity = shipPhysics->getVelocity();

    // Update warp visual state
    bool warping = shipPhysics->isWarping();
    m_warpVisual.active = warping;
    if (warping) {
        m_warpVisual.progress = shipPhysics->getWarpProgress();
        m_warpVisual.speedAU = shipPhysics->getWarpSpeedAU();
        m_warpVisual.direction = shipPhysics->getHeading();

        auto phase = shipPhysics->getWarpPhase();
        switch (phase) {
            case ShipPhysics::WarpPhase::ALIGNING:     m_warpVisual.phase = 1; break;
            case ShipPhysics::WarpPhase::ACCELERATING:  m_warpVisual.phase = 2; break;
            case ShipPhysics::WarpPhase::CRUISING:      m_warpVisual.phase = 3; break;
            case ShipPhysics::WarpPhase::DECELERATING:  m_warpVisual.phase = 4; break;
            default:                                    m_warpVisual.phase = 0; break;
        }
    } else {
        m_warpVisual.phase = 0;
        m_warpVisual.speedAU = 0.0f;
    }
}

const Celestial* SolarSystemScene::getNearestCelestial(const glm::vec3& position) const {
    const Celestial* nearest = nullptr;
    float minDist = std::numeric_limits<float>::max();

    for (const auto& c : m_celestials) {
        float dist = glm::length(c.position - position);
        if (dist < minDist) {
            minDist = dist;
            nearest = &c;
        }
    }
    return nearest;
}

bool SolarSystemScene::isInDockingRange(const glm::vec3& position,
                                         const std::string& stationId,
                                         float dockingRadius) const {
    const Celestial* station = findCelestial(stationId);
    if (!station || station->type != Celestial::Type::STATION) return false;

    float dist = glm::length(station->position - position);
    return dist <= dockingRadius;
}

std::vector<ShipPhysics::CelestialCollisionZone> SolarSystemScene::getCollisionZones() const {
    std::vector<ShipPhysics::CelestialCollisionZone> zones;
    for (const auto& c : m_celestials) {
        // All celestials with significant radius get collision zones
        if (c.radius > 0.0f) {
            ShipPhysics::CelestialCollisionZone zone;
            zone.position = c.position;
            zone.radius = c.radius;
            zone.collisionRadius = c.radius * COLLISION_MULTIPLIER;
            zones.push_back(zone);
        }
    }
    return zones;
}

bool SolarSystemScene::isInsideCelestialCollisionZone(const glm::vec3& position) const {
    for (const auto& c : m_celestials) {
        if (c.radius > 0.0f) {
            float dist = glm::length(position - c.position);
            if (dist < c.radius * COLLISION_MULTIPLIER) {
                return true;
            }
        }
    }
    return false;
}

void SolarSystemScene::warpTo(const std::string& celestialId, ShipPhysics* shipPhysics,
                               float warpDistance) {
    if (!shipPhysics) return;

    const Celestial* target = findCelestial(celestialId);
    if (!target) {
        std::cerr << "[SolarSystem] Unknown celestial: " << celestialId << std::endl;
        return;
    }

    // Check if ship is inside a collision zone (bouncing — cannot warp)
    auto zones = getCollisionZones();
    if (shipPhysics->isInsideCollisionZone(zones)) {
        std::cerr << "[SolarSystem] Cannot warp: ship is inside a celestial collision zone (bumped)" << std::endl;
        return;
    }

    // Calculate warp destination (offset by warpDistance from the celestial)
    glm::vec3 destination = target->position;
    if (warpDistance > 0.0f) {
        glm::vec3 dir = glm::normalize(shipPhysics->getPosition() - target->position);
        destination = target->position + dir * warpDistance;
    } else {
        // Default: land at the edge of the collision zone rather than inside
        float collisionRadius = target->radius * COLLISION_MULTIPLIER;
        if (collisionRadius > 0.0f) {
            glm::vec3 dir = glm::normalize(shipPhysics->getPosition() - target->position);
            destination = target->position + dir * (collisionRadius + WARP_LANDING_MARGIN);
        }
    }

    // Check if warp path passes through any celestial collision zone
    if (shipPhysics->isWarpPathBlocked(shipPhysics->getPosition(), destination, zones)) {
        std::cerr << "[SolarSystem] Cannot warp: path blocked by celestial body" << std::endl;
        return;
    }

    std::cout << "[SolarSystem] Warping to " << target->name
              << " (" << target->distanceFromSun_AU << " AU from sun)" << std::endl;

    shipPhysics->warpTo(destination);

    if (m_onWarp) {
        m_onWarp(celestialId);
    }
}

void SolarSystemScene::generateSystem(uint32_t seed, const std::string& systemName) {
    std::mt19937 rng(seed);

    // Security level: seeded range 0.0 - 1.0
    std::uniform_real_distribution<float> secDist(0.0f, 1.0f);
    float security = secDist(rng);
    // Round to one decimal
    security = std::floor(security * 10.0f) / 10.0f;

    std::ostringstream idStream;
    idStream << "sys_" << seed;
    initialize(idStream.str(), systemName, security);

    // --- Star types ---
    struct StarTemplate {
        const char* suffix;
        glm::vec3 color;
        float intensity;
        float radius;
    };
    static const StarTemplate starTypes[] = {
        {"Yellow Star",  {1.0f, 0.95f, 0.85f}, 1.5f, 500000.0f},
        {"Blue Star",    {0.7f, 0.85f, 1.0f},  2.0f, 800000.0f},
        {"Red Giant",    {1.0f, 0.55f, 0.3f},  1.2f, 1200000.0f},
        {"White Dwarf",  {0.95f, 0.95f, 1.0f}, 1.8f, 200000.0f},
        {"Orange Star",  {1.0f, 0.78f, 0.45f}, 1.3f, 600000.0f},
    };
    int starIdx = static_cast<int>(rng() % 5);
    const auto& starTpl = starTypes[starIdx];

    Celestial sun;
    sun.id = "sun";
    sun.name = systemName + " - " + starTpl.suffix;
    sun.type = Celestial::Type::SUN;
    sun.position = glm::vec3(0.0f);
    sun.radius = starTpl.radius;
    sun.distanceFromSun_AU = 0.0f;
    sun.lightColor = starTpl.color;
    sun.lightIntensity = starTpl.intensity;
    addCelestial(sun);

    // --- Planets: 2-8 ---
    std::uniform_int_distribution<int> planetCountDist(2, 8);
    int numPlanets = planetCountDist(rng);

    std::uniform_real_distribution<float> angleDist(0.0f, 6.2831853f);
    std::uniform_real_distribution<float> radiusVarDist(0.8f, 1.2f);
    std::uniform_real_distribution<float> yOffsetDist(-0.05f, 0.05f);

    static const char* romanNumerals[] = {
        "I", "II", "III", "IV", "V", "VI", "VII", "VIII"
    };

    for (int i = 0; i < numPlanets; i++) {
        float baseAU = 3.0f + i * 4.5f + (rng() % 100) * 0.03f;
        float angle = angleDist(rng);
        float rv = radiusVarDist(rng);
        float distAU = baseAU * rv;

        float px = std::cos(angle) * distAU * AU_IN_METERS;
        float pz = std::sin(angle) * distAU * AU_IN_METERS;
        float py = yOffsetDist(rng) * distAU * AU_IN_METERS;

        // Planet radius: inner planets smaller, outer larger
        std::uniform_real_distribution<float> pRadDist(4000.0f + i * 3000.0f,
                                                        8000.0f + i * 8000.0f);
        float pRadius = pRadDist(rng);

        Celestial planet;
        std::ostringstream pid;
        pid << "planet_" << (i + 1);
        planet.id = pid.str();
        planet.name = systemName + " " + (i < 8 ? romanNumerals[i] : std::to_string(i + 1));
        planet.type = Celestial::Type::PLANET;
        planet.position = glm::vec3(px, py, pz);
        planet.radius = pRadius;
        planet.distanceFromSun_AU = distAU;
        addCelestial(planet);

        // --- Moons: 0-3 per planet (more likely for larger/outer planets) ---
        int maxMoons = std::min(3, i);  // inner planets get fewer moons
        std::uniform_int_distribution<int> moonCountDist(0, maxMoons);
        int numMoons = moonCountDist(rng);

        for (int m = 0; m < numMoons; m++) {
            float moonAngle = angleDist(rng);
            float moonDist = pRadius * (8.0f + m * 5.0f);  // In meters from planet

            Celestial moon;
            std::ostringstream mid;
            mid << "moon_" << (i + 1) << "_" << (m + 1);
            moon.id = mid.str();
            moon.name = systemName + " " + (i < 8 ? romanNumerals[i] : std::to_string(i + 1)) + " - Moon " + std::to_string(m + 1);
            moon.type = Celestial::Type::MOON;
            moon.position = glm::vec3(
                px + std::cos(moonAngle) * moonDist,
                py + 100.0f * (m + 1),
                pz + std::sin(moonAngle) * moonDist
            );
            moon.radius = pRadius * 0.15f + (rng() % 1000);
            moon.distanceFromSun_AU = distAU;
            addCelestial(moon);
        }
    }

    // --- Asteroid Belts: 1-4 (almost always at least 1) ---
    std::uniform_int_distribution<int> beltCountDist(1, 4);
    int numBelts = beltCountDist(rng);

    for (int b = 0; b < numBelts; b++) {
        float beltAU = 6.0f + b * 7.0f + (rng() % 100) * 0.05f;
        float beltAngle = angleDist(rng);

        Celestial belt;
        std::ostringstream bid;
        bid << "belt_" << (b + 1);
        belt.id = bid.str();
        belt.name = systemName + " - Asteroid Belt " +
            (b < 8 ? romanNumerals[b] : std::to_string(b + 1));
        belt.type = Celestial::Type::ASTEROID_BELT;
        belt.position = glm::vec3(
            std::cos(beltAngle) * beltAU * AU_IN_METERS,
            0.0f,
            std::sin(beltAngle) * beltAU * AU_IN_METERS
        );
        belt.radius = 30000.0f + (rng() % 40000);
        belt.distanceFromSun_AU = beltAU;
        addCelestial(belt);
    }

    // --- Stations: 1-3 (almost always at least 1) ---
    static const char* stationPrefixes[] = {
        "Caldari Navy Assembly Plant", "Gallente Federation Bureau",
        "Amarr Imperial Academy", "Minmatar Fleet Logistics",
        "Blood Raider Assembly Plant", "Serpentis Corporation Depot",
        "ORE Refinery", "Sisters of EVE Bureau",
    };
    std::uniform_int_distribution<int> stationCountDist(1, 3);
    int numStations = stationCountDist(rng);

    for (int s = 0; s < numStations; s++) {
        // Place station near a random planet
        int nearPlanet = static_cast<int>(rng() % numPlanets);
        const Celestial* parentPlanet = nullptr;
        int planetIdx = 0;
        for (const auto& c : m_celestials) {
            if (c.type == Celestial::Type::PLANET) {
                if (planetIdx == nearPlanet) {
                    parentPlanet = &c;
                    break;
                }
                planetIdx++;
            }
        }

        Celestial station;
        std::ostringstream sid;
        sid << "station_" << (s + 1);
        station.id = sid.str();

        int prefixIdx = static_cast<int>(rng() % 8);
        if (parentPlanet) {
            station.name = systemName + " " + (nearPlanet < 8 ? romanNumerals[nearPlanet] : std::to_string(nearPlanet + 1)) + " - " + stationPrefixes[prefixIdx];
            station.position = parentPlanet->position + glm::vec3(
                (rng() % 2000) - 1000.0f,
                500.0f + (rng() % 1000),
                (rng() % 2000) - 1000.0f
            );
            station.distanceFromSun_AU = parentPlanet->distanceFromSun_AU;
        } else {
            station.name = systemName + " - " + stationPrefixes[prefixIdx];
            float stAU = 10.0f + (rng() % 200) * 0.1f;
            float stAngle = angleDist(rng);
            station.position = glm::vec3(
                std::cos(stAngle) * stAU * AU_IN_METERS, 500.0f,
                std::sin(stAngle) * stAU * AU_IN_METERS
            );
            station.distanceFromSun_AU = stAU;
        }

        station.type = Celestial::Type::STATION;
        station.radius = 3000.0f + (rng() % 5000);
        station.services = {"repair", "fitting", "market"};
        addCelestial(station);
    }

    // --- Stargates: 1-2 ---
    static const char* gateDestinations[] = {
        "Perimeter", "Uedama", "Niarja", "Ahbazon",
        "Tama", "Amamake", "Rancer", "Old Man Star"
    };
    std::uniform_int_distribution<int> gateCountDist(1, 2);
    int numGates = gateCountDist(rng);

    for (int g = 0; g < numGates; g++) {
        float gateAU = 20.0f + g * 15.0f + (rng() % 100) * 0.1f;
        float gateAngle = angleDist(rng);
        int destIdx = static_cast<int>(rng() % 8);

        Celestial gate;
        std::ostringstream gid;
        gid << "gate_" << (g + 1);
        gate.id = gid.str();
        gate.name = "Stargate (" + std::string(gateDestinations[destIdx]) + ")";
        gate.type = Celestial::Type::STARGATE;
        gate.position = glm::vec3(
            std::cos(gateAngle) * gateAU * AU_IN_METERS,
            -1000.0f + (rng() % 2000),
            std::sin(gateAngle) * gateAU * AU_IN_METERS
        );
        gate.radius = 2500.0f;
        gate.distanceFromSun_AU = gateAU;
        gate.linkedSystem = gateDestinations[destIdx];
        addCelestial(gate);
    }

    std::cout << "[SolarSystem] Generated system '" << systemName
              << "' (seed=" << seed << ", sec=" << security
              << ") with " << m_celestials.size() << " celestials ("
              << numPlanets << " planets, " << numBelts << " belts, "
              << numStations << " stations, " << numGates << " gates)"
              << std::endl;
}

} // namespace eve
