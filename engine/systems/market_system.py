"""
Market and Economy System
Handles buy/sell orders, trading, and market mechanics
Inspired by EVE Online's market system
"""

from typing import Optional, Dict, List
from dataclasses import dataclass, field
from enum import Enum
from engine.core.ecs import World, Entity


class OrderType(Enum):
    """Market order types"""
    BUY = "buy"
    SELL = "sell"


@dataclass
class MarketOrder:
    """Individual market order"""
    order_id: str
    item_id: str
    order_type: OrderType
    price: float  # ISK per unit
    quantity: int
    remaining: int
    min_volume: int = 1  # Minimum quantity per transaction
    location_id: str = "jita_4_4"  # Station/citadel ID
    owner_id: Optional[Entity] = None
    issued_time: float = 0.0
    duration_seconds: float = 86400.0  # 24 hours default
    
    def __post_init__(self):
        if self.remaining is None:
            self.remaining = self.quantity


@dataclass
class Wallet:
    """ISK wallet component"""
    isk: float = 10000000.0  # Start with 10M ISK
    
    def can_afford(self, amount: float) -> bool:
        """Check if wallet has enough ISK"""
        return self.isk >= amount
    
    def withdraw(self, amount: float) -> bool:
        """Remove ISK from wallet"""
        if self.can_afford(amount):
            self.isk -= amount
            return True
        return False
    
    def deposit(self, amount: float):
        """Add ISK to wallet"""
        self.isk += amount


@dataclass
class MarketAccess:
    """Component for entities that can access markets"""
    location_id: str = "jita_4_4"  # Current market location
    active_orders: List[str] = field(default_factory=list)  # Order IDs owned by this entity


