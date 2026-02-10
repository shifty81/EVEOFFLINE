#include "core/solar_system_scene.h"
#include "core/ship_physics.h"
#include <algorithm>
#include <cmath>
#include <iostream>

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
    sun.radius = 500000.0f;  // 500km radius
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

void SolarSystemScene::warpTo(const std::string& celestialId, ShipPhysics* shipPhysics,
                               float warpDistance) {
    if (!shipPhysics) return;

    const Celestial* target = findCelestial(celestialId);
    if (!target) {
        std::cerr << "[SolarSystem] Unknown celestial: " << celestialId << std::endl;
        return;
    }

    // Calculate warp destination (offset by warpDistance from the celestial)
    glm::vec3 destination = target->position;
    if (warpDistance > 0.0f) {
        glm::vec3 dir = glm::normalize(shipPhysics->getPosition() - target->position);
        destination = target->position + dir * warpDistance;
    }

    std::cout << "[SolarSystem] Warping to " << target->name
              << " (" << target->distanceFromSun_AU << " AU from sun)" << std::endl;

    shipPhysics->warpTo(destination);

    if (m_onWarp) {
        m_onWarp(celestialId);
    }
}

} // namespace eve
