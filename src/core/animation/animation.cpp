#include "core/animation/animation.hpp"

#include <algorithm>

Animation::Animation(const std::string &name, float duration, float tick) : m_name(name), m_duration(duration), m_tick(tick) {}

void Animation::setName(const std::string &name) {
    m_name = name;
}

std::string Animation::getName() const {
    return m_name;
}

void Animation::setDuration(float duration) {
    m_duration = duration;
}

float Animation::getDuration() const {
    return m_duration;
}

void Animation::setTick(float tick) {
    m_tick = tick;
}

float Animation::getTick() const {
    return m_tick;
}

Keyframe *Animation::findKeyframe(const std::string &name) {
    auto it = m_keyframes.find(name);
    if (it != m_keyframes.end()) {
        return &(*it).second;
    }
    return nullptr;
}
