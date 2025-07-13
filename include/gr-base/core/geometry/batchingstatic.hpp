#pragma once

// core/geometry
#include "gr-base/core/geometry/gpumesh.hpp"

// script
#include "gr-base/script/default.hpp"

class BatchingStatic
{
public:
    BatchingStatic(const std::vector<Material *> &materiallist, std::vector<Mesh> *meshlist);

    void insert(Transform *transform);

    void erase(Transform *transform);

    void render();
private:
};


