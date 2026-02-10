#include "systems/market_system.h"
#include "ecs/world.h"
#include "components/game_components.h"
#include <algorithm>
#include <limits>

namespace eve {
namespace systems {

MarketSystem::MarketSystem(ecs::World* world)
    : System(world) {
}

void MarketSystem::update(float delta_time) {
    // Tick order durations and expire/clean up
    auto entities = world_->getEntities<components::MarketHub>();
    for (auto* entity : entities) {
        auto* hub = entity->getComponent<components::MarketHub>();
        if (!hub) continue;

        for (auto& order : hub->orders) {
            if (order.fulfilled) continue;
            if (order.duration_remaining < 0.0f) continue; // permanent
            order.duration_remaining -= delta_time;
            if (order.duration_remaining <= 0.0f) {
                order.fulfilled = true;
            }
        }

        // Remove fulfilled orders
        hub->orders.erase(
            std::remove_if(hub->orders.begin(), hub->orders.end(),
                [](const components::MarketHub::Order& o) { return o.fulfilled; }),
            hub->orders.end());
    }
}

std::string MarketSystem::placeSellOrder(const std::string& station_id,
                                          const std::string& seller_id,
                                          const std::string& item_id,
                                          const std::string& item_name,
                                          int quantity,
                                          double price_per_unit) {
    auto* station = world_->getEntity(station_id);
    if (!station) return "";

    auto* hub = station->getComponent<components::MarketHub>();
    if (!hub) return "";

    auto* seller = world_->getEntity(seller_id);
    if (!seller) return "";

    auto* player = seller->getComponent<components::Player>();
    if (!player) return "";

    // Deduct broker fee
    double broker_fee = price_per_unit * quantity * hub->broker_fee_rate;
    if (player->isk < broker_fee) return "";
    player->isk -= broker_fee;

    // Create order
    components::MarketHub::Order order;
    order.order_id = "order_" + std::to_string(++order_counter_);
    order.item_id = item_id;
    order.item_name = item_name;
    order.owner_id = seller_id;
    order.is_buy_order = false;
    order.price_per_unit = price_per_unit;
    order.quantity = quantity;
    order.quantity_remaining = quantity;
    hub->orders.push_back(order);

    return order.order_id;
}

std::string MarketSystem::placeBuyOrder(const std::string& station_id,
                                         const std::string& buyer_id,
                                         const std::string& item_id,
                                         const std::string& item_name,
                                         int quantity,
                                         double price_per_unit) {
    auto* station = world_->getEntity(station_id);
    if (!station) return "";

    auto* hub = station->getComponent<components::MarketHub>();
    if (!hub) return "";

    auto* buyer = world_->getEntity(buyer_id);
    if (!buyer) return "";

    auto* player = buyer->getComponent<components::Player>();
    if (!player) return "";

    // Escrow total cost + broker fee
    double total_cost = price_per_unit * quantity;
    double broker_fee = total_cost * hub->broker_fee_rate;
    double escrow = total_cost + broker_fee;
    if (player->isk < escrow) return "";
    player->isk -= escrow;

    // Create order
    components::MarketHub::Order order;
    order.order_id = "order_" + std::to_string(++order_counter_);
    order.item_id = item_id;
    order.item_name = item_name;
    order.owner_id = buyer_id;
    order.is_buy_order = true;
    order.price_per_unit = price_per_unit;
    order.quantity = quantity;
    order.quantity_remaining = quantity;
    hub->orders.push_back(order);

    return order.order_id;
}

int MarketSystem::buyFromMarket(const std::string& station_id,
                                 const std::string& buyer_id,
                                 const std::string& item_id,
                                 int quantity) {
    auto* station = world_->getEntity(station_id);
    if (!station) return 0;

    auto* hub = station->getComponent<components::MarketHub>();
    if (!hub) return 0;

    auto* buyer = world_->getEntity(buyer_id);
    if (!buyer) return 0;

    auto* buyer_player = buyer->getComponent<components::Player>();
    if (!buyer_player) return 0;

    int total_bought = 0;
    int remaining = quantity;

    while (remaining > 0) {
        // Find cheapest sell order for this item
        components::MarketHub::Order* best = nullptr;
        for (auto& order : hub->orders) {
            if (order.fulfilled || order.is_buy_order) continue;
            if (order.item_id != item_id) continue;
            if (order.quantity_remaining <= 0) continue;
            if (!best || order.price_per_unit < best->price_per_unit) {
                best = &order;
            }
        }
        if (!best) break;

        int can_buy = std::min(remaining, best->quantity_remaining);
        double cost = best->price_per_unit * can_buy;
        double tax = cost * hub->sales_tax_rate;
        double total = cost + tax;

        if (buyer_player->isk < total) break;

        buyer_player->isk -= total;

        // Pay seller
        auto* seller = world_->getEntity(best->owner_id);
        if (seller) {
            auto* seller_player = seller->getComponent<components::Player>();
            if (seller_player) {
                seller_player->isk += cost;
            }
        }

        best->quantity_remaining -= can_buy;
        if (best->quantity_remaining <= 0) {
            best->fulfilled = true;
        }

        total_bought += can_buy;
        remaining -= can_buy;
    }

    return total_bought;
}

double MarketSystem::getLowestSellPrice(const std::string& station_id,
                                         const std::string& item_id) {
    auto* station = world_->getEntity(station_id);
    if (!station) return -1.0;

    auto* hub = station->getComponent<components::MarketHub>();
    if (!hub) return -1.0;

    double lowest = -1.0;
    for (const auto& order : hub->orders) {
        if (order.fulfilled || order.is_buy_order) continue;
        if (order.item_id != item_id) continue;
        if (order.quantity_remaining <= 0) continue;
        if (lowest < 0.0 || order.price_per_unit < lowest) {
            lowest = order.price_per_unit;
        }
    }
    return lowest;
}

double MarketSystem::getHighestBuyPrice(const std::string& station_id,
                                         const std::string& item_id) {
    auto* station = world_->getEntity(station_id);
    if (!station) return -1.0;

    auto* hub = station->getComponent<components::MarketHub>();
    if (!hub) return -1.0;

    double highest = -1.0;
    for (const auto& order : hub->orders) {
        if (order.fulfilled || !order.is_buy_order) continue;
        if (order.item_id != item_id) continue;
        if (order.quantity_remaining <= 0) continue;
        if (highest < 0.0 || order.price_per_unit > highest) {
            highest = order.price_per_unit;
        }
    }
    return highest;
}

int MarketSystem::getOrderCount(const std::string& station_id) {
    auto* station = world_->getEntity(station_id);
    if (!station) return 0;

    auto* hub = station->getComponent<components::MarketHub>();
    if (!hub) return 0;

    int count = 0;
    for (const auto& order : hub->orders) {
        if (!order.fulfilled && order.quantity_remaining > 0) {
            count++;
        }
    }
    return count;
}

} // namespace systems
} // namespace eve
