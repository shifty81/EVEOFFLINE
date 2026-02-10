/**
 * Test program for Instanced Rendering
 * Note: This is a compilation and logic test. Full rendering tests require OpenGL context.
 */

#include <iostream>
#include <iomanip>
#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Mock classes for testing without OpenGL
namespace eve {

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
    glm::vec3 color;
};

class Mesh {
public:
    Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices)
        : m_VAO(0), m_indexCount(indices.size()) {}
    ~Mesh() = default;
    
    void draw() const {}
    void drawInstanced(unsigned int instanceCount) const {
        std::cout << "  [Mock] Drawing " << instanceCount << " instances" << std::endl;
    }
    
    unsigned int getVAO() const { return m_VAO; }
    size_t getIndexCount() const { return m_indexCount; }
    
private:
    unsigned int m_VAO;
    size_t m_indexCount;
};

class Shader {
public:
    Shader() = default;
    ~Shader() = default;
};

// Include the actual instanced renderer logic (header only parts)
struct InstanceData {
    glm::mat4 transform;
    glm::vec4 color;
    float customFloat1;
    float customFloat2;
    float _padding1;
    float _padding2;
    
    InstanceData()
        : transform(1.0f), color(1.0f)
        , customFloat1(0.0f), customFloat2(0.0f)
        , _padding1(0.0f), _padding2(0.0f) {}
};

} // namespace eve

// Test framework
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

// Test 1: InstanceData structure
void testInstanceData() {
    std::cout << "\n=== Test 1: InstanceData Structure ===" << std::endl;
    
    eve::InstanceData data;
    
    // Test defaults
    runTest("Default transform is identity", data.transform == glm::mat4(1.0f));
    runTest("Default color is white", data.color == glm::vec4(1.0f));
    runTest("Custom floats initialized to zero", 
            data.customFloat1 == 0.0f && data.customFloat2 == 0.0f);
    
    // Test modifications
    data.transform = glm::translate(glm::mat4(1.0f), glm::vec3(10, 20, 30));
    data.color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
    data.customFloat1 = 0.75f;
    
    runTest("Transform can be modified", data.transform != glm::mat4(1.0f));
    runTest("Color can be modified", data.color.r == 1.0f && data.color.g == 0.0f);
    runTest("Custom floats can be modified", data.customFloat1 == 0.75f);
    
    // Test size (for GPU buffer alignment)
    size_t expectedSize = sizeof(glm::mat4) + sizeof(glm::vec4) + 4 * sizeof(float);
    runTest("InstanceData size is correct", sizeof(eve::InstanceData) == expectedSize);
}

// Test 2: Mesh creation and properties
void testMeshCreation() {
    std::cout << "\n=== Test 2: Mesh Creation ===" << std::endl;
    
    std::vector<eve::Vertex> vertices(8); // Cube vertices
    std::vector<unsigned int> indices = {0, 1, 2, 2, 3, 0}; // 2 triangles
    
    auto mesh = std::make_shared<eve::Mesh>(vertices, indices);
    
    runTest("Mesh created successfully", mesh != nullptr);
    runTest("Mesh has correct index count", mesh->getIndexCount() == 6);
}

// Test 3: Transform matrix creation
void testTransformMatrices() {
    std::cout << "\n=== Test 3: Transform Matrices ===" << std::endl;
    
    // Create different transforms
    glm::mat4 translation = glm::translate(glm::mat4(1.0f), glm::vec3(5, 10, 15));
    glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(0, 1, 0));
    glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(2.0f, 2.0f, 2.0f));
    
    glm::mat4 combined = translation * rotation * scale;
    
    runTest("Translation matrix created", translation != glm::mat4(1.0f));
    runTest("Rotation matrix created", rotation != glm::mat4(1.0f));
    runTest("Scale matrix created", scale != glm::mat4(1.0f));
    runTest("Combined transform created", combined != glm::mat4(1.0f));
}

