"""
Test market and trading system
Tests buy/sell orders, instant transactions, wallet management, and market mechanics
"""
import sys
import os

# Add parent directory to path
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from engine.core.ecs import World, Entity
from engine.systems.market_system import (
    MarketSystem, MarketOrder, Wallet, MarketAccess, OrderType
)
from engine.systems.industry_system import Inventory


def test_wallet_basic():
    """Test wallet creation and basic operations"""
    print("\n=== Testing Wallet Basic Operations ===")
    
    wallet = Wallet(isk=10000000.0)
    
    # Test initial ISK
    assert wallet.isk == 10000000.0, "Wallet should start with 10M ISK"
    print(f"✓ Wallet created with {wallet.isk:,.0f} ISK")
    
    # Test can_afford
    assert wallet.can_afford(5000000.0), "Should afford 5M ISK"
    assert not wallet.can_afford(15000000.0), "Should not afford 15M ISK"
    print("✓ can_afford() works correctly")
    
    # Test deposit
    wallet.deposit(1000000.0)
    assert wallet.isk == 11000000.0, "Should deposit ISK"
    print(f"✓ Deposit works: {wallet.isk:,.0f} ISK")
    
    # Test withdraw success
    success = wallet.withdraw(1000000.0)
    assert success, "Should withdraw successfully"
    assert wallet.isk == 10000000.0, "ISK should decrease"
    print(f"✓ Withdraw success: {wallet.isk:,.0f} ISK")
    
    # Test withdraw failure
    success = wallet.withdraw(15000000.0)
    assert not success, "Should fail to withdraw more than available"
    assert wallet.isk == 10000000.0, "ISK should not change on failed withdrawal"
    print("✓ Withdraw failure handled correctly")


def test_market_access():
    """Test market access component"""
    print("\n=== Testing Market Access Component ===")
    
    market_access = MarketAccess(location_id="jita_4_4")
    
    # Test initial state
    assert market_access.location_id == "jita_4_4", "Should set location"
    assert market_access.active_orders == [], "Should start with no orders"
    print("✓ Market access initialized correctly")
    
    # Test adding orders
    market_access.active_orders.append("order_1")
    market_access.active_orders.append("order_2")
    assert len(market_access.active_orders) == 2, "Should track orders"
    print(f"✓ Tracking {len(market_access.active_orders)} active orders")


def test_market_order_creation():
    """Test market order creation"""
    print("\n=== Testing Market Order Creation ===")
    
    order = MarketOrder(
        order_id="order_1",
        item_id="tritanium",
        order_type=OrderType.BUY,
        price=100.0,
        quantity=1000,
        remaining=1000,
        location_id="jita_4_4"
    )
    
    assert order.order_id == "order_1", "Should set order ID"
    assert order.item_id == "tritanium", "Should set item ID"
    assert order.order_type == OrderType.BUY, "Should set order type"
    assert order.price == 100.0, "Should set price"
    assert order.quantity == 1000, "Should set quantity"
    assert order.remaining == 1000, "Should set remaining"
    print("✓ Market order created successfully")
    print(f"  Order: {order.quantity} units of {order.item_id} at {order.price} ISK")


def test_place_buy_order():
    """Test placing a buy order"""
    print("\n=== Testing Placing Buy Order ===")
    
    world = World()
    market = MarketSystem(world)
    
    # Create trader entity with wallet and market access
    trader = world.create_entity()
    wallet = Wallet(isk=10000000.0)
    market_access = MarketAccess(location_id="jita_4_4")
    inventory = Inventory()
    
    trader.add_component(wallet)
    trader.add_component(market_access)
    trader.add_component(inventory)
    
    # Place buy order for 1000 units at 100 ISK each
    # Cost: 1000 * 100 = 100,000 ISK
    # With 3% broker fee: 100,000 * 1.03 = 103,000 ISK
    order_id = market.place_order(
        entity=trader,
        item_id="tritanium",
        order_type=OrderType.BUY,
        price=100.0,
        quantity=1000,
        current_time=0.0
    )
    
    assert order_id is not None, "Should return order ID"
    assert order_id == "order_1", "Should be first order"
    assert wallet.isk == 10000000.0 - 103000.0, "Should escrow ISK with broker fee"
    assert order_id in market_access.active_orders, "Should track order"
    print(f"✓ Buy order placed: {order_id}")
    print(f"  1000 units at 100 ISK (total with fee: 103,000 ISK)")
    print(f"  Remaining ISK: {wallet.isk:,.0f}")


