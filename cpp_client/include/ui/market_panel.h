#ifndef MARKET_PANEL_H
#define MARKET_PANEL_H

#include <string>
#include <vector>
#include <functional>

namespace UI {

// Market order entry
struct MarketOrder {
    std::string order_id;
    std::string item_name;
    std::string item_id;
    bool is_buy_order;  // true = buy order, false = sell order
    float price;
    int quantity;
    int min_volume;
    std::string location;
    float range;  // in jumps
    std::string expires;  // expiration time
    
    MarketOrder() = default;
    MarketOrder(const std::string& id, const std::string& item, const std::string& item_i,
                bool is_buy, float p, int qty, const std::string& loc)
        : order_id(id), item_name(item), item_id(item_i), is_buy_order(is_buy), 
          price(p), quantity(qty), min_volume(1), location(loc), range(0.0f) {}
};

// Market item (for searching)
struct MarketItem {
    std::string item_id;
    std::string name;
    std::string category;
    std::string group;
    float base_price;
    
    MarketItem() = default;
    MarketItem(const std::string& id, const std::string& n, const std::string& cat,
               const std::string& grp, float price)
        : item_id(id), name(n), category(cat), group(grp), base_price(price) {}
};

// Callback types
using BuyOrderCallback = std::function<void(const std::string& order_id, int quantity)>;
using SellOrderCallback = std::function<void(const std::string& item_id, int quantity, float price)>;
using QuickBuyCallback = std::function<void(const std::string& item_id, int quantity)>;
using QuickSellCallback = std::function<void(const std::string& item_id, int quantity)>;

class MarketPanel {
public:
    MarketPanel();
    ~MarketPanel() = default;
    
    // Render the market panel
    void Render();
    
    // Set market data
    void SetBuyOrders(const std::vector<MarketOrder>& orders);
    void SetSellOrders(const std::vector<MarketOrder>& orders);
    void SetAvailableItems(const std::vector<MarketItem>& items);
    
    // Visibility
    void SetVisible(bool visible) { m_visible = visible; }
    bool IsVisible() const { return m_visible; }
    
    // Callbacks
    void SetBuyOrderCallback(BuyOrderCallback callback) { m_onBuyOrder = callback; }
    void SetSellOrderCallback(SellOrderCallback callback) { m_onSellOrder = callback; }
    void SetQuickBuyCallback(QuickBuyCallback callback) { m_onQuickBuy = callback; }
    void SetQuickSellCallback(QuickSellCallback callback) { m_onQuickSell = callback; }
    
    // Response feedback methods
    void ShowSuccess(const std::string& message);
    void ShowError(const std::string& message);
    void SetPendingOperation(bool pending) { m_pendingOperation = pending; }
    
private:
    bool m_visible;
    
    // Market data
    std::vector<MarketOrder> m_buyOrders;
    std::vector<MarketOrder> m_sellOrders;
    std::vector<MarketItem> m_availableItems;
    std::vector<MarketItem> m_filteredItems;
    
    // UI state
    int m_viewMode;  // 0 = browse, 1 = my orders, 2 = quick trade
    char m_searchBuffer[128];
    std::string m_selectedItemId;
    int m_selectedBuyOrderIndex;
    int m_selectedSellOrderIndex;
    
    // Quick trade inputs
    int m_quickTradeQuantity;
    float m_quickTradePrice;
    
    // Callbacks
    BuyOrderCallback m_onBuyOrder;
    SellOrderCallback m_onSellOrder;
    QuickBuyCallback m_onQuickBuy;
    QuickSellCallback m_onQuickSell;
    
    // Response feedback state
    bool m_pendingOperation;
    std::string m_feedbackMessage;
    bool m_feedbackIsError;
    float m_feedbackTimer;
    
    // Helper functions
    void RenderViewTabs();
    void RenderSearchBar();
    void RenderBrowseView();
    void RenderQuickTradeView();
    void RenderOrderBook();
    void RenderBuyOrders();
    void RenderSellOrders();
    void RenderItemRow(const MarketItem& item, int index);
    void RenderOrderRow(const MarketOrder& order, int index, bool selected);
    void ApplySearch();
    std::vector<MarketOrder> GetItemBuyOrders() const;
    std::vector<MarketOrder> GetItemSellOrders() const;
};

} // namespace UI

#endif // MARKET_PANEL_H
