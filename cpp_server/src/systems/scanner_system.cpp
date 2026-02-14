#include "systems/scanner_system.h"
#include "ecs/world.h"
#include <cmath>
#include <algorithm>

namespace atlas {
namespace systems {

static constexpr int OPTIMAL_PROBE_COUNT = 8;       // baseline probe count for full strength
static constexpr float MIN_SIGNAL_GAIN = 0.01f;     // minimum gain per cycle to ensure progress

ScannerSystem::ScannerSystem(ecs::World* world)
    : System(world) {
}

void ScannerSystem::update(float delta_time) {
    for (auto* entity : world_->getAllEntities()) {
        auto* scanner = entity->getComponent<components::Scanner>();
        if (!scanner || !scanner->scanning) continue;

        scanner->scan_progress += delta_time;
        if (scanner->scan_progress >= scanner->scan_duration) {
            completeScanCycle(entity);
            scanner->scan_progress = 0.0f;
        }
    }
}

// -----------------------------------------------------------------------
// Start / Stop scanning
// -----------------------------------------------------------------------

bool ScannerSystem::startScan(const std::string& scanner_id,
                               const std::string& system_id) {
    auto* entity = world_->getEntity(scanner_id);
    if (!entity) return false;

    auto* scanner = entity->getComponent<components::Scanner>();
    if (!scanner) return false;

    scanner->scanning = true;
    scanner->target_system_id = system_id;
    scanner->scan_progress = 0.0f;
    // Don't clear previous results — they accumulate
    return true;
}

bool ScannerSystem::stopScan(const std::string& scanner_id) {
    auto* entity = world_->getEntity(scanner_id);
    if (!entity) return false;

    auto* scanner = entity->getComponent<components::Scanner>();
    if (!scanner || !scanner->scanning) return false;

    scanner->scanning = false;
    scanner->scan_progress = 0.0f;
    return true;
}

std::vector<components::Scanner::ScanResult>
ScannerSystem::getScanResults(const std::string& scanner_id) const {
    auto* entity = world_->getEntity(scanner_id);
    if (!entity) return {};

    auto* scanner = entity->getComponent<components::Scanner>();
    if (!scanner) return {};

    return scanner->results;
}

int ScannerSystem::getActiveScannerCount() const {
    int count = 0;
    for (auto* entity : world_->getAllEntities()) {
        auto* scanner = entity->getComponent<components::Scanner>();
        if (scanner && scanner->scanning) ++count;
    }
    return count;
}

// -----------------------------------------------------------------------
// Scan cycle completion
// -----------------------------------------------------------------------

void ScannerSystem::completeScanCycle(ecs::Entity* scanner_entity) {
    auto* scanner = scanner_entity->getComponent<components::Scanner>();
    if (!scanner) return;

    float eff_strength = effectiveScanStrength(scanner->scan_strength,
                                                scanner->probe_count);

    // Find all anomalies in the target system
    for (auto* entity : world_->getAllEntities()) {
        auto* anom = entity->getComponent<components::Anomaly>();
        if (!anom || anom->system_id != scanner->target_system_id) continue;
        if (anom->completed) continue;

        float gain = signalGainPerCycle(eff_strength, anom->signature_strength);

        // Find or create scan result for this anomaly
        bool found = false;
        for (auto& result : scanner->results) {
            if (result.anomaly_id == anom->anomaly_id) {
                result.signal_strength = std::min(1.0f,
                    result.signal_strength + gain);
                // Deviation decreases as signal improves
                result.deviation = scanner->scan_deviation *
                    (1.0f - result.signal_strength);
                result.warpable = result.signal_strength >= 1.0f;
                
                // Mark anomaly as discovered once signal >= 0.25
                if (result.signal_strength >= 0.25f) {
                    anom->discovered = true;
                }
                found = true;
                break;
            }
        }

        if (!found) {
            components::Scanner::ScanResult result;
            result.anomaly_id = anom->anomaly_id;
            result.signal_strength = std::min(1.0f, gain);
            result.deviation = scanner->scan_deviation *
                (1.0f - result.signal_strength);
            result.warpable = result.signal_strength >= 1.0f;
            if (result.signal_strength >= 0.25f) {
                anom->discovered = true;
            }
            scanner->results.push_back(result);
        }
    }
}

// -----------------------------------------------------------------------
// Static helpers
// -----------------------------------------------------------------------

float ScannerSystem::effectiveScanStrength(float base_strength,
                                            int probe_count) {
    // More probes = better triangulation
    // Diminishing returns: sqrt scaling, minimum 1 probe
    int probes = std::max(1, probe_count);
    return base_strength * std::sqrt(static_cast<float>(probes)) / std::sqrt(static_cast<float>(OPTIMAL_PROBE_COUNT));
    // 8 probes at 50 base = 50.0 effective
    // 4 probes at 50 base = 35.35 effective
    // 1 probe  at 50 base = 17.68 effective
}

float ScannerSystem::signalGainPerCycle(float effective_strength,
                                         float anomaly_signature) {
    // Signal gain = (effective_strength / 100) * anomaly_signature
    // A strong scanner + strong signal = fast scan
    // A weak scanner + weak signal = many cycles needed
    float gain = (effective_strength / 100.0f) * anomaly_signature;
    return std::clamp(gain, MIN_SIGNAL_GAIN, 1.0f);
}

} // namespace systems
} // namespace atlas
