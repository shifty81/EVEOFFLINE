#include "ui/entity_picker.h"
#include "core/entity.h"
#include "rendering/camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <limits>
#include <iostream>

namespace UI {

EntityPicker::EntityPicker()
    : m_pickingRadius(20.0f)
    , m_lastRayOrigin(0.0f)
    , m_lastRayDirection(0.0f, 0.0f, -1.0f)
{
}

EntityPicker::~EntityPicker() {
}

std::string EntityPicker::pickEntity(
    double mouseX,
    double mouseY,
    int screenWidth,
    int screenHeight,
    const ::eve::Camera& camera,
    const std::vector<std::shared_ptr<::eve::Entity>>& entities
) {
    // Convert screen coordinates to world space ray
    glm::vec3 rayOrigin, rayDirection;
    screenToWorldRay(
        mouseX, mouseY,
        screenWidth, screenHeight,
        camera.getViewMatrix(),
        camera.getProjectionMatrix(),
        rayOrigin, rayDirection
    );
    
    // Store for debugging
    m_lastRayOrigin = rayOrigin;
    m_lastRayDirection = rayDirection;
    
    // Find closest entity that intersects ray
    std::string closestEntityId;
    float closestDistance = std::numeric_limits<float>::max();
    
    for (const auto& entity : entities) {
        if (!entity) continue;
        
        // Get entity position
        glm::vec3 entityPos = entity->getPosition();
        
        // Determine picking radius based on ship type and size
        float pickRadius = m_pickingRadius;  // Default
        std::string shipType = entity->getShipType();
        // Check more specific types first to avoid substring matching issues
        if (shipType.find("Capital") != std::string::npos || 
            shipType.find("Carrier") != std::string::npos ||
            shipType.find("Dreadnought") != std::string::npos) {
            pickRadius = m_pickingRadius * 3.0f;
        } else if (shipType.find("Battlecruiser") != std::string::npos) {
            pickRadius = m_pickingRadius * 2.0f;
        } else if (shipType.find("Battleship") != std::string::npos) {
            pickRadius = m_pickingRadius * 2.0f;
        } else if (shipType.find("Cruiser") != std::string::npos) {
            pickRadius = m_pickingRadius * 1.5f;
        }
        
        // Test ray-sphere intersection
        float hitDistance = raySphereIntersection(
            rayOrigin, rayDirection,
            entityPos, pickRadius
        );
        
        // Check if hit and closer than previous
        if (hitDistance >= 0.0f && hitDistance < closestDistance) {
            closestDistance = hitDistance;
            closestEntityId = entity->getId();
        }
    }
    
    if (!closestEntityId.empty()) {
        std::cout << "[EntityPicker] Picked entity: " << closestEntityId 
                  << " at distance: " << closestDistance << std::endl;
    }
    
    return closestEntityId;
}

void EntityPicker::screenToWorldRay(
    double mouseX,
    double mouseY,
    int screenWidth,
    int screenHeight,
    const glm::mat4& viewMatrix,
    const glm::mat4& projectionMatrix,
    glm::vec3& outRayOrigin,
    glm::vec3& outRayDirection
) {
    // Convert mouse coordinates from screen space to normalized device coordinates (NDC)
    // OpenGL: origin at bottom-left, Y-axis points up
    float x = (2.0f * mouseX) / screenWidth - 1.0f;
    float y = 1.0f - (2.0f * mouseY) / screenHeight;  // Flip Y axis
    float z = 1.0f;  // Point on far plane
    
    // Normalized device coordinates
    glm::vec3 rayNDS(x, y, z);
    
    // Homogeneous clip coordinates
    glm::vec4 rayClip(rayNDS.x, rayNDS.y, -1.0f, 1.0f);
    
    // Eye coordinates (inverse projection)
    glm::mat4 invProj = glm::inverse(projectionMatrix);
    glm::vec4 rayEye = invProj * rayClip;
    rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);
    
    // World coordinates (inverse view)
    glm::mat4 invView = glm::inverse(viewMatrix);
    glm::vec4 rayWorld = invView * rayEye;
    outRayDirection = glm::normalize(glm::vec3(rayWorld));
    
    // Ray origin is camera position (extracted from view matrix)
    glm::mat4 viewMatrixInv = glm::inverse(viewMatrix);
    outRayOrigin = glm::vec3(viewMatrixInv[3]);
}

float EntityPicker::raySphereIntersection(
    const glm::vec3& rayOrigin,
    const glm::vec3& rayDirection,
    const glm::vec3& sphereCenter,
    float sphereRadius
) {
    // Ray: P = O + t*D
    // Sphere: |P - C|^2 = r^2
    // Substituting: |O + t*D - C|^2 = r^2
    // Let L = O - C
    // |L + t*D|^2 = r^2
    // (L + t*D)·(L + t*D) = r^2
    // L·L + 2t(D·L) + t^2(D·D) = r^2
    // Since D is normalized, D·D = 1
    // t^2 + 2t(D·L) + (L·L - r^2) = 0
    
    glm::vec3 L = rayOrigin - sphereCenter;
    
    float a = glm::dot(rayDirection, rayDirection);  // Should be 1.0 if normalized
    float b = 2.0f * glm::dot(rayDirection, L);
    float c = glm::dot(L, L) - sphereRadius * sphereRadius;
    
    // Solve quadratic equation
    float discriminant = b * b - 4.0f * a * c;
    
    if (discriminant < 0.0f) {
        // No intersection
        return -1.0f;
    }
    
    // Two solutions: t1 and t2
    float sqrtDisc = std::sqrt(discriminant);
    float t1 = (-b - sqrtDisc) / (2.0f * a);
    float t2 = (-b + sqrtDisc) / (2.0f * a);
    
    // Return closest positive intersection
    if (t1 >= 0.0f) {
        return t1;
    } else if (t2 >= 0.0f) {
        return t2;
    } else {
        // Ray points away from sphere
        return -1.0f;
    }
}

} // namespace UI
