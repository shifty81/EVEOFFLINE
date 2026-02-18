/**
 * Tests for Atlas HUD panel data structures and state management.
 *
 * These tests validate the data binding layer for the dockable HUD panels
 * (Station Services, Inventory, Fitting, Market, Fleet) without requiring
 * any OpenGL context.  They test struct defaults, setters/getters, toggle
 * logic, and callback registration.
 */

#include "../cpp_client/include/ui/atlas/atlas_hud.h"
#include <iostream>
#include <cassert>
#include <string>

using namespace atlas;

// ── Station Services panel tests ──────────────────────────────────

void test_station_panel_defaults() {
    AtlasHUD hud;
    hud.init(1280, 720);

    assert(!hud.isStationOpen());

    const auto& data = hud.getStationData();
    assert(data.stationName.empty());
    assert(data.distance == 0.0f);
    assert(data.dockingRange == 2500.0f);
    assert(!data.isDocked);
    assert(data.shieldPct == 1.0f);
    assert(data.armorPct == 1.0f);
    assert(data.hullPct == 1.0f);
    assert(data.repairCostIsk == 0.0f);

    std::cout << "[PASS] test_station_panel_defaults" << std::endl;
}

void test_station_panel_toggle() {
    AtlasHUD hud;
    hud.init(1280, 720);

    assert(!hud.isStationOpen());
    hud.toggleStation();
    assert(hud.isStationOpen());
    hud.toggleStation();
    assert(!hud.isStationOpen());

    std::cout << "[PASS] test_station_panel_toggle" << std::endl;
}

void test_station_panel_set_data() {
    AtlasHUD hud;
    hud.init(1280, 720);

    AtlasHUD::StationPanelData data;
    data.stationName = "Jita IV - Moon 4";
    data.distance = 1500.0f;
    data.dockingRange = 2500.0f;
    data.isDocked = true;
    data.shieldPct = 0.8f;
    data.armorPct = 0.5f;
    data.hullPct = 1.0f;
    data.repairCostIsk = 25000.0f;

    hud.setStationData(data);

    const auto& d = hud.getStationData();
    assert(d.stationName == "Jita IV - Moon 4");
    assert(d.distance == 1500.0f);
    assert(d.isDocked == true);
    assert(d.shieldPct == 0.8f);
    assert(d.armorPct == 0.5f);
    assert(d.hullPct == 1.0f);
    assert(d.repairCostIsk == 25000.0f);

    std::cout << "[PASS] test_station_panel_set_data" << std::endl;
}

void test_station_panel_callbacks() {
    AtlasHUD hud;
    hud.init(1280, 720);

    bool dockCalled = false;
    bool undockCalled = false;
    bool repairCalled = false;

    hud.setStationDockCb([&]() { dockCalled = true; });
    hud.setStationUndockCb([&]() { undockCalled = true; });
    hud.setStationRepairCb([&]() { repairCalled = true; });

    // Callbacks are stored; we verify they can be set without crash
    assert(!dockCalled);
    assert(!undockCalled);
    assert(!repairCalled);

    std::cout << "[PASS] test_station_panel_callbacks" << std::endl;
}

// ── Inventory panel tests ──────────────────────────────────────────

void test_inventory_panel_defaults() {
    AtlasHUD hud;
    hud.init(1280, 720);

    const auto& data = hud.getInventoryData();
    assert(data.usedCapacity == 0.0f);
    assert(data.maxCapacity == 100.0f);
    assert(data.activeTab == 0);
    assert(data.items.empty());

    std::cout << "[PASS] test_inventory_panel_defaults" << std::endl;
}

void test_inventory_panel_set_data() {
    AtlasHUD hud;
    hud.init(1280, 720);

    AtlasHUD::InventoryData inv;
    inv.usedCapacity = 45.0f;
    inv.maxCapacity = 200.0f;
    inv.activeTab = 1;

    AtlasHUD::InventoryItem item1;
    item1.name = "Antimatter Charge S";
    item1.type = "Ammo";
    item1.quantity = 100;
    item1.volume = 0.01f;

    AtlasHUD::InventoryItem item2;
    item2.name = "1MN Afterburner II";
    item2.type = "Module";
    item2.quantity = 1;
    item2.volume = 5.0f;

    inv.items.push_back(item1);
    inv.items.push_back(item2);

    hud.setInventoryData(inv);

    const auto& d = hud.getInventoryData();
    assert(d.usedCapacity == 45.0f);
    assert(d.maxCapacity == 200.0f);
    assert(d.activeTab == 1);
    assert(d.items.size() == 2);
    assert(d.items[0].name == "Antimatter Charge S");
    assert(d.items[0].quantity == 100);
    assert(d.items[1].name == "1MN Afterburner II");
    assert(d.items[1].volume == 5.0f);

    std::cout << "[PASS] test_inventory_panel_set_data" << std::endl;
}

