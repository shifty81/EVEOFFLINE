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

// ─── Slider test ───────────────────────────────────────────────────

void testSlider() {
    std::cout << "\n=== Slider ===" << std::endl;
    photon::PhotonContext ctx;
    ctx.init();

    float value = 50.0f;
    photon::Rect sliderRect(100, 100, 200, 20);

    // Frame 1: Render slider without interaction
    {
        photon::InputState input;
        input.windowW = 1920;
        input.windowH = 1080;
        input.mousePos = {300.0f, 300.0f};  // Outside
        ctx.beginFrame(input);
        bool changed = photon::slider(ctx, "TestSlider", sliderRect, &value, 0.0f, 100.0f, "%.0f");
        assertTrue(!changed, "Slider no change when not interacted with");
        assertClose(value, 50.0f, "Slider value unchanged");
        ctx.endFrame();
    }

    // Frame 2: Click inside slider track to set value
    {
        photon::InputState input;
        input.windowW = 1920;
        input.windowH = 1080;
        // Click at 75% of slider width (x=100 + 200*0.75 = 250)
        input.mousePos = {250.0f, 110.0f};
        input.mouseClicked[0] = true;
        input.mouseDown[0] = true;
        ctx.beginFrame(input);
        bool changed = photon::slider(ctx, "TestSlider", sliderRect, &value, 0.0f, 100.0f, "%.0f");
        assertTrue(changed, "Slider value changes on click");
        assertClose(value, 75.0f, "Slider set to 75% on click at 75% position");
        ctx.endFrame();
    }

    // Frame 3: Drag to new position
    {
        photon::InputState input;
        input.windowW = 1920;
        input.windowH = 1080;
        // Drag to 25% position (x=100 + 200*0.25 = 150)
        input.mousePos = {150.0f, 110.0f};
        input.mouseDown[0] = true;
        ctx.beginFrame(input);
        bool changed = photon::slider(ctx, "TestSlider", sliderRect, &value, 0.0f, 100.0f, "%.0f");
        assertTrue(changed, "Slider value changes on drag");
        assertClose(value, 25.0f, "Slider set to 25% on drag to 25% position");
        ctx.endFrame();
    }

    // Test with null value pointer (should not crash)
    {
        photon::InputState input;
        input.windowW = 1920;
        input.windowH = 1080;
        ctx.beginFrame(input);
        bool changed = photon::slider(ctx, "NullSlider", sliderRect, nullptr, 0.0f, 100.0f);
        assertTrue(!changed, "Slider with null value returns false");
        ctx.endFrame();
    }

    ctx.shutdown();
}

// ─── Text Input test ──────────────────────────────────────────────

void testTextInput() {
    std::cout << "\n=== TextInput ===" << std::endl;
    photon::PhotonContext ctx;
    ctx.init();

    photon::TextInputState inputState;
    inputState.text = "";
    photon::Rect inputRect(100, 100, 200, 24);

    // Frame 1: Render without interaction (unfocused)
    {
        photon::InputState input;
        input.windowW = 1920;
        input.windowH = 1080;
        input.mousePos = {300.0f, 300.0f};
        ctx.beginFrame(input);
        photon::textInput(ctx, "TestInput", inputRect, inputState, "Search...");
        assertTrue(!inputState.focused, "TextInput starts unfocused");
        ctx.endFrame();
    }

    // Frame 2: Click inside to focus
    {
        photon::InputState input;
        input.windowW = 1920;
        input.windowH = 1080;
        input.mousePos = {150.0f, 110.0f};
        input.mouseClicked[0] = true;
        input.mouseDown[0] = true;
        ctx.beginFrame(input);
        photon::textInput(ctx, "TestInput", inputRect, inputState, "Search...");
        assertTrue(inputState.focused, "TextInput focused after click inside");
        ctx.endFrame();
    }

    // Frame 3: Click outside to unfocus
    {
        photon::InputState input;
        input.windowW = 1920;
        input.windowH = 1080;
        input.mousePos = {500.0f, 500.0f};
        input.mouseClicked[0] = true;
        input.mouseDown[0] = true;
        ctx.beginFrame(input);
        photon::textInput(ctx, "TestInput", inputRect, inputState, "Search...");
        assertTrue(!inputState.focused, "TextInput unfocused after click outside");
        ctx.endFrame();
    }

    // Test with pre-filled text
    inputState.text = "Hello World";
    inputState.cursorPos = 5;
    {
        photon::InputState input;
        input.windowW = 1920;
        input.windowH = 1080;
        input.mousePos = {150.0f, 110.0f};
        input.mouseClicked[0] = true;
        ctx.beginFrame(input);
        photon::textInput(ctx, "TestInput", inputRect, inputState, "Search...");
        assertTrue(inputState.focused, "TextInput focuses with pre-filled text");
        assertTrue(inputState.text == "Hello World", "TextInput preserves existing text");
        ctx.endFrame();
    }

    ctx.shutdown();
}

