#include "gr-base/ecs/chunkManager.hpp"

BaseChunk::BaseChunk() : init(nullptr), start(nullptr), update(nullptr), enable(nullptr), disable(nullptr), serialize(nullptr), deserialize(nullptr), destroy(nullptr), updateall(nullptr)
{
}

std::vector<Field> BaseChunk::getFields() const 
{
    return m_fieldMap;
}