def test_place_sell_order():
    """Test placing a sell order"""
    print("\n=== Testing Placing Sell Order ===")
    
    world = World()
    market = MarketSystem(world)
    
    # Create seller with items
    seller = world.create_entity()
    wallet = Wallet(isk=5000000.0)
    market_access = MarketAccess(location_id="jita_4_4")
    inventory = Inventory()
    inventory.items["tritanium"] = 5000
    
    seller.add_component(wallet)
    seller.add_component(market_access)
    seller.add_component(inventory)
    
    # Place sell order for 1000 units at 120 ISK each
    order_id = market.place_order(
        entity=seller,
        item_id="tritanium",
        order_type=OrderType.SELL,
        price=120.0,
        quantity=1000,
        current_time=0.0
    )
    
    assert order_id is not None, "Should return order ID"
    assert order_id == "order_1", "Should be first order"
    # Items should be escrowed (removed from inventory)
    assert inventory.items.get("tritanium", 0) == 4000, "Should escrow items"
    assert order_id in market_access.active_orders, "Should track order"
    print(f"✓ Sell order placed: {order_id}")
    print(f"  1000 units at 120 ISK")
    print(f"  Items remaining in inventory: {inventory.items.get('tritanium', 0)}")


def test_place_sell_order_insufficient_items():
    """Test placing sell order with insufficient items"""
    print("\n=== Testing Sell Order with Insufficient Items ===")
    
    world = World()
    market = MarketSystem(world)
    
    seller = world.create_entity()
    wallet = Wallet()
    market_access = MarketAccess()
    inventory = Inventory()
    inventory.items["tritanium"] = 500  # Only have 500, trying to sell 1000
    
    seller.add_component(wallet)
    seller.add_component(market_access)
    seller.add_component(inventory)
    
    # Try to sell more than available
    order_id = market.place_order(
        entity=seller,
        item_id="tritanium",
        order_type=OrderType.SELL,
        price=120.0,
        quantity=1000,
        current_time=0.0
    )
    
    assert order_id is None, "Should fail with insufficient items"
    assert inventory.items["tritanium"] == 500, "Inventory should not change"
    assert len(market_access.active_orders) == 0, "Should not create order"
    print("✓ Sell order correctly rejected with insufficient items")


def test_place_buy_order_insufficient_isk():
    """Test placing buy order with insufficient ISK"""
    print("\n=== Testing Buy Order with Insufficient ISK ===")
    
    world = World()
    market = MarketSystem(world)
    
    trader = world.create_entity()
    wallet = Wallet(isk=100000.0)  # Only 100K ISK
    market_access = MarketAccess()
    inventory = Inventory()
    
    trader.add_component(wallet)
    trader.add_component(market_access)
    trader.add_component(inventory)
    
    # Try to buy 1000 units at 100 ISK (100K + 3% fee = 103K needed)
    order_id = market.place_order(
        entity=trader,
        item_id="tritanium",
        order_type=OrderType.BUY,
        price=100.0,
        quantity=1000,
        current_time=0.0
    )
    
    assert order_id is None, "Should fail with insufficient ISK"
    assert wallet.isk == 100000.0, "ISK should not change"
    assert len(market_access.active_orders) == 0, "Should not create order"
    print("✓ Buy order correctly rejected with insufficient ISK")


def test_instant_buy_single_order():
    """Test instant buying from a single sell order"""
    print("\n=== Testing Instant Buy (Single Order) ===")
    
    world = World()
    market = MarketSystem(world)
    
    # Create seller
    seller = world.create_entity()
    seller_wallet = Wallet(isk=1000000.0)
    seller_market = MarketAccess(location_id="jita_4_4")
    seller_inventory = Inventory()
    seller_inventory.items["tritanium"] = 5000
    
    seller.add_component(seller_wallet)
    seller.add_component(seller_market)
    seller.add_component(seller_inventory)
    
    # Seller places sell order for 1000 units at 100 ISK
    sell_order_id = market.place_order(
        entity=seller,
        item_id="tritanium",
        order_type=OrderType.SELL,
        price=100.0,
        quantity=1000,
        current_time=0.0
    )
    
    initial_seller_isk = seller_wallet.isk
    print(f"✓ Seller placed order: {sell_order_id}")
    
    # Create buyer
    buyer = world.create_entity()
    buyer_wallet = Wallet(isk=10000000.0)
    buyer_market = MarketAccess(location_id="jita_4_4")
    buyer_inventory = Inventory()
    
    buyer.add_component(buyer_wallet)
    buyer.add_component(buyer_market)
    buyer.add_component(buyer_inventory)
    
    initial_buyer_isk = buyer_wallet.isk
    
    # Buyer does instant buy of 500 units
    success = market.instant_buy(
        entity=buyer,
        item_id="tritanium",
        quantity=500
    )
    
    assert success, "Instant buy should succeed"
    
    # Check buyer
    assert buyer_inventory.items.get("tritanium", 0) == 500, "Should receive 500 units"
    # Cost: 500 * 100 = 50,000 ISK + 2% tax = 51,000 ISK
    expected_buyer_cost = 500 * 100 * 1.02
    assert buyer_wallet.isk == initial_buyer_isk - expected_buyer_cost, \
        f"Should deduct {expected_buyer_cost} ISK (buyer has {buyer_wallet.isk}, started with {initial_buyer_isk})"
    print(f"✓ Buyer received 500 units")
    print(f"  Cost: {expected_buyer_cost:,.0f} ISK (including 2% tax)")
    
    # Check seller - items were already escrowed (removed) when sell order was placed
    # So seller inventory should be 4000 (5000 - 1000 escrowed)
    assert seller_inventory.items.get("tritanium", 0) == 4000, "Seller should have 4000 left (1000 still escrowed)"
    # Seller receives 50,000 ISK (no tax on seller side in our implementation)
    expected_seller_revenue = 500 * 100
    assert seller_wallet.isk == initial_seller_isk + expected_seller_revenue, \
        f"Seller should gain {expected_seller_revenue} ISK"
    print(f"✓ Seller received {expected_seller_revenue:,.0f} ISK")
    
    # Check order is partially filled
    orders = market.order_book["jita_4_4"]["tritanium"]["sell"]
    assert len(orders) == 1, "Should still have sell order"
    assert orders[0].remaining == 500, "Order should have 500 remaining"
    print(f"✓ Order partially filled: {orders[0].remaining} remaining")


