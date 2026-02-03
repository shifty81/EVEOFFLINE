#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace eve {

/**
 * Camera class for 3D view
 * Implements EVE-style orbit camera
 */
class Camera {
public:
    Camera(float fov = 45.0f, float aspectRatio = 16.0f / 9.0f, float nearPlane = 0.1f, float farPlane = 10000.0f);

    /**
     * Update camera (call each frame)
     */
    void update(float deltaTime);

    /**
     * Get view matrix
     */
    glm::mat4 getViewMatrix() const;

    /**
     * Get projection matrix
     */
    glm::mat4 getProjectionMatrix() const;

    /**
     * Set camera target (what to look at)
     */
    void setTarget(const glm::vec3& target);

    /**
     * Get camera position
     */
    glm::vec3 getPosition() const;

    /**
     * Camera controls
     */
    void zoom(float delta);
    void rotate(float deltaYaw, float deltaPitch);
    void pan(float deltaX, float deltaY);

    /**
     * Set aspect ratio (e.g., on window resize)
     */
    void setAspectRatio(float aspectRatio);

    /**
     * Get camera distance from target
     */
    float getDistance() const { return m_distance; }

    /**
     * Set camera distance from target
     */
    void setDistance(float distance);

private:
    void updateVectors();

    // Camera parameters
    glm::vec3 m_target;
    float m_distance;
    float m_yaw;
    float m_pitch;

    // Projection parameters
    float m_fov;
    float m_aspectRatio;
    float m_nearPlane;
    float m_farPlane;

    // Camera vectors
    glm::vec3 m_position;
    glm::vec3 m_forward;
    glm::vec3 m_right;
    glm::vec3 m_up;

    // Limits
    static constexpr float MIN_DISTANCE = 10.0f;
    static constexpr float MAX_DISTANCE = 5000.0f;
    static constexpr float MIN_PITCH = -89.0f;
    static constexpr float MAX_PITCH = 89.0f;
};

} // namespace eve
