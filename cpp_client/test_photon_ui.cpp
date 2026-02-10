/**
 * Test program for the Photon UI system.
 *
 * Validates that the core Photon types, context, renderer, and widget
 * functions work correctly in a headless (no OpenGL) environment.
 * GPU rendering is stubbed out, so these tests verify logic, hit-testing,
 * color/theme values, ID hashing, and widget state management.
 */

#include "ui/photon/photon_types.h"
#include "ui/photon/photon_context.h"
#include "ui/photon/photon_widgets.h"
#include "ui/photon/photon_hud.h"
#include <iostream>
#include <cassert>
#include <string>
#include <cmath>

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

void assertClose(float a, float b, const std::string& testName, float eps = 0.001f) {
    assertTrue(std::fabs(a - b) < eps, testName);
}

// ─── Vec2 tests ────────────────────────────────────────────────────────

void testVec2() {
    std::cout << "\n=== Vec2 ===" << std::endl;
    photon::Vec2 a(3.0f, 4.0f);
    photon::Vec2 b(1.0f, 2.0f);
    auto c = a + b;
    assertTrue(c.x == 4.0f && c.y == 6.0f, "Vec2 addition");
    auto d = a - b;
    assertTrue(d.x == 2.0f && d.y == 2.0f, "Vec2 subtraction");
    auto e = a * 2.0f;
    assertTrue(e.x == 6.0f && e.y == 8.0f, "Vec2 scalar multiply");
}

// ─── Rect tests ────────────────────────────────────────────────────────

void testRect() {
    std::cout << "\n=== Rect ===" << std::endl;
    photon::Rect r(10.0f, 20.0f, 100.0f, 50.0f);
    assertTrue(r.right() == 110.0f, "Rect right()");
    assertTrue(r.bottom() == 70.0f, "Rect bottom()");
    auto c = r.center();
    assertClose(c.x, 60.0f, "Rect center X");
    assertClose(c.y, 45.0f, "Rect center Y");
    assertTrue(r.contains({50.0f, 40.0f}), "Rect contains inside point");
    assertTrue(!r.contains({5.0f, 40.0f}), "Rect does not contain outside point");
    assertTrue(r.contains({10.0f, 20.0f}), "Rect contains top-left corner");
    assertTrue(r.contains({110.0f, 70.0f}), "Rect contains bottom-right corner");
    assertTrue(!r.contains({111.0f, 70.0f}), "Rect excludes just outside right");
}

// ─── Color tests ───────────────────────────────────────────────────────

void testColor() {
    std::cout << "\n=== Color ===" << std::endl;
    photon::Color c(0.5f, 0.6f, 0.7f, 0.8f);
    auto c2 = c.withAlpha(0.3f);
    assertTrue(c2.r == 0.5f && c2.g == 0.6f && c2.b == 0.7f && c2.a == 0.3f,
               "Color withAlpha preserves RGB");
    auto c3 = photon::Color::fromRGBA(255, 128, 0, 255);
    assertClose(c3.r, 1.0f, "Color fromRGBA red");
    assertClose(c3.g, 128.0f / 255.0f, "Color fromRGBA green");
    assertClose(c3.b, 0.0f, "Color fromRGBA blue");
    assertClose(c3.a, 1.0f, "Color fromRGBA alpha");
}

// ─── Theme defaults ────────────────────────────────────────────────────

void testTheme() {
    std::cout << "\n=== Theme ===" << std::endl;
    const photon::Theme& t = photon::defaultTheme();
    assertTrue(t.bgPanel.a > 0.9f, "Panel background is nearly opaque");
    assertTrue(t.accentPrimary.r < t.accentPrimary.g, "Accent is teal (G > R)");
    assertTrue(t.accentPrimary.b > t.accentPrimary.g, "Accent is teal (B > G)");
    assertTrue(t.shield.b > t.shield.r, "Shield color is blue");
    assertTrue(t.armor.r > t.armor.b, "Armor color is gold (R > B)");
    assertTrue(t.hull.r > t.hull.g, "Hull color is red");
    assertTrue(t.headerHeight > 0.0f, "Header height is positive");
    assertTrue(t.padding > 0.0f, "Padding is positive");
}

