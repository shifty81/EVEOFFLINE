#ifndef EVE_SYSTEMS_BACKGROUND_SIMULATION_SYSTEM_H
#define EVE_SYSTEMS_BACKGROUND_SIMULATION_SYSTEM_H

#include "ecs/system.h"
#include "components/game_components.h"
#include <string>
#include <vector>

namespace atlas {
namespace systems {

/**
 * @brief Continuous background simulation for star systems
 *
 * Updates per-system state vectors each tick: traffic, economy, security,
 * faction influence.  Triggers threshold-based events (pirate surge,
 * resource shortage, lockdown) when conditions are met.
 */
class BackgroundSimulationSystem : public ecs::System {
public:
    explicit BackgroundSimulationSystem(ecs::World* world);
    ~BackgroundSimulationSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "BackgroundSimulationSystem"; }

    // --- Query API ---

    /** Get the current system state for a star system entity */
    const components::SimStarSystemState* getSystemState(const std::string& system_id) const;

    /** Check if a specific event is active in a system */
    bool isEventActive(const std::string& system_id, const std::string& event_type) const;

    /** Get list of systems currently in a specific event state */
    std::vector<std::string> getSystemsWithEvent(const std::string& event_type) const;

    // --- Configuration ---

    /** Thresholds for triggering events */
    float pirate_surge_threshold = 0.7f;      // pirate_activity above this triggers surge
    float shortage_threshold = 0.2f;           // resource_availability below this triggers shortage
    float lockdown_threat_threshold = 0.8f;    // threat_level above this triggers lockdown

    /** Decay/growth rates per second */
    float threat_decay_rate = 0.01f;           // threat naturally decays
    float economic_recovery_rate = 0.005f;     // economy recovers slowly
    float traffic_fluctuation_rate = 0.02f;    // traffic drifts toward baseline
    float resource_regen_rate = 0.002f;        // resources slowly regenerate
    float event_duration = 300.0f;             // default event duration in seconds

private:
    void updateSystemState(components::SimStarSystemState* state, float dt);
    void evaluateEvents(const std::string& system_id, components::SimStarSystemState* state);
    void tickEventTimers(components::SimStarSystemState* state, float dt);
};

} // namespace systems
} // namespace atlas

#endif // EVE_SYSTEMS_BACKGROUND_SIMULATION_SYSTEM_H
