#include "gr-base/ecs/chunkManager.hpp"

std::vector<Field> BaseChunk::getFields() const 
{
    return m_fieldMap;
}