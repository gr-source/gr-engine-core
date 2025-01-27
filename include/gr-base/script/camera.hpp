#pragma once

#include "default.hpp"

#include "gr-base/ecs/entity.hpp"

class Camera : public Component {
public:
    static std::vector<Camera*> s_camera;
    
    static Camera *Main();

    static Camera *Current();

    Camera();

    void Init() override;

    void Destroy() override;

    Matrix4x4 getPerspesctive() const;

    Matrix4x4 getViewmatrix() const;

    void setAspectRatio(float aspect);

    void setActive(bool value);

    Vector3 getPosition() const;

    void lookIt(const Vector3 &direction);

    void Enable() override;

    void Disable() override;
    
private:
    Transform *m_transform;

    Matrix4x4 m_projection;

    float m_nearPlane;
    
    float m_farPlane;
    
    float m_aspect;

    float m_fov;

    static Camera *s_current;

    void updateProjection();
};

REGISTER_COMPONENT(Camera);






