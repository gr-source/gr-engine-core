#pragma once

#include "keyframe.hpp"

#include <string>

class Animation {
public:
    Animation(const std::string &name = "default", float duration = 0.0f, float tick = 0.0f);

    void setName(const std::string &name);

    std::string getName() const;

    void setDuration(float duration);

    float getDuration() const;

    void setTick(float tick);

    float getTick() const;

    /*Keyframe *findKeyframe(const std::string &name);*/

    template <typename ...Args>
    Keyframe *addkeyframe(Args &&... args);

    void setKeyFrame(const std::vector<Keyframe> &keyframes);

    Keyframe *getKeyFrame(int index);

    bool serialize(Stream *stream) const;

    bool deserialize(Stream *stream);
    
private:
    std::string m_name;

    float m_duration;

    float m_tick;

    std::vector<Keyframe> m_keyframes;
};

template <typename ...Args>
inline Keyframe *Animation::addkeyframe(Args &&...args)
{
    return &m_keyframes.emplace_back(std::forward<Args>(args)...);
}


