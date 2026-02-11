/**
 * Test program for the Layout Manager (Phase 4.10).
 *
 * Validates JSON serialization/deserialization roundtrip, preset
 * creation, save/load, and panel opacity management.
 * Runs headless — no GPU or OpenGL required.
 */

#include "ui/layout_manager.h"
#include <iostream>
#include <string>
#include <cmath>
#include <cstdio>

// ─── Test helpers ──────────────────────────────────────────────────────

int testsRun = 0;
int testsPassed = 0;

void assertTrue(bool condition, const std::string& testName) {
    testsRun++;
    if (condition) {
        testsPassed++;
        std::cout << "  \xe2\x9c\x93 " << testName << std::endl;
    } else {
        std::cout << "  \xe2\x9c\x97 FAIL: " << testName << std::endl;
    }
}

void assertClose(float a, float b, const std::string& testName, float eps = 0.01f) {
    assertTrue(std::fabs(a - b) < eps, testName);
}

// ─── Serialization tests ──────────────────────────────────────────────

void testSerializeEmpty() {
    std::cout << "\n=== Serialize Empty Layout ===" << std::endl;

    std::unordered_map<std::string, UI::PanelLayout> panels;
    std::string json = UI::LayoutManager::SerializeToJson("empty", panels);

    assertTrue(!json.empty(), "JSON output is non-empty");
    assertTrue(json.find("\"name\": \"empty\"") != std::string::npos,
               "JSON contains preset name");
    assertTrue(json.find("\"panels\": [") != std::string::npos,
               "JSON contains panels array");
}

void testSerializeRoundtrip() {
    std::cout << "\n=== Serialize/Deserialize Roundtrip ===" << std::endl;

    // Build a layout with 3 panels
    std::unordered_map<std::string, UI::PanelLayout> original;

    UI::PanelLayout overview;
    overview.id = "overview";
    overview.x = 890.0f;
    overview.y = 50.0f;
    overview.w = 380.0f;
    overview.h = 400.0f;
    overview.visible = true;
    overview.minimized = false;
    overview.opacity = 0.92f;
    original["overview"] = overview;

    UI::PanelLayout inventory;
    inventory.id = "inventory";
    inventory.x = 50.0f;
    inventory.y = 300.0f;
    inventory.w = 350.0f;
    inventory.h = 400.0f;
    inventory.visible = false;
    inventory.minimized = true;
    inventory.opacity = 0.75f;
    original["inventory"] = inventory;

    UI::PanelLayout dscan;
    dscan.id = "dscan";
    dscan.x = 920.0f;
    dscan.y = 460.0f;
    dscan.w = 350.0f;
    dscan.h = 300.0f;
    dscan.visible = true;
    dscan.minimized = false;
    dscan.opacity = 0.80f;
    original["dscan"] = dscan;

    // Serialize
    std::string json = UI::LayoutManager::SerializeToJson("test_layout", original);
    assertTrue(!json.empty(), "Serialization produces output");

    // Deserialize
    std::string loadedName;
    std::unordered_map<std::string, UI::PanelLayout> loaded;
    bool ok = UI::LayoutManager::DeserializeFromJson(json, loadedName, loaded);

    assertTrue(ok, "Deserialization succeeds");
    assertTrue(loadedName == "test_layout", "Preset name roundtrips");
    assertTrue(loaded.size() == 3, "Panel count roundtrips (3)");

    // Verify overview
    assertTrue(loaded.count("overview") == 1, "Overview panel found");
    const auto& ov = loaded["overview"];
    assertClose(ov.x, 890.0f, "Overview x roundtrips");
    assertClose(ov.y, 50.0f, "Overview y roundtrips");
    assertClose(ov.w, 380.0f, "Overview w roundtrips");
    assertClose(ov.h, 400.0f, "Overview h roundtrips");
    assertTrue(ov.visible == true, "Overview visible roundtrips");
    assertTrue(ov.minimized == false, "Overview minimized roundtrips");
    assertClose(ov.opacity, 0.92f, "Overview opacity roundtrips");

    // Verify inventory
    assertTrue(loaded.count("inventory") == 1, "Inventory panel found");
    const auto& inv = loaded["inventory"];
    assertClose(inv.x, 50.0f, "Inventory x roundtrips");
    assertClose(inv.y, 300.0f, "Inventory y roundtrips");
    assertTrue(inv.visible == false, "Inventory visible roundtrips");
    assertTrue(inv.minimized == true, "Inventory minimized roundtrips");
    assertClose(inv.opacity, 0.75f, "Inventory opacity roundtrips");

    // Verify dscan
    assertTrue(loaded.count("dscan") == 1, "DScan panel found");
    const auto& ds = loaded["dscan"];
    assertClose(ds.opacity, 0.80f, "DScan opacity roundtrips");
    assertTrue(ds.visible == true, "DScan visible roundtrips");
}

void testDeserializeInvalid() {
    std::cout << "\n=== Deserialize Invalid JSON ===" << std::endl;

    std::string outName;
    std::unordered_map<std::string, UI::PanelLayout> panels;

    // Empty string
    bool ok = UI::LayoutManager::DeserializeFromJson("", outName, panels);
    assertTrue(!ok, "Empty string fails gracefully");

    // Garbage
    ok = UI::LayoutManager::DeserializeFromJson("not json at all", outName, panels);
    assertTrue(!ok, "Garbage input fails gracefully");

    // Valid JSON but missing name
    ok = UI::LayoutManager::DeserializeFromJson("{\"panels\": []}", outName, panels);
    assertTrue(!ok, "Missing name field fails gracefully");
}

