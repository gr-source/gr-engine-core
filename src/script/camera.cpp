#include "script/camera.hpp"

#include "script/transform.hpp"

#include "core/display.hpp"

#include "ecs/gameObject.hpp"

#include <algorithm>

std::vector<Camera *> Camera::s_camera;

Camera *Camera::s_current = nullptr;

Camera *Camera::Main() {
    if (s_camera.empty()) {
        return s_current;
    }
    return s_camera[0];
}

Camera *Camera::Current() {
    return s_current;
}

Camera::Camera() : m_transform(nullptr), m_fov(45.0f), m_aspect(1.0f), m_farPlane(1000.0f), m_nearPlane(0.001f), m_projection(Matrix4x4::identityMatrix) {
    auto rect = Display::GetDisplayRect();

    m_aspect = static_cast<float>(rect.w) / static_cast<float>(rect.h);
    
    updateProjection();
}

void Camera::Init() {
    m_transform = entity->getComponent<Transform>();

    s_camera.push_back(this);

    if (!s_current) {
        s_current = this;
    }

    auto rect = Display::GetDisplayRect();

    m_aspect = static_cast<float>(rect.w) / static_cast<float>(rect.h);

    updateProjection();
}

void Camera::Destroy() {
    if (this == s_current) {
        s_current = nullptr;
    }
    s_camera.erase(std::remove(s_camera.begin(), s_camera.end(), this), s_camera.end());
}

Matrix4x4 Camera::getPerspesctive() const {
    return m_projection;
}

Matrix4x4 Camera::getViewmatrix() const {
    return Math::lookAt(m_transform->getPosition(), m_transform->getPosition() + m_transform->getForward(), m_transform->getUp());
}

void Camera::setAspectRatio(float aspect) {
    if (aspect <= 0.0f) {
        return;
    }
    m_aspect = aspect;

    updateProjection();
}

void Camera::setActive(bool value) {
    if (value) {
        s_current = this;
        return;
    }
    if (s_current == this) {
        s_current = nullptr;
    }
}

Vector3 Camera::getPosition() const {
    return m_transform->getPosition();
}

void Camera::lookIt(const Vector3 &direction) {
    m_transform->setLocalRotation(Math::normalize(Math::lookRotation(direction, Vector3::up)));
}

void Camera::Enable() {
    s_camera.push_back(this);

    if (!s_current) {
        s_current = this;
    }

    auto rect = Display::GetDisplayRect();

    m_aspect = static_cast<float>(rect.w) / static_cast<float>(rect.h);

    updateProjection();
}

void Camera::Disable() {
    if (this == s_current) {
        s_current = nullptr;
    }
    s_camera.erase(std::remove(s_camera.begin(), s_camera.end(), this), s_camera.end());
}

void Camera::updateProjection() {
    m_projection = Math::perspective(Math::rad(m_fov), m_aspect, m_nearPlane, m_farPlane);
}