def test_instant_buy_multiple_orders():
    """Test instant buying from multiple sell orders"""
    print("\n=== Testing Instant Buy (Multiple Orders) ===")
    
    world = World()
    market = MarketSystem(world)
    
    # Create seller 1 with cheap sell order
    seller1 = world.create_entity()
    seller1_wallet = Wallet(isk=1000000.0)
    seller1_market = MarketAccess(location_id="jita_4_4")
    seller1_inventory = Inventory()
    seller1_inventory.items["tritanium"] = 5000
    
    seller1.add_component(seller1_wallet)
    seller1.add_component(seller1_market)
    seller1.add_component(seller1_inventory)
    
    # Seller 1 places sell order for 500 units at 90 ISK
    market.place_order(
        entity=seller1,
        item_id="tritanium",
        order_type=OrderType.SELL,
        price=90.0,
        quantity=500,
        current_time=0.0
    )
    
    # Create seller 2 with expensive sell order
    seller2 = world.create_entity()
    seller2_wallet = Wallet(isk=1000000.0)
    seller2_market = MarketAccess(location_id="jita_4_4")
    seller2_inventory = Inventory()
    seller2_inventory.items["tritanium"] = 5000
    
    seller2.add_component(seller2_wallet)
    seller2.add_component(seller2_market)
    seller2.add_component(seller2_inventory)
    
    # Seller 2 places sell order for 500 units at 100 ISK
    market.place_order(
        entity=seller2,
        item_id="tritanium",
        order_type=OrderType.SELL,
        price=100.0,
        quantity=500,
        current_time=0.0
    )
    
    # Create buyer
    buyer = world.create_entity()
    buyer_wallet = Wallet(isk=10000000.0)
    buyer_market = MarketAccess(location_id="jita_4_4")
    buyer_inventory = Inventory()
    
    buyer.add_component(buyer_wallet)
    buyer.add_component(buyer_market)
    buyer.add_component(buyer_inventory)
    
    initial_buyer_isk = buyer_wallet.isk
    
    # Buyer does instant buy of 800 units (should get 500 at 90, 300 at 100)
    success = market.instant_buy(
        entity=buyer,
        item_id="tritanium",
        quantity=800
    )
    
    assert success, "Instant buy should succeed"
    assert buyer_inventory.items.get("tritanium", 0) == 800, "Should receive 800 units"
    
    # Cost: (500 * 90) + (300 * 100) = 45,000 + 30,000 = 75,000 + 2% = 76,500 ISK
    expected_cost = (500 * 90 + 300 * 100) * 1.02
    assert buyer_wallet.isk == initial_buyer_isk - expected_cost, \
        f"Should deduct {expected_cost} ISK"
    
    print(f"✓ Instant buy from multiple orders succeeded")
    print(f"  Bought 500 units at 90 ISK + 300 units at 100 ISK")
    print(f"  Total cost: {expected_cost:,.0f} ISK (including 2% tax)")


def test_instant_buy_max_price():
    """Test instant buy respects max price"""
    print("\n=== Testing Instant Buy with Max Price ===")
    
    world = World()
    market = MarketSystem(world)
    
    # Create seller with expensive order
    seller = world.create_entity()
    seller_wallet = Wallet(isk=1000000.0)
    seller_market = MarketAccess(location_id="jita_4_4")
    seller_inventory = Inventory()
    seller_inventory.items["tritanium"] = 5000
    
    seller.add_component(seller_wallet)
    seller.add_component(seller_market)
    seller.add_component(seller_inventory)
    
    # Seller places sell order at 110 ISK
    market.place_order(
        entity=seller,
        item_id="tritanium",
        order_type=OrderType.SELL,
        price=110.0,
        quantity=1000,
        current_time=0.0
    )
    
    # Create buyer
    buyer = world.create_entity()
    buyer_wallet = Wallet(isk=10000000.0)
    buyer_market = MarketAccess(location_id="jita_4_4")
    buyer_inventory = Inventory()
    
    buyer.add_component(buyer_wallet)
    buyer.add_component(buyer_market)
    buyer.add_component(buyer_inventory)
    
    # Try to buy with max price of 100 ISK (too low)
    success = market.instant_buy(
        entity=buyer,
        item_id="tritanium",
        quantity=500,
        max_price=100.0
    )
    
    assert not success, "Should fail when all orders exceed max price"
    assert buyer_inventory.items.get("tritanium", 0) == 0, "Should not receive items"
    print("✓ Instant buy correctly rejected when price exceeds max")


