#ifndef EVE_SYSTEMS_FLEET_CHATTER_SYSTEM_H
#define EVE_SYSTEMS_FLEET_CHATTER_SYSTEM_H

#include "ecs/system.h"
#include <string>

namespace atlas {
namespace systems {

class FleetChatterSystem : public ecs::System {
public:
    explicit FleetChatterSystem(ecs::World* world);
    ~FleetChatterSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "FleetChatterSystem"; }

    void setActivity(const std::string& entity_id, const std::string& activity);
    std::string getNextChatterLine(const std::string& entity_id);
    bool isOnCooldown(const std::string& entity_id) const;
    int getTotalLinesSpoken(const std::string& entity_id) const;

    /**
     * @brief Get a personality-contextual chatter line
     *
     * Picks from a richer line pool filtered by the captain's dominant
     * personality trait, falling back to the generic activity pool when
     * no personality component is present.
     */
    std::string getContextualLine(const std::string& entity_id);

    // ---- Phase 9 additions ----

    /**
     * @brief Attempt to interrupt an entity's current speech with a
     *        higher-priority line.
     * @return true if the interrupt succeeded (new priority > speaking priority)
     */
    bool interruptChatter(const std::string& entity_id, float new_priority);

    /**
     * @brief Check whether ANY entity in the fleet is currently speaking
     *        (fleet-wide overlap prevention).
     */
    bool isAnyoneSpeaking() const;

    /**
     * @brief Get a silence-aware chatter line.
     *
     * If PlayerPresence tracks long silence, returns a "silence
     * interpretation" line (e.g. "Quiet today, boss.").
     * Otherwise behaves like getContextualLine().
     */
    std::string getSilenceAwareLine(const std::string& entity_id,
                                    const std::string& player_entity_id);

    /**
     * @brief Propagate a rumor from one captain to another during chatter.
     *
     * Picks a random rumor from the speaker's RumorLog and copies it
     * to the listener, increasing belief_strength if already present.
     */
    void propagateRumor(const std::string& speaker_id,
                        const std::string& listener_id);

    /**
     * @brief Compute disagreement score for a captain.
     *
     * Based on: risk × (1 - aggression) + losses × (1 - optimism)
     *           + task_mismatch_penalty
     * High score → captain may request departure or transfer.
     */
    float computeDisagreement(const std::string& entity_id,
                              float current_risk,
                              bool task_mismatch) const;
};

} // namespace systems
} // namespace atlas

#endif // EVE_SYSTEMS_FLEET_CHATTER_SYSTEM_H
