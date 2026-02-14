#include "systems/anomaly_spawner_system.h"
#include "ecs/world.h"
#include "components/game_components.h"
#include <algorithm>
#include <cmath>
#include <sstream>
#include <random>

namespace atlas {
namespace systems {

AnomalySpawnerSystem::AnomalySpawnerSystem(ecs::World* world)
    : System(world) {
}

void AnomalySpawnerSystem::update(float /* delta_time */) {
    // Respawn logic is handled externally or via spawnInitialAnomalies
}

int AnomalySpawnerSystem::spawnInitialAnomalies(const std::string& system_entity_id) {
    auto* sys_entity = world_->getEntity(system_entity_id);
    if (!sys_entity) return 0;

    auto* sigs = sys_entity->getComponent<components::SolarSystemSignatures>();
    if (!sigs) return 0;

    int target = targetSignatureCount(sigs->security_level, sigs->max_signatures);
    int current = static_cast<int>(sigs->signature_ids.size());
    int to_spawn = std::max(0, target - current);

    // Deterministic RNG from system seed
    std::mt19937 rng(sigs->system_seed);

    // Signature type distribution weighted by security level
    // High-sec: more combat, fewer wormholes
    // Null-sec: all types equally
    static const std::vector<std::string> types = {
        "combat", "relic", "data", "gas", "wormhole"
    };

    int spawned = 0;
    for (int i = 0; i < to_spawn; ++i) {
        // Pick type
        int type_idx = static_cast<int>(rng() % types.size());
        const std::string& sig_type = types[type_idx];

        // Difficulty scaled by security: low-sec/null = harder
        float scale = difficultyScale(sigs->security_level);
        int base_diff = 1 + static_cast<int>(rng() % 3); // 1-3
        int difficulty = std::min(5, static_cast<int>(
            static_cast<float>(base_diff) * (1.0f + scale)));

        // Name from type
        std::string name;
        if (sig_type == "combat")   name = "Unknown Combat Site";
        else if (sig_type == "relic")  name = "Unknown Relic Site";
        else if (sig_type == "data")   name = "Unknown Data Site";
        else if (sig_type == "gas")    name = "Unknown Gas Site";
        else                           name = "Unknown Wormhole";

        // Random position in system (±1000 km from origin)
        float px = (static_cast<float>(rng() % 2000) - 1000.0f) * 1000.0f;
        float py = (static_cast<float>(rng() % 200)  - 100.0f)  * 1000.0f;
        float pz = (static_cast<float>(rng() % 2000) - 1000.0f) * 1000.0f;

        std::string id = spawnAnomaly(system_entity_id, sig_type, name,
                                       difficulty, px, py, pz);
        if (!id.empty()) spawned++;
    }
    return spawned;
}

std::string AnomalySpawnerSystem::spawnAnomaly(
    const std::string& system_entity_id,
    const std::string& sig_type,
    const std::string& site_name,
    int difficulty,
    float x, float y, float z) {

    auto* sys_entity = world_->getEntity(system_entity_id);
    if (!sys_entity) return "";

    auto* sigs = sys_entity->getComponent<components::SolarSystemSignatures>();
    if (!sigs) return "";

    // Create anomaly entity
    ++sig_counter_;
    std::ostringstream oss;
    oss << "sig_" << sig_counter_;
    std::string entity_id = oss.str();

    auto* entity = world_->createEntity(entity_id);
    if (!entity) return "";

    auto pos = std::make_unique<components::Position>();
    pos->x = x;
    pos->y = y;
    pos->z = z;
    entity->addComponent(std::move(pos));

    auto sig = std::make_unique<components::AnomalySignature>();
    sig->signature_id = entity_id;
    sig->signature_type = sig_type;
    sig->site_name = site_name;
    sig->difficulty = std::max(1, std::min(5, difficulty));
    sig->signal_strength = 0.0f;
    sig->base_scan_difficulty = 1.0f + (sig->difficulty - 1) * 0.5f;
    sig->x = x;
    sig->y = y;
    sig->z = z;
    entity->addComponent(std::move(sig));

    sigs->signature_ids.push_back(entity_id);
    return entity_id;
}

bool AnomalySpawnerSystem::despawnAnomaly(const std::string& anomaly_entity_id) {
    auto* entity = world_->getEntity(anomaly_entity_id);
    if (!entity) return false;

    auto* sig = entity->getComponent<components::AnomalySignature>();
    if (!sig) return false;

    sig->despawned = true;
    return true;
}

int AnomalySpawnerSystem::getActiveAnomalyCount(
    const std::string& system_entity_id) const {
    auto* sys_entity = world_->getEntity(system_entity_id);
    if (!sys_entity) return 0;

    auto* sigs = sys_entity->getComponent<components::SolarSystemSignatures>();
    if (!sigs) return 0;

    int count = 0;
    for (const auto& id : sigs->signature_ids) {
        auto* e = world_->getEntity(id);
        if (!e) continue;
        auto* sig = e->getComponent<components::AnomalySignature>();
        if (sig && !sig->despawned) count++;
    }
    return count;
}

float AnomalySpawnerSystem::difficultyScale(float security_level) {
    // 1.0 sec → 0.0 scale, 0.0 sec → 1.0 scale (linear inverse)
    return std::max(0.0f, std::min(1.0f, 1.0f - security_level));
}

int AnomalySpawnerSystem::targetSignatureCount(float security_level,
                                                int max_signatures) {
    // High-sec (1.0): ~30% of max; null-sec (0.0): 100% of max
    float fraction = 0.3f + 0.7f * (1.0f - security_level);
    return std::max(1, static_cast<int>(
        static_cast<float>(max_signatures) * fraction));
}

} // namespace systems
} // namespace atlas
