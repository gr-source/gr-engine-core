#pragma once

#include <gr-math/quaternion.hpp>
#include <gr-math/vector3.hpp>
#include <string>
#include <vector>

#include "gr-base/core/file/default.hpp"

#include <gr-math/gmath.hpp>

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
    Keyframe(const std::string &name);

    const std::string &getName() const;

    void setName(const std::string &name);

    const Matrix4x4 getLocalMatrix(float time) const;

    const Vector3 getInterpolateScaling(float time) const;

    const Quaternion getInterpolateRotation(float time) const;

    const Vector3 getInterpolatePosition(float time) const;

    Key<Vector3> getScaleFromTime(float t) const;

    Key<Quaternion> getRotationFromTime(float t) const;

    Key<Vector3> getPositionFromTime(float t) const;

    static float GetScaleFactor(float last, float next, float time);

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

private:
    std::string m_name;

    std::vector<Key<Vector3>> m_keyPosition;

    std::vector<Key<Vector3>> m_keyScale;

    std::vector<Key<Quaternion>> m_keyRotation;

    template <typename T>
    const grm::u32 getIndexFromTime(float time, const std::vector<Key<T>>& keys) const;
};

template <typename T>
inline const grm::u32 Keyframe::getIndexFromTime(float time, const std::vector<Key<T>>& keys) const {
    for (auto i=0;i<keys.size() - 1;i++) {
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