def test_instant_sell_single_order():
    """Test instant selling to a single buy order"""
    print("\n=== Testing Instant Sell (Single Order) ===")
    
    world = World()
    market = MarketSystem(world)
    
    # Create buyer
    buyer = world.create_entity()
    buyer_wallet = Wallet(isk=10000000.0)
    buyer_market = MarketAccess(location_id="jita_4_4")
    buyer_inventory = Inventory()
    
    buyer.add_component(buyer_wallet)
    buyer.add_component(buyer_market)
    buyer.add_component(buyer_inventory)
    
    # Buyer places buy order for 1000 units at 100 ISK
    buy_order_id = market.place_order(
        entity=buyer,
        item_id="tritanium",
        order_type=OrderType.BUY,
        price=100.0,
        quantity=1000,
        current_time=0.0
    )
    
    initial_buyer_wallet = buyer_wallet.isk
    print(f"✓ Buyer placed order: {buy_order_id}")
    
    # Create seller
    seller = world.create_entity()
    seller_wallet = Wallet(isk=1000000.0)
    seller_market = MarketAccess(location_id="jita_4_4")
    seller_inventory = Inventory()
    seller_inventory.items["tritanium"] = 5000
    
    seller.add_component(seller_wallet)
    seller.add_component(seller_market)
    seller.add_component(seller_inventory)
    
    initial_seller_isk = seller_wallet.isk
    
    # Seller does instant sell of 500 units
    success = market.instant_sell(
        entity=seller,
        item_id="tritanium",
        quantity=500
    )
    
    assert success, "Instant sell should succeed"
    
    # Check seller
    assert seller_inventory.items.get("tritanium", 0) == 4500, "Should have 4500 left"
    # Revenue: 500 * 100 - 2% tax = 49,000 ISK
    expected_revenue = 500 * 100 * 0.98
    assert seller_wallet.isk == initial_seller_isk + expected_revenue, \
        f"Seller should gain {expected_revenue} ISK"
    print(f"✓ Seller received {expected_revenue:,.0f} ISK (after 2% tax)")
    
    # Check buyer
    assert buyer_inventory.items.get("tritanium", 0) == 500, "Buyer should receive 500 units"
    print(f"✓ Buyer received 500 units")
    
    # Check order is partially filled
    orders = market.order_book["jita_4_4"]["tritanium"]["buy"]
    assert len(orders) == 1, "Should still have buy order"
    assert orders[0].remaining == 500, "Order should have 500 remaining"
    print(f"✓ Buy order partially filled: {orders[0].remaining} remaining")


def test_instant_sell_min_price():
    """Test instant sell respects min price"""
    print("\n=== Testing Instant Sell with Min Price ===")
    
    world = World()
    market = MarketSystem(world)
    
    # Create buyer with low offer
    buyer = world.create_entity()
    buyer_wallet = Wallet(isk=10000000.0)
    buyer_market = MarketAccess(location_id="jita_4_4")
    buyer_inventory = Inventory()
    
    buyer.add_component(buyer_wallet)
    buyer.add_component(buyer_market)
    buyer.add_component(buyer_inventory)
    
    # Buyer places buy order at 80 ISK
    market.place_order(
        entity=buyer,
        item_id="tritanium",
        order_type=OrderType.BUY,
        price=80.0,
        quantity=1000,
        current_time=0.0
    )
    
    # Create seller
    seller = world.create_entity()
    seller_wallet = Wallet(isk=1000000.0)
    seller_market = MarketAccess(location_id="jita_4_4")
    seller_inventory = Inventory()
    seller_inventory.items["tritanium"] = 5000
    
    seller.add_component(seller_wallet)
    seller.add_component(seller_market)
    seller.add_component(seller_inventory)
    
    # Try to sell with min price of 100 ISK (too high)
    success = market.instant_sell(
        entity=seller,
        item_id="tritanium",
        quantity=500,
        min_price=100.0
    )
    
    assert not success, "Should fail when all orders are below min price"
    assert seller_inventory.items.get("tritanium", 0) == 5000, "Items should not be sold"
    print("✓ Instant sell correctly rejected when price below minimum")


