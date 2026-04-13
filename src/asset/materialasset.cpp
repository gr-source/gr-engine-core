#include "gr-base/asset/materialasset.hpp"

#include "gr-base/core/file/stream.hpp"

bool MaterialAsset::serialize(Stream* stream) const
{
    uint64_t count = static_cast<uint64_t>(m_overrides.size());
    if (!stream->write(count))
        return false;

    for (auto&& [name, buffer] : m_overrides)
    {
        if (!stream->write(name))
            return false;

        uint64_t length = static_cast<uint64_t>(buffer.size());
        if (!stream->write(length))
            return false;

        if (!stream->write(buffer.data(), length))
            return false;
    }

    return true;
}

bool MaterialAsset::deserialize(Stream* stream)
{
    uint64_t count = 0;
    if (!stream->read(count))
        return false;

    for (uint64_t i=0;i<count;i++)
    {
        std::string name;
        if (!stream->read(name))
            return false;

        uint64_t length = 0;
        if (!stream->read(length))
            return false;

        std::vector<uint8_t> buffer(length, 0);
        if (!stream->read(buffer.data(), length))
            return false;

        m_overrides.emplace(name, buffer);
    }

    return true;
}