// ─── Notification test ───────────────────────────────────────────

void testNotification() {
    std::cout << "\n=== Notification ===" << std::endl;
    photon::PhotonContext ctx;
    ctx.init();

    photon::InputState input;
    input.windowW = 1920;
    input.windowH = 1080;
    input.mousePos = {500.0f, 400.0f};
    ctx.beginFrame(input);

    // Should not crash with default color
    photon::notification(ctx, "Warp drive active");
    assertTrue(true, "Notification renders without crash (default color)");

    // Should not crash with custom color
    photon::notification(ctx, "Shield warning!", photon::Color(1.0f, 0.2f, 0.2f, 1.0f));
    assertTrue(true, "Notification renders without crash (custom color)");

    ctx.endFrame();
    ctx.shutdown();
}

// ─── TextInputState defaults test ──────────────────────────────────

void testTextInputStateDefaults() {
    std::cout << "\n=== TextInputState Defaults ===" << std::endl;
    photon::TextInputState state;
    assertTrue(state.text.empty(), "TextInputState text defaults to empty");
    assertTrue(state.cursorPos == 0, "TextInputState cursorPos defaults to 0");
    assertTrue(!state.focused, "TextInputState focused defaults to false");
}

// ─── Module Slot with Overheat test ────────────────────────────────

void testModuleSlotEx() {
    std::cout << "\n=== ModuleSlotEx (Overheat) ===" << std::endl;
    photon::PhotonContext ctx;
    ctx.init();

    {
        photon::InputState input;
        input.windowW = 1920;
        input.windowH = 1080;
        input.mousePos = {500.0f, 500.0f};  // Away from module
        ctx.beginFrame(input);

        // Module with no overheat
        bool clicked = photon::moduleSlotEx(ctx, {200.0f, 200.0f}, 14.0f,
                                             true, 0.5f,
                                             photon::Color(0.8f, 0.2f, 0.2f, 1.0f),
                                             0.0f, 1.0f);
        assertTrue(!clicked, "ModuleSlotEx not clicked when mouse is away");

        // Module with moderate overheat
        clicked = photon::moduleSlotEx(ctx, {250.0f, 200.0f}, 14.0f,
                                       true, 0.0f,
                                       photon::Color(0.8f, 0.2f, 0.2f, 1.0f),
                                       0.5f, 2.0f);
        assertTrue(!clicked, "ModuleSlotEx with 50% overheat renders without crash");

        // Module fully burnt out
        clicked = photon::moduleSlotEx(ctx, {300.0f, 200.0f}, 14.0f,
                                       false, 0.0f,
                                       photon::Color(0.5f, 0.5f, 0.5f, 1.0f),
                                       1.0f, 3.0f);
        assertTrue(!clicked, "ModuleSlotEx at 100% overheat (burnt out) renders");

        ctx.endFrame();
    }

    ctx.shutdown();
}

// ─── Capacitor Ring Animated test ──────────────────────────────────

