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
};

} // namespace systems
} // namespace atlas

#endif // EVE_SYSTEMS_FLEET_CHATTER_SYSTEM_H
