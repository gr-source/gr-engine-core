#include "gr-base/core/animation/keyframe.hpp"

#include "gr-base/file/stream.hpp"

Keyframe::Keyframe()
{}

Vector3 Keyframe::getInterpolatePosition(float time) const {
    if (1 == m_keyPosition.size())
        return m_keyPosition[0].value;

    auto current_frame = GetIndexFromTime<Vector3>(time, m_keyPosition);

    return GetInterpolatePosition(time, m_keyPosition[current_frame], m_keyPosition[current_frame + 1]);
}

Quaternion Keyframe::getInterpolateRotation(float time) const {
    if (1 == m_keyRotation.size()) {
        return m_keyRotation[0].value;
    }

    auto current_frame = GetIndexFromTime<Quaternion>(time, m_keyRotation);

    return GetInterpolateRotation(time, m_keyRotation[current_frame], m_keyRotation[current_frame + 1]);
}

Vector3 Keyframe::getInterpolateScaling(float time) const {
    if (1 == m_keyScale.size())
        return m_keyScale[0].value;

    auto current_frame = GetIndexFromTime<Vector3>(time, m_keyScale);

    return GetInterpolateScaling(time, m_keyScale[current_frame], m_keyScale[current_frame + 1]);
}

Matrix4x4 Keyframe::getLocalMatrix(float time) const {
    return Math::CreateTRS(getInterpolatePosition(time), getInterpolateRotation(time), getInterpolateScaling(time));
}

Key<Vector3> Keyframe::getPositionFromTime(float t) const
{
    return t == 0.0f ? Key<Vector3>(0.0f, type_traits<Vector3>::zero) : m_keyPosition[GetIndexFromTime(t, m_keyPosition)];
}

Key<Quaternion> Keyframe::getRotationFromTime(float t) const
{
    return t == 0.0f ? Key<Quaternion>(0.0f, type_traits<Quaternion>::identity) : m_keyRotation[GetIndexFromTime(t, m_keyRotation)];
}

Key<Vector3> Keyframe::getScaleFromTime(float t) const
{
    return t == 0.0f ? Key<Vector3>(0.0f, type_traits<Vector3>::zero) : m_keyScale[GetIndexFromTime(t, m_keyScale)];
}

bool Keyframe::serialize(Stream *stream) const
{
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
    return true;
}

bool Keyframe::deserialize(Stream *stream)
{
    // write key position
    {
        std::size_t ksize = 0;
        if (!stream->read(reinterpret_cast<char *>(&ksize), sizeof(std::size_t)))
        {
            return false;
        }

        m_keyPosition.resize(ksize, Key<Vector3>(0.0f, type_traits<Vector3>::zero));
        if (!stream->read(reinterpret_cast<char *>(m_keyPosition.data()), ksize * sizeof(Key<Vector3>)))
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

        m_keyRotation.resize(ksize, Key<Quaternion>(0.0f, type_traits<Quaternion>::identity));
        if (!stream->read(reinterpret_cast<char *>(m_keyRotation.data()), ksize * sizeof(Key<Quaternion>)))
        {
            return false;
        }
    }

    // write key scale
    {
        std::size_t ksize = 0;
        if (!stream->read(reinterpret_cast<char *>(&ksize), sizeof(std::size_t)))
        {
            return false;
        }

        m_keyScale.resize(ksize, Key<Vector3>(0.0f, type_traits<Vector3>::zero));
        if (!stream->read(reinterpret_cast<char *>(m_keyScale.data()), ksize * sizeof(Key<Vector3>)))
        {
            return false;
        }
    }
    return true;
}

Vector3 Keyframe::GetInterpolatePosition(float time, Key<Vector3> p0, Key<Vector3> p1) {
    float scaleFactor = GetScaleFactor(p0.time, p1.time, time);
    
    return Math::mix(p0.value, p1.value, scaleFactor);
}

Quaternion Keyframe::GetInterpolateRotation(float time, Key<Quaternion> p0, Key<Quaternion> p1) {
    float scaleFactor = GetScaleFactor(p0.time, p1.time, time);
    
    return Math::normalize(Math::slerp(p1.value, p0.value, scaleFactor));
}

Vector3 Keyframe::GetInterpolateScaling(float time, Key<Vector3> p0, Key<Vector3> p1)
{
    float scaleFactor = GetScaleFactor(p0.time, p1.time, time);

    return Math::mix(p0.value, p1.value, scaleFactor);
}

float Keyframe::GetScaleFactor(float last, float next, float time) {
    if (next == last)
        return 0.0f;
    return ((time - last) / (next - last));
}




