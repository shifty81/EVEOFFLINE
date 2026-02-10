#ifndef EVE_SYSTEMS_INVENTORY_SYSTEM_H
#define EVE_SYSTEMS_INVENTORY_SYSTEM_H

#include "ecs/system.h"
#include <string>

namespace eve {
namespace systems {

/**
 * @brief Manages entity inventories (cargo hold)
 *
 * Provides item add/remove/transfer operations that respect
 * capacity limits measured in m3.
 */
class InventorySystem : public ecs::System {
public:
    explicit InventorySystem(ecs::World* world);
    ~InventorySystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "InventorySystem"; }

    /**
     * @brief Add an item to an entity's inventory
     * @return true if the item fits within capacity
     */
    bool addItem(const std::string& entity_id,
                 const std::string& item_id,
                 const std::string& name,
                 const std::string& type,
                 int quantity,
                 float volume);

    /**
     * @brief Remove quantity of an item from inventory
     * @return Amount actually removed (may be less than requested)
     */
    int removeItem(const std::string& entity_id,
                   const std::string& item_id,
                   int quantity);

    /**
     * @brief Transfer items between two entities
     * @return true if transfer succeeded
     */
    bool transferItem(const std::string& from_id,
                      const std::string& to_id,
                      const std::string& item_id,
                      int quantity);

    /**
     * @brief Get total count of an item in inventory
     */
    int getItemCount(const std::string& entity_id,
                     const std::string& item_id);

    /**
     * @brief Check if entity has at least 'quantity' of an item
     */
    bool hasItem(const std::string& entity_id,
                 const std::string& item_id,
                 int quantity = 1);
};

} // namespace systems
} // namespace eve

#endif // EVE_SYSTEMS_INVENTORY_SYSTEM_H