// ─── File I/O tests ───────────────────────────────────────────────────

void testSaveLoad() {
    std::cout << "\n=== Save/Load File I/O ===" << std::endl;

    UI::LayoutManager mgr;
    mgr.SetLayoutDirectory("/tmp/eveoffline_test_layouts");

    // Ensure directory exists
    std::system("mkdir -p /tmp/eveoffline_test_layouts");

    // Build test layout
    std::unordered_map<std::string, UI::PanelLayout> panels;
    UI::PanelLayout p;
    p.id = "overview";
    p.x = 100.0f; p.y = 200.0f; p.w = 300.0f; p.h = 400.0f;
    p.visible = true; p.opacity = 0.88f;
    panels["overview"] = p;

    p.id = "chat";
    p.x = 50.0f; p.y = 500.0f; p.w = 350.0f; p.h = 200.0f;
    p.visible = false; p.opacity = 0.70f;
    panels["chat"] = p;

    // Save
    bool saved = mgr.SaveLayout("test_save", panels);
    assertTrue(saved, "SaveLayout succeeds");

    // Load
    std::unordered_map<std::string, UI::PanelLayout> loaded;
    bool ok = mgr.LoadLayout("test_save", loaded);
    assertTrue(ok, "LoadLayout succeeds");
    assertTrue(loaded.size() == 2, "Loaded 2 panels");

    if (loaded.count("overview")) {
        assertClose(loaded["overview"].x, 100.0f, "Loaded overview x");
        assertClose(loaded["overview"].opacity, 0.88f, "Loaded overview opacity");
        assertTrue(loaded["overview"].visible, "Loaded overview visible");
    }

    if (loaded.count("chat")) {
        assertTrue(!loaded["chat"].visible, "Loaded chat not visible");
        assertClose(loaded["chat"].opacity, 0.70f, "Loaded chat opacity");
    }

    // Load non-existent
    ok = mgr.LoadLayout("does_not_exist", loaded);
    assertTrue(!ok, "Loading non-existent layout fails gracefully");

    // Delete
    bool deleted = mgr.DeletePreset("test_save");
    assertTrue(deleted, "DeletePreset succeeds");

    // Cleanup
    std::system("rm -rf /tmp/eveoffline_test_layouts");
}

// ─── Default presets test ─────────────────────────────────────────────

void testDefaultPresets() {
    std::cout << "\n=== Default Presets ===" << std::endl;

    UI::LayoutManager mgr;
    mgr.SetLayoutDirectory("/tmp/eveoffline_test_presets");
    std::system("mkdir -p /tmp/eveoffline_test_presets");

    // Create defaults
    mgr.CreateDefaultPresets(1280, 720);

    // Check each default preset loads
    std::unordered_map<std::string, UI::PanelLayout> panels;

    bool ok = mgr.LoadLayout("default", panels);
    assertTrue(ok, "Default layout loads");
    assertTrue(panels.size() == 9, "Default has 9 panels");
    assertTrue(panels.count("overview") == 1, "Default has overview");
    assertTrue(panels.count("inventory") == 1, "Default has inventory");

    ok = mgr.LoadLayout("combat", panels);
    assertTrue(ok, "Combat layout loads");
    assertTrue(panels.size() == 9, "Combat has 9 panels");
    // Combat layout should have dscan visible
    if (panels.count("dscan")) {
        assertTrue(panels["dscan"].visible, "Combat dscan is visible");
    }

    ok = mgr.LoadLayout("mining", panels);
    assertTrue(ok, "Mining layout loads");
    assertTrue(panels.size() == 9, "Mining has 9 panels");
    // Mining layout should have inventory visible
    if (panels.count("inventory")) {
        assertTrue(panels["inventory"].visible, "Mining inventory is visible");
    }

    // Check available presets
    auto presets = mgr.GetAvailablePresets();
    assertTrue(presets.size() >= 3, "At least 3 presets available");

    // Cleanup
    std::system("rm -rf /tmp/eveoffline_test_presets");
}

// ─── Panel layout struct test ─────────────────────────────────────────

void testPanelLayoutDefaults() {
    std::cout << "\n=== PanelLayout Defaults ===" << std::endl;

    UI::PanelLayout pl;
    assertTrue(pl.id.empty(), "Default id is empty");
    assertClose(pl.x, 0.0f, "Default x is 0");
    assertClose(pl.y, 0.0f, "Default y is 0");
    assertClose(pl.w, 300.0f, "Default w is 300");
    assertClose(pl.h, 400.0f, "Default h is 400");
    assertTrue(pl.visible, "Default visible is true");
    assertTrue(!pl.minimized, "Default minimized is false");
    assertClose(pl.opacity, 0.92f, "Default opacity is 0.92");
}

// ─── Main ─────────────────────────────────────────────────────────────

int main() {
    std::cout << "=== Layout Manager Tests ===" << std::endl;

    testPanelLayoutDefaults();
    testSerializeEmpty();
    testSerializeRoundtrip();
    testDeserializeInvalid();
    testSaveLoad();
    testDefaultPresets();

    std::cout << "\n========================================" << std::endl;
    std::cout << testsPassed << "/" << testsRun << " tests passed" << std::endl;

    if (testsPassed == testsRun) {
        std::cout << "\xe2\x9c\x93 ALL TESTS PASSED" << std::endl;
    } else {
        std::cout << "\xe2\x9c\x97 SOME TESTS FAILED" << std::endl;
    }

    return (testsPassed == testsRun) ? 0 : 1;
}
