#pragma once

#include "default.hpp"

#include "gr-base/core/rendertarget.hpp"

#include <gr-render/gRender.h>

#include "gr-base/ecs/behavior.hpp"

class Camera : public BehaviorImpl<Camera>
{
public:
    static std::vector<Camera*> GetCameras();

    static Camera *Main();

    static Camera *Current();

    Camera();
    ~Camera() override;

    void Init() override;

    void Enable() override;

    void Disable() override;

    bool Serialize(Stream *stream) const override;

    bool Deserialize(Stream *stream) override;

    Matrix4x4 getPerspesctive() const;

    Matrix4x4 getViewmatrix() const;

    void setActive(bool value);

    Vector3 getPosition() const;

    void lookIt(const Vector3 &direction);
    
    void setRect(const Rect &rect);

    Rect getRect() const;

    RenderTarget* getRenderTarget();

    Color background;

private:
    static std::vector<Camera*> m_cameraList;

    RenderTarget rendertarget;

    Matrix4x4 m_projection;

    float m_nearPlane;
    
    float m_farPlane;
    
    float m_aspect;

    float m_fov;

    Rect m_rect;

    static Camera *s_current;

    void updateProjection();
};

REGISTER_COMPONENT(Camera)