def test_cancel_buy_order():
    """Test cancelling a buy order"""
    print("\n=== Testing Cancel Buy Order ===")
    
    world = World()
    market = MarketSystem(world)
    
    # Create trader
    trader = world.create_entity()
    wallet = Wallet(isk=10000000.0)
    market_access = MarketAccess(location_id="jita_4_4")
    inventory = Inventory()
    
    trader.add_component(wallet)
    trader.add_component(market_access)
    trader.add_component(inventory)
    
    initial_isk = wallet.isk
    
    # Place buy order
    order_id = market.place_order(
        entity=trader,
        item_id="tritanium",
        order_type=OrderType.BUY,
        price=100.0,
        quantity=1000,
        current_time=0.0
    )
    
    # ISK should be escrowed
    assert wallet.isk < initial_isk, "ISK should be escrowed"
    escrowed_isk = initial_isk - wallet.isk
    print(f"✓ Buy order placed, {escrowed_isk:,.0f} ISK escrowed")
    
    # Cancel order
    success = market.cancel_order(trader, order_id)
    assert success, "Should cancel order"
    
    # ISK should be refunded
    assert wallet.isk == initial_isk, "ISK should be refunded"
    assert order_id not in market_access.active_orders, "Should remove from active orders"
    print(f"✓ Order cancelled, ISK refunded: {wallet.isk:,.0f} ISK")


def test_cancel_sell_order():
    """Test cancelling a sell order"""
    print("\n=== Testing Cancel Sell Order ===")
    
    world = World()
    market = MarketSystem(world)
    
    # Create seller
    seller = world.create_entity()
    wallet = Wallet(isk=1000000.0)
    market_access = MarketAccess(location_id="jita_4_4")
    inventory = Inventory()
    inventory.items["tritanium"] = 5000
    
    seller.add_component(wallet)
    seller.add_component(market_access)
    seller.add_component(inventory)
    
    # Place sell order
    order_id = market.place_order(
        entity=seller,
        item_id="tritanium",
        order_type=OrderType.SELL,
        price=100.0,
        quantity=1000,
        current_time=0.0
    )
    
    # Items should be escrowed
    assert inventory.items["tritanium"] == 4000, "Items should be escrowed"
    print(f"✓ Sell order placed, 1000 items escrowed")
    
    # Cancel order
    success = market.cancel_order(seller, order_id)
    assert success, "Should cancel order"
    
    # Items should be returned
    assert inventory.items["tritanium"] == 5000, "Items should be returned"
    assert order_id not in market_access.active_orders, "Should remove from active orders"
    print(f"✓ Order cancelled, items returned: {inventory.items['tritanium']} total")


def test_market_price_with_orders():
    """Test getting market prices"""
    print("\n=== Testing Market Price Calculation ===")
    
    world = World()
    market = MarketSystem(world)
    
    # Create sellers
    seller1 = world.create_entity()
    seller1_wallet = Wallet()
    seller1_market = MarketAccess(location_id="jita_4_4")
    seller1_inventory = Inventory()
    seller1_inventory.items["tritanium"] = 10000
    seller1.add_component(seller1_wallet)
    seller1.add_component(seller1_market)
    seller1.add_component(seller1_inventory)
    
    seller2 = world.create_entity()
    seller2_wallet = Wallet()
    seller2_market = MarketAccess(location_id="jita_4_4")
    seller2_inventory = Inventory()
    seller2_inventory.items["tritanium"] = 10000
    seller2.add_component(seller2_wallet)
    seller2.add_component(seller2_market)
    seller2.add_component(seller2_inventory)
    
    # Create buyers
    buyer1 = world.create_entity()
    buyer1_wallet = Wallet(isk=10000000.0)
    buyer1_market = MarketAccess(location_id="jita_4_4")
    buyer1_inventory = Inventory()
    buyer1.add_component(buyer1_wallet)
    buyer1.add_component(buyer1_market)
    buyer1.add_component(buyer1_inventory)
    
    buyer2 = world.create_entity()
    buyer2_wallet = Wallet(isk=10000000.0)
    buyer2_market = MarketAccess(location_id="jita_4_4")
    buyer2_inventory = Inventory()
    buyer2.add_component(buyer2_wallet)
    buyer2.add_component(buyer2_market)
    buyer2.add_component(buyer2_inventory)
    
    # Place orders
    market.place_order(seller1, "tritanium", OrderType.SELL, 95.0, 1000)  # Lowest sell
    market.place_order(seller2, "tritanium", OrderType.SELL, 105.0, 1000)  # Higher sell
    market.place_order(buyer1, "tritanium", OrderType.BUY, 90.0, 1000)   # Highest buy
    market.place_order(buyer2, "tritanium", OrderType.BUY, 85.0, 1000)   # Lower buy
    
    # Get prices
    prices = market.get_market_price("tritanium", "jita_4_4")
    
    assert prices['sell'] == 95.0, "Lowest sell should be 95"
    assert prices['buy'] == 90.0, "Highest buy should be 90"
    assert prices['average'] == (95.0 + 90.0) / 2, "Average should be midpoint"
    
    print(f"✓ Market prices retrieved:")
    print(f"  Highest buy: {prices['buy']} ISK")
    print(f"  Lowest sell: {prices['sell']} ISK")
    print(f"  Average: {prices['average']} ISK")