// ─── Widget ID hashing ─────────────────────────────────────────────────

void testHashID() {
    std::cout << "\n=== Widget ID Hashing ===" << std::endl;
    photon::WidgetID a = photon::hashID("Overview");
    photon::WidgetID b = photon::hashID("Overview");
    photon::WidgetID c = photon::hashID("Fitting");
    assertTrue(a == b, "Same string produces same ID");
    assertTrue(a != c, "Different strings produce different IDs");
    assertTrue(photon::hashID("") != photon::hashID("x"),
               "Empty vs non-empty are different");
}

// ─── Context tests ─────────────────────────────────────────────────────

void testContext() {
    std::cout << "\n=== PhotonContext ===" << std::endl;
    photon::PhotonContext ctx;
    // init() will create stub GL resources in headless mode
    assertTrue(ctx.init(), "Context init succeeds (headless)");

    photon::InputState input;
    input.windowW = 1920;
    input.windowH = 1080;
    input.mousePos = {500.0f, 400.0f};
    input.mouseDown[0] = false;
    input.mouseClicked[0] = false;
    input.mouseReleased[0] = false;

    ctx.beginFrame(input);

    // Hover test
    photon::Rect inside(400, 350, 200, 100);
    photon::Rect outside(800, 800, 100, 100);
    assertTrue(ctx.isHovered(inside), "Mouse is inside rect");
    assertTrue(!ctx.isHovered(outside), "Mouse is outside rect");

    // Hot/Active state
    photon::WidgetID testID = photon::hashID("testWidget");
    ctx.setHot(testID);
    assertTrue(ctx.isHot(testID), "Widget is hot after setHot");
    ctx.setActive(testID);
    assertTrue(ctx.isActive(testID), "Widget is active after setActive");
    ctx.clearActive();
    assertTrue(!ctx.isActive(testID), "Widget is not active after clearActive");

    ctx.endFrame();

    // ID stack
    ctx.beginFrame(input);
    ctx.pushID("parent");
    photon::WidgetID idA = ctx.currentID("child");
    ctx.popID();
    ctx.pushID("other_parent");
    photon::WidgetID idB = ctx.currentID("child");
    ctx.popID();
    assertTrue(idA != idB, "Same child label under different parents produces different IDs");
    ctx.endFrame();

    ctx.shutdown();
}

// ─── Button behavior test ──────────────────────────────────────────────

