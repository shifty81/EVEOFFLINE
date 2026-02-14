#ifndef EVE_SYSTEMS_ANOMALY_SYSTEM_H
#define EVE_SYSTEMS_ANOMALY_SYSTEM_H

#include "ecs/system.h"
#include "components/game_components.h"
#include <string>
#include <vector>

namespace atlas {
namespace systems {

/**
 * @brief Generates and manages in-space anomalies per solar system
 *
 * Uses a deterministic seed per system to spawn combat, mining, data,
 * relic, gas, and wormhole anomalies.  The difficulty scales with
 * the system's security status (lower sec = harder content).
 */
class AnomalySystem : public ecs::System {
public:
    explicit AnomalySystem(ecs::World* world);
    ~AnomalySystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "AnomalySystem"; }

    /**
     * @brief Generate anomalies for a solar system based on its seed
     * @param system_id  Entity id of the SolarSystem
     * @param seed       Deterministic seed for procedural generation
     * @param security   Security status (1.0 = highsec, 0.0 = nullsec)
     * @return number of anomalies created
     */
    int generateAnomalies(const std::string& system_id, uint32_t seed,
                          float security = 0.5f);

    /**
     * @brief Get all anomalies in a given solar system
     */
    std::vector<std::string> getAnomaliesInSystem(const std::string& system_id) const;

    /**
     * @brief Get the count of active (not completed, not despawned) anomalies
     */
    int getActiveAnomalyCount(const std::string& system_id) const;

    /**
     * @brief Mark an anomaly as completed (content cleared)
     */
    bool completeAnomaly(const std::string& anomaly_id);

    /**
     * @brief Calculate difficulty for a given security status
     */
    static components::Anomaly::Difficulty difficultyFromSecurity(float security);

    /**
     * @brief Calculate NPC count from difficulty
     */
    static int npcCountFromDifficulty(components::Anomaly::Difficulty diff);

    /**
     * @brief Calculate loot multiplier from difficulty
     */
    static float lootMultiplierFromDifficulty(components::Anomaly::Difficulty diff);

private:
    int anomaly_counter_ = 0;

    /**
     * @brief Pick an anomaly type from the seed
     */
    static components::Anomaly::Type typeFromSeed(uint32_t val);

    /**
     * @brief Generate a name for an anomaly based on type and index
     */
    static std::string generateName(components::Anomaly::Type type, int index);
};

} // namespace systems
} // namespace atlas

#endif // EVE_SYSTEMS_ANOMALY_SYSTEM_H
