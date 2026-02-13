#ifndef EVE_SYSTEMS_TACTICAL_OVERLAY_SYSTEM_H
#define EVE_SYSTEMS_TACTICAL_OVERLAY_SYSTEM_H

#include "ecs/system.h"
#include <string>
#include <vector>

namespace atlas {
namespace systems {

class TacticalOverlaySystem : public ecs::System {
public:
    explicit TacticalOverlaySystem(ecs::World* world);
    ~TacticalOverlaySystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "TacticalOverlaySystem"; }

    void toggleOverlay(const std::string& entity_id);
    bool isEnabled(const std::string& entity_id) const;
    void setToolRange(const std::string& entity_id, float range, const std::string& tool_type);
    std::vector<float> getRingDistances(const std::string& entity_id) const;
    void setRingDistances(const std::string& entity_id, const std::vector<float>& distances);
};

} // namespace systems
} // namespace atlas

#endif // EVE_SYSTEMS_TACTICAL_OVERLAY_SYSTEM_H
