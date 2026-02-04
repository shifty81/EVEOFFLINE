/**
 * Test program for Frustum Culling
 * Validates frustum extraction and entity culling
 */

#include <iostream>
#include <iomanip>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "rendering/frustum_culler.h"
#include "rendering/lod_manager.h"

using namespace eve;

// Simple test framework
struct TestResult {
    std::string name;
    bool passed;
    std::string message;
};

std::vector<TestResult> g_testResults;

void runTest(const std::string& name, bool result, const std::string& message = "") {
    g_testResults.push_back({name, result, message});
    std::cout << (result ? "[PASS] " : "[FAIL] ") << name;
    if (!message.empty() && !result) {
        std::cout << ": " << message;
    }
    std::cout << std::endl;
}

void printTestSummary() {
    int passed = 0;
    int failed = 0;
    
    for (const auto& result : g_testResults) {
        if (result.passed) passed++;
        else failed++;
    }
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "Test Summary: " << passed << " passed, " << failed << " failed" << std::endl;
    std::cout << "========================================\n" << std::endl;
}

// Test 1: Frustum plane extraction
void testFrustumExtraction() {
    std::cout << "\n=== Test 1: Frustum Plane Extraction ===" << std::endl;
    
    // Create a simple perspective projection matrix
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 16.0f / 9.0f, 0.1f, 1000.0f);
    glm::mat4 view = glm::lookAt(
        glm::vec3(0, 0, 10),  // Camera position
        glm::vec3(0, 0, 0),   // Look at origin
        glm::vec3(0, 1, 0)    // Up vector
    );
    glm::mat4 viewProj = projection * view;
    
    Frustum frustum;
    frustum.extractFromMatrix(viewProj);
    
    // Check that planes were extracted (normals should be non-zero)
    bool planesExtracted = true;
    for (int i = 0; i < 6; i++) {
        const Plane& plane = frustum.getPlane(static_cast<Frustum::FrustumPlane>(i));
        float normalLength = glm::length(plane.normal);
        if (normalLength < 0.9f || normalLength > 1.1f) {
            planesExtracted = false;
            break;
        }
    }
    
    runTest("Frustum planes extracted and normalized", planesExtracted);
}

// Test 2: Point containment
void testPointContainment() {
    std::cout << "\n=== Test 2: Point Containment ===" << std::endl;
    
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);
    glm::mat4 view = glm::lookAt(
        glm::vec3(0, 0, 10),
        glm::vec3(0, 0, 0),
        glm::vec3(0, 1, 0)
    );
    glm::mat4 viewProj = projection * view;
    
    Frustum frustum;
    frustum.extractFromMatrix(viewProj);
    
    // Point in front of camera should be visible
    bool centerVisible = frustum.containsPoint(glm::vec3(0, 0, 0));
    runTest("Point at origin visible", centerVisible);
    
    // Point behind camera should not be visible
    bool behindVisible = frustum.containsPoint(glm::vec3(0, 0, 20));
    runTest("Point behind camera not visible", !behindVisible);
    
    // Point far away should not be visible
    bool farVisible = frustum.containsPoint(glm::vec3(0, 0, -200));
    runTest("Point beyond far plane not visible", !farVisible);
}

// Test 3: Sphere culling
void testSphereContainment() {
    std::cout << "\n=== Test 3: Sphere Culling ===" << std::endl;
    
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);
    glm::mat4 view = glm::lookAt(
        glm::vec3(0, 0, 10),
        glm::vec3(0, 0, 0),
        glm::vec3(0, 1, 0)
    );
    glm::mat4 viewProj = projection * view;
    
    Frustum frustum;
    frustum.extractFromMatrix(viewProj);
    
    // Sphere at origin with small radius
    bool centerSphere = frustum.containsSphere(glm::vec3(0, 0, 0), 1.0f);
    runTest("Sphere at origin visible", centerSphere);
    
    // Sphere far to the right (outside frustum)
    bool farRightSphere = frustum.containsSphere(glm::vec3(100, 0, 0), 1.0f);
    runTest("Sphere far right not visible", !farRightSphere);
    
    // Large sphere partially in frustum
    bool largeSphere = frustum.containsSphere(glm::vec3(0, 0, -110), 20.0f);
    runTest("Large sphere at far plane edge visible", largeSphere);
}

// Test 4: AABB culling
void testAABBContainment() {
    std::cout << "\n=== Test 4: AABB Culling ===" << std::endl;
    
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);
    glm::mat4 view = glm::lookAt(
        glm::vec3(0, 0, 10),
        glm::vec3(0, 0, 0),
        glm::vec3(0, 1, 0)
    );
    glm::mat4 viewProj = projection * view;
    
    Frustum frustum;
    frustum.extractFromMatrix(viewProj);
    
    // Small box at origin
    bool centerBox = frustum.containsAABB(glm::vec3(-1, -1, -1), glm::vec3(1, 1, 1));
    runTest("Box at origin visible", centerBox);
    
    // Box completely outside
    bool outsideBox = frustum.containsAABB(glm::vec3(100, 100, 100), glm::vec3(101, 101, 101));
    runTest("Box far outside not visible", !outsideBox);
}

