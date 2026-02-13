#ifndef EVE_SYSTEMS_EMOTIONAL_ARC_SYSTEM_H
#define EVE_SYSTEMS_EMOTIONAL_ARC_SYSTEM_H

#include "ecs/system.h"
#include <string>

namespace atlas {
namespace systems {

class EmotionalArcSystem : public ecs::System {
public:
    explicit EmotionalArcSystem(ecs::World* world);
    ~EmotionalArcSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "EmotionalArcSystem"; }

    void onCombatVictory(const std::string& entity_id);
    void onCombatDefeat(const std::string& entity_id);
    void onRest(const std::string& entity_id);
    void onPlayerTrust(const std::string& entity_id);
    void onPlayerBetray(const std::string& entity_id);

    float getConfidence(const std::string& entity_id) const;
    float getTrust(const std::string& entity_id) const;
    float getFatigue(const std::string& entity_id) const;
    float getHope(const std::string& entity_id) const;
};

} // namespace systems
} // namespace atlas

#endif // EVE_SYSTEMS_EMOTIONAL_ARC_SYSTEM_H