void testCapacitorRingAnimated() {
    std::cout << "\n=== CapacitorRingAnimated ===" << std::endl;
    photon::PhotonContext ctx;
    ctx.init();

    float displayFrac = 1.0f;  // Start at full cap

    {
        photon::InputState input;
        input.windowW = 1920;
        input.windowH = 1080;
        ctx.beginFrame(input);

        // Animate toward 50% over several frames
        photon::capacitorRingAnimated(ctx, {960.0f, 540.0f}, 40.0f, 48.0f,
                                       0.5f, displayFrac, 1.0f / 60.0f, 16);
        assertTrue(displayFrac < 1.0f, "Display frac moves toward target after one frame");
        assertTrue(displayFrac > 0.5f, "Display frac hasn't reached target in one frame");

        ctx.endFrame();
    }

    // Simulate many frames to converge
    for (int i = 0; i < 300; ++i) {
        photon::InputState input;
        input.windowW = 1920;
        input.windowH = 1080;
        ctx.beginFrame(input);
        photon::capacitorRingAnimated(ctx, {960.0f, 540.0f}, 40.0f, 48.0f,
                                       0.5f, displayFrac, 1.0f / 60.0f, 16);
        ctx.endFrame();
    }
    assertClose(displayFrac, 0.5f, "Display frac converges to target after many frames", 0.01f);

    // Test snap-to-target when very close
    displayFrac = 0.5005f;
    {
        photon::InputState input;
        input.windowW = 1920;
        input.windowH = 1080;
        ctx.beginFrame(input);
        photon::capacitorRingAnimated(ctx, {960.0f, 540.0f}, 40.0f, 48.0f,
                                       0.5f, displayFrac, 1.0f / 60.0f, 16);
        assertClose(displayFrac, 0.5f, "Display frac snaps when diff < 0.001");
        ctx.endFrame();
    }

    ctx.shutdown();
}

// ─── ModuleInfo Overheat Field test ─────────────────────────────────

void testModuleInfoOverheat() {
    std::cout << "\n=== ModuleInfo Overheat Field ===" << std::endl;

    // Test that overheat defaults to 0
    photon::ShipHUDData::ModuleInfo mod;
    assertClose(mod.overheat, 0.0f, "ModuleInfo overheat defaults to 0.0");
    assertTrue(mod.fitted == false, "ModuleInfo fitted defaults to false");
    assertTrue(mod.active == false, "ModuleInfo active defaults to false");
    assertClose(mod.cooldown, 0.0f, "ModuleInfo cooldown defaults to 0.0");

    // Test backward-compatible aggregate init (existing code style)
    photon::ShipHUDData::ModuleInfo mod2 = {true, true, 0.3f, {0.8f, 0.2f, 0.2f, 1.0f}};
    assertTrue(mod2.fitted == true, "Aggregate init: fitted");
    assertTrue(mod2.active == true, "Aggregate init: active");
    assertClose(mod2.cooldown, 0.3f, "Aggregate init: cooldown");
    assertClose(mod2.overheat, 0.0f, "Aggregate init: overheat defaults to 0 (backward compat)");
}

// ─── RmlUiManager Data Structure tests ─────────────────────────────

#include "ui/rml_ui_manager.h"