// ── Fitting panel tests ────────────────────────────────────────────

void test_fitting_panel_defaults() {
    AtlasHUD hud;
    hud.init(1280, 720);

    const auto& data = hud.getFittingData();
    assert(data.shipName == "Current Ship");
    assert(data.cpuUsed == 0.0f);
    assert(data.cpuMax == 0.0f);
    assert(data.pgUsed == 0.0f);
    assert(data.pgMax == 0.0f);
    assert(data.calibrationMax == 400.0f);
    assert(data.highSlots.empty());
    assert(data.midSlots.empty());
    assert(data.lowSlots.empty());
    assert(data.effectiveHP == 0.0f);
    assert(data.dps == 0.0f);
    assert(!data.capStable);

    std::cout << "[PASS] test_fitting_panel_defaults" << std::endl;
}

void test_fitting_panel_set_data() {
    AtlasHUD hud;
    hud.init(1280, 720);

    AtlasHUD::FittingData fit;
    fit.shipName = "Caracal";
    fit.cpuUsed = 280.0f;
    fit.cpuMax = 325.0f;
    fit.pgUsed = 650.0f;
    fit.pgMax = 700.0f;
    fit.calibrationUsed = 50.0f;

    AtlasHUD::FittingSlot high1;
    high1.fitted = true;
    high1.moduleName = "Rapid Light Missile Launcher II";
    high1.online = true;
    fit.highSlots.push_back(high1);

    AtlasHUD::FittingSlot high2;
    high2.fitted = false;
    fit.highSlots.push_back(high2);

    AtlasHUD::FittingSlot mid1;
    mid1.fitted = true;
    mid1.moduleName = "10MN Afterburner II";
    mid1.online = true;
    fit.midSlots.push_back(mid1);

    fit.effectiveHP = 12000.0f;
    fit.dps = 350.5f;
    fit.maxVelocity = 1200.0f;
    fit.capStable = true;

    hud.setFittingData(fit);

    const auto& d = hud.getFittingData();
    assert(d.shipName == "Caracal");
    assert(d.cpuUsed == 280.0f);
    assert(d.cpuMax == 325.0f);
    assert(d.highSlots.size() == 2);
    assert(d.highSlots[0].fitted == true);
    assert(d.highSlots[0].moduleName == "Rapid Light Missile Launcher II");
    assert(d.highSlots[1].fitted == false);
    assert(d.midSlots.size() == 1);
    assert(d.effectiveHP == 12000.0f);
    assert(d.dps == 350.5f);
    assert(d.capStable == true);

    std::cout << "[PASS] test_fitting_panel_set_data" << std::endl;
}

// ── Market panel tests ──────────────────────────────────────────────

void test_market_panel_defaults() {
    AtlasHUD hud;
    hud.init(1280, 720);

    const auto& data = hud.getMarketData();
    assert(data.activeTab == 0);
    assert(data.sellOrders.empty());
    assert(data.buyOrders.empty());

    std::cout << "[PASS] test_market_panel_defaults" << std::endl;
}

void test_market_panel_set_data() {
    AtlasHUD hud;
    hud.init(1280, 720);

    AtlasHUD::MarketData mkt;
    mkt.activeTab = 1;

    AtlasHUD::MarketOrder sell;
    sell.itemName = "Tritanium";
    sell.price = 4.50f;
    sell.quantity = 10000;
    sell.location = "Jita IV-4";
    mkt.sellOrders.push_back(sell);

    AtlasHUD::MarketOrder buy;
    buy.itemName = "Tritanium";
    buy.price = 4.20f;
    buy.quantity = 50000;
    buy.location = "Jita IV-4";
    mkt.buyOrders.push_back(buy);

    hud.setMarketData(mkt);

    const auto& d = hud.getMarketData();
    assert(d.activeTab == 1);
    assert(d.sellOrders.size() == 1);
    assert(d.sellOrders[0].itemName == "Tritanium");
    assert(d.sellOrders[0].price == 4.50f);
    assert(d.sellOrders[0].quantity == 10000);
    assert(d.buyOrders.size() == 1);
    assert(d.buyOrders[0].price == 4.20f);
    assert(d.buyOrders[0].quantity == 50000);

    std::cout << "[PASS] test_market_panel_set_data" << std::endl;
}

// ── Fleet panel tests ──────────────────────────────────────────────

