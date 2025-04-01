#pragma once

#include <unordered_map>
#include <memory>
#include <string>

#include "gr-base/script/default.hpp"

#include "gr-base/core/geometry/default.hpp"

#include <gr-render/gVertexArray.h>
#include <unordered_set>
#include <vector>

typedef struct MeshData
{
    grr::gVertexArray *vertexArray;

    grm::u32 vbo;

    grm::u32 count;

    void draw();

} MeshData;

class Batching
{
public:
    static Batching *Create(const std::string &name);

    static Batching *Find(const std::string &name);

    static void Render();

    Batching();

    void insert(Transform *transform, const std::vector<Material *> *materialList);

    void erase(Transform *transform);

    void setMesh(std::vector<Mesh> *meshList);

private:
    std::unordered_set<Transform *> mInstances;

    std::vector<std::tuple<Transform *, const std::vector<Material *> *>> mInstanceRender;

    std::vector<MeshData> mMeshData;

    void render();

protected:
    static std::unordered_map<std::string, std::unique_ptr<Batching>> mBatcheInstances;

    static std::vector<Batching *> mBatcheList;
};












