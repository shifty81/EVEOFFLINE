/**
 * Test ship physics system
 *
 * Validates inertia-based heading, ship-class turn rates,
 * heading-driven thrust, roll banking, and gradual turning.
 */

#include "core/ship_physics.h"
#include <iostream>
#include <string>
#include <cmath>

using namespace atlas;

// Test counters
static int testsRun = 0;
static int testsPassed = 0;

static void assertTrue(bool condition, const std::string& testName) {
    testsRun++;
    if (condition) {
        testsPassed++;
        std::cout << "  \xe2\x9c\x93 " << testName << std::endl;
    } else {
        std::cout << "  \xe2\x9c\x97 " << testName << " FAILED" << std::endl;
    }
}

static bool approxEqual(float a, float b, float epsilon = 0.01f) {
    return std::fabs(a - b) < epsilon;
}

// ==================== Turn Rate Tests ====================

void testTurnRateScalesWithShipClass() {
    std::cout << "\n=== Turn Rate Scales With Ship Class ===" << std::endl;

    // Frigate — low mass, low inertia → fast turn rate
    ShipPhysics frigate;
    ShipPhysics::ShipStats frigateStats;
    frigateStats.mass = 1200000.0f;
    frigateStats.inertiaModifier = 2.8f;
    frigateStats.maxVelocity = 325.0f;
    frigateStats.signatureRadius = 35.0f;
    frigate.setShipStats(frigateStats);

    // Battleship — high inertia → slow turn rate
    ShipPhysics battleship;
    ShipPhysics::ShipStats bsStats;
    bsStats.mass = 1200000.0f;
    bsStats.inertiaModifier = 4.5f;
    bsStats.maxVelocity = 105.0f;
    bsStats.signatureRadius = 400.0f;
    battleship.setShipStats(bsStats);

    // Capital — very high inertia → very slow turn rate
    ShipPhysics capital;
    ShipPhysics::ShipStats capStats;
    capStats.mass = 1200000.0f;
    capStats.inertiaModifier = 8.5f;
    capStats.maxVelocity = 80.0f;
    capStats.signatureRadius = 3000.0f;
    capital.setShipStats(capStats);

    float frigateTurn = frigate.getMaxTurnRate();
    float bsTurn = battleship.getMaxTurnRate();
    float capTurn = capital.getMaxTurnRate();

    std::cout << "  Frigate turn rate: " << frigateTurn << " deg/s" << std::endl;
    std::cout << "  Battleship turn rate: " << bsTurn << " deg/s" << std::endl;
    std::cout << "  Capital turn rate: " << capTurn << " deg/s" << std::endl;

    assertTrue(frigateTurn > bsTurn, "Frigate turns faster than battleship");
    assertTrue(bsTurn > capTurn, "Battleship turns faster than capital");
    assertTrue(frigateTurn >= 3.0f && frigateTurn <= 60.0f, "Frigate turn rate in valid range");
    assertTrue(capTurn >= 3.0f && capTurn <= 60.0f, "Capital turn rate in valid range");
}

// ==================== Heading-Based Thrust Tests ====================

void testHeadingDrivenThrust() {
    std::cout << "\n=== Heading-Driven Thrust ===" << std::endl;

    ShipPhysics ship;
    // Ship starts facing +Z
    // Set desired direction to +X (90 degree turn)
    ship.setDesiredDirection(glm::vec3(1.0f, 0.0f, 0.0f));

    // After one small time step, the ship should NOT have velocity purely in +X
    // because it hasn't turned to face +X yet — it's still mostly facing +Z
    ship.update(0.016f);  // ~1 frame at 60fps

    glm::vec3 vel = ship.getVelocity();
    glm::vec3 heading = ship.getHeading();

    // Heading should still be mostly toward +Z (not instant snap to +X)
    assertTrue(heading.z > heading.x, "Heading still mostly +Z after one frame");

    // Velocity should be mostly toward where the ship is facing (heading)
    // not purely toward the desired direction (+X)
    float velDotHeading = glm::dot(glm::normalize(vel), heading);
    assertTrue(velDotHeading > 0.5f, "Velocity aligns with heading, not desired direction");
}

// ==================== Gradual Turning Test ====================

