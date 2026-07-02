#include "gr-base/ecs/detail.hpp"

#include <cstdlib>

namespace engine::ecs::detail
{

void layout::Graphy::grow(size_t new_capacity)
{
    if (capacity >= new_capacity)
        return;

    size_t oldCapacity = capacity;

    capacity = new_capacity;

    removeEdges = (ArchetypeID*)realloc(removeEdges, capacity * sizeof(ArchetypeID));
    addEdges = (ArchetypeID*)realloc(addEdges, capacity * sizeof(ArchetypeID));

    for (size_t i=oldCapacity;i<capacity;i++)
    {
        removeEdges[i] = INVALID_ARCHETYPE;
        addEdges[i] = INVALID_ARCHETYPE;
    }
}


}