def test_market_price_no_orders():
    """Test market price with NPC base prices"""
    print("\n=== Testing Market Price with NPC Prices ===")
    
    world = World()
    market = MarketSystem(world)
    
    # Initialize NPC prices
    market.initialize_npc_prices({
        "tritanium": 100.0,
        "pyerite": 50.0,
    })
    
    # Get price for item with no orders
    prices = market.get_market_price("tritanium", "jita_4_4")
    
    assert prices['average'] == 100.0, "Average should be NPC price"
    assert abs(prices['buy'] - 90.0) < 0.01, "Buy should be 90% of NPC price"
    assert abs(prices['sell'] - 110.0) < 0.01, "Sell should be 110% of NPC price"
    
    print(f"✓ NPC prices applied:")
    print(f"  Base price: 100 ISK")
    print(f"  Buy: {prices['buy']:.1f} ISK (90%)")
    print(f"  Sell: {prices['sell']:.1f} ISK (110%)")


def test_transaction_history():
    """Test transaction history recording"""
    print("\n=== Testing Transaction History ===")
    
    world = World()
    market = MarketSystem(world)
    
    # Create seller
    seller = world.create_entity()
    seller_wallet = Wallet(isk=1000000.0)
    seller_market = MarketAccess(location_id="jita_4_4")
    seller_inventory = Inventory()
    seller_inventory.items["tritanium"] = 5000
    seller.add_component(seller_wallet)
    seller.add_component(seller_market)
    seller.add_component(seller_inventory)
    
    # Place sell order
    market.place_order(
        entity=seller,
        item_id="tritanium",
        order_type=OrderType.SELL,
        price=100.0,
        quantity=1000,
        current_time=0.0
    )
    
    # Create buyer
    buyer = world.create_entity()
    buyer_wallet = Wallet(isk=10000000.0)
    buyer_market = MarketAccess(location_id="jita_4_4")
    buyer_inventory = Inventory()
    buyer.add_component(buyer_wallet)
    buyer.add_component(buyer_market)
    buyer.add_component(buyer_inventory)
    
    # Instant buy
    market.instant_buy(buyer, "tritanium", 500)
    
    # Check transaction history
    assert len(market.transaction_history) > 0, "Should record transactions"
    transaction = market.transaction_history[0]
    assert transaction['item_id'] == "tritanium", "Should record item"
    assert transaction['quantity'] == 500, "Should record quantity"
    assert transaction['price'] == 100.0, "Should record price"
    assert transaction['buyer'] == buyer, "Should record buyer"
    assert transaction['seller'] == seller, "Should record seller"
    
    print(f"✓ Transaction recorded:")
    print(f"  Item: {transaction['item_id']}")
    print(f"  Quantity: {transaction['quantity']}")
    print(f"  Price: {transaction['price']} ISK per unit")


def test_order_sorting():
    """Test that orders are sorted correctly"""
    print("\n=== Testing Order Sorting ===")
    
    world = World()
    market = MarketSystem(world)
    
    # Create entities with items
    sellers = []
    for i in range(3):
        seller = world.create_entity()
        seller_wallet = Wallet()
        seller_market = MarketAccess(location_id="jita_4_4")
        seller_inventory = Inventory()
        seller_inventory.items["tritanium"] = 10000
        seller.add_component(seller_wallet)
        seller.add_component(seller_market)
        seller.add_component(seller_inventory)
        sellers.append(seller)
    
    buyers = []
    for i in range(3):
        buyer = world.create_entity()
        buyer_wallet = Wallet(isk=100000000.0)
        buyer_market = MarketAccess(location_id="jita_4_4")
        buyer_inventory = Inventory()
        buyer.add_component(buyer_wallet)
        buyer.add_component(buyer_market)
        buyer.add_component(buyer_inventory)
        buyers.append(buyer)
    
    # Place sell orders in random order: 105, 95, 100
    market.place_order(sellers[0], "tritanium", OrderType.SELL, 105.0, 1000)
    market.place_order(sellers[1], "tritanium", OrderType.SELL, 95.0, 1000)
    market.place_order(sellers[2], "tritanium", OrderType.SELL, 100.0, 1000)
    
    # Place buy orders in random order: 85, 95, 90
    market.place_order(buyers[0], "tritanium", OrderType.BUY, 85.0, 1000)
    market.place_order(buyers[1], "tritanium", OrderType.BUY, 95.0, 1000)
    market.place_order(buyers[2], "tritanium", OrderType.BUY, 90.0, 1000)
    
    # Check sell orders are sorted by price (lowest first)
    sell_orders = market.order_book["jita_4_4"]["tritanium"]["sell"]
    assert sell_orders[0].price == 95.0, "First sell should be lowest (95)"
    assert sell_orders[1].price == 100.0, "Second sell should be 100"
    assert sell_orders[2].price == 105.0, "Third sell should be highest (105)"
    print("✓ Sell orders sorted by price (lowest first)")
    
    # Check buy orders are sorted by price (highest first)
    buy_orders = market.order_book["jita_4_4"]["tritanium"]["buy"]
    assert buy_orders[0].price == 95.0, "First buy should be highest (95)"
    assert buy_orders[1].price == 90.0, "Second buy should be 90"
    assert buy_orders[2].price == 85.0, "Third buy should be lowest (85)"
    print("✓ Buy orders sorted by price (highest first)")


