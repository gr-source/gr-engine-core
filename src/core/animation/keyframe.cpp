#include "gr-base/core/animation/keyframe.hpp"
#include "gr-base/core/file/stream.hpp"
#include <cstddef>
#include <gr-math/gmath.hpp>
#include <gr-math/vector3.hpp>
#include <iterator>

Keyframe::Keyframe(const std::string &name) : m_name(name) {}

const std::string &Keyframe::getName() const {
    return m_name;
}

const Matrix4x4 Keyframe::getLocalMatrix(float time) const {
    return Math::CreateTRS(getInterpolateScaling(time), getInterpolateRotation(time), getInterpolatePosition(time));;// Math::translate<Matrix4x4>() * Math::rotate<Matrix4x4>() * Math::scale<Matrix4x4>();
}

const Vector3 Keyframe::getInterpolateScaling(float time) const {
    if (1 == m_keyScale.size())
        return m_keyScale[0].value;

    auto p0Index = getIndexFromTime<Vector3>(time, m_keyScale);
    auto p1Index = p0Index + 1;

    float scaleFactor = GetScaleFactor(m_keyScale[p0Index].time, m_keyScale[p1Index].time, time);

    const Vector3 finalScale = Math::mix(m_keyScale[p0Index].value, m_keyScale[p1Index].value, scaleFactor);

    return finalScale;
}

const Quaternion Keyframe::getInterpolateRotation(float time) const {
    if (1 == m_keyRotation.size()) {
        return m_keyRotation[0].value;
    }

    int p0Index = getIndexFromTime<Quaternion>(time, m_keyRotation);
    int p1Index = p0Index + 1;

    float scaleFactor = GetScaleFactor(m_keyRotation[p0Index].time, m_keyRotation[p1Index].time, time);
    
    return Math::normalize(Math::slerp(m_keyRotation[p1Index].value, m_keyRotation[p0Index].value, scaleFactor));
}

const Vector3 Keyframe::getInterpolatePosition(float time) const {
    if (1 == m_keyPosition.size())
        return m_keyPosition[0].value;

    int p0Index = getIndexFromTime<Vector3>(time, m_keyPosition);
    int p1Index = p0Index + 1;

    float scaleFactor = GetScaleFactor(m_keyPosition[p0Index].time, m_keyPosition[p1Index].time, time);
    
    const Vector3 finalPosition = Math::mix(m_keyPosition[p0Index].value, m_keyPosition[p1Index].value, scaleFactor);

    return finalPosition;
}

float Keyframe::GetScaleFactor(float last, float next, float time) {
    if (next == last)
        return 0.0f;
    return ((time - last) / (next - last));
}

Key<Vector3> Keyframe::getScaleFromTime(float t) const
{
    return t == 0.0f ? Key<Vector3>(0.0f, Vector3::zero) : m_keyScale[getIndexFromTime(t, m_keyScale)];
}

Key<Quaternion> Keyframe::getRotationFromTime(float t) const
{
    return t == 0.0f ? Key<Quaternion>(0.0f, Quaternion::identity) : m_keyRotation[getIndexFromTime(t, m_keyRotation)];
}

Key<Vector3> Keyframe::getPositionFromTime(float t) const
{
    return t == 0.0f ? Key<Vector3>(0.0f, Vector3::zero) : m_keyPosition[getIndexFromTime(t, m_keyPosition)];
}

bool Keyframe::serialize(Stream *stream) const
{
    // write name
    {
        auto nsize = m_name.size();
        if (!stream->write(&nsize, sizeof(std::size_t)))
        {
            return false;
        }

        if (!stream->write(m_name.c_str(), nsize * sizeof(char)))
        {
            return false;
        }
    }

    // write key scale
    {
        auto ksize = m_keyScale.size();
        if (!stream->write(&ksize, sizeof(std::size_t)))
        {
            return false;
        }

        if (!stream->write(m_keyScale.data(), ksize * sizeof(Key<Vector3>)))
        {
            return false;
        }
    }


    // write key rotation
    {
        auto ksize = m_keyRotation.size();
        if (!stream->write(&ksize, sizeof(std::size_t)))
        {
            return false;
        }

        if (!stream->write(m_keyRotation.data(), ksize * sizeof(Key<Quaternion>)))
        {
            return false;
        }
    }


    // write key position
    {
        auto ksize = m_keyPosition.size();
        if (!stream->write(&ksize, sizeof(std::size_t)))
        {
            return false;
        }

        if (!stream->write(m_keyPosition.data(), ksize * sizeof(Key<Vector3>)))
        {
            return false;
        }
    }
    return true;
}

bool Keyframe::deserialize(Stream *stream)
{
    // read name
    {
        std::size_t nsize = 0;
        if (!stream->read(reinterpret_cast<char *>(&nsize), sizeof(std::size_t)))
        {
            return false;
        }

        std::string name(nsize, '\0');
        if (!stream->read(name.data(), nsize * sizeof(char)))
        {
            return false;
        }

        m_name = name;
    }

    // write key scale
    {
        std::size_t ksize = 0;
        if (!stream->read(reinterpret_cast<char *>(&ksize), sizeof(std::size_t)))
        {
            return false;
        }

        m_keyScale.resize(ksize, Key<Vector3>(0.0f, Vector3::zero));
        if (!stream->read(reinterpret_cast<char *>(m_keyScale.data()), ksize * sizeof(Key<Vector3>)))
        {
            return false;
        }
    }


    // write key rotation
    {
        std::size_t ksize = 0;
        if (!stream->read(reinterpret_cast<char *>(&ksize), sizeof(std::size_t)))
        {
            return false;
        }

        m_keyRotation.resize(ksize, Key<Quaternion>(0.0f, Quaternion::identity));
        if (!stream->read(reinterpret_cast<char *>(m_keyRotation.data()), ksize * sizeof(Key<Quaternion>)))
        {
            return false;
        }
    }


    // write key position
    {
        std::size_t ksize = 0;
        if (!stream->read(reinterpret_cast<char *>(&ksize), sizeof(std::size_t)))
        {
            return false;
        }

        m_keyPosition.resize(ksize, Key<Vector3>(0.0f, Vector3::zero));
        if (!stream->read(reinterpret_cast<char *>(m_keyPosition.data()), ksize * sizeof(Key<Vector3>)))
        {
            return false;
        }
    }
    return true;
}


Vector3 Keyframe::GetInterpolateScaling(float time, Key<Vector3> p0, Key<Vector3> p1)
{
    float scaleFactor = GetScaleFactor(p0.time, p1.time, time);

    const Vector3 finalScale = Math::mix(p0.value, p1.value, scaleFactor);

    return finalScale;
}


Quaternion Keyframe::GetInterpolateRotation(float time, Key<Quaternion> p0, Key<Quaternion> p1) {
    float scaleFactor = GetScaleFactor(p0.time, p1.time, time);
    
    return Math::normalize(Math::slerp(p1.value, p0.value, scaleFactor));
}

Vector3 Keyframe::GetInterpolatePosition(float time, Key<Vector3> p0, Key<Vector3> p1) {
    float scaleFactor = GetScaleFactor(p0.time, p1.time, time);
    
    const Vector3 finalPosition = Math::mix(p0.value, p1.value, scaleFactor);

    return finalPosition;
}



