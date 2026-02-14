#ifndef EVE_SYSTEMS_SHIP_FITTING_SYSTEM_H
#define EVE_SYSTEMS_SHIP_FITTING_SYSTEM_H

#include "ecs/system.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Manages ship module fitting (install/remove) into high/mid/low slots
 *
 * Enforces slot count limits per ship class and validates CPU/powergrid
 * budgets when fitting new modules. Works alongside ModuleSystem which
 * handles module activation/cycling.
 *
 * Slot counts by ship class:
 *   Frigate:     3 high, 3 mid, 2 low
 *   Destroyer:   4 high, 3 mid, 3 low
 *   Cruiser:     5 high, 4 mid, 4 low
 *   Battlecruiser: 6 high, 4 mid, 5 low
 *   Battleship:  7 high, 5 mid, 5 low
 *   Capital:     8 high, 6 mid, 6 low
 *   Titan:       8 high, 6 mid, 6 low
 */
class ShipFittingSystem : public ecs::System {
public:
    explicit ShipFittingSystem(ecs::World* world);
    ~ShipFittingSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "ShipFittingSystem"; }

    /**
     * @brief Fit a module into a ship slot
     * @param entity_id Ship entity to fit module on
     * @param module_id Unique module identifier
     * @param module_name Display name
     * @param slot_type "high", "mid", or "low"
     * @param cpu_usage CPU required
     * @param powergrid_usage Powergrid required
     * @param capacitor_cost Capacitor cost per cycle
     * @param cycle_time Seconds per activation cycle
     * @return true if module was fitted successfully
     */
    bool fitModule(const std::string& entity_id,
                   const std::string& module_id,
                   const std::string& module_name,
                   const std::string& slot_type,
                   float cpu_usage = 10.0f,
                   float powergrid_usage = 5.0f,
                   float capacitor_cost = 5.0f,
                   float cycle_time = 5.0f);

    /**
     * @brief Remove a module from a ship slot
     * @param entity_id Ship entity
     * @param slot_type "high", "mid", or "low"
     * @param slot_index Index of the module within that slot type
     * @return true if module was removed
     */
    bool removeModule(const std::string& entity_id,
                      const std::string& slot_type,
                      int slot_index);

    /**
     * @brief Get the maximum number of slots for a given ship class and slot type
     * @param ship_class e.g. "Frigate", "Cruiser", "Battleship"
     * @param slot_type "high", "mid", or "low"
     * @return maximum slot count
     */
    static int getSlotCapacity(const std::string& ship_class,
                               const std::string& slot_type);

    /**
     * @brief Get the number of currently fitted modules in a slot type
     * @param entity_id Ship entity
     * @param slot_type "high", "mid", or "low"
     * @return number of fitted modules, or -1 on error
     */
    int getFittedCount(const std::string& entity_id,
                       const std::string& slot_type) const;

    /**
     * @brief Check whether the current fitting is valid (CPU + powergrid within budget)
     * @param entity_id Ship entity
     * @return true if fitting is valid
     */
    bool validateFitting(const std::string& entity_id) const;
};

} // namespace systems
} // namespace atlas

#endif // EVE_SYSTEMS_SHIP_FITTING_SYSTEM_H