class MarketSystem:
    """
    Market and trading system
    Based on EVE Online's regional market
    """
    
    def __init__(self, world: World):
        self.world = world
        self.order_counter = 0
        
        # Market order book: {location_id: {item_id: {'buy': [...], 'sell': [...]}}}
        self.order_book: Dict[str, Dict[str, Dict[str, List[MarketOrder]]]] = {}
        
        # NPC base prices for seeding market
        self.npc_base_prices = {}
        
        # Transaction history for price tracking
        self.transaction_history: List[Dict] = []
    
    def initialize_npc_prices(self, price_data: Dict[str, float]):
        """Initialize NPC base prices from data"""
        self.npc_base_prices = price_data.copy()
    
    def place_order(
        self,
        entity: Entity,
        item_id: str,
        order_type: OrderType,
        price: float,
        quantity: int,
        current_time: float = 0.0,
        duration: float = 86400.0
    ) -> Optional[str]:
        """
        Place a buy or sell order on the market
        
        Args:
            entity: Entity placing the order (must have Wallet and MarketAccess)
            item_id: Item being traded
            order_type: BUY or SELL
            price: Price per unit in ISK
            quantity: Total quantity
            current_time: Current game time
            duration: Order duration in seconds
            
        Returns:
            order_id if successful, None otherwise
        """
        wallet = entity.get_component(Wallet)
        market_access = entity.get_component(MarketAccess)
        
        from engine.systems.industry_system import Inventory
        inventory = entity.get_component(Inventory)
        
        if not all([wallet, market_access]):
            return None
        
        location = market_access.location_id
        
        if order_type == OrderType.SELL:
            # Must have items to sell
            if not inventory or inventory.items.get(item_id, 0) < quantity:
                return None
            
            # Remove items from inventory (escrowed)
            inventory.remove_item(item_id, quantity)
            
        elif order_type == OrderType.BUY:
            # Must have ISK to pay (escrow)
            total_cost = price * quantity
            broker_fee = total_cost * 0.03  # 3% broker fee
            total_with_fees = total_cost + broker_fee
            
            if not wallet.withdraw(total_with_fees):
                return None
        
        # Create order
        self.order_counter += 1
        order = MarketOrder(
            order_id=f"order_{self.order_counter}",
            item_id=item_id,
            order_type=order_type,
            price=price,
            quantity=quantity,
            remaining=quantity,
            location_id=location,
            owner_id=entity,
            issued_time=current_time,
            duration_seconds=duration
        )
        
        # Add to order book
        if location not in self.order_book:
            self.order_book[location] = {}
        if item_id not in self.order_book[location]:
            self.order_book[location][item_id] = {'buy': [], 'sell': []}
        
        order_list = self.order_book[location][item_id][order_type.value]
        order_list.append(order)
        
        # Sort orders (sell: lowest first, buy: highest first)
        if order_type == OrderType.SELL:
            order_list.sort(key=lambda o: o.price)
        else:
            order_list.sort(key=lambda o: -o.price)
        
        # Track order
        market_access.active_orders.append(order.order_id)
        
        return order.order_id
    
    def instant_buy(
        self,
        entity: Entity,
        item_id: str,
        quantity: int,
        max_price: Optional[float] = None
    ) -> bool:
        """
        Instantly buy items from lowest sell orders
        
        Args:
            entity: Buyer entity
            item_id: Item to buy
            quantity: Quantity to buy
            max_price: Maximum price willing to pay per unit
            
        Returns:
            True if purchase successful
        """
        wallet = entity.get_component(Wallet)
        market_access = entity.get_component(MarketAccess)
        
        from engine.systems.industry_system import Inventory
        inventory = entity.get_component(Inventory)
        
        if not all([wallet, market_access, inventory]):
            return False
        
        location = market_access.location_id
        
        # Get sell orders
        if location not in self.order_book or item_id not in self.order_book[location]:
            return False
        
        sell_orders = self.order_book[location][item_id]['sell']
        if not sell_orders:
            return False
        
        remaining = quantity
        total_cost = 0.0
        purchases = []  # [(order, qty)]
        
        # Match with sell orders
        for order in sell_orders:
            if order.remaining <= 0:
                continue
            
            if max_price and order.price > max_price:
                break
            
            qty_to_buy = min(remaining, order.remaining)
            cost = qty_to_buy * order.price
            
            if not wallet.can_afford(total_cost + cost):
                break
            
            purchases.append((order, qty_to_buy))
            total_cost += cost
            remaining -= qty_to_buy
            
            if remaining <= 0:
                break
        
        if remaining > 0:
            # Couldn't fulfill entire order
            return False
        
        # Execute purchases
        sales_tax = total_cost * 0.02  # 2% sales tax
        total_with_tax = total_cost + sales_tax
        
        if not wallet.withdraw(total_with_tax):
            return False
        
        # Transfer items and ISK
        for order, qty in purchases:
            order.remaining -= qty
            
            # Pay seller
            if order.owner_id:
                seller_wallet = order.owner_id.get_component(Wallet)
                if seller_wallet:
                    seller_wallet.deposit(order.price * qty)
            
            # Add items to buyer
            inventory.add_item(item_id, qty)
            
            # Record transaction
            self.transaction_history.append({
                'item_id': item_id,
                'quantity': qty,
                'price': order.price,
                'buyer': entity,
                'seller': order.owner_id,
                'location': location
            })
        
        # Clean up empty orders
        self._cleanup_orders(location, item_id)
        
        return True
    
    def instant_sell(
        self,
        entity: Entity,
        item_id: str,
        quantity: int,
        min_price: Optional[float] = None
    ) -> bool:
        """
        Instantly sell items to highest buy orders
        
        Args:
            entity: Seller entity
            item_id: Item to sell
            quantity: Quantity to sell
            min_price: Minimum price willing to accept per unit
            
        Returns:
            True if sale successful
        """
        wallet = entity.get_component(Wallet)
        market_access = entity.get_component(MarketAccess)
        
        from engine.systems.industry_system import Inventory
        inventory = entity.get_component(Inventory)
        
        if not all([wallet, market_access, inventory]):
            return False
        
        # Must have items to sell
        if inventory.items.get(item_id, 0) < quantity:
            return False
        
        location = market_access.location_id
        
        # Get buy orders
        if location not in self.order_book or item_id not in self.order_book[location]:
            return False
        
        buy_orders = self.order_book[location][item_id]['buy']
        if not buy_orders:
            return False
        
        remaining = quantity
        total_revenue = 0.0
        sales = []  # [(order, qty)]
        
        # Match with buy orders
        for order in buy_orders:
            if order.remaining <= 0:
                continue
            
            if min_price and order.price < min_price:
                break
            
            qty_to_sell = min(remaining, order.remaining)
            revenue = qty_to_sell * order.price
            
            sales.append((order, qty_to_sell))
            total_revenue += revenue
            remaining -= qty_to_sell
            
            if remaining <= 0:
                break
        
        if remaining > 0:
            # Couldn't sell everything
            return False
        
        # Execute sales
        sales_tax = total_revenue * 0.02  # 2% sales tax
        net_revenue = total_revenue - sales_tax
        
        # Remove items from seller
        inventory.remove_item(item_id, quantity)
        
        # Pay seller
        wallet.deposit(net_revenue)
        
        # Transfer to buyers
        for order, qty in sales:
            order.remaining -= qty
            
            if order.owner_id:
                buyer_inventory = self.world.get_component(
                    order.owner_id, 
                    Inventory
                )
                if buyer_inventory:
                    buyer_inventory.add_item(item_id, qty)
            
            # Record transaction
            self.transaction_history.append({
                'item_id': item_id,
                'quantity': qty,
                'price': order.price,
                'buyer': order.owner_id,
                'seller': entity,
                'location': location
            })
        
        # Clean up empty orders
        self._cleanup_orders(location, item_id)
        
        return True
    
    def cancel_order(self, entity: Entity, order_id: str) -> bool:
        """Cancel an existing order"""
        market_access = entity.get_component(MarketAccess)
        if not market_access or order_id not in market_access.active_orders:
            return False
        
        # Find and remove order
        for location in self.order_book.values():
            for item_orders in location.values():
                for order_type in ['buy', 'sell']:
                    orders = item_orders[order_type]
                    for i, order in enumerate(orders):
                        if order.order_id == order_id and order.owner_id == entity:
                            orders.pop(i)
                            market_access.active_orders.remove(order_id)
                            
                            # Refund escrow
                            self._refund_order(entity, order)
                            return True
        
        return False
    
    def _refund_order(self, entity: Entity, order: MarketOrder):
        """Refund escrow from cancelled order"""
        wallet = entity.get_component(Wallet)
        
        from engine.systems.industry_system import Inventory
        inventory = entity.get_component(Inventory)
        
        if order.order_type == OrderType.BUY:
            # Refund ISK
            if wallet:
                wallet.deposit(order.remaining * order.price)
        else:
            # Return items
            if inventory:
                inventory.add_item(order.item_id, order.remaining)
    
    def _cleanup_orders(self, location: str, item_id: str):
        """Remove completed orders"""
        if location in self.order_book and item_id in self.order_book[location]:
            for order_type in ['buy', 'sell']:
                self.order_book[location][item_id][order_type] = [
                    o for o in self.order_book[location][item_id][order_type]
                    if o.remaining > 0
                ]
    
    def get_market_price(self, item_id: str, location: str = "jita_4_4") -> Dict[str, float]:
        """
        Get current market prices for an item
        
        Returns:
            Dict with 'buy' (highest buy), 'sell' (lowest sell), 'average'
        """
        result = {
            'buy': 0.0,
            'sell': 0.0,
            'average': 0.0
        }
        
        if location not in self.order_book or item_id not in self.order_book[location]:
            # Use NPC base price if available
            if item_id in self.npc_base_prices:
                result['average'] = self.npc_base_prices[item_id]
                result['buy'] = self.npc_base_prices[item_id] * 0.9
                result['sell'] = self.npc_base_prices[item_id] * 1.1
            return result
        
        orders = self.order_book[location][item_id]
        
        # Highest buy order
        buy_orders = [o for o in orders['buy'] if o.remaining > 0]
        if buy_orders:
            result['buy'] = buy_orders[0].price
        
        # Lowest sell order
        sell_orders = [o for o in orders['sell'] if o.remaining > 0]
        if sell_orders:
            result['sell'] = sell_orders[0].price
        
        # Average
        if result['buy'] > 0 and result['sell'] > 0:
            result['average'] = (result['buy'] + result['sell']) / 2
        elif result['sell'] > 0:
            result['average'] = result['sell']
        elif result['buy'] > 0:
            result['average'] = result['buy']
        
        return result
    
    def update(self, delta_time: float, current_time: float):
        """Update market (expire old orders, etc.)"""
        expired_orders = []
        
        # Find expired orders
        for location in self.order_book.values():
            for item_orders in location.values():
                for order_type in ['buy', 'sell']:
                    orders = item_orders[order_type]
                    for order in orders:
                        if current_time - order.issued_time > order.duration_seconds:
                            expired_orders.append((order, order_type))
        
        # Cancel expired orders
        for order, _ in expired_orders:
            if order.owner_id:
                self.cancel_order(order.owner_id, order.order_id)
