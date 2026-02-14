#ifndef EVE_SYSTEMS_ANOMALY_SPAWNER_SYSTEM_H
#define EVE_SYSTEMS_ANOMALY_SPAWNER_SYSTEM_H

#include "ecs/system.h"
#include <string>
#include <vector>
#include <map>

namespace atlas {
namespace systems {

/**
 * @brief Generates and manages cosmic anomaly signatures in solar systems
 *
 * Uses a deterministic seed per solar system to spawn anomaly signatures.
 * Anomaly count and difficulty scale with the system's security level:
 *   high-sec → fewer, easier sites
 *   null-sec → more, harder sites
 *
 * When a site is completed (despawned), the system will respawn a new one
 * after a configurable delay.
 */
class AnomalySpawnerSystem : public ecs::System {
public:
    explicit AnomalySpawnerSystem(ecs::World* world);
    ~AnomalySpawnerSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "AnomalySpawnerSystem"; }

    /**
     * @brief Seed a solar system with initial anomaly signatures
     * @param system_entity_id Entity with SolarSystemSignatures component
     * @return number of anomalies spawned
     */
    int spawnInitialAnomalies(const std::string& system_entity_id);

    /**
     * @brief Manually spawn a single anomaly in a system
     * @param system_entity_id Entity with SolarSystemSignatures component
     * @param sig_type  Signature type ("combat", "relic", "data", "gas", "wormhole")
     * @param site_name Human-readable site name
     * @param difficulty 1-5
     * @param x, y, z   Position in space
     * @return entity id of the new anomaly, or empty string on failure
     */
    std::string spawnAnomaly(const std::string& system_entity_id,
                             const std::string& sig_type,
                             const std::string& site_name,
                             int difficulty,
                             float x, float y, float z);

    /**
     * @brief Mark an anomaly as despawned (completed / expired)
     * @return true if anomaly was found and marked
     */
    bool despawnAnomaly(const std::string& anomaly_entity_id);

    /**
     * @brief Get the number of active (non-despawned) anomalies in a system
     */
    int getActiveAnomalyCount(const std::string& system_entity_id) const;

    /**
     * @brief Get difficulty scaling factor for a security level
     *
     * Returns a multiplier: 1.0 for null-sec, lower for high-sec.
     */
    static float difficultyScale(float security_level);

    /**
     * @brief Compute how many signatures a system should have based on security
     */
    static int targetSignatureCount(float security_level, int max_signatures);

private:
    int sig_counter_ = 0;
};

} // namespace systems
} // namespace atlas

#endif // EVE_SYSTEMS_ANOMALY_SPAWNER_SYSTEM_H
