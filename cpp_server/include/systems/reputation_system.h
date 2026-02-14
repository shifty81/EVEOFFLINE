#ifndef EVE_SYSTEMS_REPUTATION_SYSTEM_H
#define EVE_SYSTEMS_REPUTATION_SYSTEM_H

#include "ecs/system.h"
#include "components/game_components.h"
#include <string>
#include <map>

namespace atlas {
namespace systems {

/**
 * @brief Manages faction reputation with derived standings, agent access
 *        gating, and faction-pair effects.
 *
 * Reputation changes are event-driven: when a standing is modified the
 * system automatically propagates derived effects to allied/enemy factions.
 */
class ReputationSystem : public ecs::System {
public:
    explicit ReputationSystem(ecs::World* world);
    ~ReputationSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "ReputationSystem"; }

    /**
     * @brief Modify faction standing with derived effects
     * @param entity_id  Entity whose standings are changed
     * @param faction    Target faction name
     * @param change     Raw standing change (-10 to +10 range)
     */
    void modifyFactionStanding(const std::string& entity_id,
                               const std::string& faction,
                               float change);

    /**
     * @brief Check if entity has enough standing for an agent
     * @return true when effective standing >= required_standing
     */
    bool hasAgentAccess(const std::string& entity_id,
                       const std::string& faction,
                       float required_standing) const;

    /**
     * @brief Get effective standing (derived effects are already baked in)
     */
    float getEffectiveStanding(const std::string& entity_id,
                               const std::string& faction) const;

    /**
     * @brief Install default faction relationships (ally/enemy pairs)
     */
    void installFactionRelationships();

    /**
     * @brief Get the disposition modifier for a faction pair
     * @return -1 enemy, 0 neutral, +1 ally (continuous scale)
     */
    float getFactionDisposition(const std::string& faction_a,
                                const std::string& faction_b) const;

private:
    // Faction pair relationships: key = "factionA:factionB", value = disposition (-1 to +1)
    std::map<std::string, float> faction_relationships_;

    std::string pairKey(const std::string& a, const std::string& b) const;
};

} // namespace systems
} // namespace atlas

#endif // EVE_SYSTEMS_REPUTATION_SYSTEM_H
