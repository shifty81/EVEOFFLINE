"""
Market Interface UI
Displays market orders and allows trading
"""

from direct.gui.DirectGui import DirectButton, DirectLabel, DirectEntry, DirectOptionMenu
from panda3d.core import TextNode, TransparencyAttrib
from typing import Dict, List, Optional, Callable
from .base_panel import EVEListPanel


class MarketWindow(EVEListPanel):
    """
    Market interface window
    Shows buy/sell orders and allows trading
    """
    
    def __init__(self, parent, pos=(0.15, 0.25), size=(0.7, 0.8)):
        """Initialize market window"""
        super().__init__(
            parent=parent,
            title="Market",
            pos=pos,
            size=size,
            closeable=True,
            draggable=True
        )
        
        # Current market state
        self.selected_item = None
        self.order_type = "sell"  # "sell" or "buy"
        self.current_location = "Jita IV - Moon 4"
        
        # Callbacks
        self.on_buy_order: Optional[Callable] = None
        self.on_sell_order: Optional[Callable] = None
        self.on_place_order: Optional[Callable] = None
        
        # Create market UI
        self._create_market_ui()
        
        print("[MarketWindow] Market window initialized")
    
    def _create_market_ui(self):
        """Create market interface UI elements"""
        # Search bar
        self._create_search_bar()
        
        # Order type tabs
        self._create_order_type_tabs()
        
        # Location display
        self._create_location_display()
        
        # Buy/Sell action panel
        self._create_action_panel()
    
    def _create_search_bar(self):
        """Create item search bar"""
        # Search label
        DirectLabel(
            text="Search:",
            text_scale=0.04,
            text_fg=(0.9, 0.95, 1.0, 1.0),
            text_align=TextNode.ALeft,
            frameColor=(0, 0, 0, 0),
            pos=(0.02, 0, -0.02),
            parent=self.content_frame
        )
        
        # Search entry
        self.search_entry = DirectEntry(
            text="",
            scale=0.04,
            width=12,
            frameColor=(0.1, 0.15, 0.2, 0.9),
            pos=(0.15, 0, -0.025),
            parent=self.content_frame,
            command=self._on_search
        )
        self.search_entry.setTransparency(TransparencyAttrib.MAlpha)
        
        # Search button
        self.search_button = DirectButton(
            text="Search",
            text_scale=0.035,
            text_fg=(0.9, 0.95, 1.0, 1.0),
            frameColor=(0.2, 0.6, 0.8, 0.8),
            frameSize=(-0.08, 0.08, -0.02, 0.02),
            pos=(self.size[0] - 0.1, 0, -0.025),
            command=self._on_search,
            parent=self.content_frame
        )
        self.search_button.setTransparency(TransparencyAttrib.MAlpha)
    
    def _create_order_type_tabs(self):
        """Create tabs for buy/sell orders"""
        button_width = 0.15
        button_height = 0.04
        
        # Sell orders tab
        self.sell_tab = DirectButton(
            text="Sell Orders",
            text_scale=0.035,
            text_fg=(0.9, 0.95, 1.0, 1.0),
            frameColor=(0.2, 0.6, 0.8, 0.8),
            frameSize=(-button_width/2, button_width/2, -button_height/2, button_height/2),
            pos=(0.15, 0, -0.08),
            command=self._switch_to_sell_orders,
            parent=self.content_frame
        )
        self.sell_tab.setTransparency(TransparencyAttrib.MAlpha)
        
        # Buy orders tab
        self.buy_tab = DirectButton(
            text="Buy Orders",
            text_scale=0.035,
            text_fg=(0.9, 0.95, 1.0, 1.0),
            frameColor=(0.1, 0.15, 0.2, 0.8),
            frameSize=(-button_width/2, button_width/2, -button_height/2, button_height/2),
            pos=(0.35, 0, -0.08),
            command=self._switch_to_buy_orders,
            parent=self.content_frame
        )
        self.buy_tab.setTransparency(TransparencyAttrib.MAlpha)
    
    def _create_location_display(self):
        """Create current location display"""
        self.location_label = DirectLabel(
            text=f"Location: {self.current_location}",
            text_scale=0.035,
            text_fg=(0.7, 0.75, 0.8, 1.0),
            text_align=TextNode.ARight,
            frameColor=(0, 0, 0, 0),
            pos=(self.size[0] - 0.02, 0, -0.08),
            parent=self.content_frame
        )
    
    def _create_action_panel(self):
        """Create buy/sell action panel at bottom"""
        panel_y = -self.size[1] + 0.12
        
        # Price entry
        DirectLabel(
            text="Price:",
            text_scale=0.035,
            text_fg=(0.9, 0.95, 1.0, 1.0),
            text_align=TextNode.ALeft,
            frameColor=(0, 0, 0, 0),
            pos=(0.02, 0, panel_y),
            parent=self.content_frame
        )
        
        self.price_entry = DirectEntry(
            text="",
            scale=0.035,
            width=8,
            frameColor=(0.1, 0.15, 0.2, 0.9),
            pos=(0.12, 0, panel_y - 0.005),
            parent=self.content_frame
        )
        self.price_entry.setTransparency(TransparencyAttrib.MAlpha)
        
        # Quantity entry
        DirectLabel(
            text="Qty:",
            text_scale=0.035,
            text_fg=(0.9, 0.95, 1.0, 1.0),
            text_align=TextNode.ALeft,
            frameColor=(0, 0, 0, 0),
            pos=(0.35, 0, panel_y),
            parent=self.content_frame
        )
        
        self.quantity_entry = DirectEntry(
            text="",
            scale=0.035,
            width=6,
            frameColor=(0.1, 0.15, 0.2, 0.9),
            pos=(0.42, 0, panel_y - 0.005),
            parent=self.content_frame
        )
        self.quantity_entry.setTransparency(TransparencyAttrib.MAlpha)
        
        # Quick buy button
        self.quick_buy_button = DirectButton(
            text="Quick Buy",
            text_scale=0.04,
            text_fg=(0.9, 0.95, 1.0, 1.0),
            frameColor=(0.2, 0.8, 0.4, 0.8),
            frameSize=(-0.1, 0.1, -0.025, 0.025),
            pos=(0.15, 0, panel_y - 0.06),
            command=self._on_quick_buy,
            parent=self.content_frame
        )
        self.quick_buy_button.setTransparency(TransparencyAttrib.MAlpha)
        
        # Quick sell button
        self.quick_sell_button = DirectButton(
            text="Quick Sell",
            text_scale=0.04,
            text_fg=(0.9, 0.95, 1.0, 1.0),
            frameColor=(0.8, 0.4, 0.2, 0.8),
            frameSize=(-0.1, 0.1, -0.025, 0.025),
            pos=(0.35, 0, panel_y - 0.06),
            command=self._on_quick_sell,
            parent=self.content_frame
        )
        self.quick_sell_button.setTransparency(TransparencyAttrib.MAlpha)
        
        # Place order button
        self.place_order_button = DirectButton(
            text="Place Order",
            text_scale=0.04,
            text_fg=(0.9, 0.95, 1.0, 1.0),
            frameColor=(0.2, 0.6, 0.8, 0.8),
            frameSize=(-0.12, 0.12, -0.025, 0.025),
            pos=(self.size[0] - 0.15, 0, panel_y - 0.06),
            command=self._on_place_order,
            parent=self.content_frame
        )
        self.place_order_button.setTransparency(TransparencyAttrib.MAlpha)
    
    def _on_search(self, search_text=None):
        """Handle search"""
        if search_text is None:
            search_text = self.search_entry.get()
        print(f"[MarketWindow] Searching for: {search_text}")
        # Would trigger search in game client
    
    def _switch_to_sell_orders(self):
        """Switch to sell orders view"""
        self.order_type = "sell"
        self.sell_tab['frameColor'] = (0.2, 0.6, 0.8, 0.8)
        self.buy_tab['frameColor'] = (0.1, 0.15, 0.2, 0.8)
        self.refresh_orders()
    
    def _switch_to_buy_orders(self):
        """Switch to buy orders view"""
        self.order_type = "buy"
        self.sell_tab['frameColor'] = (0.1, 0.15, 0.2, 0.8)
        self.buy_tab['frameColor'] = (0.2, 0.6, 0.8, 0.8)
        self.refresh_orders()
    
    def _on_quick_buy(self):
        """Handle quick buy"""
        if self.on_buy_order and self.selected_item:
            quantity = self.quantity_entry.get() or "1"
            print(f"[MarketWindow] Quick buy: {self.selected_item}, quantity: {quantity}")
            self.on_buy_order(self.selected_item, int(quantity))
    
    def _on_quick_sell(self):
        """Handle quick sell"""
        if self.on_sell_order and self.selected_item:
            quantity = self.quantity_entry.get() or "1"
            print(f"[MarketWindow] Quick sell: {self.selected_item}, quantity: {quantity}")
            self.on_sell_order(self.selected_item, int(quantity))
    
    def _on_place_order(self):
        """Handle place order"""
        if self.on_place_order:
            price = self.price_entry.get() or "0"
            quantity = self.quantity_entry.get() or "1"
            print(f"[MarketWindow] Place order: {self.selected_item}, price: {price}, quantity: {quantity}")
            self.on_place_order(self.selected_item, float(price), int(quantity), self.order_type)
    
    def update_orders(self, orders: List[Dict]):
        """
        Update market orders display
        
        Args:
            orders: List of order dictionaries with 'price', 'quantity', 'location'
        """
        self.clear_list()
        
        for order in orders:
            item_id = order.get('item_id', 'Unknown')
            price = order.get('price', 0.0)
            quantity = order.get('quantity', 0)
            location = order.get('location', 'Unknown')
            
            order_text = f"{item_id} - {price:.2f} ISK x{quantity} @ {location}"
            self.add_list_item(order_text, data=order)
    
    def refresh_orders(self):
        """Refresh orders from current data (placeholder)"""
        # Would normally fetch from game client
        self.update_orders([])
    
    def set_buy_callback(self, callback: Callable):
        """Set callback for buy action"""
        self.on_buy_order = callback
    
    def set_sell_callback(self, callback: Callable):
        """Set callback for sell action"""
        self.on_sell_order = callback
    
    def set_place_order_callback(self, callback: Callable):
        """Set callback for place order action"""
        self.on_place_order = callback
