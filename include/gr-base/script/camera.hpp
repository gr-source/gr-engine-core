#pragma once

#include "default.hpp"

#include "gr-base/ecs/entity.hpp"

#include "gr-base/core/rendertarget.hpp"

#include <gr-render/gRender.h>

class Camera : public Component {
public:
    static std::vector<Camera *> GetCameras();

    static Camera *Main();

    static Camera *Current();

    Camera();

    void Init() override;

    void Start() override;

    void Destroy() override;

    void Enable() override;

    void Disable() override;

    bool Serialize(Stream *stream) override;

    bool Deserialize(Stream *stream) override;

    Matrix4x4 getPerspesctive() const;

    Matrix4x4 getViewmatrix() const;

    void setAspectRatio(float aspect);

    float get_aspect_ratio() const;

    void setActive(bool value);

    Vector3 getPosition() const;

    void lookIt(const Vector3 &direction);
    
    void setRect(const Rect &rect);

    Rect getRect() const;

    RenderTarget rendertarget;

private:
    static std::vector<Camera *> m_cameraList;

    Transform *m_transform;

    Matrix4x4 m_projection;

    float m_nearPlane;
    
    float m_farPlane;
    
    float m_aspect;

    float m_fov;

    Rect m_rect;

    static Camera *s_current;

    void updateProjection();

    void addInGlobalCam();
};

REGISTER_COMPONENT(Camera, false)






