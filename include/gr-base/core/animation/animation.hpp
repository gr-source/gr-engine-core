#pragma once

#include "keyframe.hpp"

#include <unordered_map>

class Animation {
public:
    Animation(const std::string &name = "default", float duration = 0.0f, float tick = 0.0f);

    void setName(const std::string &name);

    std::string getName() const;

    void setDuration(float duration);

    float getDuration() const;

    void setTick(float tick);

    float getTick() const;

    Keyframe *findKeyframe(const std::string &name);

    template <typename ...Args>
    Keyframe &addkeyframe(Args &&... args);
    
private:
    std::string m_name;

    float m_duration;

    float m_tick;

    std::unordered_map<std::string, Keyframe> m_keyframes;
};

template <typename ...Args>
inline Keyframe &Animation::addkeyframe(Args &&...args) {
    auto &&[it, added] = m_keyframes.emplace(std::forward<Args>(args)...);

    return (*it).second;
}