void testButtonBehavior() {
    std::cout << "\n=== Button Behavior ===" << std::endl;
    photon::PhotonContext ctx;
    ctx.init();

    photon::Rect btn(100, 100, 80, 30);
    photon::WidgetID btnID = photon::hashID("testBtn");

    // Frame 1: mouse hovers over button
    {
        photon::InputState input;
        input.windowW = 1920;
        input.windowH = 1080;
        input.mousePos = {140.0f, 115.0f};
        ctx.beginFrame(input);
        bool clicked = ctx.buttonBehavior(btn, btnID);
        assertTrue(!clicked, "Button not clicked (just hovering)");
        assertTrue(ctx.isHot(btnID), "Button is hot when hovered");
        ctx.endFrame();
    }

    // Frame 2: mouse presses (clicked)
    {
        photon::InputState input;
        input.windowW = 1920;
        input.windowH = 1080;
        input.mousePos = {140.0f, 115.0f};
        input.mouseClicked[0] = true;
        input.mouseDown[0] = true;
        ctx.beginFrame(input);
        bool clicked = ctx.buttonBehavior(btn, btnID);
        assertTrue(!clicked, "Button not 'clicked' on press (click fires on release)");
        assertTrue(ctx.isActive(btnID), "Button is active when pressed");
        ctx.endFrame();
    }

    // Frame 3: mouse releases (click completes)
    {
        photon::InputState input;
        input.windowW = 1920;
        input.windowH = 1080;
        input.mousePos = {140.0f, 115.0f};
        input.mouseReleased[0] = true;
        ctx.beginFrame(input);
        bool clicked = ctx.buttonBehavior(btn, btnID);
        assertTrue(clicked, "Button clicked on release while hovering");
        ctx.endFrame();
    }

    // Frame 4: mouse releases outside button (no click)
    {
        // First, press inside
        photon::InputState input;
        input.windowW = 1920;
        input.windowH = 1080;
        input.mousePos = {140.0f, 115.0f};
        input.mouseClicked[0] = true;
        input.mouseDown[0] = true;
        ctx.beginFrame(input);
        ctx.buttonBehavior(btn, btnID);
        ctx.endFrame();
    }
    {
        // Then release outside
        photon::InputState input;
        input.windowW = 1920;
        input.windowH = 1080;
        input.mousePos = {300.0f, 300.0f};  // outside button
        input.mouseReleased[0] = true;
        ctx.beginFrame(input);
        bool clicked = ctx.buttonBehavior(btn, btnID);
        assertTrue(!clicked, "Button NOT clicked when released outside");
        ctx.endFrame();
    }

    ctx.shutdown();
}

// ─── Renderer text measurement ─────────────────────────────────────────

void testTextMeasurement() {
    std::cout << "\n=== Text Measurement ===" << std::endl;
    photon::PhotonRenderer renderer;
    renderer.init();

    float w1 = renderer.measureText("Hello");
    float w2 = renderer.measureText("Hello World");
    assertTrue(w1 > 0.0f, "Text measurement returns positive width");
    assertTrue(w2 > w1, "Longer text measures wider");
    assertClose(w1, 5.0f * 8.0f, "5-char text = 5 * 8px wide at scale 1.0");
    float w3 = renderer.measureText("Hi", 2.0f);
    assertClose(w3, 2.0f * 8.0f * 2.0f, "2-char text at scale 2.0 = 2 * 16px");

    renderer.shutdown();
}

// ─── InputState defaults ───────────────────────────────────────────────

void testInputState() {
    std::cout << "\n=== InputState Defaults ===" << std::endl;
    photon::InputState input;
    assertTrue(input.mouseDown[0] == false, "mouseDown[0] defaults to false");
    assertTrue(input.mouseClicked[0] == false, "mouseClicked[0] defaults to false");
    assertTrue(input.mouseReleased[0] == false, "mouseReleased[0] defaults to false");
    assertTrue(input.scrollY == 0.0f, "scrollY defaults to 0");
    assertTrue(input.windowW == 1280, "windowW defaults to 1280");
    assertTrue(input.windowH == 720, "windowH defaults to 720");
}

// ─── Tooltip rendering test ───────────────────────────────────────────

void testTooltip() {
    std::cout << "\n=== Tooltip ===" << std::endl;
    photon::PhotonContext ctx;
    ctx.init();

    photon::InputState input;
    input.windowW = 1920;
    input.windowH = 1080;
    input.mousePos = {500.0f, 400.0f};
    ctx.beginFrame(input);

    // Should not crash and should draw tooltip elements
    photon::tooltip(ctx, "This is a test tooltip");
    assertTrue(true, "Tooltip renders without crash");

    ctx.endFrame();
    ctx.shutdown();
}

// ─── Checkbox test ───────────────────────────────────────────────────

