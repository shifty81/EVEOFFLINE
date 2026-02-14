#ifndef EVE_SYSTEMS_REFINING_SYSTEM_H
#define EVE_SYSTEMS_REFINING_SYSTEM_H

#include "ecs/system.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Handles ore refining at stations with refinery services
 *
 * Players submit ore from their inventory to a RefiningFacility.
 * The system processes each job over time, then deposits refined
 * minerals back into the player's inventory.
 */
class RefiningSystem : public ecs::System {
public:
    explicit RefiningSystem(ecs::World* world);
    ~RefiningSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "RefiningSystem"; }

    /**
     * @brief Start a refining job at a station
     * @param station_id  Entity with a RefiningFacility component
     * @param owner_id    Entity with an Inventory component (the player)
     * @param ore_type    Type of ore to refine
     * @param ore_quantity Number of ore units to refine
     * @return job_id or empty string on failure
     */
    std::string startRefining(const std::string& station_id,
                              const std::string& owner_id,
                              const std::string& ore_type,
                              int ore_quantity);

    /**
     * @brief Get the number of active refining jobs at a station
     */
    int getActiveJobCount(const std::string& station_id);

    /**
     * @brief Get the number of completed jobs at a station
     */
    int getCompletedJobCount(const std::string& station_id);

    /**
     * @brief Seed a station with standard ore recipes
     *
     * Adds recipes for Veldspar→Tritanium, Scordite→Pyerite,
     * Pyroxeres→Mexallon, Plagioclase→Isogen, Omber→Nocxium,
     * Kernite→Zydrine, Arkonor→Megacyte.
     */
    void seedStandardRecipes(const std::string& station_id);

private:
    int job_counter_ = 0;
};

} // namespace systems
} // namespace atlas

#endif // EVE_SYSTEMS_REFINING_SYSTEM_H
