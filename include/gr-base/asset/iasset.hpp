#pragma once

#include "gr-base/file/default.hpp"

struct IAsset
{
    virtual bool serialize(Stream* stream) const = 0;

    virtual bool deserialize(Stream* stream) = 0;
};