void testFittingRmlData() {
    std::cout << "\n=== FittingRmlData ===" << std::endl;

    UI::RmlUiManager::FittingSlotInfo slot;
    assertTrue(slot.name.empty(), "FittingSlotInfo name defaults to empty");
    assertTrue(slot.online == false, "FittingSlotInfo online defaults to false");

    UI::RmlUiManager::FittingRmlData data;
    assertTrue(data.shipName.empty(), "FittingRmlData shipName defaults to empty");
    assertTrue(data.highSlots.empty(), "FittingRmlData highSlots defaults to empty");
    assertTrue(data.midSlots.empty(), "FittingRmlData midSlots defaults to empty");
    assertTrue(data.lowSlots.empty(), "FittingRmlData lowSlots defaults to empty");
    assertClose(data.cpuUsed, 0.0f, "FittingRmlData cpuUsed defaults to 0");
    assertClose(data.cpuMax, 1.0f, "FittingRmlData cpuMax defaults to 1");
    assertClose(data.pgUsed, 0.0f, "FittingRmlData pgUsed defaults to 0");
    assertClose(data.pgMax, 1.0f, "FittingRmlData pgMax defaults to 1");
    assertClose(data.ehp, 0.0f, "FittingRmlData ehp defaults to 0");
    assertClose(data.dps, 0.0f, "FittingRmlData dps defaults to 0");
    assertTrue(data.capStable == false, "FittingRmlData capStable defaults to false");

    // Populate and verify
    data.shipName = "Rifter";
    data.highSlots.push_back({"200mm AC", true});
    data.highSlots.push_back({"200mm AC", true});
    data.midSlots.push_back({"1MN AB", true});
    data.lowSlots.push_back({"Gyro", true});
    data.cpuUsed = 85.0f;
    data.cpuMax = 120.0f;
    data.pgUsed = 42.5f;
    data.pgMax = 50.0f;
    data.ehp = 4250.0f;
    data.dps = 185.0f;
    data.maxVelocity = 380.0f;
    data.capStable = true;

    assertTrue(data.shipName == "Rifter", "FittingRmlData shipName set correctly");
    assertTrue(data.highSlots.size() == 2, "FittingRmlData has 2 high slots");
    assertTrue(data.highSlots[0].name == "200mm AC", "High slot 0 name correct");
    assertTrue(data.highSlots[0].online == true, "High slot 0 online correct");
    assertClose(data.cpuUsed, 85.0f, "FittingRmlData cpuUsed set correctly");
    assertClose(data.ehp, 4250.0f, "FittingRmlData ehp set correctly");
    assertTrue(data.capStable == true, "FittingRmlData capStable set correctly");
}

void testMarketOrderInfo() {
    std::cout << "\n=== MarketOrderInfo ===" << std::endl;

    UI::RmlUiManager::MarketOrderInfo order;
    assertClose(order.price, 0.0f, "MarketOrderInfo price defaults to 0");
    assertTrue(order.quantity == 0, "MarketOrderInfo quantity defaults to 0");
    assertTrue(order.location.empty(), "MarketOrderInfo location defaults to empty");

    order.price = 15000.50f;
    order.quantity = 100;
    order.location = "Jita IV - Moon 4";
    assertClose(order.price, 15000.50f, "MarketOrderInfo price set correctly");
    assertTrue(order.quantity == 100, "MarketOrderInfo quantity set correctly");
    assertTrue(order.location == "Jita IV - Moon 4", "MarketOrderInfo location set correctly");
}

void testMissionRmlInfo() {
    std::cout << "\n=== MissionRmlInfo ===" << std::endl;

    UI::RmlUiManager::MissionObjectiveInfo obj;
    assertTrue(obj.text.empty(), "MissionObjectiveInfo text defaults to empty");
    assertTrue(obj.complete == false, "MissionObjectiveInfo complete defaults to false");

    UI::RmlUiManager::MissionRmlInfo mission;
    assertTrue(mission.title.empty(), "MissionRmlInfo title defaults to empty");
    assertTrue(mission.objectives.empty(), "MissionRmlInfo objectives defaults to empty");
    assertClose(mission.iskReward, 0.0f, "MissionRmlInfo iskReward defaults to 0");
    assertTrue(mission.lpReward == 0, "MissionRmlInfo lpReward defaults to 0");

    mission.title = "Crimson Order Assault";
    mission.agentName = "Commander Voss";
    mission.level = "L3 Security";
    mission.description = "Eliminate hostiles near Keldari station.";
    mission.objectives.push_back({"Warp to site", true});
    mission.objectives.push_back({"Destroy vessels", false});
    mission.iskReward = 450000.0f;
    mission.bonusIsk = 150000.0f;
    mission.standingReward = "+0.15 Keldari Navy";
    mission.lpReward = 800;

    assertTrue(mission.title == "Crimson Order Assault", "MissionRmlInfo title set correctly");
    assertTrue(mission.objectives.size() == 2, "MissionRmlInfo has 2 objectives");
    assertTrue(mission.objectives[0].complete == true, "Objective 0 is complete");
    assertTrue(mission.objectives[1].complete == false, "Objective 1 is incomplete");
    assertClose(mission.iskReward, 450000.0f, "MissionRmlInfo iskReward set correctly");
    assertTrue(mission.lpReward == 800, "MissionRmlInfo lpReward set correctly");
}

