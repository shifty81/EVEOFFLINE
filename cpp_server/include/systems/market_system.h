#ifndef EVE_SYSTEMS_MARKET_SYSTEM_H
#define EVE_SYSTEMS_MARKET_SYSTEM_H

#include "ecs/system.h"
#include <string>
#include <vector>

namespace atlas {
namespace systems {

class MarketSystem : public ecs::System {
public:
    explicit MarketSystem(ecs::World* world);
    ~MarketSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "MarketSystem"; }

    /**
     * @brief Place a sell order at a station
     * @return order_id or empty string on failure
     */
    std::string placeSellOrder(const std::string& station_id,
                                const std::string& seller_id,
                                const std::string& item_id,
                                const std::string& item_name,
                                int quantity,
                                double price_per_unit);

    /**
     * @brief Place a buy order at a station
     * @return order_id or empty string on failure
     */
    std::string placeBuyOrder(const std::string& station_id,
                               const std::string& buyer_id,
                               const std::string& item_id,
                               const std::string& item_name,
                               int quantity,
                               double price_per_unit);

    /**
     * @brief Buy directly from the lowest-priced sell order
     * @return quantity actually bought
     */
    int buyFromMarket(const std::string& station_id,
                      const std::string& buyer_id,
                      const std::string& item_id,
                      int quantity);

    /**
     * @brief Get lowest sell price for an item at a station
     * @return lowest price, or -1 if none available
     */
    double getLowestSellPrice(const std::string& station_id,
                               const std::string& item_id);

    /**
     * @brief Get highest buy price for an item at a station
     * @return highest price, or -1 if none available
     */
    double getHighestBuyPrice(const std::string& station_id,
                                const std::string& item_id);

    /**
     * @brief Get number of active orders at a station
     */
    int getOrderCount(const std::string& station_id);

private:
    int order_counter_ = 0;
};

} // namespace systems
} // namespace atlas

#endif // EVE_SYSTEMS_MARKET_SYSTEM_H
