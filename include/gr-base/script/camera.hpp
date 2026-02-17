#pragma once

#include "default.hpp"

#include "gr-base/core/rendertarget.hpp"

#include <gr-render/gRender.h>
#include <memory>

#include "gr-base/ecs/behavior.hpp"

class Camera : public BehaviorImpl<Camera>
{
public:
    static Camera *Current();

    Camera();
    ~Camera() override;

    void Init() override;

    bool Serialize(Stream *stream) const override;

    bool Deserialize(Stream *stream) override;

    Matrix4x4 getPerspesctive() const;

    Matrix4x4 getViewmatrix() const;

    Vector3 getPosition() const;

    void lookIt(const Vector3 &direction);
    
    void setRect(const Rect &rect);

    Rect getRect() const;

    RenderTarget* getRenderTarget();

    Color background;

private:
    RenderTarget* rendertarget;

    Matrix4x4 m_projection;

    float m_nearPlane;
    
    float m_farPlane;
    
    float m_aspect;

    float m_fov;

    Rect m_rect;

    void updateProjection();
};

REGISTER_COMPONENT(Camera, (uint64_t)-1)






