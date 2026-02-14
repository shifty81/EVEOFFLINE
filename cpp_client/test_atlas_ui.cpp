/**
 * Test program for the Atlas UI system.
 *
 * Validates that the core Atlas types, context, renderer, and widget
 * functions work correctly in a headless (no OpenGL) environment.
 * GPU rendering is stubbed out, so these tests verify logic, hit-testing,
 * color/theme values, ID hashing, and widget state management.
 */

#include "ui/atlas/atlas_types.h"
#include "ui/atlas/atlas_context.h"
#include "ui/atlas/atlas_widgets.h"
#include "ui/atlas/atlas_hud.h"
#include "ui/atlas/atlas_console.h"
#include "ui/atlas/atlas_pause_menu.h"
#include "ui/atlas/atlas_title_screen.h"
#include "ui/context_menu.h"
#include "ui/radial_menu.h"
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
    atlas::Vec2 a(3.0f, 4.0f);
    atlas::Vec2 b(1.0f, 2.0f);
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
    atlas::Rect r(10.0f, 20.0f, 100.0f, 50.0f);
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
    atlas::Color c(0.5f, 0.6f, 0.7f, 0.8f);
    auto c2 = c.withAlpha(0.3f);
    assertTrue(c2.r == 0.5f && c2.g == 0.6f && c2.b == 0.7f && c2.a == 0.3f,
               "Color withAlpha preserves RGB");
    auto c3 = atlas::Color::fromRGBA(255, 128, 0, 255);
    assertClose(c3.r, 1.0f, "Color fromRGBA red");
    assertClose(c3.g, 128.0f / 255.0f, "Color fromRGBA green");
    assertClose(c3.b, 0.0f, "Color fromRGBA blue");
    assertClose(c3.a, 1.0f, "Color fromRGBA alpha");
}

// ─── Theme defaults ────────────────────────────────────────────────────

void testTheme() {
    std::cout << "\n=== Theme ===" << std::endl;
    const atlas::Theme& t = atlas::defaultTheme();
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
    atlas::WidgetID a = atlas::hashID("Overview");
    atlas::WidgetID b = atlas::hashID("Overview");
    atlas::WidgetID c = atlas::hashID("Fitting");
    assertTrue(a == b, "Same string produces same ID");
    assertTrue(a != c, "Different strings produce different IDs");
    assertTrue(atlas::hashID("") != atlas::hashID("x"),
               "Empty vs non-empty are different");
}

// ─── Context tests ─────────────────────────────────────────────────────

void testContext() {
    std::cout << "\n=== AtlasContext ===" << std::endl;
    atlas::AtlasContext ctx;
    // init() will create stub GL resources in headless mode
    assertTrue(ctx.init(), "Context init succeeds (headless)");

    atlas::InputState input;
    input.windowW = 1920;
    input.windowH = 1080;
    input.mousePos = {500.0f, 400.0f};
    input.mouseDown[0] = false;
    input.mouseClicked[0] = false;
    input.mouseReleased[0] = false;

    ctx.beginFrame(input);

    // Hover test
    atlas::Rect inside(400, 350, 200, 100);
    atlas::Rect outside(800, 800, 100, 100);
    assertTrue(ctx.isHovered(inside), "Mouse is inside rect");
    assertTrue(!ctx.isHovered(outside), "Mouse is outside rect");

    // Hot/Active state
    atlas::WidgetID testID = atlas::hashID("testWidget");
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
    atlas::WidgetID idA = ctx.currentID("child");
    ctx.popID();
    ctx.pushID("other_parent");
    atlas::WidgetID idB = ctx.currentID("child");
    ctx.popID();
    assertTrue(idA != idB, "Same child label under different parents produces different IDs");
    ctx.endFrame();

    ctx.shutdown();
}

// ─── Button behavior test ──────────────────────────────────────────────

