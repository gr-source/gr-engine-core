#pragma once

#include "gr-base/core/geometry/mesh.hpp"

#include "gr-base/ecs/entity.hpp"

#include "gr-base/script/default.hpp"

typedef struct LODLevel
{
    float distance;

    GameObject *level;
} LODLevel;

class LODGroup : public Component
{
CLASS_FUNCTIONS
public:
    LODGroup();

    void onInit() override;

    void onStart() override;

    int GetLODCount() const;

    int AddLOD(float distance, GameObject *level);

    LODLevel &GetLOD(int i);

    void Remove(int i);

private:
    std::array<LODLevel, 5> m_LODlist;

    Transform *m_transform;

    int m_LODcount;
};

REGISTER_COMPONENT(LODGroup)


