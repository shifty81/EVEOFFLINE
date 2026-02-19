#ifndef EVE_SYSTEMS_SECURITY_RESPONSE_SYSTEM_H
#define EVE_SYSTEMS_SECURITY_RESPONSE_SYSTEM_H

#include "ecs/system.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <string>
#include <vector>

namespace atlas {
namespace systems {

/**
 * @brief CONCORD-style delayed security response in high-sec systems
 *
 * When a system's threat exceeds a threshold and its security_level
 * is high enough, a security response timer begins.  After the delay
 * elapses the system enters "security_responding" state.  The delay
 * is shorter in higher-security systems.
 *
 * Response delay formula:
 *   delay = base_delay * (1.0 - security_level * speed_factor)
 *   clamped to [min_delay, base_delay]
 */
class SecurityResponseSystem : public ecs::System {
public:
    explicit SecurityResponseSystem(ecs::World* world);
    ~SecurityResponseSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "SecurityResponseSystem"; }

    // --- Query API ---

    /** Check whether a security response is active in a system */
    bool isResponding(const std::string& system_id) const;

    /** Get the remaining delay before response activates */
    float getResponseTimer(const std::string& system_id) const;

    /** Get list of systems with active security response */
    std::vector<std::string> getRespondingSystems() const;

    // --- Configuration ---
    float threat_threshold = 0.3f;    // threat needed to trigger response
    float security_min_level = 0.4f;  // systems below this have no response
    float base_delay = 30.0f;         // max response delay (seconds)
    float min_delay = 5.0f;           // fastest response (1.0 sec systems)
    float speed_factor = 0.8f;        // how much security_level speeds response
    float response_duration = 120.0f; // how long response stays active

private:
    void evaluateSystem(ecs::Entity* entity,
                        components::SecurityResponseState* resp,
                        const components::SimStarSystemState* state,
                        float dt);
};

} // namespace systems
} // namespace atlas

#endif // EVE_SYSTEMS_SECURITY_RESPONSE_SYSTEM_H
