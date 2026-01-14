#pragma once

#include "default.hpp"

#include <gr-ecs/behavior.hpp>

#include <gr-math/gmath.hpp>

class Light : public BehaviorImpl<Light>
{
public:
    enum LightType : int {
        eDirection = 0,
        ePoint     = 1,
        eSpot      = 2,
    };

    static std::vector<Light *> GetListLight();

    Light(uint8_t type = eDirection, const Color& color = Color::white, float aConstant = 1.0f, float aLinear = 1.0f, float aQuadract = 1.0f, float inneCone = 1.0f, float outerCone = 1.0f);
    ~Light();
    
    void Enable() override;

    void Disable() override;

    bool Serialize(Stream *stream) const override;

    bool Deserialize(Stream *stream) override;

    Vector3 getPosition() const;

    Vector3 getForward() const;

    Color getColor() const;

    void setColor(const Color& color);

    void setType(int value);

    int getType() const;

    float getAttenuationConstant() const;

    void setAttenuationConstant(float value);

    float getAttenuationLinear() const;

    void setAttenuationLinear(float value);
    
    float getAttenuationQuadractc() const;

    void setAttenuationQuadractc(float value);

    float getInnecone() const;

    void setInnecone(float value);

    float getOutercone() const;

    void setOutercone(float value);

    Matrix4x4 getProjetion() const;

private:
    static std::vector<Light *> m_lights;

    static Matrix4x4 mProjection;

    int m_type;

    Transform* m_transform;

    Color m_color;

    float m_attenuationConstant;

    float m_attenuationLinear;

    float m_attenuationQuadratic;

    float m_angleInnerCone;

    float m_angleOuterCone;

    void addInGlobalLight();
};

REGISTER_COMPONENT(Light)