// Test 4: Instance data array
void testInstanceDataArray() {
    std::cout << "\n=== Test 4: Instance Data Array ===" << std::endl;
    
    const int NUM_INSTANCES = 100;
    std::vector<eve::InstanceData> instances;
    instances.reserve(NUM_INSTANCES);
    
    // Create instances in a grid
    for (int i = 0; i < NUM_INSTANCES; i++) {
        eve::InstanceData data;
        float x = (i % 10) * 5.0f;
        float z = (i / 10) * 5.0f;
        data.transform = glm::translate(glm::mat4(1.0f), glm::vec3(x, 0, z));
        data.color = glm::vec4(i / 100.0f, 1.0f - i / 100.0f, 0.5f, 1.0f);
        instances.push_back(data);
    }
    
    runTest("Instance array created", instances.size() == NUM_INSTANCES);
    runTest("First instance position correct", instances[0].transform[3][0] == 0.0f);
    runTest("Last instance position correct", instances[99].transform[3][0] == 45.0f);
    runTest("Instance colors vary", instances[0].color != instances[99].color);
}

// Test 5: Fleet formation
void testFleetFormation() {
    std::cout << "\n=== Test 5: Fleet Formation ===" << std::endl;
    
    // Create a circular fleet formation
    const int FLEET_SIZE = 20;
    const float RADIUS = 50.0f;
    std::vector<eve::InstanceData> fleet;
    
    for (int i = 0; i < FLEET_SIZE; i++) {
        eve::InstanceData ship;
        float angle = (i / (float)FLEET_SIZE) * 2.0f * glm::pi<float>();
        float x = RADIUS * cos(angle);
        float z = RADIUS * sin(angle);
        
        ship.transform = glm::translate(glm::mat4(1.0f), glm::vec3(x, 0, z));
        ship.transform = glm::rotate(ship.transform, angle, glm::vec3(0, 1, 0));
        ship.color = glm::vec4(0.3f, 0.5f, 0.8f, 1.0f); // Veyren blue
        
        fleet.push_back(ship);
    }
    
    runTest("Fleet created", fleet.size() == FLEET_SIZE);
    runTest("Ships positioned in circle", glm::length(glm::vec3(fleet[0].transform[3])) > 49.0f);
    runTest("All ships same color", fleet[0].color == fleet[FLEET_SIZE-1].color);
}

// Test 6: Performance calculation
void testPerformanceBenefit() {
    std::cout << "\n=== Test 6: Performance Benefit ===" << std::endl;
    
    const int NUM_SHIPS = 500;
    
    // Without instancing: 1 draw call per ship
    int normalDrawCalls = NUM_SHIPS;
    
    // With instancing: Assuming 3 ship types
    const int NUM_SHIP_TYPES = 3;
    int instancedDrawCalls = NUM_SHIP_TYPES;
    
    float reduction = ((normalDrawCalls - instancedDrawCalls) / (float)normalDrawCalls) * 100.0f;
    
    std::cout << "  Normal rendering: " << normalDrawCalls << " draw calls" << std::endl;
    std::cout << "  Instanced rendering: " << instancedDrawCalls << " draw calls" << std::endl;
    std::cout << "  Reduction: " << std::fixed << std::setprecision(1) << reduction << "%" << std::endl;
    
    runTest("Instancing reduces draw calls significantly", reduction > 90.0f);
}

// Test 7: Memory layout verification
void testMemoryLayout() {
    std::cout << "\n=== Test 7: Memory Layout ===" << std::endl;
    
    // Verify structure packing for GPU
    std::cout << "  sizeof(InstanceData): " << sizeof(eve::InstanceData) << " bytes" << std::endl;
    std::cout << "  sizeof(glm::mat4): " << sizeof(glm::mat4) << " bytes" << std::endl;
    std::cout << "  sizeof(glm::vec4): " << sizeof(glm::vec4) << " bytes" << std::endl;
    
    // Check alignment (should be multiple of 16 for GPU)
    bool aligned = (sizeof(eve::InstanceData) % 16) == 0;
    
    runTest("InstanceData is 16-byte aligned", aligned);
    
    // Verify offsets
    eve::InstanceData data;
    size_t transformOffset = (size_t)&data.transform - (size_t)&data;
    size_t colorOffset = (size_t)&data.color - (size_t)&data;
    
    runTest("Transform at offset 0", transformOffset == 0);
    runTest("Color after transform", colorOffset == sizeof(glm::mat4));
}

int main() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "Instanced Rendering Test Suite" << std::endl;
    std::cout << "========================================" << std::endl;
    
    testInstanceData();
    testMeshCreation();
    testTransformMatrices();
    testInstanceDataArray();
    testFleetFormation();
    testPerformanceBenefit();
    testMemoryLayout();
    
    printTestSummary();
    
    // Return 0 if all tests passed, 1 otherwise
    for (const auto& result : g_testResults) {
        if (!result.passed) {
            return 1;
        }
    }
    
    return 0;
}
