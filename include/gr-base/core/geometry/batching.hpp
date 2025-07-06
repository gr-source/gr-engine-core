#pragma once

#include <cstddef>
#include <gr-math/matrix4x4.hpp>
#include <unordered_map>
#include <memory>
#include <string>

#include "gr-base/script/default.hpp"

#include "gr-base/core/geometry/default.hpp"

#include <gr-render/gVertexArray.h>
#include <unordered_set>
#include <vector>

#include "gr-base/script/default.hpp"

typedef struct MeshData
{
    grr::gVertexArray *vertexArray;

    grm::u32 vbo, pbo;

    grm::u32 abo;

    grm::u32 count;

    int instanceCount;

    void draw();

    void update(Matrix4x4 *data, std::size_t size);
} MeshData;

struct RenderEntity
{
    Transform *transform;

    Component *component;

    const std::vector<Material *> *materiallist;

    bool operator ==(const Transform *other) const
    {
        return transform == other;
    }
};

#define MESHID_STATIC    (1 << 0)  // 0b0001 → 1
#define MESHID_DYNAMIC   (1 << 1)  // 0b0010 → 2
#define MESHID_ANIMATED  (1 << 2)  // 0b0100 → 4

struct BatchingRenderer
{
    Transform *transform;

    const std::vector<Material *> *materialList;

    int meshID;
};

struct BatchingRendererInstanced
{
    std::vector<Transform *> transformlist;

    const std::vector<Material *> *materialList;

    int meshID;
};

struct GpuMesh
{
    std::vector<MeshData> m_meshdata;
    std::vector<grr::gShader *> m_shaderlist;

    void draw_mesh(int id)
    {
        if (id <= m_meshdata.size())
            m_meshdata[id].draw();
    }

    void resize(std::size_t size)
    {
        m_meshdata.resize(size);
    }

    MeshData *get(std::size_t id)
    {
        return &m_meshdata[id];
    }

    void update(Matrix4x4 *data, std::size_t size)
    {
        for (auto &mesh : m_meshdata)
            mesh.update(data, size);
    }
};

class Batching
{
public:
    static Batching *Create(const std::string &name, std::vector<Mesh> *meshList);

    static Batching *Find(const std::string &name);

    static void Render();

    Batching(std::vector<Mesh> *meshList);

    void insert(Transform *transform, const std::vector<Material *> *materialList);

    void erase(Transform *transform);

    void setMesh();

private:
    std::vector<Mesh> *m_meshlist;

    std::vector<RenderEntity> m_renderdata[3];

    std::unique_ptr<BatchingRenderer> m_firstrenderer;

    std::unique_ptr<BatchingRendererInstanced> m_dynamicinstancedrenderer;

    std::unique_ptr<GpuMesh> m_gpuMesh[3];

    std::size_t m_drawcount;

    void render();

    int create_gpu_mesh(int meshID);

    void create_shader(int meshID, const std::vector<Material *> *materialList);

    void build(GpuMesh *gpuMesh, int meshID);

protected:
    static std::unordered_map<std::string, std::unique_ptr<Batching>> mBatcheInstances;

    static std::vector<Batching *> mBatcheList;
};











