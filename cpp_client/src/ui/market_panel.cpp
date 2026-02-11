#include "ui/market_panel.h"
#include "ui/ui_manager.h"
#include "ui/eve_colors.h"
#include <imgui.h>
#include <algorithm>
#include <cstring>

namespace UI {

MarketPanel::MarketPanel()
    : m_visible(false)
    , m_viewMode(0)
    , m_selectedBuyOrderIndex(-1)
    , m_selectedSellOrderIndex(-1)
    , m_quickTradeQuantity(1)
    , m_quickTradePrice(0.0f)
    , m_pendingOperation(false)
    , m_feedbackIsError(false)
    , m_feedbackTimer(0.0f)
{
    memset(m_searchBuffer, 0, sizeof(m_searchBuffer));
}

void MarketPanel::Render() {
    if (!m_visible) return;
    
    // Set window size and position
    ImGui::SetNextWindowSize(ImVec2(1000, 700), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(150, 80), ImGuiCond_FirstUseEver);
    
    // EVE-style window flags
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse;
    
    if (!ImGui::Begin("Market", &m_visible, flags)) {
        ImGui::End();
        return;
    }
    
    // View tabs
    RenderViewTabs();
    ImGui::Separator();
    ImGui::Spacing();
    
    // Render based on view mode
    if (m_viewMode == 0) {
        RenderBrowseView();
    } else if (m_viewMode == 2) {
        RenderQuickTradeView();
    }
    
    // Render feedback message if active
    if (m_feedbackTimer > 0.0f) {
        ImGui::Spacing();
        ImGui::Separator();
        ImVec4 color = m_feedbackIsError ? ImVec4(1.0f, 0.3f, 0.3f, 1.0f) : ImVec4(0.3f, 1.0f, 0.3f, 1.0f);
        ImGui::PushStyleColor(ImGuiCol_Text, color);
        ImGui::TextWrapped("%s %s", m_feedbackIsError ? "✗" : "✓", m_feedbackMessage.c_str());
        ImGui::PopStyleColor();
        m_feedbackTimer -= ImGui::GetIO().DeltaTime;
    }
    
    // Show pending operation indicator
    if (m_pendingOperation) {
        ImGui::Spacing();
        ImGui::Text("⏳ Transaction in progress...");
    }
    
    ImGui::End();
}

void MarketPanel::SetBuyOrders(const std::vector<MarketOrder>& orders) {
    m_buyOrders = orders;
}

void MarketPanel::SetSellOrders(const std::vector<MarketOrder>& orders) {
    m_sellOrders = orders;
}

void MarketPanel::SetAvailableItems(const std::vector<MarketItem>& items) {
    m_availableItems = items;
    m_filteredItems = items;
}

void MarketPanel::RenderViewTabs() {
    // Browse tab — Photon UI active tab uses teal accent
    if (m_viewMode == 0) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(
            EVEColors::ACCENT_DIM[0], EVEColors::ACCENT_DIM[1],
            EVEColors::ACCENT_DIM[2], 0.8f));
    } else {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(
            EVEColors::BG_HEADER[0], EVEColors::BG_HEADER[1],
            EVEColors::BG_HEADER[2], 0.8f));
    }
    
    if (ImGui::Button("Browse", ImVec2(150, 30))) {
        m_viewMode = 0;
    }
    ImGui::PopStyleColor();
    
    ImGui::SameLine();
    
    // Quick Trade tab
    if (m_viewMode == 2) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(
            EVEColors::ACCENT_DIM[0], EVEColors::ACCENT_DIM[1],
            EVEColors::ACCENT_DIM[2], 0.8f));
    } else {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(
            EVEColors::BG_HEADER[0], EVEColors::BG_HEADER[1],
            EVEColors::BG_HEADER[2], 0.8f));
    }
    
    if (ImGui::Button("Quick Trade", ImVec2(150, 30))) {
        m_viewMode = 2;
    }
    ImGui::PopStyleColor();
}

void MarketPanel::RenderSearchBar() {
    ImGui::Text("Search:");
    ImGui::SameLine();
    
    ImGui::SetNextItemWidth(400);
    if (ImGui::InputText("##search", m_searchBuffer, sizeof(m_searchBuffer))) {
        ApplySearch();
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Clear")) {
        memset(m_searchBuffer, 0, sizeof(m_searchBuffer));
        ApplySearch();
    }
    
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(
        EVEColors::TEXT_SECONDARY[0], EVEColors::TEXT_SECONDARY[1],
        EVEColors::TEXT_SECONDARY[2], EVEColors::TEXT_SECONDARY[3]),
        "(%zu items)", m_filteredItems.size());
}

