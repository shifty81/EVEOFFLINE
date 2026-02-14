#include "systems/scanner_system.h"
#include "ecs/world.h"
#include "components/game_components.h"
#include <algorithm>

namespace atlas {
namespace systems {

ScannerSystem::ScannerSystem(ecs::World* world)
    : System(world) {
}

void ScannerSystem::update(float delta_time) {
    for (auto* entity : world_->getAllEntities()) {
        auto* scanner = entity->getComponent<components::Scanner>();
        if (!scanner || !scanner->scanning) continue;

        // Find the target signature
        auto* sig_entity = world_->getEntity(scanner->scan_target_id);
        if (!sig_entity) {
            scanner->scanning = false;
            scanner->scan_progress = 0.0f;
            continue;
        }

        auto* sig = sig_entity->getComponent<components::AnomalySignature>();
        if (!sig || sig->despawned) {
            scanner->scanning = false;
            scanner->scan_progress = 0.0f;
            continue;
        }

        // Advance scan timer
        scanner->scan_progress += delta_time;

        // Check if a scan cycle completed
        if (scanner->scan_progress >= scanner->scan_duration) {
            scanner->scan_progress -= scanner->scan_duration;

            // Calculate signal gain: strength / (difficulty * base_scan_difficulty)
            float gain = scanner->scan_strength /
                         (100.0f * sig->base_scan_difficulty *
                          static_cast<float>(std::max(sig->difficulty, 1)));

            // Apply deviation reduction: high deviation lowers effective gain
            gain *= (1.0f - scanner->scan_deviation * 0.5f);

            sig->signal_strength = std::min(1.0f, sig->signal_strength + gain);

            // If fully resolved, stop scanning
            if (sig->isResolved()) {
                scanner->scanning = false;
                scanner->scan_progress = 0.0f;
            }
        }
    }
}

bool ScannerSystem::startScan(const std::string& scanner_id,
                              const std::string& signature_id) {
    auto* scanner_entity = world_->getEntity(scanner_id);
    if (!scanner_entity) return false;

    auto* scanner = scanner_entity->getComponent<components::Scanner>();
    if (!scanner) return false;

    auto* sig_entity = world_->getEntity(signature_id);
    if (!sig_entity) return false;

    auto* sig = sig_entity->getComponent<components::AnomalySignature>();
    if (!sig || sig->despawned || sig->isResolved()) return false;

    scanner->scanning = true;
    scanner->scan_progress = 0.0f;
    scanner->scan_target_id = signature_id;
    return true;
}

bool ScannerSystem::stopScan(const std::string& scanner_id) {
    auto* scanner_entity = world_->getEntity(scanner_id);
    if (!scanner_entity) return false;

    auto* scanner = scanner_entity->getComponent<components::Scanner>();
    if (!scanner || !scanner->scanning) return false;

    scanner->scanning = false;
    scanner->scan_progress = 0.0f;
    return true;
}

float ScannerSystem::getSignalStrength(const std::string& signature_id) const {
    auto* entity = world_->getEntity(signature_id);
    if (!entity) return -1.0f;

    auto* sig = entity->getComponent<components::AnomalySignature>();
    if (!sig) return -1.0f;

    return sig->signal_strength;
}

bool ScannerSystem::isResolved(const std::string& signature_id) const {
    auto* entity = world_->getEntity(signature_id);
    if (!entity) return false;

    auto* sig = entity->getComponent<components::AnomalySignature>();
    if (!sig) return false;

    return sig->isResolved();
}

int ScannerSystem::getActiveScanCount() const {
    int count = 0;
    for (auto* entity : world_->getAllEntities()) {
        auto* scanner = entity->getComponent<components::Scanner>();
        if (scanner && scanner->scanning) count++;
    }
    return count;
}

} // namespace systems
} // namespace atlas