void testChatMessageInfo() {
    std::cout << "\n=== ChatMessageInfo ===" << std::endl;

    UI::RmlUiManager::ChatMessageInfo msg;
    assertTrue(msg.time.empty(), "ChatMessageInfo time defaults to empty");
    assertTrue(msg.sender.empty(), "ChatMessageInfo sender defaults to empty");
    assertTrue(msg.text.empty(), "ChatMessageInfo text defaults to empty");
    assertTrue(msg.senderClass.empty(), "ChatMessageInfo senderClass defaults to empty");

    msg.time = "12:34";
    msg.sender = "Player1";
    msg.text = "Hello world";
    msg.senderClass = "self";

    assertTrue(msg.time == "12:34", "ChatMessageInfo time set correctly");
    assertTrue(msg.sender == "Player1", "ChatMessageInfo sender set correctly");
    assertTrue(msg.text == "Hello world", "ChatMessageInfo text set correctly");
    assertTrue(msg.senderClass == "self", "ChatMessageInfo senderClass set correctly");
}

void testRmlUiManagerStub() {
    std::cout << "\n=== RmlUiManager Stub ===" << std::endl;

    UI::RmlUiManager mgr;
    assertTrue(!mgr.IsInitialized(), "RmlUiManager starts uninitialized");

    // All stubs should be callable without crash
    mgr.SetShipStatus(UI::ShipStatusData{});
    assertTrue(true, "SetShipStatus stub callable");

    mgr.SetTarget("t1", "Test", 1.0f, 1.0f, 1.0f, 100.0f, false, false);
    mgr.RemoveTarget("t1");
    mgr.ClearTargets();
    assertTrue(true, "Target stubs callable");

    mgr.AddCombatLogMessage("test");
    assertTrue(true, "AddCombatLogMessage stub callable");

    mgr.UpdateInventoryData({}, {}, {}, {}, 0.0f, 0.0f);
    assertTrue(true, "UpdateInventoryData stub callable");

    mgr.UpdateDScanResults({}, {}, {});
    assertTrue(true, "UpdateDScanResults stub callable");

    mgr.UpdateDroneBayData({}, {}, 0, 0, 0.0f, 0.0f);
    assertTrue(true, "UpdateDroneBayData stub callable");

    mgr.UpdateFittingData(UI::RmlUiManager::FittingRmlData{});
    assertTrue(true, "UpdateFittingData stub callable");

    mgr.UpdateMarketData("", "", {}, {});
    assertTrue(true, "UpdateMarketData stub callable");

    mgr.UpdateMissionList({});
    assertTrue(true, "UpdateMissionList stub callable");

    mgr.UpdateMissionDetail(UI::RmlUiManager::MissionRmlInfo{});
    assertTrue(true, "UpdateMissionDetail stub callable");

    mgr.AddChatMessage(UI::RmlUiManager::ChatMessageInfo{});
    assertTrue(true, "AddChatMessage stub callable");

    mgr.SetChatChannel("local", 5);
    assertTrue(true, "SetChatChannel stub callable");

    mgr.ShowContextMenu("Entity", "Frigate", 100.0f, 200.0f);
    assertTrue(true, "ShowContextMenu stub callable");

    mgr.HideContextMenu();
    assertTrue(true, "HideContextMenu stub callable");

    assertTrue(!mgr.WantsMouseInput(), "WantsMouseInput returns false when uninitialized");
    assertTrue(!mgr.WantsKeyboardInput(), "WantsKeyboardInput returns false when uninitialized");
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

    testSlider();
    testTextInput();
    testNotification();
    testTextInputStateDefaults();
    testModuleSlotEx();
    testCapacitorRingAnimated();
    testModuleInfoOverheat();

    testFittingRmlData();
    testMarketOrderInfo();
    testMissionRmlInfo();
    testChatMessageInfo();
    testRmlUiManagerStub();

    std::cout << "\n========================================" << std::endl;
    std::cout << "Results: " << testsPassed << "/" << testsRun
              << " tests passed" << std::endl;
    std::cout << "========================================" << std::endl;

    return (testsPassed == testsRun) ? 0 : 1;
}
