#ifndef EVE_SYSTEMS_FLEET_MORALE_SYSTEM_H
#define EVE_SYSTEMS_FLEET_MORALE_SYSTEM_H

#include "ecs/system.h"
#include <string>

namespace atlas {
namespace systems {

class FleetMoraleSystem : public ecs::System {
public:
    explicit FleetMoraleSystem(ecs::World* world);
    ~FleetMoraleSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "FleetMoraleSystem"; }

    // Record combat outcomes
    void recordWin(const std::string& entity_id);
    void recordLoss(const std::string& entity_id);
    void recordShipLost(const std::string& entity_id);
    void recordSavedByPlayer(const std::string& entity_id);
    void recordPlayerSaved(const std::string& entity_id);
    void recordMissionTogether(const std::string& entity_id);

    // Queries
    float getMoraleScore(const std::string& entity_id) const;
    std::string getMoraleState(const std::string& entity_id) const;
};

} // namespace systems
} // namespace atlas

#endif // EVE_SYSTEMS_FLEET_MORALE_SYSTEM_H
