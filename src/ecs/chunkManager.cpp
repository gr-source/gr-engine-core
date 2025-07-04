#include "gr-base/ecs/chunkManager.hpp"

IComponentStorage::IComponentStorage() : bCanEnableUpdate(false), init(nullptr), start(nullptr), update(nullptr), enable(nullptr), disable(nullptr), serialize(nullptr), deserialize(nullptr), destroy(nullptr), initall(nullptr), updateall(nullptr), allocatein(nullptr), deallocatein(nullptr)
{
}

std::vector<Field> IComponentStorage::getFields() const 
{
    return m_fieldMap;
}




