#ifndef EVE_SYSTEMS_CAPTAIN_PERSONALITY_SYSTEM_H
#define EVE_SYSTEMS_CAPTAIN_PERSONALITY_SYSTEM_H

#include "ecs/system.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Assigns and manages personality traits for AI fleet captains
 *
 * Each captain has four personality axes (aggression, sociability,
 * optimism, professionalism) whose baseline values are influenced
 * by the captain's faction culture with a small random variation.
 */
class CaptainPersonalitySystem : public ecs::System {
public:
    explicit CaptainPersonalitySystem(ecs::World* world);
    ~CaptainPersonalitySystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "CaptainPersonalitySystem"; }

    /**
     * @brief Assign personality traits based on faction culture
     *
     * Faction defaults (before random variation of +/- 0.15):
     *  - Solari:   higher professionalism, moderate aggression
     *  - Veyren:   higher sociability, balanced
     *  - Aurelian: higher optimism, lower professionalism
     *  - Keldari:  higher aggression, lower sociability
     *
     * @param entity_id   Entity to assign the personality to
     * @param captain_name Display name for the captain
     * @param faction      One of "Solari", "Veyren", "Aurelian", "Keldari"
     */
    void assignPersonality(const std::string& entity_id,
                           const std::string& captain_name,
                           const std::string& faction);

    /**
     * @brief Set an individual personality trait by name
     * @param trait One of "aggression", "sociability", "optimism", "professionalism"
     * @param value Trait value clamped to [0, 1]
     */
    void setPersonalityTrait(const std::string& entity_id,
                             const std::string& trait,
                             float value);

    /**
     * @brief Get a personality trait value
     * @return Trait value or 0.0f if entity/trait not found
     */
    float getPersonalityTrait(const std::string& entity_id,
                              const std::string& trait) const;

    /**
     * @brief Get the faction string stored on an entity's personality
     * @return Faction name or empty string if not found
     */
    std::string getCaptainFaction(const std::string& entity_id) const;
};

} // namespace systems
} // namespace atlas

#endif // EVE_SYSTEMS_CAPTAIN_PERSONALITY_SYSTEM_H