def test_broker_fee_calculation():
    """Test broker fee calculation on buy orders"""
    print("\n=== Testing Broker Fee Calculation ===")
    
    world = World()
    market = MarketSystem(world)
    
    trader = world.create_entity()
    wallet = Wallet(isk=10000000.0)
    market_access = MarketAccess()
    inventory = Inventory()
    
    trader.add_component(wallet)
    trader.add_component(market_access)
    trader.add_component(inventory)
    
    initial_isk = wallet.isk
    
    # Place buy order for 1000 units at 100 ISK
    # Base cost: 100,000 ISK
    # Broker fee (3%): 3,000 ISK
    # Total: 103,000 ISK
    market.place_order(
        entity=trader,
        item_id="tritanium",
        order_type=OrderType.BUY,
        price=100.0,
        quantity=1000,
        current_time=0.0
    )
    
    expected_fee = 100000 * 0.03
    expected_total = 100000 + expected_fee
    actual_deducted = initial_isk - wallet.isk
    
    assert actual_deducted == expected_total, \
        f"Should deduct {expected_total} (100k + 3% fee), got {actual_deducted}"
    assert expected_fee == 3000, "Broker fee should be 3,000 ISK"
    
    print(f"✓ Broker fee correctly applied:")
    print(f"  Base cost: 100,000 ISK")
    print(f"  Broker fee (3%): {expected_fee:,.0f} ISK")
    print(f"  Total escrowed: {expected_total:,.0f} ISK")


def test_sales_tax_calculation():
    """Test sales tax on instant buy"""
    print("\n=== Testing Sales Tax Calculation ===")
    
    world = World()
    market = MarketSystem(world)
    
    # Create seller
    seller = world.create_entity()
    seller_wallet = Wallet()
    seller_market = MarketAccess(location_id="jita_4_4")
    seller_inventory = Inventory()
    seller_inventory.items["tritanium"] = 5000
    seller.add_component(seller_wallet)
    seller.add_component(seller_market)
    seller.add_component(seller_inventory)
    
    # Place sell order
    market.place_order(
        entity=seller,
        item_id="tritanium",
        order_type=OrderType.SELL,
        price=100.0,
        quantity=1000,
        current_time=0.0
    )
    
    # Create buyer
    buyer = world.create_entity()
    buyer_wallet = Wallet(isk=10000000.0)
    buyer_market = MarketAccess(location_id="jita_4_4")
    buyer_inventory = Inventory()
    buyer.add_component(buyer_wallet)
    buyer.add_component(buyer_market)
    buyer.add_component(buyer_inventory)
    
    initial_buyer_isk = buyer_wallet.isk
    
    # Instant buy 1000 units at 100 ISK
    # Base cost: 100,000 ISK
    # Sales tax (2%): 2,000 ISK
    # Total: 102,000 ISK
    market.instant_buy(
        entity=buyer,
        item_id="tritanium",
        quantity=1000
    )
    
    expected_tax = 100000 * 0.02
    expected_total = 100000 + expected_tax
    actual_deducted = initial_buyer_isk - buyer_wallet.isk
    
    assert actual_deducted == expected_total, \
        f"Should deduct {expected_total} (100k + 2% tax), got {actual_deducted}"
    assert expected_tax == 2000, "Sales tax should be 2,000 ISK"
    
    print(f"✓ Sales tax correctly applied (on buy):")
    print(f"  Base cost: 100,000 ISK")
    print(f"  Sales tax (2%): {expected_tax:,.0f} ISK")
    print(f"  Total paid: {expected_total:,.0f} ISK")


def test_multiple_items_in_market():
    """Test trading multiple different items"""
    print("\n=== Testing Multiple Items in Market ===")
    
    world = World()
    market = MarketSystem(world)
    
    # Create trader
    trader = world.create_entity()
    wallet = Wallet(isk=100000000.0)
    market_access = MarketAccess(location_id="jita_4_4")
    inventory = Inventory()
    
    trader.add_component(wallet)
    trader.add_component(market_access)
    trader.add_component(inventory)
    
    # Place orders for different items
    market.place_order(
        entity=trader,
        item_id="tritanium",
        order_type=OrderType.BUY,
        price=100.0,
        quantity=1000,
        current_time=0.0
    )
    
    market.place_order(
        entity=trader,
        item_id="pyerite",
        order_type=OrderType.BUY,
        price=50.0,
        quantity=2000,
        current_time=0.0
    )
    
    market.place_order(
        entity=trader,
        item_id="mexallon",
        order_type=OrderType.BUY,
        price=75.0,
        quantity=1500,
        current_time=0.0
    )
    
    # Check orders are separated
    assert "tritanium" in market.order_book["jita_4_4"], "Should have tritanium"
    assert "pyerite" in market.order_book["jita_4_4"], "Should have pyerite"
    assert "mexallon" in market.order_book["jita_4_4"], "Should have mexallon"
    
    assert len(market_access.active_orders) == 3, "Should track 3 orders"
    
    print(f"✓ Multiple items tracked separately:")
    print(f"  Active orders: {len(market_access.active_orders)}")
    print(f"  Items: tritanium, pyerite, mexallon")


