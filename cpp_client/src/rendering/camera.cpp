#include "rendering/camera.h"
#include <algorithm>

namespace eve {

Camera::Camera(float fov, float aspectRatio, float nearPlane, float farPlane)
    : m_target(0.0f, 0.0f, 0.0f)
    , m_distance(500.0f)
    , m_yaw(0.0f)
    , m_pitch(30.0f)
    , m_fov(fov)
    , m_aspectRatio(aspectRatio)
    , m_nearPlane(nearPlane)
    , m_farPlane(farPlane)
    , m_position(0.0f, 0.0f, 0.0f)
    , m_forward(0.0f, 0.0f, -1.0f)
    , m_right(1.0f, 0.0f, 0.0f)
    , m_up(0.0f, 1.0f, 0.0f)
{
    updateVectors();
}

void Camera::update(float deltaTime) {
    // Can add smooth interpolation here if needed
    updateVectors();
}

glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(m_position, m_target, m_up);
}

glm::mat4 Camera::getProjectionMatrix() const {
    return glm::perspective(glm::radians(m_fov), m_aspectRatio, m_nearPlane, m_farPlane);
}

void Camera::setTarget(const glm::vec3& target) {
    m_target = target;
    updateVectors();
}

glm::vec3 Camera::getPosition() const {
    return m_position;
}

void Camera::zoom(float delta) {
    m_distance -= delta * 50.0f;
    m_distance = std::clamp(m_distance, MIN_DISTANCE, MAX_DISTANCE);
    updateVectors();
}

void Camera::rotate(float deltaYaw, float deltaPitch) {
    m_yaw += deltaYaw;
    m_pitch += deltaPitch;
    
    // Clamp pitch to prevent camera flipping
    m_pitch = std::clamp(m_pitch, MIN_PITCH, MAX_PITCH);
    
    updateVectors();
}

void Camera::pan(float deltaX, float deltaY) {
    // Pan perpendicular to view direction
    float panSpeed = m_distance * 0.001f;
    m_target += m_right * deltaX * panSpeed;
    m_target += m_up * deltaY * panSpeed;
    updateVectors();
}

void Camera::setAspectRatio(float aspectRatio) {
    m_aspectRatio = aspectRatio;
}

void Camera::setDistance(float distance) {
    m_distance = std::clamp(distance, MIN_DISTANCE, MAX_DISTANCE);
    updateVectors();
}

void Camera::updateVectors() {
    // Calculate position based on spherical coordinates
    float yawRad = glm::radians(m_yaw);
    float pitchRad = glm::radians(m_pitch);
    
    glm::vec3 offset;
    offset.x = m_distance * cos(pitchRad) * sin(yawRad);
    offset.y = m_distance * sin(pitchRad);
    offset.z = m_distance * cos(pitchRad) * cos(yawRad);
    
    m_position = m_target + offset;
    
    // Calculate camera vectors
    m_forward = glm::normalize(m_target - m_position);
    m_right = glm::normalize(glm::cross(m_forward, glm::vec3(0.0f, 1.0f, 0.0f)));
    m_up = glm::normalize(glm::cross(m_right, m_forward));
}

} // namespace eve
