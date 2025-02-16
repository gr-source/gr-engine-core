#include "gr-base/ecs/chunkManager.hpp"

BaseChunk::BaseChunk() : bCanEnableUpdate(false), init(nullptr), start(nullptr), update(nullptr), enable(nullptr), disable(nullptr), serialize(nullptr), deserialize(nullptr), destroy(nullptr), initall(nullptr), updateall(nullptr), allocatein(nullptr), deallocatein(nullptr)
{
}

std::vector<Field> BaseChunk::getFields() const 
{
    return m_fieldMap;
}




