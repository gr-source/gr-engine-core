#pragma once

#include "gr-base/core/file/default.hpp"

#include <string>

class Asset
{
public:
    virtual ~Asset() {}

    virtual bool serialize(Stream* stream) const = 0;

    virtual bool deserialize(Stream* stream) = 0;
};


