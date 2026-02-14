#ifndef EVE_SYSTEMS_SCANNER_SYSTEM_H
#define EVE_SYSTEMS_SCANNER_SYSTEM_H

#include "ecs/system.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <string>
#include <vector>

namespace atlas {
namespace systems {

/**
 * @brief Handles probe scanning for discovering anomalies
 *
 * Players deploy probes and initiate scans.  Each scan cycle
 * improves signal strength on nearby anomalies based on
 * probe count, scan strength, and the anomaly's signature.
 */
class ScannerSystem : public ecs::System {
public:
    explicit ScannerSystem(ecs::World* world);
    ~ScannerSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "ScannerSystem"; }

    /**
     * @brief Begin scanning a solar system for anomalies
     * @param scanner_id  Entity with a Scanner component
     * @param system_id   Solar system to scan
     * @return true if scanning started
     */
    bool startScan(const std::string& scanner_id,
                   const std::string& system_id);

    /**
     * @brief Stop an active scan
     */
    bool stopScan(const std::string& scanner_id);

    /**
     * @brief Get scan results for a scanner entity
     */
    std::vector<components::Scanner::ScanResult>
    getScanResults(const std::string& scanner_id) const;

    /**
     * @brief Calculate effective scan strength from probes and base strength
     */
    static float effectiveScanStrength(float base_strength, int probe_count);

    /**
     * @brief Calculate signal gain per scan cycle
     */
    static float signalGainPerCycle(float effective_strength,
                                     float anomaly_signature);

    /**
     * @brief Get the number of currently active scanners
     */
    int getActiveScannerCount() const;

private:
    /**
     * @brief Process a completed scan cycle
     */
    void completeScanCycle(ecs::Entity* scanner_entity);
};

} // namespace systems
} // namespace atlas

#endif // EVE_SYSTEMS_SCANNER_SYSTEM_H
