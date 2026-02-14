#ifndef EVE_SYSTEMS_REFINING_SYSTEM_H
#define EVE_SYSTEMS_REFINING_SYSTEM_H

#include "ecs/system.h"
#include "ecs/entity.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Converts raw ore into refined minerals at a refining facility
 *
 * Players dock at a station with a RefiningFacility component, then
 * submit ore from their inventory.  The system converts ore batches
 * into minerals based on the facility's recipes and efficiency.
 *
 * Yield formula:
 *   output = base_quantity * facility_efficiency * (1 - tax_rate)
 */
class RefiningSystem : public ecs::System {
public:
    explicit RefiningSystem(ecs::World* world);
    ~RefiningSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "RefiningSystem"; }

    /**
     * @brief Refine ore from a player's inventory at a station
     * @param player_id   Entity id of the player
     * @param station_id  Entity id of the station with RefiningFacility
     * @param ore_type    Name of the ore to refine (e.g. "Veldspar")
     * @param batches     Number of batches to refine (each consumes ore_units_required)
     * @return Number of batches actually refined (0 on failure)
     */
    int refineOre(const std::string& player_id,
                  const std::string& station_id,
                  const std::string& ore_type,
                  int batches = 1);

    /**
     * @brief Set up default refining recipes on a facility
     *
     * Installs standard recipes for common ore types:
     * Veldspar→Tritanium, Scordite→Tritanium+Pyerite,
     * Pyroxeres→Pyerite+Nocxidium, Plagioclase→Tritanium+Pyerite+Mexallon
     *
     * @param station_id Entity with a RefiningFacility component
     * @return true if recipes were installed
     */
    bool installDefaultRecipes(const std::string& station_id);
};

} // namespace systems
} // namespace atlas

#endif // EVE_SYSTEMS_REFINING_SYSTEM_H
