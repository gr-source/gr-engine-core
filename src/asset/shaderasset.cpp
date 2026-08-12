#include "gr-base/asset/shaderasset.hpp"

#include "gr-base/file/stream.hpp"

#include <cstdint>

void ShaderAsset::AddUniform(const std::string& name, const std::string& typesname, int count)
{
    parameters.emplace(name, std::make_tuple(typesname, count));
}

bool ShaderAsset::serialize(Stream* stream) const
{
    if (!stream->write(name))
        return false;

    if (!stream->write(fragment))
        return false;

    if (!stream->write(vertex))
        return false;

    if (!stream->write(flags))
        return false;

    uint64_t length = static_cast<uint64_t>(parameters.size());
    if (!stream->write(length))
        return false;

    for (auto &&[name, info] : parameters)
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
    if (!stream->read(name))
        return false;

    if (!stream->read(fragment))
        return false;

    if (!stream->read(vertex))
        return false;

    if (!stream->read(flags))
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