void MarketPanel::RenderBrowseView() {
    RenderSearchBar();
    ImGui::Spacing();
    
    // Split view: item list on left, order book on right
    ImGui::BeginChild("BrowseContent", ImVec2(0, 0), false);
    
    // Item list (left, 35%)
    ImGui::BeginChild("ItemList", ImVec2(ImGui::GetContentRegionAvail().x * 0.35f, 0), true);
    
    ImGui::Text("Items");
    ImGui::Separator();
    
    // Item rows
    ImGui::BeginChild("ItemScroll", ImVec2(0, 0));
    for (size_t i = 0; i < m_filteredItems.size(); ++i) {
        RenderItemRow(m_filteredItems[i], static_cast<int>(i));
    }
    ImGui::EndChild();
    
    ImGui::EndChild();
    
    ImGui::SameLine();
    
    // Order book (right, 65%)
    ImGui::BeginChild("OrderBook", ImVec2(0, 0), false);
    RenderOrderBook();
    ImGui::EndChild();
    
    ImGui::EndChild();
}

void MarketPanel::RenderQuickTradeView() {
    if (m_selectedItemId.empty()) {
        ImGui::TextColored(ImVec4(
            EVEColors::TEXT_SECONDARY[0], EVEColors::TEXT_SECONDARY[1],
            EVEColors::TEXT_SECONDARY[2], EVEColors::TEXT_SECONDARY[3]),
            "Select an item from Browse view first");
        return;
    }
    
    // Find selected item
    MarketItem* selectedItem = nullptr;
    for (auto& item : m_availableItems) {
        if (item.item_id == m_selectedItemId) {
            selectedItem = &item;
            break;
        }
    }
    
    if (!selectedItem) return;
    
    ImGui::TextColored(ImVec4(
        EVEColors::ACCENT_PRIMARY[0], EVEColors::ACCENT_PRIMARY[1],
        EVEColors::ACCENT_PRIMARY[2], EVEColors::ACCENT_PRIMARY[3]),
        "Quick Trade: %s", selectedItem->name.c_str());
    ImGui::Separator();
    ImGui::Spacing();
    
    // Get best prices
    auto sellOrders = GetItemSellOrders();
    auto buyOrders = GetItemBuyOrders();
    
    float bestSellPrice = sellOrders.empty() ? selectedItem->base_price : sellOrders[0].price;
    float bestBuyPrice = buyOrders.empty() ? selectedItem->base_price * 0.9f : buyOrders[0].price;
    
    // Quick Buy section
    ImGui::BeginChild("QuickBuySection", ImVec2(0, 250), true);
    ImGui::TextColored(ImVec4(
        EVEColors::SUCCESS[0], EVEColors::SUCCESS[1],
        EVEColors::SUCCESS[2], EVEColors::SUCCESS[3]), "QUICK BUY");
    ImGui::Separator();
    ImGui::Spacing();
    
    ImGui::Text("Best Sell Price:");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "%.2f ISK", bestSellPrice);
    
    ImGui::Spacing();
    ImGui::Text("Quantity:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(150);
    ImGui::InputInt("##buy_qty", &m_quickTradeQuantity);
    if (m_quickTradeQuantity < 1) m_quickTradeQuantity = 1;
    
    ImGui::Spacing();
    ImGui::Text("Total Cost:");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "%.2f ISK", 
                      bestSellPrice * m_quickTradeQuantity);
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    if (ImGui::Button("Buy Now", ImVec2(-1, 40))) {
        if (m_onQuickBuy) {
            m_onQuickBuy(m_selectedItemId, m_quickTradeQuantity);
        }
    }
    
    ImGui::EndChild();
    
    ImGui::Spacing();
    
    // Quick Sell section
    ImGui::BeginChild("QuickSellSection", ImVec2(0, 0), true);
    ImGui::TextColored(ImVec4(
        EVEColors::WARNING[0], EVEColors::WARNING[1],
        EVEColors::WARNING[2], EVEColors::WARNING[3]), "QUICK SELL");
    ImGui::Separator();
    ImGui::Spacing();
    
    ImGui::Text("Best Buy Price:");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "%.2f ISK", bestBuyPrice);
    
    ImGui::Spacing();
    ImGui::Text("Quantity:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(150);
    ImGui::InputInt("##sell_qty", &m_quickTradeQuantity);
    if (m_quickTradeQuantity < 1) m_quickTradeQuantity = 1;
    
    ImGui::Spacing();
    ImGui::Text("Total Revenue:");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "%.2f ISK", 
                      bestBuyPrice * m_quickTradeQuantity);
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    if (ImGui::Button("Sell Now", ImVec2(-1, 40))) {
        if (m_onQuickSell) {
            m_onQuickSell(m_selectedItemId, m_quickTradeQuantity);
        }
    }
    
    ImGui::EndChild();
}