void testCheckbox() {
    std::cout << "\n=== Checkbox ===" << std::endl;
    photon::PhotonContext ctx;
    ctx.init();

    bool checked = false;
    photon::Rect cbRect(100, 100, 200, 20);

    // Frame 1: Click on checkbox
    {
        photon::InputState input;
        input.windowW = 1920;
        input.windowH = 1080;
        input.mousePos = {110.0f, 110.0f};  // Inside the checkbox box
        input.mouseClicked[0] = true;
        input.mouseDown[0] = true;
        ctx.beginFrame(input);
        photon::checkbox(ctx, "Test Check", cbRect, &checked);
        ctx.endFrame();
    }

    // Frame 2: Release on checkbox
    {
        photon::InputState input;
        input.windowW = 1920;
        input.windowH = 1080;
        input.mousePos = {110.0f, 110.0f};
        input.mouseReleased[0] = true;
        ctx.beginFrame(input);
        bool changed = photon::checkbox(ctx, "Test Check", cbRect, &checked);
        assertTrue(changed, "Checkbox value changes on click-release");
        assertTrue(checked, "Checkbox becomes checked after click");
        ctx.endFrame();
    }

    // Frame 3: Click again to uncheck
    {
        photon::InputState input;
        input.windowW = 1920;
        input.windowH = 1080;
        input.mousePos = {110.0f, 110.0f};
        input.mouseClicked[0] = true;
        input.mouseDown[0] = true;
        ctx.beginFrame(input);
        photon::checkbox(ctx, "Test Check", cbRect, &checked);
        ctx.endFrame();
    }
    {
        photon::InputState input;
        input.windowW = 1920;
        input.windowH = 1080;
        input.mousePos = {110.0f, 110.0f};
        input.mouseReleased[0] = true;
        ctx.beginFrame(input);
        bool changed = photon::checkbox(ctx, "Test Check", cbRect, &checked);
        assertTrue(changed, "Checkbox value changes on second click");
        assertTrue(!checked, "Checkbox becomes unchecked after second click");
        ctx.endFrame();
    }

    ctx.shutdown();
}

// ─── ComboBox test ─────────────────────────────────────────────────

void testComboBox() {
    std::cout << "\n=== ComboBox ===" << std::endl;
    photon::PhotonContext ctx;
    ctx.init();

    std::vector<std::string> items = {"All", "Combat", "Mining", "Custom"};
    int selected = 0;
    bool dropdownOpen = false;
    photon::Rect cbRect(100, 100, 200, 24);

    // Frame 1: Render combo in closed state
    {
        photon::InputState input;
        input.windowW = 1920;
        input.windowH = 1080;
        input.mousePos = {300.0f, 300.0f};  // Outside
        ctx.beginFrame(input);
        bool changed = photon::comboBox(ctx, "TestCombo", cbRect, items, &selected, &dropdownOpen);
        assertTrue(!changed, "ComboBox no change when not interacted with");
        assertTrue(!dropdownOpen, "ComboBox starts closed");
        ctx.endFrame();
    }

    assertTrue(selected == 0, "ComboBox initial selection is 0");

    ctx.shutdown();
}

// ─── PanelState test ──────────────────────────────────────────────────

void testPanelState() {
    std::cout << "\n=== PanelState ===" << std::endl;
    photon::PanelState state;
    state.bounds = {100, 100, 300, 400};
    assertTrue(state.open, "PanelState defaults to open");
    assertTrue(!state.minimized, "PanelState defaults to not minimized");
    assertTrue(!state.dragging, "PanelState defaults to not dragging");
    
    photon::PhotonContext ctx;
    ctx.init();

    // Render a stateful panel
    {
        photon::InputState input;
        input.windowW = 1920;
        input.windowH = 1080;
        input.mousePos = {500.0f, 500.0f};  // Outside panel
        ctx.beginFrame(input);
        photon::PanelFlags flags;
        bool contentVisible = photon::panelBeginStateful(ctx, "Test Panel", state, flags);
        assertTrue(contentVisible, "Stateful panel content is visible when open");
        photon::panelEnd(ctx);
        ctx.endFrame();
    }
    
    ctx.shutdown();
}

