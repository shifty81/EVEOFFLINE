#ifndef EVE_SYSTEMS_SCANNER_SYSTEM_H
#define EVE_SYSTEMS_SCANNER_SYSTEM_H

#include "ecs/system.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Handles probe scanning and anomaly signal resolution
 *
 * Players with a Scanner component can initiate scans to resolve
 * AnomalySignature entities.  Each scan cycle advances signal_strength
 * based on the scanner's scan_strength vs the anomaly's base_scan_difficulty.
 * When signal_strength reaches 1.0 the site is fully resolved and can be
 * warped to.
 */
class ScannerSystem : public ecs::System {
public:
    explicit ScannerSystem(ecs::World* world);
    ~ScannerSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "ScannerSystem"; }

    /**
     * @brief Begin scanning a signature
     * @param scanner_id   Entity with a Scanner component
     * @param signature_id Entity with an AnomalySignature component
     * @return true if scan started
     */
    bool startScan(const std::string& scanner_id,
                   const std::string& signature_id);

    /**
     * @brief Cancel an active scan
     * @return true if scan was active and is now cancelled
     */
    bool stopScan(const std::string& scanner_id);

    /**
     * @brief Query resolved signal strength of a signature
     * @return signal strength (0.0–1.0), or -1 if entity not found
     */
    float getSignalStrength(const std::string& signature_id) const;

    /**
     * @brief Check if a signature is fully resolved
     */
    bool isResolved(const std::string& signature_id) const;

    /**
     * @brief Get the count of active scans in the world
     */
    int getActiveScanCount() const;
};

} // namespace systems
} // namespace atlas

#endif // EVE_SYSTEMS_SCANNER_SYSTEM_H
