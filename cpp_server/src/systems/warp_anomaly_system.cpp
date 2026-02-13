#include "systems/warp_anomaly_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <functional>
#include <array>

namespace atlas {
namespace systems {

// Predefined anomalies by category
struct AnomalyTemplate {
    std::string name;
    std::string description;
    std::string category;
    float duration;
};

static const std::array<AnomalyTemplate, 4> visual_anomalies = {{
    {"Chromatic Bloom", "Colors shift across the tunnel walls.", "visual", 4.0f},
    {"Light Cascade", "Bright flashes ripple through the warp field.", "visual", 3.0f},
    {"Nebula Ghost", "A faint nebula image appears in the tunnel.", "visual", 5.0f},
    {"Star Echo", "Distant stars seem to echo in the field.", "visual", 4.5f}
}};

static const std::array<AnomalyTemplate, 3> sensory_anomalies = {{
    {"Hull Resonance", "The ship vibrates at a low frequency.", "sensory", 6.0f},
    {"Static Whisper", "Faint static crackles across comms.", "sensory", 5.0f},
    {"Gravity Pulse", "A brief gravity fluctuation passes through.", "sensory", 4.0f}
}};

static const std::array<AnomalyTemplate, 2> shear_anomalies = {{
    {"Warp Shear", "The tunnel buckles momentarily.", "shear", 3.0f},
    {"Field Fracture", "A crack appears in the warp field.", "shear", 4.0f}
}};

static const std::array<AnomalyTemplate, 1> legendary_anomalies = {{
    {"The Convergence", "All light bends toward a single point ahead.", "legendary", 10.0f}
}};

WarpAnomalySystem::WarpAnomalySystem(ecs::World* world)
    : System(world) {
}

void WarpAnomalySystem::update(float /*delta_time*/) {
    auto entities = world_->getEntities<components::WarpState>();
    for (auto* entity : entities) {
        auto* warp = entity->getComponent<components::WarpState>();
        if (warp && warp->phase == components::WarpState::WarpPhase::Cruise) {
            tryTriggerAnomaly(entity->getId());
        }
    }
}

bool WarpAnomalySystem::tryTriggerAnomaly(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* warp = entity->getComponent<components::WarpState>();
    if (!warp) return false;

    if (warp->warp_time < 20.0f) return false;

    // Deterministic rolling based on entity_id hash + warp_time
    std::hash<std::string> hasher;
    size_t base_hash = hasher(entity_id);
    size_t time_factor = static_cast<size_t>(warp->warp_time * 100.0f);
    size_t roll = (base_hash ^ (time_factor * 2654435761u)) % 200;

    const AnomalyTemplate* selected = nullptr;

    if (roll == 0) {
        // legendary: 1/200
        size_t idx = (base_hash + time_factor) % legendary_anomalies.size();
        selected = &legendary_anomalies[idx];
    } else if (roll < 4) {
        // shear: ~1/50 (indices 1-3)
        size_t idx = (base_hash + time_factor) % shear_anomalies.size();
        selected = &shear_anomalies[idx];
    } else if (roll < 24) {
        // sensory: ~1/10 (indices 4-23)
        size_t idx = (base_hash + time_factor) % sensory_anomalies.size();
        selected = &sensory_anomalies[idx];
    } else if (roll < 91) {
        // visual: ~1/3 (indices 24-90)
        size_t idx = (base_hash + time_factor) % visual_anomalies.size();
        selected = &visual_anomalies[idx];
    } else {
        return false;
    }

    if (!selected) return false;

    WarpAnomaly anomaly;
    anomaly.anomaly_id = selected->category + "_" + std::to_string(time_factor);
    anomaly.name = selected->name;
    anomaly.description = selected->description;
    anomaly.category = selected->category;
    anomaly.duration = selected->duration;

    last_anomalies_[entity_id] = anomaly;
    anomaly_counts_[entity_id]++;

    return true;
}

WarpAnomaly WarpAnomalySystem::getLastAnomaly(const std::string& entity_id) const {
    auto it = last_anomalies_.find(entity_id);
    if (it != last_anomalies_.end()) {
        return it->second;
    }
    return WarpAnomaly{};
}

void WarpAnomalySystem::clearAnomaly(const std::string& entity_id) {
    last_anomalies_.erase(entity_id);
}

int WarpAnomalySystem::getAnomalyCount(const std::string& entity_id) const {
    auto it = anomaly_counts_.find(entity_id);
    if (it != anomaly_counts_.end()) {
        return it->second;
    }
    return 0;
}

} // namespace systems
} // namespace atlas
