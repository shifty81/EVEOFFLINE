#ifndef EVE_SYSTEMS_FLEET_FORMATION_SYSTEM_H
#define EVE_SYSTEMS_FLEET_FORMATION_SYSTEM_H

#include "ecs/system.h"
#include "components/game_components.h"
#include <string>
#include <vector>

namespace atlas {
namespace systems {

/**
 * @brief Manages fleet member spatial formation offsets
 *
 * When a fleet is warping or travelling, each member is assigned an
 * offset relative to the fleet commander based on the active formation
 * type and their slot index.
 */
class FleetFormationSystem : public ecs::System {
public:
    explicit FleetFormationSystem(ecs::World* world);
    ~FleetFormationSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "FleetFormationSystem"; }

    /**
     * @brief Set the formation type for a fleet member
     */
    void setFormation(const std::string& entity_id,
                      components::FleetFormation::FormationType type,
                      int slot_index);

    /**
     * @brief Get the current formation type for an entity
     */
    components::FleetFormation::FormationType getFormation(
        const std::string& entity_id) const;

    /**
     * @brief Compute and store formation offsets for all entities
     *        that have a FleetFormation component.
     *
     * Slot 0 is the leader (offset 0,0,0). Other slots fan out based
     * on the formation pattern.
     */
    void computeOffsets();

    /**
     * @brief Get the computed formation offset for an entity
     * @param[out] ox, oy, oz  Offset in metres
     * @return true if the entity has a FleetFormation component
     */
    bool getOffset(const std::string& entity_id,
                   float& ox, float& oy, float& oz) const;

    /**
     * @brief Spacing between formation slots in metres
     */
    static constexpr float kDefaultSpacing = 500.0f;

private:
    void computeArrow(components::FleetFormation* f);
    void computeLine(components::FleetFormation* f);
    void computeWedge(components::FleetFormation* f);
    void computeSpread(components::FleetFormation* f);
    void computeDiamond(components::FleetFormation* f);
};

} // namespace systems
} // namespace atlas

#endif // EVE_SYSTEMS_FLEET_FORMATION_SYSTEM_H