// ─── PhotonHUD test ────────────────────────────────────────────────────

void testPhotonHUD() {
    std::cout << "\n=== PhotonHUD ===" << std::endl;
    photon::PhotonContext ctx;
    ctx.init();

    photon::PhotonHUD hud;
    hud.init(1920, 1080);

    assertTrue(hud.isOverviewOpen(), "HUD overview defaults to open");
    assertTrue(hud.isSelectedItemOpen(), "HUD selected item defaults to open");

    // Toggle overview
    hud.toggleOverview();
    assertTrue(!hud.isOverviewOpen(), "HUD overview toggled to closed");
    hud.toggleOverview();
    assertTrue(hud.isOverviewOpen(), "HUD overview toggled back to open");

    // Render a full HUD frame
    photon::ShipHUDData ship;
    ship.shieldPct = 0.85f;
    ship.armorPct = 1.0f;
    ship.hullPct = 1.0f;
    ship.capacitorPct = 0.72f;
    ship.currentSpeed = 150.0f;
    ship.maxSpeed = 250.0f;
    ship.highSlots = {{true, true, 0.3f, {0.8f, 0.2f, 0.2f}},
                      {true, false, 0.0f, {0.8f, 0.2f, 0.2f}}};
    ship.midSlots = {{true, false, 0.0f, {0.2f, 0.6f, 1.0f}}};
    ship.lowSlots = {{true, false, 0.0f, {0.5f, 0.5f, 0.5f}}};

    std::vector<photon::TargetCardInfo> targets = {
        {"Pirate Frigate", 0.6f, 0.3f, 0.9f, 12000.0f, true, true},
        {"Asteroid", 1.0f, 1.0f, 1.0f, 5000.0f, false, false},
    };

    std::vector<photon::OverviewEntry> overview = {
        {"Pirate Frigate", "Frigate", 12000.0f, 350.0f, {0.8f, 0.2f, 0.2f}, true},
        {"Mining Barge", "Mining Barge", 5000.0f, 0.0f, {0.2f, 0.6f, 1.0f}, false},
        {"Station", "Station", 45000.0f, 0.0f, {0.667f, 0.667f, 0.667f}, false},
    };

    photon::SelectedItemInfo selected;
    selected.name = "Pirate Frigate";
    selected.distance = 12000.0f;
    selected.distanceUnit = "m";

    {
        photon::InputState input;
        input.windowW = 1920;
        input.windowH = 1080;
        input.mousePos = {960.0f, 540.0f};
        ctx.beginFrame(input);
        hud.update(ctx, ship, targets, overview, selected);
        ctx.endFrame();
    }

    assertTrue(true, "Full HUD renders without crash");

    // Test with module callback
    int clickedModule = -1;
    hud.setModuleCallback([&clickedModule](int idx) {
        clickedModule = idx;
    });
    assertTrue(true, "Module callback set without crash");

    // Test with neocom callback
    int clickedNeocom = -1;
    hud.setNeocomCallback([&clickedNeocom](int idx) {
        clickedNeocom = idx;
    });
    assertTrue(true, "Neocom callback set without crash");

    ctx.shutdown();
}

// ─── Main ──────────────────────────────────────────────────────────────

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "Photon UI System Tests" << std::endl;
    std::cout << "========================================" << std::endl;

    testVec2();
    testRect();
    testColor();
    testTheme();
    testHashID();
    testContext();
    testButtonBehavior();
    testTextMeasurement();
    testInputState();

    testTooltip();
    testCheckbox();
    testComboBox();
    testPanelState();
    testPhotonHUD();

    std::cout << "\n========================================" << std::endl;
    std::cout << "Results: " << testsPassed << "/" << testsRun
              << " tests passed" << std::endl;
    std::cout << "========================================" << std::endl;

    return (testsPassed == testsRun) ? 0 : 1;
}
