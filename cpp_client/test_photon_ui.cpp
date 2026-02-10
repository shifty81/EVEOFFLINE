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

    std::cout << "\n========================================" << std::endl;
    std::cout << "Results: " << testsPassed << "/" << testsRun
              << " tests passed" << std::endl;
    std::cout << "========================================" << std::endl;

    return (testsPassed == testsRun) ? 0 : 1;
}
