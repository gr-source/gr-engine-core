#include "gr-base/script/light.hpp"

#include "gr-base/script/transform.hpp"

#include "gr-base/core/file/stream.hpp"

#include "gr-base/core/gameObject.hpp"

#include <algorithm>

std::vector<Light *> Light::m_lights;

Matrix4x4 Light::mProjection = Math::orthographic(-10.0f, 10.0f, -10.0f, 10.0f, 0.1f, 100.0f);

std::vector<Light *> Light::GetListLight()
{
    return m_lights;
}

Light::Light(std::uint8_t type, const Color& color, float aConstant, float aLinear, float aQuadract, float inneCone, float outerCone) : m_type(static_cast<int>(type)), m_color(color), m_attenuationConstant(aConstant), m_attenuationLinear(aLinear), m_attenuationQuadratic(aQuadract), m_angleInnerCone(inneCone), m_angleOuterCone(outerCone)
{
    m_lights.push_back(this);
}

Light::~Light()
{
    m_lights.erase(std::remove(m_lights.begin(), m_lights.end(), this), m_lights.end());
}

void Light::Enable()
{
    m_lights.push_back(this);
}

void Light::Disable()
{
    m_lights.erase(std::remove(m_lights.begin(), m_lights.end(), this), m_lights.end());
}

bool Light::Serialize(Stream *stream) const
{
    stream->write(reinterpret_cast<const char *>(&m_type), sizeof(int));
    stream->write(reinterpret_cast<const char *>(&m_color), sizeof(Color));
    stream->write(reinterpret_cast<const char *>(&m_attenuationConstant), sizeof(float));
    stream->write(reinterpret_cast<const char *>(&m_attenuationLinear), sizeof(float));
    stream->write(reinterpret_cast<const char *>(&m_attenuationQuadratic), sizeof(float));
    stream->write(reinterpret_cast<const char *>(&m_angleInnerCone), sizeof(float));
    stream->write(reinterpret_cast<const char *>(&m_angleOuterCone), sizeof(float));
    return true;
}

bool Light::Deserialize(Stream *stream)
{
    stream->read(reinterpret_cast<char *>(&m_type), sizeof(int));
    stream->read(reinterpret_cast<char *>(&m_color), sizeof(Color));
    stream->read(reinterpret_cast<char *>(&m_attenuationConstant), sizeof(float));
    stream->read(reinterpret_cast<char *>(&m_attenuationLinear), sizeof(float));
    stream->read(reinterpret_cast<char *>(&m_attenuationQuadratic), sizeof(float));
    stream->read(reinterpret_cast<char *>(&m_angleInnerCone), sizeof(float));
    stream->read(reinterpret_cast<char *>(&m_angleOuterCone), sizeof(float));
    return true;
}

Vector3 Light::getPosition() const {
    return GetGameObject()->getTransform()->getPosition();
}

Vector3 Light::getForward() const {
    return GetGameObject()->getTransform()->getForward();
}

Color Light::getColor() const {
    return m_color;
}

void Light::setColor(const Color &color) {
    m_color = color;
}

void Light::setType(int value) {
    m_type = value;
}

int Light::getType() const {
    return m_type;
}

float Light::getAttenuationConstant() const {
    return m_attenuationConstant;
}

void Light::setAttenuationConstant(float value) {
    m_attenuationConstant = value;
}

float Light::getAttenuationLinear() const {
    return m_attenuationLinear;
}

void Light::setAttenuationLinear(float value) {
    m_attenuationLinear = value;
}

float Light::getAttenuationQuadractc() const {
    return m_attenuationQuadratic;
}

void Light::setAttenuationQuadractc(float value) {
    m_attenuationQuadratic = value;
}

float Light::getInnecone() const {
    return m_angleInnerCone;
}

void Light::setInnecone(float value) {
    m_angleInnerCone = value;
}

float Light::getOutercone() const {
    return m_angleOuterCone;
}

void Light::setOutercone(float value) {
    m_angleOuterCone = value;
}

Matrix4x4 Light::getProjetion() const
{
    auto transform = GetGameObject()->getTransform();

    return mProjection * Math::lookAt(transform->getPosition(), transform->getPosition() + transform->getForward(), transform->getUp());
}

