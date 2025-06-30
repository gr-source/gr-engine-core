#include "gr-base/script/camera.hpp"

#include "gr-base/core/rendertarget.hpp"
#include "gr-base/core/thread/threadpool.hpp"
#include "gr-base/script/transform.hpp"

#include "gr-base/core/display.hpp"

#include "gr-base/ecs/gameObject.hpp"

#include "gr-base/core/file/stream.hpp"

#include <algorithm>

#include <gr-render/gTexture.h>

std::vector<Camera *> Camera::m_cameraList;

Camera *Camera::s_current = nullptr;

std::vector<Camera *> Camera::GetCameras() {
    return m_cameraList;
}

Camera *Camera::Main() {
    if (m_cameraList.empty()) {
        return s_current;
    }
    return m_cameraList[0];
}

Camera *Camera::Current() {
    return s_current;
}

Camera::Camera() : m_transform(nullptr), m_fov(45.0f), m_aspect(1.0f), m_farPlane(1000.0f), m_nearPlane(0.001f), m_projection(Matrix4x4::identityMatrix), m_rect({0.0f, 0.0f, 1.0f, 1.0f}), rendertarget(1024, 1024)
{
}

void Camera::Init()
{
    m_transform = entity->getComponent<Transform>();

    addInGlobalCam();

    if (!s_current)
    {
        s_current = this;
    }
    
    auto rect = Display::GetDisplayRect();
    
    m_aspect = static_cast<float>(rect.w) / static_cast<float>(rect.h);

    updateProjection();

    if (Threadpool::IsMainThread())
        rendertarget.init();
}

void Camera::Destroy()
{
    if (this == s_current)
    {
        s_current = nullptr;
    }
    m_cameraList.erase(std::remove(m_cameraList.begin(), m_cameraList.end(), this), m_cameraList.end());

    if (Threadpool::IsMainThread())
        rendertarget.shutdown();
}

void Camera::Enable() {
    addInGlobalCam();

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
    m_cameraList.erase(std::remove(m_cameraList.begin(), m_cameraList.end(), this), m_cameraList.end());
}

bool Camera::Serialize(Stream *stream)
{
    if (!stream->write(&m_projection, sizeof(Matrix4x4)))
    {
        return false;
    }

    if (!stream->write(&m_nearPlane, sizeof(float)))
    {
        return false;
    }

    if (!stream->write(&m_farPlane, sizeof(float)))
    {
        return false;
    }

    if (!stream->write(&m_aspect, sizeof(float)))
    {
        return false;
    }

    if (!stream->write(&m_fov, sizeof(float)))
    {
        return false;
    }

    if (!stream->write(&m_rect, sizeof(Rect)))
    {
        return false;
    }
    return false;
}

bool Camera::Deserialize(Stream *stream)
{
    if (!stream->read(reinterpret_cast<char *>(&m_projection), sizeof(Matrix4x4)))
    {
        return false;
    }

    if (!stream->read(reinterpret_cast<char *>(&m_nearPlane), sizeof(float)))
    {
        return false;
    }

    if (!stream->read(reinterpret_cast<char *>(&m_farPlane), sizeof(float)))
    {
        return false;
    }

    if (!stream->read(reinterpret_cast<char *>(&m_aspect), sizeof(float)))
    {
        return false;
    }

    if (!stream->read(reinterpret_cast<char *>(&m_fov), sizeof(float)))
    {
        return false;
    }

    if (!stream->read(reinterpret_cast<char *>(&m_rect), sizeof(Rect)))
    {
        return false;
    }
    return false;
}

Matrix4x4 Camera::getPerspesctive() const
{
    return m_projection;
}

Matrix4x4 Camera::getViewmatrix() const
{
    return Math::lookAt(m_transform->getPosition(), m_transform->getPosition() + m_transform->getForward(), m_transform->getUp());
}

void Camera::setAspectRatio(float aspect)
{
    if (aspect <= 0.0f) {
        return;
    }
    m_aspect = aspect;

    updateProjection();
}

float Camera::get_aspect_ratio() const
{
    return m_aspect;
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

void Camera::setRect(const Rect &rect) {
    m_rect = rect;
}

Rect Camera::getRect() const {
    return m_rect;
}

void Camera::updateProjection() {
    m_projection = Math::perspective(Math::rad(m_fov), m_aspect, m_nearPlane, m_farPlane);
}

void Camera::addInGlobalCam()
{
    auto it = std::find(m_cameraList.begin(), m_cameraList.end(), this);
    if (it != m_cameraList.end())
    {
        return;
    }
    m_cameraList.push_back(this);
}


