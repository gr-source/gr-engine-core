#include "gr-base/asset/materialasset.hpp"

#include "gr-base/core/file/stream.hpp"

#include <cstring>

bool MaterialAsset::serialize(Stream* stream) const
{
    if (!stream->write(m_name))
        return false;

    if (!stream->write(m_shader))
        return false;

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
    if (!stream->read(m_name))
        return false;

    if (!stream->read(m_shader))
        return false;

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

// private
void MaterialAsset::Set(const std::string& name, const void* data, size_t size)
{
    auto& cache = m_overrides[name];
    cache.resize(size, 0);

    memcpy(cache.data(), data, size);
}

