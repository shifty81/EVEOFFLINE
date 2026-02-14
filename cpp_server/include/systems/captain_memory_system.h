#ifndef EVE_SYSTEMS_CAPTAIN_MEMORY_SYSTEM_H
#define EVE_SYSTEMS_CAPTAIN_MEMORY_SYSTEM_H

#include "ecs/system.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Stores and queries persistent memories for AI captains
 *
 * Captains accumulate MemoryEntry records as significant events occur.
 * Other systems (chatter, morale, relationship) query memories to
 * drive contextual behaviour.
 */
class CaptainMemorySystem : public ecs::System {
public:
    explicit CaptainMemorySystem(ecs::World* world);
    ~CaptainMemorySystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "CaptainMemorySystem"; }

    /**
     * @brief Record a new memory for an entity
     * @param event_type  e.g. "combat_win", "combat_loss", "ship_lost",
     *                    "saved_by_player", "warp_anomaly"
     * @param context     Free-form detail
     * @param timestamp   In-game time of the event
     * @param weight      Emotional weight (-1 traumatic .. +1 uplifting)
     */
    void recordMemory(const std::string& entity_id,
                      const std::string& event_type,
                      const std::string& context,
                      float timestamp,
                      float weight);

    /**
     * @brief Count memories of a given type
     */
    int countMemories(const std::string& entity_id,
                      const std::string& event_type) const;

    /**
     * @brief Average emotional weight across all memories
     */
    float averageEmotionalWeight(const std::string& entity_id) const;

    /**
     * @brief Total number of memories stored
     */
    int totalMemories(const std::string& entity_id) const;

    /**
     * @brief Get the most recent memory's event_type (or "")
     */
    std::string mostRecentEvent(const std::string& entity_id) const;
};

} // namespace systems
} // namespace atlas

#endif // EVE_SYSTEMS_CAPTAIN_MEMORY_SYSTEM_H