void MarketPanel::RenderOrderBook() {
    if (m_selectedItemId.empty()) {
        ImGui::TextColored(ImVec4(
            EVEColors::TEXT_SECONDARY[0], EVEColors::TEXT_SECONDARY[1],
            EVEColors::TEXT_SECONDARY[2], EVEColors::TEXT_SECONDARY[3]),
            "Select an item to view orders");
        return;
    }
    
    // Split into buy and sell orders
    ImGui::Text("Order Book");
    ImGui::Separator();
    ImGui::Spacing();
    
    // Sell orders (top half)
    ImGui::BeginChild("SellOrders", ImVec2(0, ImGui::GetContentRegionAvail().y * 0.5f), true);
    RenderSellOrders();
    ImGui::EndChild();
    
    ImGui::Spacing();
    
    // Buy orders (bottom half)
    ImGui::BeginChild("BuyOrders", ImVec2(0, 0), true);
    RenderBuyOrders();
    ImGui::EndChild();
}

void MarketPanel::RenderBuyOrders() {
    ImGui::TextColored(ImVec4(
        EVEColors::SUCCESS[0], EVEColors::SUCCESS[1],
        EVEColors::SUCCESS[2], EVEColors::SUCCESS[3]), "BUY ORDERS");
    ImGui::Separator();
    
    auto orders = GetItemBuyOrders();
    
    if (orders.empty()) {
        ImGui::TextColored(ImVec4(
            EVEColors::TEXT_DISABLED[0], EVEColors::TEXT_DISABLED[1],
            EVEColors::TEXT_DISABLED[2], 1.0f), "No buy orders");
        return;
    }
    
    // Header
    ImGui::Columns(4, "BuyOrderColumns");
    ImGui::Text("Price"); ImGui::NextColumn();
    ImGui::Text("Quantity"); ImGui::NextColumn();
    ImGui::Text("Location"); ImGui::NextColumn();
    ImGui::Text("Range"); ImGui::NextColumn();
    ImGui::Separator();
    
    // Orders
    for (size_t i = 0; i < orders.size(); ++i) {
        RenderOrderRow(orders[i], static_cast<int>(i), m_selectedBuyOrderIndex == static_cast<int>(i));
    }
    
    ImGui::Columns(1);
}

void MarketPanel::RenderSellOrders() {
    ImGui::TextColored(ImVec4(
        EVEColors::WARNING[0], EVEColors::WARNING[1],
        EVEColors::WARNING[2], EVEColors::WARNING[3]), "SELL ORDERS");
    ImGui::Separator();
    
    auto orders = GetItemSellOrders();
    
    if (orders.empty()) {
        ImGui::TextColored(ImVec4(
            EVEColors::TEXT_DISABLED[0], EVEColors::TEXT_DISABLED[1],
            EVEColors::TEXT_DISABLED[2], 1.0f), "No sell orders");
        return;
    }
    
    // Header
    ImGui::Columns(4, "SellOrderColumns");
    ImGui::Text("Price"); ImGui::NextColumn();
    ImGui::Text("Quantity"); ImGui::NextColumn();
    ImGui::Text("Location"); ImGui::NextColumn();
    ImGui::Text("Range"); ImGui::NextColumn();
    ImGui::Separator();
    
    // Orders
    for (size_t i = 0; i < orders.size(); ++i) {
        RenderOrderRow(orders[i], static_cast<int>(i), m_selectedSellOrderIndex == static_cast<int>(i));
    }
    
    ImGui::Columns(1);
}

void MarketPanel::RenderItemRow(const MarketItem& item, int index) {
    bool isSelected = (item.item_id == m_selectedItemId);
    
    if (ImGui::Selectable(item.name.c_str(), isSelected)) {
        m_selectedItemId = item.item_id;
        m_selectedBuyOrderIndex = -1;
        m_selectedSellOrderIndex = -1;
    }
    
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::Text("%s", item.name.c_str());
        ImGui::TextColored(ImVec4(
            EVEColors::TEXT_SECONDARY[0], EVEColors::TEXT_SECONDARY[1],
            EVEColors::TEXT_SECONDARY[2], EVEColors::TEXT_SECONDARY[3]),
            "%s - %s", item.category.c_str(), item.group.c_str());
        ImGui::TextColored(ImVec4(
            EVEColors::WARNING[0], EVEColors::WARNING[1],
            EVEColors::WARNING[2], EVEColors::WARNING[3]),
            "Base Price: %.2f ISK", item.base_price);
        ImGui::EndTooltip();
    }
}