void testGradualTurning() {
    std::cout << "\n=== Gradual Turning (Not Instant) ===" << std::endl;

    ShipPhysics ship;
    // Ship starts facing +Z
    glm::vec3 initialHeading = ship.getHeading();
    assertTrue(approxEqual(initialHeading.z, 1.0f, 0.01f), "Initial heading is +Z");

    // Command a 135 degree turn (face -X-Z direction)
    ship.setDesiredDirection(glm::vec3(-1.0f, 0.0f, -1.0f));

    // After 0.5 seconds, heading should NOT be fully reversed yet
    for (int i = 0; i < 30; i++) {
        ship.update(1.0f / 60.0f);
    }

    glm::vec3 midHeading = ship.getHeading();
    glm::vec3 targetDir = glm::normalize(glm::vec3(-1.0f, 0.0f, -1.0f));
    float dotWithTarget = glm::dot(midHeading, targetDir);
    assertTrue(dotWithTarget < 0.95f, "Heading not fully reversed after 0.5s (gradual turn)");

    // But it should have started turning (not still at +Z)
    float dotWithOriginal = glm::dot(midHeading, glm::vec3(0.0f, 0.0f, 1.0f));
    assertTrue(dotWithOriginal < 0.99f, "Heading has started turning from +Z");
}

// ==================== Roll Angle Test ====================

void testRollAngleDuringTurn() {
    std::cout << "\n=== Roll Angle During Turn ===" << std::endl;

    ShipPhysics ship;
    // Ship starts facing +Z, command turn to +X
    ship.setDesiredDirection(glm::vec3(1.0f, 0.0f, 0.0f));

    // Simulate a few frames
    for (int i = 0; i < 10; i++) {
        ship.update(1.0f / 60.0f);
    }

    float rollAngle = ship.getRollAngle();
    assertTrue(std::fabs(rollAngle) > 0.001f, "Roll angle is non-zero during turn");
    assertTrue(std::fabs(rollAngle) <= 0.36f, "Roll angle within max bounds (~20 deg)");
}

// ==================== Angular Velocity Test ====================

void testAngularVelocity() {
    std::cout << "\n=== Angular Velocity ===" << std::endl;

    ShipPhysics ship;
    // At rest, no angular velocity
    ship.update(0.016f);
    assertTrue(ship.getAngularVelocity() < 0.01f, "No angular velocity when not turning");

    // Start turning
    ship.setDesiredDirection(glm::vec3(1.0f, 0.0f, 0.0f));
    ship.update(0.016f);
    assertTrue(ship.getAngularVelocity() > 0.0f, "Angular velocity > 0 when turning");
}

// ==================== Ship Stops Turning When Aligned ====================

void testStopTurningWhenAligned() {
    std::cout << "\n=== Stop Turning When Aligned ===" << std::endl;

    ShipPhysics ship;
    // Already facing +Z, desired direction is also +Z
    ship.setDesiredDirection(glm::vec3(0.0f, 0.0f, 1.0f));
    ship.update(0.016f);

    float angVel = ship.getAngularVelocity();
    assertTrue(angVel < 0.01f, "No turning needed when already aligned");
}

// ==================== Warp Still Works ====================

void testWarpBehaviorUnchanged() {
    std::cout << "\n=== Warp Behavior Unchanged ===" << std::endl;

    ShipPhysics ship;
    glm::vec3 dest(0.0f, 0.0f, 200000.0f);  // 200km away
    ship.warpTo(dest);

    assertTrue(ship.isWarping(), "Ship enters warp");
    assertTrue(ship.getWarpPhase() == ShipPhysics::WarpPhase::ALIGNING,
               "Warp starts with aligning phase");
}

// ==================== Stop Command ====================

void testStopDecaysRollAndAngularVelocity() {
    std::cout << "\n=== Stop Decays Roll and Angular Velocity ===" << std::endl;

    ShipPhysics ship;
    // Build up some roll and angular velocity
    ship.setDesiredDirection(glm::vec3(1.0f, 0.0f, 0.0f));
    for (int i = 0; i < 30; i++) {
        ship.update(1.0f / 60.0f);
    }
    assertTrue(std::fabs(ship.getRollAngle()) > 0.001f, "Roll angle built up during turn");

    // Now stop
    ship.stop();
    for (int i = 0; i < 120; i++) {
        ship.update(1.0f / 60.0f);
    }

    assertTrue(std::fabs(ship.getRollAngle()) < 0.05f, "Roll angle decays after stop");
}

// ==================== Main ====================

int main() {
    std::cout << "=== Ship Physics Tests ===" << std::endl;

    testTurnRateScalesWithShipClass();
    testHeadingDrivenThrust();
    testGradualTurning();
    testRollAngleDuringTurn();
    testAngularVelocity();
    testStopTurningWhenAligned();
    testWarpBehaviorUnchanged();
    testStopDecaysRollAndAngularVelocity();

    std::cout << "\n=== Results: " << testsPassed << "/" << testsRun << " passed ===" << std::endl;
    return (testsPassed == testsRun) ? 0 : 1;
}
