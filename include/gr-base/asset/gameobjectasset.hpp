#pragma once

#include "gr-base/core/file/default.hpp"

#include "gr-base/ecs/default.hpp"

class GameObjectAsset
{
public:
    GameObjectAsset(GameObjectHandle handle);

    bool Serialize(Stream* stream);

    bool Deserialize(Stream* stream);
};