void MarketPanel::RenderOrderRow(const MarketOrder& order, int index, bool selected) {
    ImGuiSelectableFlags flags = ImGuiSelectableFlags_SpanAllColumns;
    
    if (ImGui::Selectable(("##order" + std::to_string(index)).c_str(), selected, flags)) {
        if (order.is_buy_order) {
            m_selectedBuyOrderIndex = index;
        } else {
            m_selectedSellOrderIndex = index;
        }
    }
    
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "%.2f", order.price); 
    ImGui::NextColumn();
    ImGui::Text("%d", order.quantity); ImGui::NextColumn();
    ImGui::Text("%s", order.location.c_str()); ImGui::NextColumn();
    ImGui::Text("%.0f jumps", order.range); ImGui::NextColumn();
}

void MarketPanel::ApplySearch() {
    m_filteredItems.clear();
    std::string search = m_searchBuffer;
    std::transform(search.begin(), search.end(), search.begin(), ::tolower);
    
    for (const auto& item : m_availableItems) {
        if (search.empty()) {
            m_filteredItems.push_back(item);
            continue;
        }
        
        std::string name = item.name;
        std::string category = item.category;
        std::string group = item.group;
        std::transform(name.begin(), name.end(), name.begin(), ::tolower);
        std::transform(category.begin(), category.end(), category.begin(), ::tolower);
        std::transform(group.begin(), group.end(), group.begin(), ::tolower);
        
        if (name.find(search) != std::string::npos ||
            category.find(search) != std::string::npos ||
            group.find(search) != std::string::npos) {
            m_filteredItems.push_back(item);
        }
    }
}

std::vector<MarketOrder> MarketPanel::GetItemBuyOrders() const {
    std::vector<MarketOrder> orders;
    for (const auto& order : m_buyOrders) {
        if (order.item_id == m_selectedItemId) {
            orders.push_back(order);
        }
    }
    // Sort by price (highest first for buy orders)
    std::sort(orders.begin(), orders.end(),
             [](const MarketOrder& a, const MarketOrder& b) {
                 return a.price > b.price;
             });
    return orders;
}

std::vector<MarketOrder> MarketPanel::GetItemSellOrders() const {
    std::vector<MarketOrder> orders;
    for (const auto& order : m_sellOrders) {
        if (order.item_id == m_selectedItemId) {
            orders.push_back(order);
        }
    }
    // Sort by price (lowest first for sell orders)
    std::sort(orders.begin(), orders.end(),
             [](const MarketOrder& a, const MarketOrder& b) {
                 return a.price < b.price;
             });
    return orders;
}

void MarketPanel::ShowSuccess(const std::string& message) {
    m_feedbackMessage = message;
    m_feedbackIsError = false;
    m_feedbackTimer = 3.0f;  // Show for 3 seconds
    m_pendingOperation = false;
}

void MarketPanel::ShowError(const std::string& message) {
    m_feedbackMessage = message;
    m_feedbackIsError = true;
    m_feedbackTimer = 5.0f;  // Show errors longer
    m_pendingOperation = false;
}

void MarketPanel::RenderContents() {
    // View tabs
    RenderViewTabs();
    ImGui::Separator();
    ImGui::Spacing();
    
    // Render based on view mode
    if (m_viewMode == 0) {
        RenderBrowseView();
    } else if (m_viewMode == 2) {
        RenderQuickTradeView();
    }
    
    // Render feedback message if active
    if (m_feedbackTimer > 0.0f) {
        ImGui::Spacing();
        ImGui::Separator();
        ImVec4 color = m_feedbackIsError ? ImVec4(1.0f, 0.3f, 0.3f, 1.0f) : ImVec4(0.3f, 1.0f, 0.3f, 1.0f);
        ImGui::PushStyleColor(ImGuiCol_Text, color);
        ImGui::TextWrapped("%s %s", m_feedbackIsError ? "✗" : "✓", m_feedbackMessage.c_str());
        ImGui::PopStyleColor();
        m_feedbackTimer -= ImGui::GetIO().DeltaTime;
    }
    
    // Show pending operation indicator
    if (m_pendingOperation) {
        ImGui::Spacing();
        ImGui::Text("⏳ Transaction in progress...");
    }
}

} // namespace UI
