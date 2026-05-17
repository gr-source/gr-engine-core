#include "gr-base/asset/shaderasset.hpp"

#include "gr-base/core/file/stream.hpp"
#include <cstdint>

ShaderAsset::ShaderAsset() : m_flags(0)
{}

void ShaderAsset::AddUniform(const std::string& name, const std::string& typesname, int count)
{
    m_parameters.emplace(name, std::make_tuple(typesname, count));
}

bool ShaderAsset::serialize(Stream* stream) const
{
    if (!stream->write(m_name))
        return false;

    if (!stream->write(m_fragment))
        return false;

    if (!stream->write(m_vertex))
        return false;

    if (!stream->write(m_flags))
        return false;

    uint64_t length = static_cast<uint64_t>(m_parameters.size());
    if (!stream->write(length))
        return false;

    for (auto &&[name, info] : m_parameters)
    {
        auto&& [typesname, count] = info;

        if (!stream->write(name))
            return false;

        if (!stream->write(typesname))
            return false;

        if (!stream->write(count))
            return false;
    }
    return true;
}

bool ShaderAsset::deserialize(Stream* stream)
{
    if (!stream->read(m_name))
        return false;

    if (!stream->read(m_fragment))
        return false;

    if (!stream->read(m_vertex))
        return false;

    if (!stream->read(m_flags))
        return false;

    uint64_t length = 0;
    if (!stream->read(length))
        return false;

    for (uint64_t i=0;i<length;i++)
    {
        std::string name;
        if (!stream->read(name))
            return false;

        std::string typesname;
        if (!stream->read(typesname))
            return false;

        int count = 1;
        if (!stream->read(count))
            return false;

        AddUniform(name, typesname, count);
    }
    return true;
}

