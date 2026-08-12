#pragma once

#include "gr-base/file/default.hpp"

#include <gr-math/gmath.hpp>

#include <vector>

template <typename T>
struct Key {
    float time;
    T value;

    Key(float time, T value);
};

template <typename T>
inline Key<T>::Key(float time, T value) : time(time), value(value) {}

class Keyframe {
public:
    Keyframe();

    Vector3 getInterpolatePosition(float time) const;

    Quaternion getInterpolateRotation(float time) const;

    Vector3 getInterpolateScaling(float time) const;

    Matrix4x4 getLocalMatrix(float time) const;

    Key<Vector3> getPositionFromTime(float t) const;

    Key<Quaternion> getRotationFromTime(float t) const;

    Key<Vector3> getScaleFromTime(float t) const;

    template <typename ...Args>
    Key<Vector3> &addKeyPosition(Args &&...args);

    template <typename ...Args>
    Key<Vector3> &addKeyScale(Args &&...args);

    template <typename ...Args>
    Key<Quaternion> &addKeyRotation(Args &&...args);

    bool serialize(Stream *stream) const;
    
    bool deserialize(Stream *stream);

    static Vector3 GetInterpolateScaling(float time, Key<Vector3> p0, Key<Vector3> p1);

    static Quaternion GetInterpolateRotation(float time, Key<Quaternion> p0, Key<Quaternion> p1);

    static Vector3 GetInterpolatePosition(float time, Key<Vector3> p0, Key<Vector3> p1);
    
    static float GetScaleFactor(float last, float next, float time);

private:
    std::vector<Key<Vector3>> m_keyPosition;

    std::vector<Key<Vector3>> m_keyScale;

    std::vector<Key<Quaternion>> m_keyRotation;

    template <typename T>
    static std::size_t GetIndexFromTime(float time, const std::vector<Key<T>>& keys);
};

template <typename T>
inline std::size_t Keyframe::GetIndexFromTime(float time, const std::vector<Key<T>>& keys)
{
    for (std::size_t i=0;i<keys.size() - 1;i++)
    {
        if (time <= keys[i + 1].time)
            return i;
    }
    return 0;
}

template <typename... Args>
inline Key<Vector3> &Keyframe::addKeyPosition(Args &&...args) {
    return m_keyPosition.emplace_back(std::forward<Args>(args)...);
}

template <typename... Args>
inline Key<Vector3> &Keyframe::addKeyScale(Args &&...args) {
    return m_keyScale.emplace_back(std::forward<Args>(args)...);
}

template <typename... Args>
inline Key<Quaternion> &Keyframe::addKeyRotation(Args &&...args) {
    return m_keyRotation.emplace_back(std::forward<Args>(args)...);
}