// Test 5: FrustumCuller integration
void testFrustumCuller() {
    std::cout << "\n=== Test 5: FrustumCuller Integration ===" << std::endl;
    
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);
    glm::mat4 view = glm::lookAt(
        glm::vec3(0, 0, 10),
        glm::vec3(0, 0, 0),
        glm::vec3(0, 1, 0)
    );
    glm::mat4 viewProj = projection * view;
    
    FrustumCuller culler;
    culler.update(viewProj);
    
    // Test visibility
    bool visible1 = culler.isVisible(glm::vec3(0, 0, 0), 1.0f);
    runTest("Entity at origin visible", visible1);
    
    bool visible2 = culler.isVisible(glm::vec3(100, 0, 0), 1.0f);
    runTest("Entity far right not visible", !visible2);
    
    // Test stats
    auto stats = culler.getStats();
    runTest("Stats tracked correctly", stats.totalTests == 2);
    runTest("Visible count correct", stats.visibleEntities == 1);
    runTest("Culled count correct", stats.culledEntities == 1);
    
    // Test enable/disable
    culler.setEnabled(false);
    runTest("Culling can be disabled", !culler.isEnabled());
    
    bool visibleWhenDisabled = culler.isVisible(glm::vec3(1000, 1000, 1000), 1.0f);
    runTest("All entities visible when disabled", visibleWhenDisabled);
}

// Test 6: LODManager with frustum culling
void testLODManagerIntegration() {
    std::cout << "\n=== Test 6: LODManager Integration ===" << std::endl;
    
    LODManager lodManager;
    
    // Register some entities
    lodManager.registerEntity(1, glm::vec3(0, 0, 0), 1.0f);      // Center (visible)
    lodManager.registerEntity(2, glm::vec3(100, 0, 0), 1.0f);    // Far right (culled)
    lodManager.registerEntity(3, glm::vec3(0, 100, 0), 1.0f);    // Far up (culled)
    lodManager.registerEntity(4, glm::vec3(0, 0, -5), 1.0f);     // In front (visible)
    
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);
    glm::mat4 view = glm::lookAt(
        glm::vec3(0, 0, 10),
        glm::vec3(0, 0, 0),
        glm::vec3(0, 1, 0)
    );
    glm::mat4 viewProj = projection * view;
    
    // Update with frustum culling
    lodManager.update(glm::vec3(0, 0, 10), 0.0f, &viewProj);
    
    auto stats = lodManager.getStats();
    runTest("LOD manager total entities correct", stats.totalEntities == 4);
    
    // Check individual entity visibility
    bool entity1Visible = lodManager.isEntityVisible(1);
    bool entity2Visible = lodManager.isEntityVisible(2);
    runTest("Center entity visible", entity1Visible);
    runTest("Far right entity culled", !entity2Visible);
    
    // Test disabling frustum culling
    lodManager.setFrustumCullingEnabled(false);
    lodManager.update(glm::vec3(0, 0, 10), 0.0f, &viewProj);
    
    bool allVisible = lodManager.isEntityVisible(1) && lodManager.isEntityVisible(2);
    runTest("All entities visible when culling disabled", allVisible);
}

// Test 7: Performance test
void testPerformance() {
    std::cout << "\n=== Test 7: Performance Test ===" << std::endl;
    
    const int NUM_ENTITIES = 1000;
    
    LODManager lodManager;
    
    // Create entities in a grid
    for (int i = 0; i < NUM_ENTITIES; i++) {
        float x = (i % 32) * 10.0f - 160.0f;
        float y = ((i / 32) % 32) * 10.0f - 160.0f;
        float z = (i / 1024) * 10.0f - 50.0f;
        
        lodManager.registerEntity(i, glm::vec3(x, y, z), 2.0f);
    }
    
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 1000.0f);
    glm::mat4 view = glm::lookAt(
        glm::vec3(0, 0, 100),
        glm::vec3(0, 0, 0),
        glm::vec3(0, 1, 0)
    );
    glm::mat4 viewProj = projection * view;
    
    // Update multiple times to simulate frames
    for (int frame = 0; frame < 100; frame++) {
        lodManager.update(glm::vec3(0, 0, 100), 0.016f, &viewProj);
    }
    
    auto stats = lodManager.getStats();
    float cullRate = (float)(NUM_ENTITIES - stats.visible) / (float)NUM_ENTITIES * 100.0f;
    
    std::cout << "  Entities: " << NUM_ENTITIES << std::endl;
    std::cout << "  Visible: " << stats.visible << std::endl;
    std::cout << "  Culled: " << (NUM_ENTITIES - stats.visible) << std::endl;
    std::cout << "  Cull rate: " << std::fixed << std::setprecision(1) << cullRate << "%" << std::endl;
    
    runTest("Performance test completed", true);
    runTest("Cull rate reasonable", cullRate > 10.0f && cullRate < 99.0f);
}

int main() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "Frustum Culling Test Suite" << std::endl;
    std::cout << "========================================" << std::endl;
    
    testFrustumExtraction();
    testPointContainment();
    testSphereContainment();
    testAABBContainment();
    testFrustumCuller();
    testLODManagerIntegration();
    testPerformance();
    
    printTestSummary();
    
    // Return 0 if all tests passed, 1 otherwise
    for (const auto& result : g_testResults) {
        if (!result.passed) {
            return 1;
        }
    }
    
    return 0;
}