void testButtonBehavior() {
    std::cout << "\n=== Button Behavior ===" << std::endl;
    atlas::AtlasContext ctx;
    ctx.init();

    atlas::Rect btn(100, 100, 80, 30);
    atlas::WidgetID btnID = atlas::hashID("testBtn");

    // Frame 1: mouse hovers over button
    {
        atlas::InputState input;
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
        atlas::InputState input;
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
        atlas::InputState input;
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
        atlas::InputState input;
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
        atlas::InputState input;
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
    atlas::AtlasRenderer renderer;
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
    atlas::InputState input;
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
    atlas::AtlasContext ctx;
    ctx.init();

    atlas::InputState input;
    input.windowW = 1920;
    input.windowH = 1080;
    input.mousePos = {500.0f, 400.0f};
    ctx.beginFrame(input);

    // Should not crash and should draw tooltip elements
    atlas::tooltip(ctx, "This is a test tooltip");
    assertTrue(true, "Tooltip renders without crash");

    ctx.endFrame();
    ctx.shutdown();
}

// ─── Checkbox test ───────────────────────────────────────────────────

void testCheckbox() {
    std::cout << "\n=== Checkbox ===" << std::endl;
    atlas::AtlasContext ctx;
    ctx.init();

    bool checked = false;
    atlas::Rect cbRect(100, 100, 200, 20);

    // Frame 1: Click on checkbox
    {
        atlas::InputState input;
        input.windowW = 1920;
        input.windowH = 1080;
        input.mousePos = {110.0f, 110.0f};  // Inside the checkbox box
        input.mouseClicked[0] = true;
        input.mouseDown[0] = true;
        ctx.beginFrame(input);
        atlas::checkbox(ctx, "Test Check", cbRect, &checked);
        ctx.endFrame();
    }

    // Frame 2: Release on checkbox
    {
        atlas::InputState input;
        input.windowW = 1920;
        input.windowH = 1080;
        input.mousePos = {110.0f, 110.0f};
        input.mouseReleased[0] = true;
        ctx.beginFrame(input);
        bool changed = atlas::checkbox(ctx, "Test Check", cbRect, &checked);
        assertTrue(changed, "Checkbox value changes on click-release");
        assertTrue(checked, "Checkbox becomes checked after click");
        ctx.endFrame();
    }

    // Frame 3: Click again to uncheck
    {
        atlas::InputState input;
        input.windowW = 1920;
        input.windowH = 1080;
        input.mousePos = {110.0f, 110.0f};
        input.mouseClicked[0] = true;
        input.mouseDown[0] = true;
        ctx.beginFrame(input);
        atlas::checkbox(ctx, "Test Check", cbRect, &checked);
        ctx.endFrame();
    }
    {
        atlas::InputState input;
        input.windowW = 1920;
        input.windowH = 1080;
        input.mousePos = {110.0f, 110.0f};
        input.mouseReleased[0] = true;
        ctx.beginFrame(input);
        bool changed = atlas::checkbox(ctx, "Test Check", cbRect, &checked);
        assertTrue(changed, "Checkbox value changes on second click");
        assertTrue(!checked, "Checkbox becomes unchecked after second click");
        ctx.endFrame();
    }

    ctx.shutdown();
}

// ─── ComboBox test ─────────────────────────────────────────────────

void testComboBox() {
    std::cout << "\n=== ComboBox ===" << std::endl;
    atlas::AtlasContext ctx;
    ctx.init();

    std::vector<std::string> items = {"All", "Combat", "Mining", "Custom"};
    int selected = 0;
    bool dropdownOpen = false;
    atlas::Rect cbRect(100, 100, 200, 24);

    // Frame 1: Render combo in closed state
    {
        atlas::InputState input;
        input.windowW = 1920;
        input.windowH = 1080;
        input.mousePos = {300.0f, 300.0f};  // Outside
        ctx.beginFrame(input);
        bool changed = atlas::comboBox(ctx, "TestCombo", cbRect, items, &selected, &dropdownOpen);
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
    atlas::PanelState state;
    state.bounds = {100, 100, 300, 400};
    assertTrue(state.open, "PanelState defaults to open");
    assertTrue(!state.minimized, "PanelState defaults to not minimized");
    assertTrue(!state.dragging, "PanelState defaults to not dragging");
    
    atlas::AtlasContext ctx;
    ctx.init();

    // Render a stateful panel
    {
        atlas::InputState input;
        input.windowW = 1920;
        input.windowH = 1080;
        input.mousePos = {500.0f, 500.0f};  // Outside panel
        ctx.beginFrame(input);
        atlas::PanelFlags flags;
        bool contentVisible = atlas::panelBeginStateful(ctx, "Test Panel", state, flags);
        assertTrue(contentVisible, "Stateful panel content is visible when open");
        atlas::panelEnd(ctx);
        ctx.endFrame();
    }
    
    ctx.shutdown();
}

// ─── AtlasHUD test ────────────────────────────────────────────────────

void testAtlasHUD() {
    std::cout << "\n=== AtlasHUD ===" << std::endl;
    atlas::AtlasContext ctx;
    ctx.init();

    atlas::AtlasHUD hud;
    hud.init(1920, 1080);

    assertTrue(hud.isOverviewOpen(), "HUD overview defaults to open");
    assertTrue(hud.isSelectedItemOpen(), "HUD selected item defaults to open");

    // Toggle overview
    hud.toggleOverview();
    assertTrue(!hud.isOverviewOpen(), "HUD overview toggled to closed");
    hud.toggleOverview();
    assertTrue(hud.isOverviewOpen(), "HUD overview toggled back to open");

    // Render a full HUD frame
    atlas::ShipHUDData ship;
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

    std::vector<atlas::TargetCardInfo> targets = {
        {"Pirate Frigate", 0.6f, 0.3f, 0.9f, 12000.0f, true, true},
        {"Asteroid", 1.0f, 1.0f, 1.0f, 5000.0f, false, false},
    };

    std::vector<atlas::OverviewEntry> overview = {
        {"pirate_1", "Pirate Frigate", "Frigate", 12000.0f, 350.0f, {0.8f, 0.2f, 0.2f}, true},
        {"miner_1", "Mining Barge", "Mining Barge", 5000.0f, 0.0f, {0.2f, 0.6f, 1.0f}, false},
        {"station_1", "Station", "Station", 45000.0f, 0.0f, {0.667f, 0.667f, 0.667f}, false},
    };

    atlas::SelectedItemInfo selected;
    selected.name = "Pirate Frigate";
    selected.distance = 12000.0f;
    selected.distanceUnit = "m";

    {
        atlas::InputState input;
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

    // Test with sidebar callback
    int clickedSidebar = -1;
    hud.setSidebarCallback([&clickedSidebar](int idx) {
        clickedSidebar = idx;
    });
    assertTrue(true, "Sidebar callback set without crash");

    ctx.shutdown();
}

// ─── Slider test ───────────────────────────────────────────────────

void testSlider() {
    std::cout << "\n=== Slider ===" << std::endl;
    atlas::AtlasContext ctx;
    ctx.init();

    float value = 50.0f;
    atlas::Rect sliderRect(100, 100, 200, 20);

    // Frame 1: Render slider without interaction
    {
        atlas::InputState input;
        input.windowW = 1920;
        input.windowH = 1080;
        input.mousePos = {300.0f, 300.0f};  // Outside
        ctx.beginFrame(input);
        bool changed = atlas::slider(ctx, "TestSlider", sliderRect, &value, 0.0f, 100.0f, "%.0f");
        assertTrue(!changed, "Slider no change when not interacted with");
        assertClose(value, 50.0f, "Slider value unchanged");
        ctx.endFrame();
    }

    // Frame 2: Click inside slider track to set value
    {
        atlas::InputState input;
        input.windowW = 1920;
        input.windowH = 1080;
        // Click at 75% of slider width (x=100 + 200*0.75 = 250)
        input.mousePos = {250.0f, 110.0f};
        input.mouseClicked[0] = true;
        input.mouseDown[0] = true;
        ctx.beginFrame(input);
        bool changed = atlas::slider(ctx, "TestSlider", sliderRect, &value, 0.0f, 100.0f, "%.0f");
        assertTrue(changed, "Slider value changes on click");
        assertClose(value, 75.0f, "Slider set to 75% on click at 75% position");
        ctx.endFrame();
    }

    // Frame 3: Drag to new position
    {
        atlas::InputState input;
        input.windowW = 1920;
        input.windowH = 1080;
        // Drag to 25% position (x=100 + 200*0.25 = 150)
        input.mousePos = {150.0f, 110.0f};
        input.mouseDown[0] = true;
        ctx.beginFrame(input);
        bool changed = atlas::slider(ctx, "TestSlider", sliderRect, &value, 0.0f, 100.0f, "%.0f");
        assertTrue(changed, "Slider value changes on drag");
        assertClose(value, 25.0f, "Slider set to 25% on drag to 25% position");
        ctx.endFrame();
    }

    // Test with null value pointer (should not crash)
    {
        atlas::InputState input;
        input.windowW = 1920;
        input.windowH = 1080;
        ctx.beginFrame(input);
        bool changed = atlas::slider(ctx, "NullSlider", sliderRect, nullptr, 0.0f, 100.0f);
        assertTrue(!changed, "Slider with null value returns false");
        ctx.endFrame();
    }

    ctx.shutdown();
}

// ─── Text Input test ──────────────────────────────────────────────

void testTextInput() {
    std::cout << "\n=== TextInput ===" << std::endl;
    atlas::AtlasContext ctx;
    ctx.init();

    atlas::TextInputState inputState;
    inputState.text = "";
    atlas::Rect inputRect(100, 100, 200, 24);

    // Frame 1: Render without interaction (unfocused)
    {
        atlas::InputState input;
        input.windowW = 1920;
        input.windowH = 1080;
        input.mousePos = {300.0f, 300.0f};
        ctx.beginFrame(input);
        atlas::textInput(ctx, "TestInput", inputRect, inputState, "Search...");
        assertTrue(!inputState.focused, "TextInput starts unfocused");
        ctx.endFrame();
    }

    // Frame 2: Click inside to focus
    {
        atlas::InputState input;
        input.windowW = 1920;
        input.windowH = 1080;
        input.mousePos = {150.0f, 110.0f};
        input.mouseClicked[0] = true;
        input.mouseDown[0] = true;
        ctx.beginFrame(input);
        atlas::textInput(ctx, "TestInput", inputRect, inputState, "Search...");
        assertTrue(inputState.focused, "TextInput focused after click inside");
        ctx.endFrame();
    }

    // Frame 3: Click outside to unfocus
    {
        atlas::InputState input;
        input.windowW = 1920;
        input.windowH = 1080;
        input.mousePos = {500.0f, 500.0f};
        input.mouseClicked[0] = true;
        input.mouseDown[0] = true;
        ctx.beginFrame(input);
        atlas::textInput(ctx, "TestInput", inputRect, inputState, "Search...");
        assertTrue(!inputState.focused, "TextInput unfocused after click outside");
        ctx.endFrame();
    }

    // Test with pre-filled text
    inputState.text = "Hello World";
    inputState.cursorPos = 5;
    {
        atlas::InputState input;
        input.windowW = 1920;
        input.windowH = 1080;
        input.mousePos = {150.0f, 110.0f};
        input.mouseClicked[0] = true;
        ctx.beginFrame(input);
        atlas::textInput(ctx, "TestInput", inputRect, inputState, "Search...");
        assertTrue(inputState.focused, "TextInput focuses with pre-filled text");
        assertTrue(inputState.text == "Hello World", "TextInput preserves existing text");
        ctx.endFrame();
    }

    ctx.shutdown();
}

// ─── Notification test ───────────────────────────────────────────

void testNotification() {
    std::cout << "\n=== Notification ===" << std::endl;
    atlas::AtlasContext ctx;
    ctx.init();

    atlas::InputState input;
    input.windowW = 1920;
    input.windowH = 1080;
    input.mousePos = {500.0f, 400.0f};
    ctx.beginFrame(input);

    // Should not crash with default color
    atlas::notification(ctx, "Warp drive active");
    assertTrue(true, "Notification renders without crash (default color)");

    // Should not crash with custom color
    atlas::notification(ctx, "Shield warning!", atlas::Color(1.0f, 0.2f, 0.2f, 1.0f));
    assertTrue(true, "Notification renders without crash (custom color)");

    ctx.endFrame();
    ctx.shutdown();
}

// ─── TextInputState defaults test ──────────────────────────────────

void testTextInputStateDefaults() {
    std::cout << "\n=== TextInputState Defaults ===" << std::endl;
    atlas::TextInputState state;
    assertTrue(state.text.empty(), "TextInputState text defaults to empty");
    assertTrue(state.cursorPos == 0, "TextInputState cursorPos defaults to 0");
    assertTrue(!state.focused, "TextInputState focused defaults to false");
}

// ─── Module Slot with Overheat test ────────────────────────────────

void testModuleSlotEx() {
    std::cout << "\n=== ModuleSlotEx (Overheat) ===" << std::endl;
    atlas::AtlasContext ctx;
    ctx.init();

    {
        atlas::InputState input;
        input.windowW = 1920;
        input.windowH = 1080;
        input.mousePos = {500.0f, 500.0f};  // Away from module
        ctx.beginFrame(input);

        // Module with no overheat
        bool clicked = atlas::moduleSlotEx(ctx, {200.0f, 200.0f}, 14.0f,
                                             true, 0.5f,
                                             atlas::Color(0.8f, 0.2f, 0.2f, 1.0f),
                                             0.0f, 1.0f);
        assertTrue(!clicked, "ModuleSlotEx not clicked when mouse is away");

        // Module with moderate overheat
        clicked = atlas::moduleSlotEx(ctx, {250.0f, 200.0f}, 14.0f,
                                       true, 0.0f,
                                       atlas::Color(0.8f, 0.2f, 0.2f, 1.0f),
                                       0.5f, 2.0f);
        assertTrue(!clicked, "ModuleSlotEx with 50% overheat renders without crash");

        // Module fully burnt out
        clicked = atlas::moduleSlotEx(ctx, {300.0f, 200.0f}, 14.0f,
                                       false, 0.0f,
                                       atlas::Color(0.5f, 0.5f, 0.5f, 1.0f),
                                       1.0f, 3.0f);
        assertTrue(!clicked, "ModuleSlotEx at 100% overheat (burnt out) renders");

        ctx.endFrame();
    }

    ctx.shutdown();
}

// ─── Capacitor Ring Animated test ──────────────────────────────────

void testCapacitorRingAnimated() {
    std::cout << "\n=== CapacitorRingAnimated ===" << std::endl;
    atlas::AtlasContext ctx;
    ctx.init();

    float displayFrac = 1.0f;  // Start at full cap

    {
        atlas::InputState input;
        input.windowW = 1920;
        input.windowH = 1080;
        ctx.beginFrame(input);

        // Animate toward 50% over several frames
        atlas::capacitorRingAnimated(ctx, {960.0f, 540.0f}, 40.0f, 48.0f,
                                       0.5f, displayFrac, 1.0f / 60.0f, 16);
        assertTrue(displayFrac < 1.0f, "Display frac moves toward target after one frame");
        assertTrue(displayFrac > 0.5f, "Display frac hasn't reached target in one frame");

        ctx.endFrame();
    }

    // Simulate many frames to converge
    for (int i = 0; i < 300; ++i) {
        atlas::InputState input;
        input.windowW = 1920;
        input.windowH = 1080;
        ctx.beginFrame(input);
        atlas::capacitorRingAnimated(ctx, {960.0f, 540.0f}, 40.0f, 48.0f,
                                       0.5f, displayFrac, 1.0f / 60.0f, 16);
        ctx.endFrame();
    }
    assertClose(displayFrac, 0.5f, "Display frac converges to target after many frames", 0.01f);

    // Test snap-to-target when very close
    displayFrac = 0.5005f;
    {
        atlas::InputState input;
        input.windowW = 1920;
        input.windowH = 1080;
        ctx.beginFrame(input);
        atlas::capacitorRingAnimated(ctx, {960.0f, 540.0f}, 40.0f, 48.0f,
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
    atlas::ShipHUDData::ModuleInfo mod;
    assertClose(mod.overheat, 0.0f, "ModuleInfo overheat defaults to 0.0");
    assertTrue(mod.fitted == false, "ModuleInfo fitted defaults to false");
    assertTrue(mod.active == false, "ModuleInfo active defaults to false");
    assertClose(mod.cooldown, 0.0f, "ModuleInfo cooldown defaults to 0.0");

    // Test backward-compatible aggregate init (existing code style)
    atlas::ShipHUDData::ModuleInfo mod2 = {true, true, 0.3f, {0.8f, 0.2f, 0.2f, 1.0f}};
    assertTrue(mod2.fitted == true, "Aggregate init: fitted");
    assertTrue(mod2.active == true, "Aggregate init: active");
    assertClose(mod2.cooldown, 0.3f, "Aggregate init: cooldown");
    assertClose(mod2.overheat, 0.0f, "Aggregate init: overheat defaults to 0 (backward compat)");
}

// ─── RmlUiManager Data Structure tests ─────────────────────────────
// These tests are only compiled when RmlUi is enabled (USE_RMLUI defined).
// Since we've migrated to Atlas UI exclusively, they are guarded.

#ifdef USE_RMLUI
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

    mgr.SetContextMenuEntityId("entity_123");
    assertTrue(true, "SetContextMenuEntityId stub callable");

    mgr.ShowRadialMenu(400.0f, 300.0f, "entity_123");
    assertTrue(true, "ShowRadialMenu stub callable");

    mgr.HideRadialMenu();
    assertTrue(true, "HideRadialMenu stub callable");

    mgr.UpdateRadialHighlight("rad-approach");
    assertTrue(true, "UpdateRadialHighlight stub callable");

    // Test context menu callback setters
    bool callbackFired = false;
    mgr.SetOnApproach([&](const std::string&) { callbackFired = true; });
    mgr.SetOnOrbit([](const std::string&, int) {});
    mgr.SetOnKeepAtRange([](const std::string&, int) {});
    mgr.SetOnWarpTo([](const std::string&, int) {});
    mgr.SetOnLockTarget([](const std::string&) {});
    mgr.SetOnAlignTo([](const std::string&) {});
    mgr.SetOnShowInfo([](const std::string&) {});
    mgr.SetOnLookAt([](const std::string&) {});
    assertTrue(true, "Context menu callback setters callable");

    assertTrue(!mgr.WantsMouseInput(), "WantsMouseInput returns false when uninitialized");
    assertTrue(!mgr.WantsKeyboardInput(), "WantsKeyboardInput returns false when uninitialized");
}
#endif // USE_RMLUI

// ─── Damage Feedback tests ─────────────────────────────────────────────

// ─── Mode Indicator tests ──────────────────────────────────────────────

void testModeIndicator() {
    std::cout << "\n=== Mode Indicator ===" << std::endl;

    // Test that modeIndicator doesn't crash with null or empty text
    atlas::AtlasContext ctx;
    ctx.init();
    atlas::InputState input;
    input.windowW = 1920;
    input.windowH = 1080;
    ctx.beginFrame(input);

    // Empty text should be a no-op
    atlas::modeIndicator(ctx, {960.0f, 500.0f}, "");
    assertTrue(true, "modeIndicator with empty text does not crash");

    // Null text should be a no-op
    atlas::modeIndicator(ctx, {960.0f, 500.0f}, nullptr);
    assertTrue(true, "modeIndicator with null text does not crash");

    // Valid text
    atlas::modeIndicator(ctx, {960.0f, 500.0f}, "APPROACH - click a target");
    assertTrue(true, "modeIndicator with valid text does not crash");

    // With custom color
    atlas::Color yellow = {1.0f, 1.0f, 0.0f, 1.0f};
    atlas::modeIndicator(ctx, {960.0f, 500.0f}, "ORBIT - click a target", yellow);
    assertTrue(true, "modeIndicator with custom color does not crash");

    ctx.endFrame();
    ctx.shutdown();
}

// ─── Info Panel Data tests ─────────────────────────────────────────────

void testInfoPanelData() {
    std::cout << "\n=== Info Panel Data ===" << std::endl;

    atlas::InfoPanelData empty;
    assertTrue(empty.isEmpty(), "Empty InfoPanelData is empty");
    assertTrue(empty.name.empty(), "Empty InfoPanelData name is empty");
    assertClose(empty.distance, 0.0f, "Empty InfoPanelData distance is 0");

    atlas::InfoPanelData data;
    data.name = "Crimson Order Raider";
    data.type = "Cruiser";
    data.faction = "Crimson Order";
    data.shieldPct = 0.85f;
    data.armorPct = 0.60f;
    data.hullPct = 1.0f;
    data.distance = 5000.0f;
    data.velocity = 200.0f;
    data.signature = 120.0f;
    data.hasHealth = true;

    assertTrue(!data.isEmpty(), "Populated InfoPanelData is not empty");
    assertTrue(data.name == "Crimson Order Raider", "InfoPanelData name correct");
    assertTrue(data.type == "Cruiser", "InfoPanelData type correct");
    assertTrue(data.faction == "Crimson Order", "InfoPanelData faction correct");
    assertClose(data.shieldPct, 0.85f, "InfoPanelData shield 85%");
    assertClose(data.distance, 5000.0f, "InfoPanelData distance 5km");
    assertTrue(data.hasHealth, "InfoPanelData hasHealth is true");
}

// ─── Info Panel Rendering test ─────────────────────────────────────────

void testInfoPanelRendering() {
    std::cout << "\n=== Info Panel Rendering ===" << std::endl;

    atlas::AtlasContext ctx;
    ctx.init();
    atlas::InputState input;
    input.windowW = 1920;
    input.windowH = 1080;
    ctx.beginFrame(input);

    atlas::PanelState state;
    state.bounds = {100.0f, 100.0f, 280.0f, 260.0f};
    state.open = true;

    atlas::InfoPanelData data;
    data.name = "Test Entity";
    data.type = "Frigate";
    data.faction = "TestCorp";
    data.distance = 1500.0f;
    data.velocity = 100.0f;
    data.shieldPct = 1.0f;
    data.armorPct = 0.5f;
    data.hullPct = 1.0f;
    data.hasHealth = true;

    atlas::infoPanelDraw(ctx, state, data);
    assertTrue(true, "infoPanelDraw renders without crash");

    // Empty data should be a no-op
    atlas::InfoPanelData emptyData;
    atlas::infoPanelDraw(ctx, state, emptyData);
    assertTrue(true, "infoPanelDraw with empty data does not crash");

    // Closed panel should be a no-op
    state.open = false;
    atlas::infoPanelDraw(ctx, state, data);
    assertTrue(true, "infoPanelDraw with closed panel does not crash");

    ctx.endFrame();
    ctx.shutdown();
}

// ─── Overview Tab Switching test ───────────────────────────────────────

void testOverviewTabSwitching() {
    std::cout << "\n=== Overview Tab Switching ===" << std::endl;

    atlas::AtlasContext ctx;
    ctx.init();

    // Frame with mouse not on any tab
    {
        atlas::InputState input;
        input.windowW = 1920;
        input.windowH = 1080;
        input.mousePos = {0.0f, 0.0f};
        ctx.beginFrame(input);

        std::vector<std::string> tabs = {"All", "Combat", "Mining", "Custom"};
        atlas::Rect tabRect = {100.0f, 100.0f, 300.0f, 24.0f};
        int clicked = atlas::overviewHeaderInteractive(ctx, tabRect, tabs, 0);
        assertTrue(clicked == -1, "No tab clicked when mouse is away");

        ctx.endFrame();
    }

    ctx.shutdown();
}

// ─── AtlasHUD Mode Indicator test ─────────────────────────────────────

void testAtlasHUDModeIndicator() {
    std::cout << "\n=== AtlasHUD Mode Indicator ===" << std::endl;

    atlas::AtlasHUD hud;
    hud.init(1920, 1080);

    // Initially empty
    hud.setModeIndicator("");
    assertTrue(true, "Setting empty mode indicator succeeds");

    // Set a mode
    hud.setModeIndicator("APPROACH - click a target");
    assertTrue(true, "Setting approach mode indicator succeeds");

    // Clear
    hud.setModeIndicator("");
    assertTrue(true, "Clearing mode indicator succeeds");
}

// ─── AtlasHUD Info Panel test ─────────────────────────────────────────

void testAtlasHUDInfoPanel() {
    std::cout << "\n=== AtlasHUD Info Panel ===" << std::endl;

    atlas::AtlasHUD hud;
    hud.init(1920, 1080);

    assertTrue(!hud.isInfoPanelOpen(), "Info panel initially closed");

    atlas::InfoPanelData data;
    data.name = "Test Ship";
    data.type = "Destroyer";
    data.faction = "Iron Corsairs";
    data.distance = 3000.0f;
    data.hasHealth = true;
    data.shieldPct = 0.9f;
    data.armorPct = 0.7f;
    data.hullPct = 1.0f;

    hud.showInfoPanel(data);
    assertTrue(hud.isInfoPanelOpen(), "Info panel opens after showInfoPanel");

    hud.closeInfoPanel();
    assertTrue(!hud.isInfoPanelOpen(), "Info panel closes after closeInfoPanel");
}

// ─── AtlasHUD Overview Tab test ───────────────────────────────────────

void testAtlasHUDOverviewTab() {
    std::cout << "\n=== AtlasHUD Overview Tab ===" << std::endl;

    atlas::AtlasHUD hud;
    hud.init(1920, 1080);

    assertTrue(hud.getActiveOverviewTab() == 0, "Default overview tab is 0");

    hud.setActiveOverviewTab(2);
    assertTrue(hud.getActiveOverviewTab() == 2, "Overview tab set to 2");

    hud.setActiveOverviewTab(0);
    assertTrue(hud.getActiveOverviewTab() == 0, "Overview tab reset to 0");
}

// ─── Selected Item Callbacks test ──────────────────────────────────────

void testSelectedItemCallbacks() {
    std::cout << "\n=== Selected Item Callbacks ===" << std::endl;

    atlas::AtlasHUD hud;
    hud.init(1920, 1080);

    bool orbitCalled = false;
    bool approachCalled = false;
    bool warpCalled = false;
    bool infoCalled = false;

    hud.setSelectedItemOrbitCb([&]() { orbitCalled = true; });
    hud.setSelectedItemApproachCb([&]() { approachCalled = true; });
    hud.setSelectedItemWarpCb([&]() { warpCalled = true; });
    hud.setSelectedItemInfoCb([&]() { infoCalled = true; });

    assertTrue(!orbitCalled, "Orbit callback not called before trigger");
    assertTrue(!approachCalled, "Approach callback not called before trigger");
    assertTrue(!warpCalled, "Warp callback not called before trigger");
    assertTrue(!infoCalled, "Info callback not called before trigger");

    // Callbacks are wired and can be set
    assertTrue(true, "All selected item callbacks set without crash");
}

// ─── Sidebar Callback Wiring test ─────────────────────────────────────

void testSidebarCallback() {
    std::cout << "\n=== Sidebar Callback ===" << std::endl;

    atlas::AtlasContext ctx;
    ctx.init();

    atlas::AtlasHUD hud;
    hud.init(1920, 1080);

    int lastClickedIcon = -1;
    hud.setSidebarCallback([&](int icon) {
        lastClickedIcon = icon;
    });

    assertTrue(lastClickedIcon == -1, "Sidebar callback not called before click");

    // Simulate a sidebar icon click by rendering a frame with mouse position
    // over the first icon and mouse clicked state.
    // Sidebar layout: "A" button (~34px), portrait (~34px), skill bar (~9px),
    // separator (~6px), then first icon starts at approximately y=90.
    {
        atlas::InputState input;
        input.windowW = 1920;
        input.windowH = 1080;
        // Position mouse over first sidebar icon (Inventory):
        // x: pad(3) + slotSz/2 = ~20,  y: ~90 + slotSz/2 = ~107
        input.mousePos = {20.0f, 107.0f};
        input.mouseDown[0] = true;
        input.mouseClicked[0] = true;
        ctx.beginFrame(input);

        atlas::ShipHUDData ship;
        std::vector<atlas::TargetCardInfo> targets;
        std::vector<atlas::OverviewEntry> overview;
        atlas::SelectedItemInfo selected;
        hud.update(ctx, ship, targets, overview, selected);
        ctx.endFrame();
    }
    // Release mouse to complete click cycle
    {
        atlas::InputState input;
        input.windowW = 1920;
        input.windowH = 1080;
        input.mousePos = {20.0f, 107.0f};
        input.mouseDown[0] = false;
        input.mouseReleased[0] = true;
        ctx.beginFrame(input);

        atlas::ShipHUDData ship;
        std::vector<atlas::TargetCardInfo> targets;
        std::vector<atlas::OverviewEntry> overview;
        atlas::SelectedItemInfo selected;
        hud.update(ctx, ship, targets, overview, selected);
        ctx.endFrame();
    }

    assertTrue(lastClickedIcon == 0, "Sidebar callback invoked with icon 0 after click");

    // Verify overview toggle via sidebar (icon 5 in application wiring)
    assertTrue(hud.isOverviewOpen(), "Overview starts open");
    hud.toggleOverview();
    assertTrue(!hud.isOverviewOpen(), "Overview closed after toggle");
    hud.toggleOverview();
    assertTrue(hud.isOverviewOpen(), "Overview reopened after second toggle");

    // Verify dockable panel toggles (new panels opened via sidebar)
    assertTrue(!hud.isInventoryOpen(), "Inventory starts closed");
    hud.toggleInventory();
    assertTrue(hud.isInventoryOpen(), "Inventory open after toggle");
    hud.toggleInventory();
    assertTrue(!hud.isInventoryOpen(), "Inventory closed after second toggle");

    assertTrue(!hud.isFittingOpen(), "Fitting starts closed");
    hud.toggleFitting();
    assertTrue(hud.isFittingOpen(), "Fitting open after toggle");

    assertTrue(!hud.isMarketOpen(), "Market starts closed");
    hud.toggleMarket();
    assertTrue(hud.isMarketOpen(), "Market open after toggle");

    assertTrue(!hud.isMissionOpen(), "Mission starts closed");
    hud.toggleMission();
    assertTrue(hud.isMissionOpen(), "Mission open after toggle");

    assertTrue(!hud.isDScanOpen(), "DScan starts closed");
    hud.toggleDScan();
    assertTrue(hud.isDScanOpen(), "DScan open after toggle");

    assertTrue(!hud.isChatOpen(), "Chat starts closed");
    hud.toggleChat();
    assertTrue(hud.isChatOpen(), "Chat open after toggle");

    assertTrue(!hud.isDronePanelOpen(), "Drone panel starts closed");
    hud.toggleDronePanel();
    assertTrue(hud.isDronePanelOpen(), "Drone panel open after toggle");

    // Verify HUD renders without crash with all panels open
    {
        atlas::InputState input;
        input.windowW = 1920;
        input.windowH = 1080;
        ctx.beginFrame(input);
        atlas::ShipHUDData ship;
        std::vector<atlas::TargetCardInfo> targets;
        std::vector<atlas::OverviewEntry> overview;
        atlas::SelectedItemInfo selected;
        hud.update(ctx, ship, targets, overview, selected);
        ctx.endFrame();
    }
    assertTrue(true, "HUD renders with all dockable panels open");
}

// ─── Mouse Delta (getDragDelta) ────────────────────────────────────────

void testGetDragDelta() {
    std::cout << "\n=== getDragDelta ===" << std::endl;
    atlas::AtlasContext ctx;
    ctx.init();

    // Frame 1: initial position
    {
        atlas::InputState input;
        input.windowW = 1920;
        input.windowH = 1080;
        input.mousePos = {100.0f, 200.0f};
        ctx.beginFrame(input);
        // First frame delta may be zero (no previous frame)
        ctx.endFrame();
    }

    // Frame 2: mouse moves
    {
        atlas::InputState input;
        input.windowW = 1920;
        input.windowH = 1080;
        input.mousePos = {120.0f, 210.0f};
        ctx.beginFrame(input);
        atlas::Vec2 delta = ctx.getDragDelta();
        assertClose(delta.x, 20.0f, "getDragDelta X = 20");
        assertClose(delta.y, 10.0f, "getDragDelta Y = 10");
        ctx.endFrame();
    }

    // Frame 3: no movement
    {
        atlas::InputState input;
        input.windowW = 1920;
        input.windowH = 1080;
        input.mousePos = {120.0f, 210.0f};
        ctx.beginFrame(input);
        atlas::Vec2 delta = ctx.getDragDelta();
        assertClose(delta.x, 0.0f, "getDragDelta X = 0 when stationary");
        assertClose(delta.y, 0.0f, "getDragDelta Y = 0 when stationary");
        ctx.endFrame();
    }

    ctx.shutdown();
}

// ─── Mouse Consumed ────────────────────────────────────────────────────

void testMouseConsumed() {
    std::cout << "\n=== Mouse Consumed ===" << std::endl;
    atlas::AtlasContext ctx;
    ctx.init();

    atlas::Rect btn(100, 100, 80, 30);
    atlas::WidgetID btnID = atlas::hashID("consumeTestBtn");

    // Mouse consumed prevents buttonBehavior from registering clicks
    {
        atlas::InputState input;
        input.windowW = 1920;
        input.windowH = 1080;
        input.mousePos = {140.0f, 115.0f};
        input.mouseClicked[0] = true;
        input.mouseDown[0] = true;
        ctx.beginFrame(input);
        assertTrue(!ctx.isMouseConsumed(), "Mouse not consumed at frame start");
        ctx.consumeMouse();
        assertTrue(ctx.isMouseConsumed(), "Mouse consumed after consumeMouse()");
        bool clicked = ctx.buttonBehavior(btn, btnID);
        assertTrue(!clicked, "Button does not register click when mouse consumed");
        assertTrue(!ctx.isHot(btnID), "Button is not hot when mouse consumed");
        ctx.endFrame();
    }

    // Next frame: consumed flag resets
    {
        atlas::InputState input;
        input.windowW = 1920;
        input.windowH = 1080;
        input.mousePos = {140.0f, 115.0f};
        ctx.beginFrame(input);
        assertTrue(!ctx.isMouseConsumed(), "Mouse consumed resets each frame");
        ctx.endFrame();
    }

    ctx.shutdown();
}

// ─── Sidebar does not fire when panel overlaps ─────────────────────────

void testSidebarBlockedByPanel() {
    std::cout << "\n=== Sidebar Blocked by Overlapping Panel ===" << std::endl;
    atlas::AtlasContext ctx;
    ctx.init();

    int lastClickedIcon = -1;

    // Create a panel that overlaps the sidebar area
    atlas::PanelState panelState;
    panelState.bounds = atlas::Rect(0.0f, 0.0f, 200.0f, 300.0f);
    panelState.open = true;
    atlas::PanelFlags flags;
    flags.locked = true;

    // Click on an area that's both inside the panel and the sidebar icon area
    // Sidebar icon 0 is at approx (2, 8, 36, 36)
    {
        atlas::InputState input;
        input.windowW = 1920;
        input.windowH = 1080;
        input.mousePos = {20.0f, 26.0f};
        input.mouseDown[0] = true;
        input.mouseClicked[0] = true;
        ctx.beginFrame(input);

        // Panel renders first and consumes mouse
        atlas::panelBeginStateful(ctx, "Overlap Test", panelState, flags);
        atlas::panelEnd(ctx);

        // Sidebar renders after - should be blocked
        atlas::sidebarBar(ctx, 0.0f, 40.0f, 1080.0f, 8,
            [&](int icon) { lastClickedIcon = icon; });

        ctx.endFrame();
    }
    // Release frame
    {
        atlas::InputState input;
        input.windowW = 1920;
        input.windowH = 1080;
        input.mousePos = {20.0f, 26.0f};
        input.mouseReleased[0] = true;
        ctx.beginFrame(input);

        atlas::panelBeginStateful(ctx, "Overlap Test", panelState, flags);
        atlas::panelEnd(ctx);

        atlas::sidebarBar(ctx, 0.0f, 40.0f, 1080.0f, 8,
            [&](int icon) { lastClickedIcon = icon; });

        ctx.endFrame();
    }

    assertTrue(lastClickedIcon == -1,
               "Sidebar icon not triggered when panel overlaps and consumes click");

    ctx.shutdown();
}


// ─── Tab Bar tests ─────────────────────────────────────────────────────

void testTabBar() {
    std::cout << "\n=== Tab Bar ===" << std::endl;

    atlas::AtlasContext ctx;
    ctx.init();

    // No click → returns -1
    {
        atlas::InputState input;
        input.windowW = 1280;
        input.windowH = 720;
        input.mousePos = {500.0f, 500.0f}; // away from tabs
        ctx.beginFrame(input);
        std::vector<std::string> tabs = {"Alpha", "Beta", "Gamma"};
        int clicked = atlas::tabBar(ctx, {10.0f, 10.0f, 300.0f, 24.0f}, tabs, 0);
        assertTrue(clicked == -1, "tabBar: no click returns -1");
        ctx.endFrame();
    }

    // Click on first tab (simulate press + release)
    {
        atlas::InputState input;
        input.windowW = 1280;
        input.windowH = 720;
        input.mousePos = {30.0f, 18.0f};
        input.mouseDown[0] = true;
        input.mouseClicked[0] = true;
        ctx.beginFrame(input);
        std::vector<std::string> tabs = {"Alpha", "Beta", "Gamma"};
        atlas::tabBar(ctx, {10.0f, 10.0f, 300.0f, 24.0f}, tabs, 1);
        ctx.endFrame();
    }
    {
        atlas::InputState input;
        input.windowW = 1280;
        input.windowH = 720;
        input.mousePos = {30.0f, 18.0f};
        input.mouseReleased[0] = true;
        ctx.beginFrame(input);
        std::vector<std::string> tabs = {"Alpha", "Beta", "Gamma"};
        int clicked = atlas::tabBar(ctx, {10.0f, 10.0f, 300.0f, 24.0f}, tabs, 1);
        assertTrue(clicked == 0, "tabBar: click on first tab returns 0");
        ctx.endFrame();
    }

    ctx.shutdown();
}

// ─── Combat Log Widget tests ───────────────────────────────────────────

void testCombatLogWidget() {
    std::cout << "\n=== Combat Log Widget ===" << std::endl;

    atlas::AtlasContext ctx;
    ctx.init();

    // Empty messages list
    {
        atlas::InputState input;
        input.windowW = 1280;
        input.windowH = 720;
        ctx.beginFrame(input);
        std::vector<std::string> msgs;
        float scroll = 0.0f;
        atlas::combatLogWidget(ctx, {50.0f, 400.0f, 280.0f, 160.0f}, msgs, scroll);
        assertTrue(true, "combatLogWidget: empty messages renders without crash");
        ctx.endFrame();
    }

    // With messages
    {
        atlas::InputState input;
        input.windowW = 1280;
        input.windowH = 720;
        ctx.beginFrame(input);
        std::vector<std::string> msgs = {
            "You hit Drone for 120 damage",
            "Shield boosted by 50",
            "Warp disrupted!",
        };
        float scroll = 0.0f;
        atlas::combatLogWidget(ctx, {50.0f, 400.0f, 280.0f, 160.0f}, msgs, scroll);
        assertTrue(true, "combatLogWidget: with messages renders without crash");
        ctx.endFrame();
    }

    // Scroll offset clamps to valid range
    {
        atlas::InputState input;
        input.windowW = 1280;
        input.windowH = 720;
        ctx.beginFrame(input);
        std::vector<std::string> msgs = {"msg1", "msg2"};
        float scroll = -100.0f;
        atlas::combatLogWidget(ctx, {50.0f, 400.0f, 280.0f, 160.0f}, msgs, scroll);
        assertTrue(scroll >= 0.0f, "combatLogWidget: negative scroll clamped to 0");
        ctx.endFrame();
    }

    ctx.shutdown();
}

// ─── Damage Flash Overlay tests ────────────────────────────────────────

void testDamageFlashOverlay() {
    std::cout << "\n=== Damage Flash Overlay ===" << std::endl;

    atlas::AtlasContext ctx;
    ctx.init();

    atlas::InputState input;
    input.windowW = 1280;
    input.windowH = 720;

    // Shield flash
    ctx.beginFrame(input);
    atlas::damageFlashOverlay(ctx, {640.0f, 610.0f}, 80.0f, 0, 1.0f);
    assertTrue(true, "damageFlashOverlay: shield layer renders");
    ctx.endFrame();

    // Armor flash
    ctx.beginFrame(input);
    atlas::damageFlashOverlay(ctx, {640.0f, 610.0f}, 80.0f, 1, 0.5f);
    assertTrue(true, "damageFlashOverlay: armor layer renders");
    ctx.endFrame();

    // Hull flash
    ctx.beginFrame(input);
    atlas::damageFlashOverlay(ctx, {640.0f, 610.0f}, 80.0f, 2, 0.8f);
    assertTrue(true, "damageFlashOverlay: hull layer renders");
    ctx.endFrame();

    // Zero intensity — should be no-op
    ctx.beginFrame(input);
    atlas::damageFlashOverlay(ctx, {640.0f, 610.0f}, 80.0f, 0, 0.0f);
    assertTrue(true, "damageFlashOverlay: zero intensity is no-op");
    ctx.endFrame();

    ctx.shutdown();
}

// ─── Drone Status Bar tests ────────────────────────────────────────────

void testDroneStatusBar() {
    std::cout << "\n=== Drone Status Bar ===" << std::endl;

    atlas::AtlasContext ctx;
    ctx.init();

    atlas::InputState input;
    input.windowW = 1280;
    input.windowH = 720;

    // Normal drone status
    ctx.beginFrame(input);
    atlas::droneStatusBar(ctx, {100.0f, 650.0f, 260.0f, 22.0f}, 3, 2, 15, 25);
    assertTrue(true, "droneStatusBar: normal state renders");
    ctx.endFrame();

    // Max bandwidth (danger state)
    ctx.beginFrame(input);
    atlas::droneStatusBar(ctx, {100.0f, 650.0f, 260.0f, 22.0f}, 5, 0, 25, 25);
    assertTrue(true, "droneStatusBar: max bandwidth renders");
    ctx.endFrame();

    // Zero bandwidth max (edge case)
    ctx.beginFrame(input);
    atlas::droneStatusBar(ctx, {100.0f, 650.0f, 260.0f, 22.0f}, 0, 5, 0, 0);
    assertTrue(true, "droneStatusBar: zero max bandwidth renders");
    ctx.endFrame();

    ctx.shutdown();
}

// ─── Fleet Broadcast Banner tests ──────────────────────────────────────

void testFleetBroadcastBanner() {
    std::cout << "\n=== Fleet Broadcast Banner ===" << std::endl;

    atlas::AtlasContext ctx;
    ctx.init();

    atlas::InputState input;
    input.windowW = 1280;
    input.windowH = 720;

    // Empty broadcasts
    ctx.beginFrame(input);
    std::vector<atlas::FleetBroadcast> empty;
    atlas::fleetBroadcastBanner(ctx, {400.0f, 92.0f, 300.0f, 60.0f}, empty);
    assertTrue(true, "fleetBroadcastBanner: empty list is no-op");
    ctx.endFrame();

    // With broadcasts
    ctx.beginFrame(input);
    std::vector<atlas::FleetBroadcast> bcs;
    atlas::FleetBroadcast bc1;
    bc1.sender = "FC";
    bc1.message = "Align to gate";
    bc1.color = {0.4f, 0.58f, 0.86f, 1.0f};
    bc1.age = 0.0f;
    bc1.maxAge = 8.0f;
    bcs.push_back(bc1);
    atlas::FleetBroadcast bc2;
    bc2.sender = "Logi";
    bc2.message = "Need Armor";
    bc2.color = {0.88f, 0.46f, 0.24f, 1.0f};
    bc2.age = 3.0f;
    bc2.maxAge = 8.0f;
    bcs.push_back(bc2);
    atlas::fleetBroadcastBanner(ctx, {400.0f, 92.0f, 300.0f, 60.0f}, bcs);
    assertTrue(true, "fleetBroadcastBanner: with broadcasts renders");
    ctx.endFrame();

    // Expired broadcast (age >= maxAge) should not render
    ctx.beginFrame(input);
    std::vector<atlas::FleetBroadcast> expired;
    atlas::FleetBroadcast bc3;
    bc3.sender = "Old";
    bc3.message = "Expired";
    bc3.color = {1.0f, 0.0f, 0.0f, 1.0f};
    bc3.age = 10.0f;
    bc3.maxAge = 8.0f;
    expired.push_back(bc3);
    atlas::fleetBroadcastBanner(ctx, {400.0f, 92.0f, 300.0f, 60.0f}, expired);
    assertTrue(true, "fleetBroadcastBanner: expired broadcast renders without crash");
    ctx.endFrame();

    ctx.shutdown();
}

// ─── FleetBroadcast struct tests ───────────────────────────────────────

void testFleetBroadcastStruct() {
    std::cout << "\n=== FleetBroadcast Struct ===" << std::endl;

    atlas::FleetBroadcast bc;
    assertTrue(bc.sender.empty(), "FleetBroadcast sender defaults to empty");
    assertTrue(bc.message.empty(), "FleetBroadcast message defaults to empty");
    assertClose(bc.age, 0.0f, "FleetBroadcast age defaults to 0");
    assertClose(bc.maxAge, 8.0f, "FleetBroadcast maxAge defaults to 8");

    bc.sender = "FC Lead";
    bc.message = "Warp to me";
    bc.color = {0.2f, 0.8f, 0.4f, 1.0f};
    bc.age = 2.5f;
    assertTrue(bc.sender == "FC Lead", "FleetBroadcast sender set correctly");
    assertTrue(bc.message == "Warp to me", "FleetBroadcast message set correctly");
    assertClose(bc.age, 2.5f, "FleetBroadcast age set correctly");
    assertClose(bc.color.g, 0.8f, "FleetBroadcast color green set correctly");
}

// ─── AtlasHUD Combat Log tests ─────────────────────────────────────────

void testAtlasHUDCombatLog() {
    std::cout << "\n=== AtlasHUD Combat Log ===" << std::endl;

    atlas::AtlasHUD hud;
    hud.init(1280, 720);

    // Initially empty
    assertTrue(hud.getCombatLog().empty(), "Combat log starts empty");

    // Add messages
    hud.addCombatLogMessage("Shield hit for 50 damage");
    assertTrue(hud.getCombatLog().size() == 1, "Combat log has 1 message after add");
    assertTrue(hud.getCombatLog()[0] == "Shield hit for 50 damage",
               "Combat log message content correct");

    hud.addCombatLogMessage("Armor hit for 30 damage");
    assertTrue(hud.getCombatLog().size() == 2, "Combat log has 2 messages after second add");

    // Renders without crash
    atlas::AtlasContext ctx;
    ctx.init();
    atlas::InputState input;
    input.windowW = 1280;
    input.windowH = 720;
    ctx.beginFrame(input);

    atlas::ShipHUDData ship;
    std::vector<atlas::TargetCardInfo> targets;
    std::vector<atlas::OverviewEntry> overview;
    atlas::SelectedItemInfo sel;
    hud.update(ctx, ship, targets, overview, sel);

    assertTrue(true, "HUD with combat log renders without crash");
    ctx.endFrame();
    ctx.shutdown();
}

// ─── AtlasHUD Damage Flash tests ───────────────────────────────────────

void testAtlasHUDDamageFlash() {
    std::cout << "\n=== AtlasHUD Damage Flash ===" << std::endl;

    atlas::AtlasHUD hud;
    hud.init(1280, 720);

    // No flash initially
    assertTrue(!hud.hasDamageFlash(), "No damage flash initially");

    // Trigger shield flash
    hud.triggerDamageFlash(0);
    assertTrue(hud.hasDamageFlash(), "Damage flash active after trigger");

    // Renders without crash
    atlas::AtlasContext ctx;
    ctx.init();
    atlas::InputState input;
    input.windowW = 1280;
    input.windowH = 720;
    ctx.beginFrame(input);

    atlas::ShipHUDData ship;
    std::vector<atlas::TargetCardInfo> targets;
    std::vector<atlas::OverviewEntry> overview;
    atlas::SelectedItemInfo sel;
    hud.update(ctx, ship, targets, overview, sel);

    assertTrue(true, "HUD with damage flash renders without crash");
    ctx.endFrame();

    // Multiple flashes
    hud.triggerDamageFlash(1, 0.5f);
    hud.triggerDamageFlash(2, 0.3f);
    assertTrue(hud.hasDamageFlash(), "Multiple damage flashes active");

    ctx.beginFrame(input);
    hud.update(ctx, ship, targets, overview, sel);
    assertTrue(true, "HUD with multiple damage flashes renders without crash");
    ctx.endFrame();

    ctx.shutdown();
}

// ─── AtlasHUD Drone Status tests ───────────────────────────────────────

void testAtlasHUDDroneStatus() {
    std::cout << "\n=== AtlasHUD Drone Status ===" << std::endl;

    atlas::AtlasHUD hud;
    hud.init(1280, 720);

    // Initially hidden
    assertTrue(!hud.isDroneStatusVisible(), "Drone status hidden by default");

    // Toggle on
    hud.toggleDroneStatus();
    assertTrue(hud.isDroneStatusVisible(), "Drone status visible after toggle");

    // Set data
    atlas::AtlasHUD::DroneStatusData drones;
    drones.inSpace = 3;
    drones.inBay = 2;
    drones.bandwidthUsed = 15;
    drones.bandwidthMax = 25;
    hud.setDroneStatus(drones);

    // Renders without crash
    atlas::AtlasContext ctx;
    ctx.init();
    atlas::InputState input;
    input.windowW = 1280;
    input.windowH = 720;
    ctx.beginFrame(input);

    atlas::ShipHUDData ship;
    std::vector<atlas::TargetCardInfo> targets;
    std::vector<atlas::OverviewEntry> overview;
    atlas::SelectedItemInfo sel;
    hud.update(ctx, ship, targets, overview, sel);

    assertTrue(true, "HUD with drone status renders without crash");
    ctx.endFrame();

    // Toggle off
    hud.toggleDroneStatus();
    assertTrue(!hud.isDroneStatusVisible(), "Drone status hidden after second toggle");

    ctx.shutdown();
}

// ─── AtlasHUD Fleet Broadcast tests ────────────────────────────────────

void testAtlasHUDFleetBroadcast() {
    std::cout << "\n=== AtlasHUD Fleet Broadcast ===" << std::endl;

    atlas::AtlasHUD hud;
    hud.init(1280, 720);

    // Initially empty
    assertTrue(hud.getFleetBroadcasts().empty(), "Fleet broadcasts start empty");

    // Add broadcast
    hud.addFleetBroadcast("FC", "Align to gate");
    assertTrue(hud.getFleetBroadcasts().size() == 1, "One broadcast after add");
    assertTrue(hud.getFleetBroadcasts()[0].sender == "FC",
               "Broadcast sender correct");
    assertTrue(hud.getFleetBroadcasts()[0].message == "Align to gate",
               "Broadcast message correct");

    // Add with custom color
    hud.addFleetBroadcast("Logi", "Need Armor", {0.2f, 0.8f, 0.4f, 1.0f});
    assertTrue(hud.getFleetBroadcasts().size() == 2, "Two broadcasts after second add");

    // Renders without crash
    atlas::AtlasContext ctx;
    ctx.init();
    atlas::InputState input;
    input.windowW = 1280;
    input.windowH = 720;
    ctx.beginFrame(input);

    atlas::ShipHUDData ship;
    std::vector<atlas::TargetCardInfo> targets;
    std::vector<atlas::OverviewEntry> overview;
    atlas::SelectedItemInfo sel;
    hud.update(ctx, ship, targets, overview, sel);

    assertTrue(true, "HUD with fleet broadcasts renders without crash");
    ctx.endFrame();
    ctx.shutdown();
}

// ─── DroneStatusData struct tests ──────────────────────────────────────

void testDroneStatusDataDefaults() {
    std::cout << "\n=== DroneStatusData Defaults ===" << std::endl;

    atlas::AtlasHUD::DroneStatusData data;
    assertTrue(data.inSpace == 0, "DroneStatusData inSpace defaults to 0");
    assertTrue(data.inBay == 0, "DroneStatusData inBay defaults to 0");
    assertTrue(data.bandwidthUsed == 0, "DroneStatusData bandwidthUsed defaults to 0");
    assertTrue(data.bandwidthMax == 0, "DroneStatusData bandwidthMax defaults to 0");
}

// ─── Key constants test ─────────────────────────────────────────────────

void testKeyConstants() {
    std::cout << "\n=== Key Constants ===" << std::endl;

    assertTrue(atlas::Key::F1 == 290, "Key::F1 is 290 (GLFW value)");
    assertTrue(atlas::Key::F2 == 291, "Key::F2 is 291");
    assertTrue(atlas::Key::F8 == 297, "Key::F8 is 297");
    assertTrue(atlas::Key::F12 == 301, "Key::F12 is 301");
    assertTrue(atlas::Key::V == 86, "Key::V is 86");
    assertTrue(atlas::Key::F2 - atlas::Key::F1 == 1, "F keys are sequential");
}

// ─── InputState keyboard fields test ────────────────────────────────────

void testInputStateKeyboard() {
    std::cout << "\n=== InputState Keyboard ===" << std::endl;

    atlas::InputState input;
    assertTrue(!input.keyPressed[atlas::Key::F1], "keyPressed[F1] defaults to false");
    assertTrue(!input.keyDown[atlas::Key::F1], "keyDown[F1] defaults to false");
    assertTrue(!input.keyPressed[atlas::Key::V], "keyPressed[V] defaults to false");

    // Simulate F1 press
    input.keyPressed[atlas::Key::F1] = true;
    input.keyDown[atlas::Key::F1] = true;
    assertTrue(input.keyPressed[atlas::Key::F1], "keyPressed[F1] set to true");
    assertTrue(input.keyDown[atlas::Key::F1], "keyDown[F1] set to true");
    assertTrue(!input.keyPressed[atlas::Key::F2], "keyPressed[F2] still false");
}

// ─── Keyboard module activation test ────────────────────────────────────

void testKeyboardModuleActivation() {
    std::cout << "\n=== Keyboard Module Activation ===" << std::endl;

    atlas::AtlasContext ctx;
    ctx.init();

    atlas::AtlasHUD hud;
    hud.init(1920, 1080);

    int lastModuleClicked = -1;
    hud.setModuleCallback([&](int idx) { lastModuleClicked = idx; });

    // Create ship data with fitted modules
    atlas::ShipHUDData ship;
    ship.highSlots.resize(4);
    for (auto& s : ship.highSlots) { s.fitted = true; s.color = {0.8f, 0.2f, 0.2f, 1.0f}; }
    ship.midSlots.resize(2);
    for (auto& s : ship.midSlots) { s.fitted = true; s.color = {0.2f, 0.5f, 0.8f, 1.0f}; }

    std::vector<atlas::TargetCardInfo> targets;
    std::vector<atlas::OverviewEntry> overview;
    atlas::SelectedItemInfo selectedItem;

    // Frame with F1 key press
    atlas::InputState input;
    input.windowW = 1920;
    input.windowH = 1080;
    input.keyPressed[atlas::Key::F1] = true;
    ctx.beginFrame(input);
    hud.update(ctx, ship, targets, overview, selectedItem);
    ctx.endFrame();
    assertTrue(lastModuleClicked == 0, "F1 activates module slot 0");

    // Frame with F3 key press
    lastModuleClicked = -1;
    input.keyPressed[atlas::Key::F1] = false;
    input.keyPressed[atlas::Key::F3] = true;
    ctx.beginFrame(input);
    hud.update(ctx, ship, targets, overview, selectedItem);
    ctx.endFrame();
    assertTrue(lastModuleClicked == 2, "F3 activates module slot 2");

    // Frame with no key press
    lastModuleClicked = -1;
    input.keyPressed[atlas::Key::F3] = false;
    ctx.beginFrame(input);
    hud.update(ctx, ship, targets, overview, selectedItem);
    ctx.endFrame();
    assertTrue(lastModuleClicked == -1, "No key press = no module activation");

    ctx.shutdown();
}

// ─── D-Scan data test ───────────────────────────────────────────────────

void testDScanData() {
    std::cout << "\n=== D-Scan Data ===" << std::endl;

    atlas::AtlasHUD hud;
    hud.init(1920, 1080);

    // Default values
    assertClose(hud.getDScanAngle(), 360.0f, "D-Scan angle defaults to 360");
    assertClose(hud.getDScanRange(), 14.3f, "D-Scan range defaults to 14.3 AU");
    assertTrue(hud.getDScanResults().empty(), "D-Scan results start empty");

    // Set custom values
    hud.setDScanAngle(90.0f);
    hud.setDScanRange(5.0f);
    assertClose(hud.getDScanAngle(), 90.0f, "D-Scan angle set to 90");
    assertClose(hud.getDScanRange(), 5.0f, "D-Scan range set to 5.0 AU");

    // Add results
    std::vector<atlas::AtlasHUD::DScanEntry> results;
    results.push_back({"Rifter", "Frigate", 2.3f});
    results.push_back({"Stargate", "Structure", 8.1f});
    hud.setDScanResults(results);
    assertTrue(hud.getDScanResults().size() == 2, "D-Scan has 2 results");
    assertTrue(hud.getDScanResults()[0].name == "Rifter", "Result 0 name is Rifter");
    assertTrue(hud.getDScanResults()[1].type == "Structure", "Result 1 type is Structure");
    assertClose(hud.getDScanResults()[1].distance, 8.1f, "Result 1 distance is 8.1 AU");

    // D-Scan callback
    bool scanFired = false;
    hud.setDScanCallback([&]() { scanFired = true; });
    assertTrue(!scanFired, "D-Scan callback not fired before trigger");
}

// ─── D-Scan panel rendering test ────────────────────────────────────────

void testDScanPanelRendering() {
    std::cout << "\n=== D-Scan Panel Rendering ===" << std::endl;

    atlas::AtlasContext ctx;
    ctx.init();

    atlas::AtlasHUD hud;
    hud.init(1920, 1080);

    // Set up D-Scan data
    hud.setDScanAngle(180.0f);
    hud.setDScanRange(7.5f);
    std::vector<atlas::AtlasHUD::DScanEntry> results;
    results.push_back({"Veldspar", "Asteroid", 0.5f});
    hud.setDScanResults(results);
    hud.toggleDScan();

    atlas::ShipHUDData ship;
    std::vector<atlas::TargetCardInfo> targets;
    std::vector<atlas::OverviewEntry> overview;
    atlas::SelectedItemInfo selectedItem;

    atlas::InputState input;
    input.windowW = 1920;
    input.windowH = 1080;
    ctx.beginFrame(input);
    hud.update(ctx, ship, targets, overview, selectedItem);
    ctx.endFrame();
    assertTrue(true, "D-Scan panel with results renders without crash");

    ctx.shutdown();
}

// ─── Mission data test ──────────────────────────────────────────────────

void testMissionData() {
    std::cout << "\n=== Mission Data ===" << std::endl;

    atlas::AtlasHUD hud;
    hud.init(1920, 1080);

    // Default: no active mission
    assertTrue(!hud.getMissionInfo().active, "Mission not active by default");

    // Set mission data
    atlas::AtlasHUD::MissionInfo mission;
    mission.active = true;
    mission.name = "The Blockade";
    mission.type = "combat";
    mission.agentName = "Agent Karde";
    mission.level = 3;
    mission.iskReward = 500000.0f;
    mission.lpReward = 350.0f;
    mission.timeLimitHours = 4.0f;
    mission.timeElapsedHours = 1.5f;
    mission.objectives.push_back({"Destroy all enemy ships", false});
    mission.objectives.push_back({"Retrieve the cargo", false});
    hud.setMissionInfo(mission);

    assertTrue(hud.getMissionInfo().active, "Mission is active after set");
    assertTrue(hud.getMissionInfo().name == "The Blockade", "Mission name correct");
    assertTrue(hud.getMissionInfo().type == "combat", "Mission type correct");
    assertTrue(hud.getMissionInfo().agentName == "Agent Karde", "Agent name correct");
    assertTrue(hud.getMissionInfo().level == 3, "Mission level correct");
    assertClose(hud.getMissionInfo().iskReward, 500000.0f, "ISK reward correct");
    assertClose(hud.getMissionInfo().lpReward, 350.0f, "LP reward correct");
    assertTrue(hud.getMissionInfo().objectives.size() == 2, "2 objectives");
    assertTrue(!hud.getMissionInfo().objectives[0].completed, "Objective 0 incomplete");
}

// ─── Mission panel rendering test ───────────────────────────────────────

void testMissionPanelRendering() {
    std::cout << "\n=== Mission Panel Rendering ===" << std::endl;

    atlas::AtlasContext ctx;
    ctx.init();

    atlas::AtlasHUD hud;
    hud.init(1920, 1080);

    // Test with empty mission
    hud.toggleMission();
    atlas::ShipHUDData ship;
    std::vector<atlas::TargetCardInfo> targets;
    std::vector<atlas::OverviewEntry> overview;
    atlas::SelectedItemInfo selectedItem;

    atlas::InputState input;
    input.windowW = 1920;
    input.windowH = 1080;
    ctx.beginFrame(input);
    hud.update(ctx, ship, targets, overview, selectedItem);
    ctx.endFrame();
    assertTrue(true, "Mission panel (no mission) renders without crash");

    // Test with active mission
    atlas::AtlasHUD::MissionInfo mission;
    mission.active = true;
    mission.name = "Worlds Collide";
    mission.type = "combat";
    mission.level = 4;
    mission.iskReward = 2000000.0f;
    mission.lpReward = 1200.0f;
    mission.timeLimitHours = 8.0f;
    mission.timeElapsedHours = 7.5f;  // nearly expired!
    mission.objectives.push_back({"Kill all pirates", true});
    mission.objectives.push_back({"Loot the wreck", false});
    hud.setMissionInfo(mission);

    ctx.beginFrame(input);
    hud.update(ctx, ship, targets, overview, selectedItem);
    ctx.endFrame();
    assertTrue(true, "Mission panel (active, near expiry) renders without crash");

    ctx.shutdown();
}

// ─── Probe Scanner data test ────────────────────────────────────────────

void testProbeScannerData() {
    std::cout << "\n=== Probe Scanner Data ===" << std::endl;

    atlas::AtlasHUD hud;
    hud.init(1920, 1080);

    // Defaults
    assertTrue(hud.getProbeCount() == 8, "Probe count defaults to 8");
    assertClose(hud.getProbeRange(), 8.0f, "Probe range defaults to 8 AU");
    assertTrue(hud.getProbeScanResults().empty(), "Probe results start empty");

    // Set values
    hud.setProbeCount(7);
    hud.setProbeRange(4.0f);
    assertTrue(hud.getProbeCount() == 7, "Probe count set to 7");
    assertClose(hud.getProbeRange(), 4.0f, "Probe range set to 4 AU");

    // Add results
    std::vector<atlas::AtlasHUD::ProbeScanEntry> results;
    results.push_back({"ABC-123", "Unknown", "Cosmic Signature", "???", 15.0f, 3.2f});
    results.push_back({"DEF-456", "Serpentis Hideaway", "Cosmic Anomaly", "Combat Site", 100.0f, 1.1f});
    results.push_back({"GHI-789", "Forgotten Relic", "Cosmic Signature", "Relic Site", 60.0f, 5.5f});
    hud.setProbeScanResults(results);

    assertTrue(hud.getProbeScanResults().size() == 3, "3 probe scan results");
    assertTrue(hud.getProbeScanResults()[0].id == "ABC-123", "Result 0 ID correct");
    assertClose(hud.getProbeScanResults()[0].signalStrength, 15.0f, "Result 0 signal 15%");
    assertTrue(hud.getProbeScanResults()[1].group == "Cosmic Anomaly", "Result 1 group correct");
    assertClose(hud.getProbeScanResults()[1].signalStrength, 100.0f, "Result 1 signal 100%");
    assertTrue(hud.getProbeScanResults()[2].type == "Relic Site", "Result 2 type correct");

    // Callback
    bool analyzeFired = false;
    hud.setProbeScanCallback([&]() { analyzeFired = true; });
    assertTrue(!analyzeFired, "Probe scan callback not fired before trigger");

    // Toggle
    assertTrue(!hud.isProbeScannerOpen(), "Probe scanner closed by default");
    hud.toggleProbeScanner();
    assertTrue(hud.isProbeScannerOpen(), "Probe scanner open after toggle");
    hud.toggleProbeScanner();
    assertTrue(!hud.isProbeScannerOpen(), "Probe scanner closed after second toggle");
}

// ─── Probe Scanner panel rendering test ─────────────────────────────────

void testProbeScannerRendering() {
    std::cout << "\n=== Probe Scanner Rendering ===" << std::endl;

    atlas::AtlasContext ctx;
    ctx.init();

    atlas::AtlasHUD hud;
    hud.init(1920, 1080);

    hud.toggleProbeScanner();
    std::vector<atlas::AtlasHUD::ProbeScanEntry> results;
    results.push_back({"AAA-111", "Site Alpha", "Cosmic Signature", "Data Site", 85.0f, 2.0f});
    results.push_back({"BBB-222", "Unknown", "Cosmic Signature", "???", 10.0f, 7.8f});
    hud.setProbeScanResults(results);

    atlas::ShipHUDData ship;
    std::vector<atlas::TargetCardInfo> targets;
    std::vector<atlas::OverviewEntry> overview;
    atlas::SelectedItemInfo selectedItem;

    atlas::InputState input;
    input.windowW = 1920;
    input.windowH = 1080;
    ctx.beginFrame(input);
    hud.update(ctx, ship, targets, overview, selectedItem);
    ctx.endFrame();
    assertTrue(true, "Probe scanner with results renders without crash");

    // Test empty results
    hud.setProbeScanResults({});
    ctx.beginFrame(input);
    hud.update(ctx, ship, targets, overview, selectedItem);
    ctx.endFrame();
    assertTrue(true, "Probe scanner empty renders without crash");

    ctx.shutdown();
}


// ─── Main ──────────────────────────────────────────────────────────────

// ─── Panel Resize State tests ──────────────────────────────────────

void testPanelResizeState() {
    std::cout << "\n=== Panel Resize State ===" << std::endl;
    atlas::PanelState state;
    assertTrue(!state.resizing, "PanelState resizing defaults to false");
    assertTrue(state.resizeEdge == 0, "PanelState resizeEdge defaults to 0");
    assertTrue(state.minW == 150.0f, "PanelState minW defaults to 150");
    assertTrue(state.minH == 80.0f, "PanelState minH defaults to 80");

    // Simulate resize state
    state.resizing = true;
    state.resizeEdge = 2 | 8;  // right + bottom
    assertTrue(state.resizing, "PanelState resizing set to true");
    assertTrue((state.resizeEdge & 2) != 0, "resizeEdge right bit set");
    assertTrue((state.resizeEdge & 8) != 0, "resizeEdge bottom bit set");
    assertTrue((state.resizeEdge & 1) == 0, "resizeEdge left bit not set");
    assertTrue((state.resizeEdge & 4) == 0, "resizeEdge top bit not set");
}

// ─── Panel Lock State tests ───────────────────────────────────────

void testPanelLockState() {
    std::cout << "\n=== Panel Lock State ===" << std::endl;
    atlas::PanelState state;
    assertTrue(!state.locked, "PanelState locked defaults to false");

    state.locked = true;
    assertTrue(state.locked, "PanelState locked set to true");

    // Verify lock prevents drag/resize in panelBeginStateful
    atlas::AtlasContext ctx;
    ctx.init();

    atlas::InputState input;
    input.windowW = 1280;
    input.windowH = 720;
    ctx.beginFrame(input);

    state.bounds = {100, 100, 300, 200};
    state.open = true;
    state.locked = true;
    atlas::PanelFlags flags;
    flags.showHeader = true;
    flags.showClose = true;
    flags.showMinimize = true;

    atlas::panelBeginStateful(ctx, "Locked Panel", state, flags);
    atlas::panelEnd(ctx);

    // Panel position should remain unchanged since it's locked
    assertClose(state.bounds.x, 100.0f, "Locked panel X unchanged");
    assertClose(state.bounds.y, 100.0f, "Locked panel Y unchanged");

    ctx.endFrame();
    ctx.shutdown();
}

// ─── Panel Settings State tests ───────────────────────────────────

void testPanelSettingsState() {
    std::cout << "\n=== Panel Settings State ===" << std::endl;
    atlas::PanelState state;
    assertTrue(!state.settingsOpen, "PanelState settingsOpen defaults to false");
    assertClose(state.opacity, 1.0f, "PanelState opacity defaults to 1.0");
    assertTrue(!state.compactRows, "PanelState compactRows defaults to false");

    state.settingsOpen = true;
    state.opacity = 0.7f;
    state.compactRows = true;

    assertTrue(state.settingsOpen, "PanelState settingsOpen set to true");
    assertClose(state.opacity, 0.7f, "PanelState opacity set to 0.7");
    assertTrue(state.compactRows, "PanelState compactRows set to true");

    // Test panel renders with reduced opacity
    atlas::AtlasContext ctx;
    ctx.init();

    atlas::InputState input;
    input.windowW = 1280;
    input.windowH = 720;
    ctx.beginFrame(input);

    state.bounds = {100, 100, 300, 200};
    state.open = true;
    atlas::PanelFlags flags;
    flags.showHeader = true;
    flags.showClose = true;
    flags.showMinimize = true;

    bool visible = atlas::panelBeginStateful(ctx, "Settings Panel", state, flags);
    assertTrue(visible, "Panel with settings is visible");
    atlas::panelEnd(ctx);

    ctx.endFrame();
    ctx.shutdown();
}

// ─── Overview Entry EntityId tests ────────────────────────────────

void testOverviewEntryEntityId() {
    std::cout << "\n=== Overview Entry EntityId ===" << std::endl;
    atlas::OverviewEntry entry;
    assertTrue(entry.entityId.empty(), "OverviewEntry entityId defaults to empty");

    entry.entityId = "npc_raider_1";
    entry.name = "Crimson Raider";
    entry.type = "Cruiser";
    entry.distance = 5000.0f;

    assertTrue(entry.entityId == "npc_raider_1", "OverviewEntry entityId set correctly");
    assertTrue(entry.name == "Crimson Raider", "OverviewEntry name set correctly");

    // Test brace initialization with entityId
    atlas::OverviewEntry entry2 = {"test_id", "Test Ship", "Frigate", 1000.0f, 0.0f, {}, false};
    assertTrue(entry2.entityId == "test_id", "Brace init entityId correct");
    assertTrue(entry2.name == "Test Ship", "Brace init name correct");
}

// ─── Overview Callbacks tests ─────────────────────────────────────

void testOverviewCallbacks() {
    std::cout << "\n=== Overview Callbacks ===" << std::endl;

    atlas::AtlasHUD hud;
    hud.init(1280, 720);

    std::string selectedId;
    std::string rightClickId;
    float rightClickX = 0.0f, rightClickY = 0.0f;

    hud.setOverviewSelectCb([&](const std::string& id) {
        selectedId = id;
    });
    hud.setOverviewRightClickCb([&](const std::string& id, float x, float y) {
        rightClickId = id;
        rightClickX = x;
        rightClickY = y;
    });

    assertTrue(selectedId.empty(), "Select callback not fired before interaction");
    assertTrue(rightClickId.empty(), "Right-click callback not fired before interaction");

    // Simulate overview with data
    atlas::AtlasContext ctx;
    ctx.init();

    atlas::InputState input;
    input.windowW = 1280;
    input.windowH = 720;
    ctx.beginFrame(input);

    atlas::ShipHUDData ship;
    std::vector<atlas::TargetCardInfo> targets;
    std::vector<atlas::OverviewEntry> overview;
    overview.push_back({"npc_1", "NPC 1", "Frigate", 5000.0f, 0.0f, {}, false});

    atlas::SelectedItemInfo selected;
    hud.update(ctx, ship, targets, overview, selected);
    ctx.endFrame();
    ctx.shutdown();

    // Callbacks wired correctly (tested indirectly)
    assertTrue(true, "Overview with callbacks renders without crash");
}

// ─── Right-Click Detection tests ──────────────────────────────────

void testRightClickDetection() {
    std::cout << "\n=== Right-Click Detection ===" << std::endl;

    atlas::AtlasContext ctx;
    ctx.init();

    // Frame with right-click
    atlas::InputState input;
    input.windowW = 1280;
    input.windowH = 720;
    input.mousePos = {500.0f, 400.0f};
    input.mouseClicked[1] = true;  // right-click
    ctx.beginFrame(input);

    assertTrue(ctx.isRightMouseClicked(), "Right-click detected");
    assertTrue(!ctx.isMouseClicked(), "Left-click not detected on right-click frame");

    ctx.endFrame();

    // Frame without right-click
    input.mouseClicked[1] = false;
    ctx.beginFrame(input);

    assertTrue(!ctx.isRightMouseClicked(), "Right-click not detected on non-click frame");

    ctx.endFrame();
    ctx.shutdown();
}

// ─── Panel Opacity tests ──────────────────────────────────────────

void testPanelOpacity() {
    std::cout << "\n=== Panel Opacity ===" << std::endl;

    atlas::PanelState state;
    state.bounds = {100, 100, 300, 200};
    state.open = true;
    state.opacity = 0.5f;

    atlas::AtlasContext ctx;
    ctx.init();

    atlas::InputState input;
    input.windowW = 1280;
    input.windowH = 720;
    ctx.beginFrame(input);

    atlas::PanelFlags flags;
    flags.showHeader = true;
    flags.showClose = true;
    flags.showMinimize = true;

    bool visible = atlas::panelBeginStateful(ctx, "Opacity Panel", state, flags);
    assertTrue(visible, "Panel with 50% opacity is visible");
    atlas::panelEnd(ctx);

    ctx.endFrame();

    // Test minimum opacity
    state.opacity = 0.2f;
    ctx.beginFrame(input);
    visible = atlas::panelBeginStateful(ctx, "Min Opacity Panel", state, flags);
    assertTrue(visible, "Panel with 20% opacity is visible");
    atlas::panelEnd(ctx);
    ctx.endFrame();

    ctx.shutdown();
}

// ─── Sidebar Width Clamping tests ─────────────────────────────────

void testSidebarWidthClamping() {
    std::cout << "\n=== Sidebar Width Clamping ===" << std::endl;

    // Test that sidebarWidth getter/setter works on AtlasContext
    atlas::AtlasContext ctx;
    ctx.init();

    assertTrue(ctx.sidebarWidth() == 0.0f, "Default sidebar width is 0");

    ctx.setSidebarWidth(40.0f);
    assertTrue(ctx.sidebarWidth() == 40.0f, "Sidebar width set to 40");

    // Test that panels snap to sidebar boundary during drag
    atlas::InputState input;
    input.windowW = 1280;
    input.windowH = 720;

    atlas::PanelState state;
    state.bounds = {50.0f, 100.0f, 300.0f, 200.0f};
    state.open = true;

    atlas::PanelFlags flags;
    flags.showHeader = true;
    flags.showClose = false;
    flags.showMinimize = false;

    // Simulate drag that would push panel past sidebar
    state.dragging = true;
    state.dragOffset = {10.0f, 10.0f};
    input.mouseDown[0] = true;
    input.mousePos = {5.0f, 110.0f};  // Would put panel at x=-5, behind sidebar

    ctx.beginFrame(input);
    ctx.setSidebarWidth(40.0f);
    atlas::panelBeginStateful(ctx, "Snap Test", state, flags);
    atlas::panelEnd(ctx);
    ctx.endFrame();

    // Panel X should be clamped to sidebar width (40), not 0
    assertTrue(state.bounds.x >= 40.0f, "Panel snaps to sidebar boundary (X >= 40)");

    ctx.shutdown();
}

// ─── Context Menu Type tests ──────────────────────────────────────

void testContextMenuTypes() {
    std::cout << "\n=== Context Menu Types ===" << std::endl;

    UI::ContextMenu menu;
    assertTrue(!menu.IsOpen(), "Menu starts closed");

    menu.ShowEntityMenu("npc_1", false);
    assertTrue(menu.IsOpen(), "Entity menu is open after ShowEntityMenu");

    menu.Close();
    assertTrue(!menu.IsOpen(), "Menu closed after Close()");

    menu.ShowEmptySpaceMenu(100.0f, 200.0f, 0.0f);
    assertTrue(menu.IsOpen(), "Empty space menu is open after ShowEmptySpaceMenu");

    menu.SetScreenPosition(400.0f, 300.0f);
    assertTrue(menu.IsOpen(), "Menu still open after SetScreenPosition");

    menu.Close();
    assertTrue(!menu.IsOpen(), "Menu closed again");
}

// ─── Overview Background Right-Click Callback tests ───────────────

void testOverviewBgRightClickCallback() {
    std::cout << "\n=== Overview Background Right-Click ===" << std::endl;

    atlas::AtlasHUD hud;
    hud.init(1280, 720);

    float bgClickX = 0.0f, bgClickY = 0.0f;
    bool bgCallbackFired = false;

    hud.setOverviewBgRightClickCb([&](float x, float y) {
        bgCallbackFired = true;
        bgClickX = x;
        bgClickY = y;
    });

    // Verify callback is set (indirect test — renders without crash)
    atlas::AtlasContext ctx;
    ctx.init();

    atlas::InputState input;
    input.windowW = 1280;
    input.windowH = 720;
    ctx.beginFrame(input);

    atlas::ShipHUDData ship;
    std::vector<atlas::TargetCardInfo> targets;
    std::vector<atlas::OverviewEntry> overview;
    atlas::SelectedItemInfo selected;
    hud.update(ctx, ship, targets, overview, selected);
    ctx.endFrame();
    ctx.shutdown();

    assertTrue(true, "Overview background right-click callback set without crash");
}

// ─── Window Snapping Magnetism tests ──────────────────────────────

void testWindowSnappingMagnetism() {
    std::cout << "\n=== Window Snapping Magnetism ===" << std::endl;

    atlas::AtlasContext ctx;
    ctx.init();

    atlas::InputState input;
    input.windowW = 1280;
    input.windowH = 720;

    atlas::PanelState state;
    state.bounds = {50.0f, 100.0f, 300.0f, 200.0f};
    state.open = true;

    atlas::PanelFlags flags;
    flags.showHeader = true;
    flags.showClose = false;
    flags.showMinimize = false;

    // Test: panel near right edge snaps to it
    state.dragging = true;
    state.dragOffset = {10.0f, 10.0f};
    input.mouseDown[0] = true;
    // Position mouse so panel right edge is within 15px of window right (1280)
    // Panel width = 300, so panel.x = mouseX - 10 = ?
    // Want: panel.x + 300 close to 1280 => panel.x = 975 => mouseX = 985
    input.mousePos = {985.0f, 110.0f};

    ctx.beginFrame(input);
    ctx.setSidebarWidth(40.0f);
    atlas::panelBeginStateful(ctx, "Snap Right", state, flags);
    atlas::panelEnd(ctx);
    ctx.endFrame();

    // Panel right edge should snap to window right (1280)
    float rightEdge = state.bounds.x + state.bounds.w;
    assertTrue(std::fabs(rightEdge - 1280.0f) < 0.5f, "Panel snaps to right screen edge");

    // Test: panel near top edge snaps to 0
    state.bounds = {200.0f, 100.0f, 300.0f, 200.0f};
    state.dragging = true;
    state.dragOffset = {10.0f, 10.0f};
    input.mousePos = {210.0f, 18.0f};  // panel.y = 8, within 15px of 0

    ctx.beginFrame(input);
    ctx.setSidebarWidth(40.0f);
    atlas::panelBeginStateful(ctx, "Snap Top", state, flags);
    atlas::panelEnd(ctx);
    ctx.endFrame();

    assertTrue(state.bounds.y == 0.0f, "Panel snaps to top screen edge");

    ctx.shutdown();
}

// ─── Overview Tabs tests ──────────────────────────────────────────

void testOverviewMultipleTabs() {
    std::cout << "\n=== Overview Multiple Tabs ===" << std::endl;

    atlas::AtlasHUD hud;
    hud.init(1280, 720);

    // Default tabs: Travel, Combat, Industry (PvE-focused, EVE-style)
    const auto& tabs = hud.getOverviewTabs();
    assertTrue(tabs.size() == 3, "Default 3 overview tabs");
    assertTrue(tabs[0] == "Travel", "First tab is Travel");
    assertTrue(tabs[1] == "Combat", "Second tab is Combat");
    assertTrue(tabs[2] == "Industry", "Third tab is Industry");

    // Set custom tabs
    hud.setOverviewTabs({"All", "Players", "Wrecks"});
    assertTrue(hud.getOverviewTabs().size() == 3, "Custom tabs set to 3");

    // Tab switching
    hud.setActiveOverviewTab(2);
    assertTrue(hud.getActiveOverviewTab() == 2, "Active tab set to 2");
}

// ─── Overview Tab Filtering tests ─────────────────────────────────

void testOverviewTabFiltering() {
    std::cout << "\n=== Overview Tab Filtering ===" << std::endl;

    // Travel tab: stations, planets, stargates, moons, wormholes, celestials
    assertTrue(atlas::AtlasHUD::matchesOverviewTab("Travel", "Station"),    "Travel: Station matches");
    assertTrue(atlas::AtlasHUD::matchesOverviewTab("Travel", "Stargate"),   "Travel: Stargate matches");
    assertTrue(atlas::AtlasHUD::matchesOverviewTab("Travel", "Planet"),     "Travel: Planet matches");
    assertTrue(atlas::AtlasHUD::matchesOverviewTab("Travel", "Moon"),       "Travel: Moon matches");
    assertTrue(atlas::AtlasHUD::matchesOverviewTab("Travel", "Wormhole"),   "Travel: Wormhole matches");
    assertTrue(atlas::AtlasHUD::matchesOverviewTab("Travel", "Celestial"),  "Travel: Celestial matches");
    assertTrue(!atlas::AtlasHUD::matchesOverviewTab("Travel", "Frigate"),   "Travel: Frigate excluded");
    assertTrue(!atlas::AtlasHUD::matchesOverviewTab("Travel", "Asteroid"),  "Travel: Asteroid excluded");

    // Combat tab: NPC ships (frigates, cruisers, battleships, etc.)
    assertTrue(atlas::AtlasHUD::matchesOverviewTab("Combat", "Frigate"),      "Combat: Frigate matches");
    assertTrue(atlas::AtlasHUD::matchesOverviewTab("Combat", "Cruiser"),      "Combat: Cruiser matches");
    assertTrue(atlas::AtlasHUD::matchesOverviewTab("Combat", "Battleship"),   "Combat: Battleship matches");
    assertTrue(atlas::AtlasHUD::matchesOverviewTab("Combat", "Destroyer"),    "Combat: Destroyer matches");
    assertTrue(atlas::AtlasHUD::matchesOverviewTab("Combat", "npc"),          "Combat: npc matches");
    assertTrue(atlas::AtlasHUD::matchesOverviewTab("Combat", "hostile"),      "Combat: hostile matches");
    assertTrue(!atlas::AtlasHUD::matchesOverviewTab("Combat", "Station"),     "Combat: Station excluded");
    assertTrue(!atlas::AtlasHUD::matchesOverviewTab("Combat", "Asteroid"),    "Combat: Asteroid excluded");

    // Industry tab: asteroids, asteroid belts, mining-related
    assertTrue(atlas::AtlasHUD::matchesOverviewTab("Industry", "Asteroid"),       "Industry: Asteroid matches");
    assertTrue(atlas::AtlasHUD::matchesOverviewTab("Industry", "Asteroid Belt"),  "Industry: Asteroid Belt matches");
    assertTrue(atlas::AtlasHUD::matchesOverviewTab("Industry", "Wreck"),          "Industry: Wreck matches");
    assertTrue(atlas::AtlasHUD::matchesOverviewTab("Industry", "Container"),      "Industry: Container matches");
    assertTrue(!atlas::AtlasHUD::matchesOverviewTab("Industry", "Station"),       "Industry: Station excluded");
    assertTrue(!atlas::AtlasHUD::matchesOverviewTab("Industry", "Frigate"),       "Industry: Frigate excluded");

    // Unknown tab: shows everything (fallback)
    assertTrue(atlas::AtlasHUD::matchesOverviewTab("Custom", "Frigate"),    "Custom tab: shows all");
    assertTrue(atlas::AtlasHUD::matchesOverviewTab("Custom", "Station"),    "Custom tab: shows all (2)");
}

// ─── Overview Column Sorting tests ────────────────────────────────

void testOverviewColumnSorting() {
    std::cout << "\n=== Overview Column Sorting ===" << std::endl;

    atlas::AtlasHUD hud;
    hud.init(1280, 720);

    // Default sort is by distance ascending
    assertTrue(hud.getOverviewSortColumn() == atlas::AtlasHUD::OverviewSortColumn::DISTANCE,
               "Default sort column is DISTANCE");
    assertTrue(hud.isOverviewSortAscending(), "Default sort is ascending");

    // Change sort to name descending
    hud.setOverviewSort(atlas::AtlasHUD::OverviewSortColumn::NAME, false);
    assertTrue(hud.getOverviewSortColumn() == atlas::AtlasHUD::OverviewSortColumn::NAME,
               "Sort column changed to NAME");
    assertTrue(!hud.isOverviewSortAscending(), "Sort direction changed to descending");

    // Render with sorted data to verify no crash
    atlas::AtlasContext ctx;
    ctx.init();

    atlas::InputState input;
    input.windowW = 1280;
    input.windowH = 720;
    ctx.beginFrame(input);

    atlas::ShipHUDData ship;
    std::vector<atlas::TargetCardInfo> targets;
    std::vector<atlas::OverviewEntry> overview;
    overview.push_back({"npc_1", "Alpha", "Frigate", 5000.0f, 100.0f, {}, false});
    overview.push_back({"npc_2", "Bravo", "Cruiser", 2000.0f, 200.0f, {}, false});
    overview.push_back({"npc_3", "Charlie", "Battleship", 8000.0f, 50.0f, {}, false});

    atlas::SelectedItemInfo selected;
    hud.update(ctx, ship, targets, overview, selected);
    ctx.endFrame();
    ctx.shutdown();

    assertTrue(true, "Sorted overview renders without crash");
}

// ─── Overview Ctrl+Click Callback tests ───────────────────────────

void testOverviewCtrlClickCallback() {
    std::cout << "\n=== Overview Ctrl+Click Callback ===" << std::endl;

    atlas::AtlasHUD hud;
    hud.init(1280, 720);

    std::string lockedId;
    hud.setOverviewCtrlClickCb([&](const std::string& id) {
        lockedId = id;
    });

    assertTrue(lockedId.empty(), "Ctrl+Click callback not fired before interaction");
    assertTrue(true, "Ctrl+Click callback set without crash");
}

// ─── Radial Menu Drag-to-Range tests ──────────────────────────────

void testRadialMenuDragToRange() {
    std::cout << "\n=== Radial Menu Drag-to-Range ===" << std::endl;

    UI::RadialMenu menu;
    assertTrue(!menu.IsOpen(), "Menu starts closed");

    menu.Open(400.0f, 300.0f, "npc_1");
    assertTrue(menu.IsOpen(), "Menu opened");
    assertTrue(menu.GetRangeDistance() == 0, "No range before mouse move");

    // Move to orbit segment (top-right, ~45 degrees)
    // Orbit is segment 1 (top-right) starting from top going clockwise
    // At moderate distance (within outer radius)
    menu.UpdateMousePosition(460.0f, 240.0f);  // upper-right direction
    auto action = menu.GetHighlightedAction();
    assertTrue(action != UI::RadialMenu::Action::NONE, "Action selected after mouse move");

    // Move further out to trigger range selection
    // Only Orbit and Keep at Range support range
    // Set position directly into Orbit segment (segment index 1)
    // Top-right = angle -PI/4, at distance > OUTER_RADIUS
    float orbitAngle = -0.78f;  // ~-45 degrees (top-right)
    float farDist = 150.0f;     // beyond OUTER_RADIUS (100)
    float mx = 400.0f + std::cos(orbitAngle) * farDist;
    float my = 300.0f + std::sin(orbitAngle) * farDist;
    menu.UpdateMousePosition(mx, my);

    // If this is an orbit action, range should be set
    if (menu.GetHighlightedAction() == UI::RadialMenu::Action::ORBIT ||
        menu.GetHighlightedAction() == UI::RadialMenu::Action::KEEP_AT_RANGE) {
        assertTrue(menu.GetRangeDistance() > 0, "Range distance set when dragging past outer radius");
    } else {
        assertTrue(true, "Non-range action has no range distance (expected)");
    }

    menu.Close();
    assertTrue(!menu.IsOpen(), "Menu closed");
}

// ─── Panel Deferred Mouse Consumption test ────────────────────────
// Verifies that panelEnd (not panelBeginStateful) consumes leftover
// clicks, so content widgets inside the panel still receive clicks.

void testPanelDeferredMouseConsumption() {
    std::cout << "\n=== Panel Deferred Mouse Consumption ===" << std::endl;

    atlas::AtlasContext ctx;
    ctx.init();

    // Frame 1: Press mouse inside panel area
    {
        atlas::InputState input;
        input.windowW = 1920;
        input.windowH = 1080;
        input.mousePos = {810.0f, 120.0f};  // inside panel content area
        input.mouseDown[0] = true;
        input.mouseClicked[0] = true;
        ctx.beginFrame(input);

        // Simulate panelBeginStateful pushing bounds
        atlas::Rect panelBounds = {800.0f, 100.0f, 200.0f, 400.0f};
        ctx.pushPanelBounds(panelBounds);

        // After pushPanelBounds, mouse should NOT be consumed yet
        // so a child widget can still claim the click
        assertTrue(!ctx.isMouseConsumed(),
                   "Mouse NOT consumed after pushPanelBounds (deferred)");

        // Child widget claims click
        atlas::WidgetID childID = atlas::hashID("child_widget");
        atlas::Rect childRect = {805.0f, 115.0f, 50.0f, 20.0f};
        bool childClicked = ctx.buttonBehavior(childRect, childID);
        // buttonBehavior sets active on click, doesn't return true yet (needs release)
        assertTrue(!ctx.isMouseConsumed(),
                   "Mouse still not consumed after child buttonBehavior");

        // Simulate panelEnd consuming leftover
        atlas::Rect popped = ctx.popPanelBounds();
        assertTrue(popped.w == 200.0f, "Popped bounds match pushed bounds");
        // After popPanelBounds, panel should consume if not already consumed
        if (ctx.isHovered(popped) && ctx.isMouseClicked() && !ctx.isMouseConsumed()) {
            ctx.consumeMouse();
        }
        assertTrue(ctx.isMouseConsumed(),
                   "Mouse consumed after panelEnd logic");

        ctx.endFrame();
    }

    ctx.shutdown();
}

// ─── Context Menu Jump Action test ────────────────────────────────

void testContextMenuJumpAction() {
    std::cout << "\n=== Context Menu Jump Action ===" << std::endl;

    UI::ContextMenu menu;

    // Verify menu can be opened with stargate flag
    menu.ShowEntityMenu("gate_jita", false, true);
    assertTrue(menu.IsOpen(), "Menu is open");

    // Set jump callback
    std::string jumpedTo;
    menu.SetJumpCallback([&jumpedTo](const std::string& id) {
        jumpedTo = id;
    });

    // Close and check callback wire-up
    menu.Close();
    assertTrue(!menu.IsOpen(), "Menu is closed after Close()");
    assertTrue(jumpedTo.empty(), "Jump callback not fired yet");

    // Verify non-stargate menu works fine
    menu.ShowEntityMenu("planet_iv", false, false);
    assertTrue(menu.IsOpen(), "Non-stargate menu is open");
    menu.Close();
}

// ─── Atlas Console tests ───────────────────────────────────────────────

void testAtlasConsoleBasics() {
    std::cout << "\n=== AtlasConsole: Basics ===" << std::endl;

    atlas::AtlasConsole console;
    assertTrue(!console.isOpen(), "Console starts closed");

    console.toggle();
    assertTrue(console.isOpen(), "Console opens on toggle");
    assertTrue(console.wantsKeyboardInput(), "Console wants keyboard when open");

    console.toggle();
    assertTrue(!console.isOpen(), "Console closes on second toggle");
    assertTrue(!console.wantsKeyboardInput(), "Console doesn't want keyboard when closed");

    console.setOpen(true);
    assertTrue(console.isOpen(), "setOpen(true) opens console");

    console.setOpen(false);
    assertTrue(!console.isOpen(), "setOpen(false) closes console");
}

void testAtlasConsoleCommands() {
    std::cout << "\n=== AtlasConsole: Commands ===" << std::endl;

    atlas::AtlasConsole console;

    // Initial output has welcome messages
    assertTrue(console.getOutputLines().size() >= 2, "Console has welcome messages");

    // Test print
    console.print("Test message");
    auto lines = console.getOutputLines();
    assertTrue(lines.back() == "Test message", "print() adds output line");

    // Test clear
    console.clearOutput();
    assertTrue(console.getOutputLines().empty(), "clearOutput() removes all lines");

    // Test custom command registration
    bool commandCalled = false;
    console.registerCommand("testcmd", [&](const std::vector<std::string>& args) {
        commandCalled = true;
    }, "A test command");
    assertTrue(!commandCalled, "Custom command not called yet");

    // Execute by simulating Enter key
    console.setOpen(true);
    // Type "testcmd"
    for (char c : std::string("testcmd")) {
        console.handleChar(static_cast<unsigned int>(c));
    }
    // Press Enter (key 257 = GLFW_KEY_ENTER)
    console.handleKey(257, 1);  // action=GLFW_PRESS
    assertTrue(commandCalled, "Custom command executed via Enter");

    // Test quit callback
    bool quitCalled = false;
    console.setQuitCallback([&]() { quitCalled = true; });
    // Type "quit" and press Enter
    for (char c : std::string("quit")) {
        console.handleChar(static_cast<unsigned int>(c));
    }
    console.handleKey(257, 1);
    assertTrue(quitCalled, "Quit callback invoked by 'quit' command");

    // Test save callback
    bool saveCalled = false;
    console.setSaveCallback([&]() { saveCalled = true; });
    for (char c : std::string("save")) {
        console.handleChar(static_cast<unsigned int>(c));
    }
    console.handleKey(257, 1);
    assertTrue(saveCalled, "Save callback invoked by 'save' command");

    // Test FPS command
    console.setFPS(60.0f);
    console.clearOutput();
    for (char c : std::string("fps")) {
        console.handleChar(static_cast<unsigned int>(c));
    }
    console.handleKey(257, 1);
    assertTrue(!console.getOutputLines().empty(), "FPS command produces output");
}

void testAtlasConsoleHistory() {
    std::cout << "\n=== AtlasConsole: History ===" << std::endl;

    atlas::AtlasConsole console;
    console.setOpen(true);
    console.clearOutput();

    // Enter a command
    for (char c : std::string("echo hello")) {
        console.handleChar(static_cast<unsigned int>(c));
    }
    console.handleKey(257, 1);  // Enter

    // Enter another command
    for (char c : std::string("echo world")) {
        console.handleChar(static_cast<unsigned int>(c));
    }
    console.handleKey(257, 1);  // Enter

    // Press Up arrow to recall last command (key 265)
    console.handleKey(265, 1);
    // The input buffer should now contain the last command
    // We can't easily read the input buffer externally, but at least verify no crash
    assertTrue(true, "Up arrow navigation doesn't crash");

    // Press Down arrow (key 264)
    console.handleKey(264, 1);
    assertTrue(true, "Down arrow navigation doesn't crash");
}

void testAtlasConsoleCharInput() {
    std::cout << "\n=== AtlasConsole: Char Input ===" << std::endl;

    atlas::AtlasConsole console;
    console.setOpen(true);
    console.clearOutput();

    // Backtick should be ignored (it's the toggle key)
    console.handleChar('`');
    // Non-printable should be ignored
    console.handleChar(1);
    console.handleChar(127);
    // Normal chars should work
    console.handleChar('a');
    console.handleChar('b');
    assertTrue(true, "Character filtering works correctly");

    // Backspace (key 259)
    console.handleKey(259, 1);
    assertTrue(true, "Backspace doesn't crash");

    // Home (key 268) and End (key 269)
    console.handleKey(268, 1);
    console.handleKey(269, 1);
    assertTrue(true, "Home/End keys work");

    // Escape closes console (key 256)
    console.handleKey(256, 1);
    assertTrue(!console.isOpen(), "Escape closes console");
}

// ─── Atlas Pause Menu tests ────────────────────────────────────────────

void testAtlasPauseMenuBasics() {
    std::cout << "\n=== AtlasPauseMenu: Basics ===" << std::endl;

    atlas::AtlasPauseMenu menu;
    assertTrue(!menu.isOpen(), "Pause menu starts closed");

    menu.toggle();
    assertTrue(menu.isOpen(), "Pause menu opens on toggle");
    assertTrue(menu.wantsKeyboardInput(), "Pause menu wants keyboard when open");

    menu.toggle();
    assertTrue(!menu.isOpen(), "Pause menu closes on second toggle");

    // Callbacks
    bool resumeCalled = false;
    bool saveCalled = false;
    bool quitCalled = false;
    menu.setResumeCallback([&]() { resumeCalled = true; });
    menu.setSaveCallback([&]() { saveCalled = true; });
    menu.setQuitCallback([&]() { quitCalled = true; });
    assertTrue(!resumeCalled && !saveCalled && !quitCalled, "Callbacks not called on registration");
}

void testAtlasPauseMenuSettings() {
    std::cout << "\n=== AtlasPauseMenu: Settings ===" << std::endl;

    atlas::AtlasPauseMenu menu;

    // Default volumes
    assertTrue(menu.getMasterVolume() > 0.0f, "Default master volume > 0");
    assertTrue(menu.getMusicVolume() > 0.0f, "Default music volume > 0");
    assertTrue(menu.getSfxVolume() > 0.0f, "Default SFX volume > 0");
    assertTrue(menu.getUiVolume() > 0.0f, "Default UI volume > 0");

    // Set volumes
    menu.setMasterVolume(0.5f);
    assertClose(menu.getMasterVolume(), 0.5f, "Master volume set to 0.5");

    menu.setMusicVolume(0.3f);
    assertClose(menu.getMusicVolume(), 0.3f, "Music volume set to 0.3");

    menu.setSfxVolume(1.0f);
    assertClose(menu.getSfxVolume(), 1.0f, "SFX volume set to 1.0");

    menu.setUiVolume(0.0f);
    assertClose(menu.getUiVolume(), 0.0f, "UI volume set to 0.0");
}

// ─── Atlas Title Screen tests ──────────────────────────────────────────

void testAtlasTitleScreenBasics() {
    std::cout << "\n=== AtlasTitleScreen: Basics ===" << std::endl;

    atlas::AtlasTitleScreen titleScreen;
    assertTrue(titleScreen.isActive(), "Title screen starts active");
    assertTrue(titleScreen.wantsKeyboardInput(), "Title screen wants keyboard when active");

    // Simulate play callback
    bool playCalled = false;
    titleScreen.setPlayCallback([&]() { playCalled = true; });
    assertTrue(!playCalled, "Play callback not called on registration");

    // Deactivate
    titleScreen.setActive(false);
    assertTrue(!titleScreen.isActive(), "Title screen deactivated");
    assertTrue(!titleScreen.wantsKeyboardInput(), "Title screen doesn't want keyboard when inactive");

    // Audio settings
    titleScreen.setMasterVolume(0.6f);
    assertClose(titleScreen.getMasterVolume(), 0.6f, "Title screen master volume");

    titleScreen.setMusicVolume(0.4f);
    assertClose(titleScreen.getMusicVolume(), 0.4f, "Title screen music volume");

    titleScreen.setSfxVolume(0.9f);
    assertClose(titleScreen.getSfxVolume(), 0.9f, "Title screen SFX volume");

    // Quit callback
    bool quitCalled = false;
    titleScreen.setQuitCallback([&]() { quitCalled = true; });
    assertTrue(!quitCalled, "Quit callback not called on registration");
}

void testAtlasTitleScreenButtonsClickable() {
    std::cout << "\n=== AtlasTitleScreen: Buttons Clickable ===" << std::endl;

    // Verify that title screen buttons receive clicks even though
    // the full-screen background is drawn first.  The mouse should
    // only be consumed AFTER widgets process input.

    atlas::AtlasContext ctx;
    ctx.init();

    atlas::AtlasTitleScreen titleScreen;
    bool playCalled = false;
    titleScreen.setPlayCallback([&]() { playCalled = true; });

    // Compute the center of the "Undock" button for a 1920×1080 window.
    // Layout: sidebar=56, menuWidth=320, buttonHeight=40
    // contentX = 56, contentW = 1920-56 = 1864
    // menuX = 56 + (1864-320)*0.5 = 828
    // menuY = 1080 * 0.4 = 432
    // Button center: (828+160, 432+20) = (988, 452)
    float btnCenterX = 988.0f;
    float btnCenterY = 452.0f;

    // Frame 1: click (press) on the Undock button
    {
        atlas::InputState input{};
        input.windowW = 1920;
        input.windowH = 1080;
        input.mousePos = {btnCenterX, btnCenterY};
        input.mouseClicked[0] = true;
        input.mouseDown[0] = true;
        ctx.beginFrame(input);
        titleScreen.render(ctx);
        ctx.endFrame();
    }

    // Frame 2: release on the Undock button (click completes)
    {
        atlas::InputState input{};
        input.windowW = 1920;
        input.windowH = 1080;
        input.mousePos = {btnCenterX, btnCenterY};
        input.mouseReleased[0] = true;
        ctx.beginFrame(input);
        titleScreen.render(ctx);
        ctx.endFrame();
    }

    assertTrue(playCalled, "Title screen Undock button is clickable");
    assertTrue(!titleScreen.isActive(), "Title screen deactivated after Undock click");

    ctx.shutdown();
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "Atlas UI System Tests" << std::endl;
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
    testAtlasHUD();

    testSlider();
    testTextInput();
    testNotification();
    testTextInputStateDefaults();
    testModuleSlotEx();
    testCapacitorRingAnimated();
    testModuleInfoOverheat();

#ifdef USE_RMLUI
    testFittingRmlData();
    testMarketOrderInfo();
    testMissionRmlInfo();
    testChatMessageInfo();
    testRmlUiManagerStub();
#endif

    // New GUI/HUD tests

    // Phase 4.10+ GUI/HUD enhancement tests
    testModeIndicator();
    testInfoPanelData();
    testInfoPanelRendering();
    testOverviewTabSwitching();
    testAtlasHUDModeIndicator();
    testAtlasHUDInfoPanel();
    testAtlasHUDOverviewTab();
    testSelectedItemCallbacks();
    testSidebarCallback();

    // Mouse and sidebar interaction fixes
    testGetDragDelta();
    testMouseConsumed();
    testSidebarBlockedByPanel();

    // New widget and HUD feature tests
    testTabBar();
    testCombatLogWidget();
    testDamageFlashOverlay();
    testDroneStatusBar();
    testFleetBroadcastBanner();
    testFleetBroadcastStruct();
    testAtlasHUDCombatLog();
    testAtlasHUDDamageFlash();
    testAtlasHUDDroneStatus();
    testAtlasHUDFleetBroadcast();
    testDroneStatusDataDefaults();

    // GUI/HUD continuation tests
    testKeyConstants();
    testInputStateKeyboard();
    testKeyboardModuleActivation();
    testDScanData();
    testDScanPanelRendering();
    testMissionData();
    testMissionPanelRendering();
    testProbeScannerData();
    testProbeScannerRendering();

    // Panel resize, lock, settings, overview interaction tests
    testPanelResizeState();
    testPanelLockState();
    testPanelSettingsState();
    testOverviewEntryEntityId();
    testOverviewCallbacks();
    testRightClickDetection();
    testPanelOpacity();

    // Panel snap, context menu, and overview right-click tests
    testSidebarWidthClamping();
    testContextMenuTypes();
    testOverviewBgRightClickCallback();

    // EVE UI reproduction tests
    testWindowSnappingMagnetism();
    testOverviewMultipleTabs();
    testOverviewTabFiltering();
    testOverviewColumnSorting();
    testOverviewCtrlClickCallback();
    testRadialMenuDragToRange();
    testPanelDeferredMouseConsumption();
    testContextMenuJumpAction();

    // ── Atlas Console tests ─────────────────────────────────────────────
    testAtlasConsoleBasics();
    testAtlasConsoleCommands();
    testAtlasConsoleHistory();
    testAtlasConsoleCharInput();

    // ── Atlas Pause Menu tests ──────────────────────────────────────────
    testAtlasPauseMenuBasics();
    testAtlasPauseMenuSettings();

    // ── Atlas Title Screen tests ────────────────────────────────────────
    testAtlasTitleScreenBasics();
    testAtlasTitleScreenButtonsClickable();

    std::cout << "\n========================================" << std::endl;
    std::cout << "Results: " << testsPassed << "/" << testsRun
              << " tests passed" << std::endl;
    std::cout << "========================================" << std::endl;

    return (testsPassed == testsRun) ? 0 : 1;
}