def test_instant_buy_insufficient_funds():
    """Test instant buy fails when buyer lacks funds"""
    print("\n=== Testing Instant Buy (Insufficient Funds) ===")
    
    world = World()
    market = MarketSystem(world)
    
    # Create seller
    seller = world.create_entity()
    seller_wallet = Wallet()
    seller_market = MarketAccess(location_id="jita_4_4")
    seller_inventory = Inventory()
    seller_inventory.items["tritanium"] = 5000
    seller.add_component(seller_wallet)
    seller.add_component(seller_market)
    seller.add_component(seller_inventory)
    
    # Place sell order
    market.place_order(
        entity=seller,
        item_id="tritanium",
        order_type=OrderType.SELL,
        price=100.0,
        quantity=1000,
        current_time=0.0
    )
    
    # Create buyer with insufficient funds
    buyer = world.create_entity()
    buyer_wallet = Wallet(isk=40000.0)  # Can't afford 1000 units at 100 + 2% tax
    buyer_market = MarketAccess(location_id="jita_4_4")
    buyer_inventory = Inventory()
    buyer.add_component(buyer_wallet)
    buyer.add_component(buyer_market)
    buyer.add_component(buyer_inventory)
    
    initial_inventory = len(buyer_inventory.items)
    
    # Try to buy
    success = market.instant_buy(
        entity=buyer,
        item_id="tritanium",
        quantity=1000
    )
    
    assert not success, "Should fail with insufficient funds"
    assert buyer_inventory.items.get("tritanium", 0) == 0, "Should not receive items"
    print("✓ Instant buy correctly rejected with insufficient funds")


def test_clean_up_empty_orders():
    """Test that empty orders are cleaned up"""
    print("\n=== Testing Empty Order Cleanup ===")
    
    world = World()
    market = MarketSystem(world)
    
    # Create seller
    seller = world.create_entity()
    seller_wallet = Wallet()
    seller_market = MarketAccess(location_id="jita_4_4")
    seller_inventory = Inventory()
    seller_inventory.items["tritanium"] = 5000
    seller.add_component(seller_wallet)
    seller.add_component(seller_market)
    seller.add_component(seller_inventory)
    
    # Place sell order for 100 units
    market.place_order(
        entity=seller,
        item_id="tritanium",
        order_type=OrderType.SELL,
        price=100.0,
        quantity=100,
        current_time=0.0
    )
    
    # Create buyer
    buyer = world.create_entity()
    buyer_wallet = Wallet(isk=100000000.0)
    buyer_market = MarketAccess(location_id="jita_4_4")
    buyer_inventory = Inventory()
    buyer.add_component(buyer_wallet)
    buyer.add_component(buyer_market)
    buyer.add_component(buyer_inventory)
    
    # Verify order exists
    assert len(market.order_book["jita_4_4"]["tritanium"]["sell"]) == 1
    
    # Buy all items (should complete the order)
    market.instant_buy(
        entity=buyer,
        item_id="tritanium",
        quantity=100
    )
    
    # Order should be removed after completion
    assert len(market.order_book["jita_4_4"]["tritanium"]["sell"]) == 0, \
        "Completed orders should be cleaned up"
    print("✓ Empty orders cleaned up after completion")


if __name__ == "__main__":
    print("=" * 60)
    print("Testing Market and Trading System")
    print("=" * 60)
    
    test_wallet_basic()
    test_market_access()
    test_market_order_creation()
    test_place_buy_order()
    test_place_sell_order()
    test_place_sell_order_insufficient_items()
    test_place_buy_order_insufficient_isk()
    test_instant_buy_single_order()
    test_instant_buy_multiple_orders()
    test_instant_buy_max_price()
    test_instant_sell_single_order()
    test_instant_sell_min_price()
    test_cancel_buy_order()
    test_cancel_sell_order()
    test_market_price_with_orders()
    test_market_price_no_orders()
    test_transaction_history()
    test_order_sorting()
    test_broker_fee_calculation()
    test_sales_tax_calculation()
    test_multiple_items_in_market()
    test_instant_buy_insufficient_funds()
    test_clean_up_empty_orders()
    
    print("\n" + "=" * 60)
    print("✅ ALL MARKET TESTS PASSED!")
    print("=" * 60)
