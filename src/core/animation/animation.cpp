#include "gr-base/core/animation/animation.hpp"

#include <algorithm>
#include <cstddef>
#include <iterator>

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

bool Animation::serialize(Stream *stream) const
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

    if (!stream->write(&m_duration, sizeof(float)))
    {
        return false;
    }

    if (!stream->write(&m_tick, sizeof(float)))
    {
        return false;
    }

    auto numkeyframe = m_keyframes.size();
    if (!stream->write(&numkeyframe, sizeof(std::size_t)))
    {
        return false;
    }

    for (auto &&[name, keyframe] : m_keyframes)
    {
        auto nsize = name.size();
        if (!stream->write(&nsize, sizeof(std::size_t)))
        {
            return false;
        }

        if (!stream->write(name.c_str(), nsize * sizeof(char)))
        {
            return false;
        }

        keyframe.serialize(stream);
    }

    return true;
}

bool Animation::deserialize(Stream *stream)
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

    if (!stream->read(reinterpret_cast<char *>(&m_duration), sizeof(float)))
    {
        return false;
    }

    if (!stream->read(reinterpret_cast<char *>(&m_tick), sizeof(float)))
    {
        return false;
    }

    std::size_t numkeyframe = 0;
    if (!stream->read(reinterpret_cast<char *>(&numkeyframe), sizeof(std::size_t)))
    {
        return false;
    }

    for (std::size_t i={};i<numkeyframe;i++)
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

        auto &keyframe = addkeyframe(name, name);

        keyframe.deserialize(stream);
    }

    return true;
}