void test_fleet_panel_defaults() {
    AtlasHUD hud;
    hud.init(1280, 720);

    assert(!hud.isFleetOpen());

    const auto& data = hud.getFleetData();
    assert(!data.inFleet);
    assert(data.fleetName.empty());
    assert(data.memberCount == 0);
    assert(data.members.empty());

    std::cout << "[PASS] test_fleet_panel_defaults" << std::endl;
}

void test_fleet_panel_toggle() {
    AtlasHUD hud;
    hud.init(1280, 720);

    assert(!hud.isFleetOpen());
    hud.toggleFleet();
    assert(hud.isFleetOpen());
    hud.toggleFleet();
    assert(!hud.isFleetOpen());

    std::cout << "[PASS] test_fleet_panel_toggle" << std::endl;
}

void test_fleet_panel_set_data() {
    AtlasHUD hud;
    hud.init(1280, 720);

    AtlasHUD::FleetData fleet;
    fleet.inFleet = true;
    fleet.fleetName = "Mining Ops Alpha";
    fleet.memberCount = 3;

    AtlasHUD::FleetMember m1;
    m1.name = "Commander Shepard";
    m1.shipType = "Caracal";
    m1.isCommander = true;
    m1.shieldPct = 1.0f;
    m1.armorPct = 1.0f;
    m1.hullPct = 1.0f;
    fleet.members.push_back(m1);

    AtlasHUD::FleetMember m2;
    m2.name = "Wingman Alpha";
    m2.shipType = "Vexor";
    m2.isCommander = false;
    m2.shieldPct = 0.6f;
    m2.armorPct = 0.9f;
    m2.hullPct = 1.0f;
    fleet.members.push_back(m2);

    AtlasHUD::FleetMember m3;
    m3.name = "Wingman Beta";
    m3.shipType = "Moa";
    m3.isCommander = false;
    m3.inRange = false;
    fleet.members.push_back(m3);

    hud.setFleetData(fleet);

    const auto& d = hud.getFleetData();
    assert(d.inFleet == true);
    assert(d.fleetName == "Mining Ops Alpha");
    assert(d.memberCount == 3);
    assert(d.members.size() == 3);
    assert(d.members[0].name == "Commander Shepard");
    assert(d.members[0].isCommander == true);
    assert(d.members[1].shieldPct == 0.6f);
    assert(d.members[2].inRange == false);

    std::cout << "[PASS] test_fleet_panel_set_data" << std::endl;
}

// ── Existing panel toggle tests ────────────────────────────────────

void test_existing_panel_toggles() {
    AtlasHUD hud;
    hud.init(1280, 720);

    // Overview starts open
    assert(hud.isOverviewOpen());
    hud.toggleOverview();
    assert(!hud.isOverviewOpen());

    // Others start closed
    assert(!hud.isInventoryOpen());
    hud.toggleInventory();
    assert(hud.isInventoryOpen());

    assert(!hud.isFittingOpen());
    hud.toggleFitting();
    assert(hud.isFittingOpen());

    assert(!hud.isMarketOpen());
    hud.toggleMarket();
    assert(hud.isMarketOpen());

    assert(!hud.isMissionOpen());
    hud.toggleMission();
    assert(hud.isMissionOpen());

    assert(!hud.isDScanOpen());
    hud.toggleDScan();
    assert(hud.isDScanOpen());

    assert(!hud.isChatOpen());
    hud.toggleChat();
    assert(hud.isChatOpen());

    assert(!hud.isDronePanelOpen());
    hud.toggleDronePanel();
    assert(hud.isDronePanelOpen());

    assert(!hud.isProbeScannerOpen());
    hud.toggleProbeScanner();
    assert(hud.isProbeScannerOpen());

    std::cout << "[PASS] test_existing_panel_toggles" << std::endl;
}

// ── Overview tab filter test ──────────────────────────────────────

void test_overview_tab_filter() {
    assert(AtlasHUD::matchesOverviewTab("Travel", "Station") == true);
    assert(AtlasHUD::matchesOverviewTab("Travel", "Stargate") == true);
    assert(AtlasHUD::matchesOverviewTab("Travel", "Frigate") == false);

    assert(AtlasHUD::matchesOverviewTab("Combat", "Frigate") == true);
    assert(AtlasHUD::matchesOverviewTab("Combat", "Battleship") == true);
    assert(AtlasHUD::matchesOverviewTab("Combat", "Station") == false);

    assert(AtlasHUD::matchesOverviewTab("Industry", "Asteroid") == true);
    assert(AtlasHUD::matchesOverviewTab("Industry", "Wreck") == true);
    assert(AtlasHUD::matchesOverviewTab("Industry", "Station") == false);

    // Unknown tab shows everything
    assert(AtlasHUD::matchesOverviewTab("Custom", "anything") == true);

    std::cout << "[PASS] test_overview_tab_filter" << std::endl;
}
