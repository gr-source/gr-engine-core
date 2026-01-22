#include "gr-base/script/camera.hpp"

#include "gr-base/core/rendertarget.hpp"

#include "gr-base/core/gameObject.hpp"
#include "gr-base/core/scene.hpp"

#include "gr-base/core/thread/threadpool.hpp"

#include "gr-base/script/transform.hpp"

#include "gr-base/core/file/stream.hpp"

#include "gr-base/ecs/entity.hpp"

#include <gr-render/gTexture.h>

Camera *Camera::Current()
{
    return Scene::GetCurrent()->getCamera();
}

Camera::Camera() : m_fov(45.0f), m_aspect(1.0f), m_farPlane(500.0f), m_nearPlane(0.001f), m_projection(Matrix4x4::identityMatrix), m_rect({0.0f, 0.0f, 1.0f, 1.0f}), rendertarget(512, 512), background(Color::black)
{}

Camera::~Camera()
{
    auto scene = Scene::GetCurrent();
    if (scene != nullptr)
        scene->RemoveCamera(GetGameObject()->GetID<Camera>());

    if (Threadpool::IsMainThread())
        rendertarget.shutdown();
}

void Camera::Init()
{
    m_aspect = static_cast<float>(m_rect.w) / static_cast<float>(m_rect.h);

    updateProjection();

    if (!Threadpool::IsMainThread())
        return;
    
    rendertarget.init();

    auto scene = Scene::GetCurrent();
    if (scene != nullptr)
        scene->AddCamera(GetGameObject()->GetID<Camera>());
}

bool Camera::Serialize(Stream *stream) const
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
    auto transform = GetEntity()->getComponent<Transform>();

    return Math::lookAt(transform->getPosition(), transform->getPosition() + transform->getForward(), transform->getUp());
}

Vector3 Camera::getPosition() const
{
    auto transform = GetEntity()->getComponent<Transform>();
    return transform->getPosition();
}

void Camera::lookIt(const Vector3 &direction)
{
    auto transform = GetEntity()->getComponent<Transform>();
    transform->setLocalRotation(Math::normalize(Math::lookRotation(direction, Vector3::up)));
}

void Camera::setRect(const Rect &rect)
{
    float aspect = static_cast<float>(rect.w) / static_cast<float>(rect.h);
    if (aspect <= 0.0f)
        return;

    if (Threadpool::IsMainThread())
        rendertarget.resize(rect.w, rect.h);

    m_aspect = aspect;
    m_rect = rect;

    updateProjection();
}

Rect Camera::getRect() const {
    return m_rect;
}

RenderTarget* Camera::getRenderTarget()
{
    return &rendertarget;
}

void Camera::updateProjection()
{
    m_projection = Math::perspective(Math::rad(m_fov), m_aspect, m_nearPlane, m_farPlane);
}




