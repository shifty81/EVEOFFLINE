#ifndef EVE_SYSTEMS_STATION_SYSTEM_H
#define EVE_SYSTEMS_STATION_SYSTEM_H

#include "ecs/system.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Manages station docking, undocking, and station services
 *
 * Stations are entities with a Station component. Players can dock
 * when within docking range, which stops movement and grants access
 * to station services (repair, refit, market, etc.).
 *
 * While docked:
 *   - The player entity is not removed but flagged as docked
 *   - Repair service restores shield/armor/hull to max for an ISK cost
 *   - The player cannot move or be targeted
 *
 * On undock the docked flag is cleared and movement resumes.
 */
class StationSystem : public ecs::System {
public:
    explicit StationSystem(ecs::World* world);
    ~StationSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "StationSystem"; }

    /**
     * @brief Create a station entity in the world
     * @param station_id  Unique entity id for the station
     * @param name        Display name (e.g. "Jita IV Trade Hub")
     * @param x,y,z       World position
     * @param docking_range  Maximum distance for docking (metres)
     * @param repair_cost_per_hp  ISK cost per hit-point repaired
     * @return true if station was created
     */
    bool createStation(const std::string& station_id,
                       const std::string& name,
                       float x, float y, float z,
                       float docking_range = 2500.0f,
                       float repair_cost_per_hp = 1.0f);

    /**
     * @brief Request docking at the nearest station
     * @param entity_id  The player entity requesting dock
     * @param station_id The station to dock at
     * @return true if docking was successful (in range, station exists)
     */
    bool dockAtStation(const std::string& entity_id,
                       const std::string& station_id);

    /**
     * @brief Undock from the current station
     * @param entity_id  The player entity requesting undock
     * @return true if undock was successful
     */
    bool undockFromStation(const std::string& entity_id);

    /**
     * @brief Repair all damage on a docked ship
     * @param entity_id  The player entity requesting repair
     * @return ISK cost charged for the repair (0.0 if not docked or no damage)
     */
    double repairShip(const std::string& entity_id);

    /**
     * @brief Check if an entity is currently docked
     */
    bool isDocked(const std::string& entity_id) const;

    /**
     * @brief Get the station id where an entity is docked
     * @return station entity id, or empty string if not docked
     */
    std::string getDockedStation(const std::string& entity_id) const;
};

} // namespace systems
} // namespace atlas

#endif // EVE_SYSTEMS_STATION_SYSTEM_H
