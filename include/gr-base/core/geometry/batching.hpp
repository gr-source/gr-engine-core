#pragma once

#include "gr-base/core/geometry/mesh.hpp"

#include "gr-base/core/geometry/default.hpp"

#include "gr-base/script/default.hpp"

#include <gr-render/gCommon.h>

struct MeshInfo
{
    grr::gVertexArray *vertexArray;

    grm::u32 vbo, pbo;

    grm::u32 abo;

    grm::u32 count;

    void push(const Matrix4x4 *data, const std::size_t &size);

    void draw(const std::size_t &instances);
};

struct GpuMesh
{
    std::vector<MeshInfo> meshinfolist;

    std::vector<grr::gShader *> shaderlist;

    void resize(const std::size_t &size);

    MeshInfo &get(const std::size_t &id);

    grr::gShader *getshader(const std::size_t &id)
    {
        return shaderlist[id];
    }

    void set(int id, grr::gShader *shader)
    {
        shaderlist[id] = shader;
    }

    void push(const Matrix4x4 *data, const std::size_t &size);

    void draw(int id, const std::size_t &instances = 0);
};

#define DYNAMIC_MESH 0
#define DYNAMIC_INSTANCED_MESH 1

class BatchingDynamic
{
public:
    BatchingDynamic(const std::vector<Material *> &materiallist, std::vector<Mesh> *meshlist);

    void insert(Transform *transform);

    void erase(Transform *transform);

    void render();

private:
    std::vector<Material *> m_materiallist;

    std::vector<Mesh> *m_meshlist;

    std::vector<Transform *> m_instances;

    std::unique_ptr<GpuMesh> m_gpuMesh[2];

    void build(GpuMesh *gpuMesh, std::vector<Mesh> *meshlist, int id);

    void buildDynamicMesh(MeshInfo &meshInfo, const Mesh &mesh);

    void buildDynamicSkinnedMesh();

    void buildInstancedDynamicMesh(MeshInfo &meshInfo, const Mesh &mesh);

    void buildInstancedDynamicSkinnedMesh();
};

class Batching
{
public:
    Batching(std::vector<Mesh> *meshlist);
    ~Batching();

    void insert(Transform *transform, const std::vector<Material *> &material);

    void erase(Transform *transform);

    void render();

private:
    std::unordered_map<std::string, std::unique_ptr<BatchingDynamic>> m_batchingdynamicmap;

    std::vector<BatchingDynamic *> m_batchingdynamiclist;

    std::vector<Mesh> *m_meshlist;

    std::size_t m_count;
};

/*
#include <cstddef>
#include <gr-math/matrix4x4.hpp>
#include <gr-render/gShader.h>
#include <unordered_map>
#include <memory>
#include <string>

#include "gr-base/script/default.hpp"

#include "gr-base/core/geometry/default.hpp"

#include <gr-render/gVertexArray.h>
#include <unordered_set>
#include <vector>

#include "gr-base/script/default.hpp"

struct MeshData
{
    grr::gVertexArray *vertexArray;

    grm::u32 vbo, pbo;

    grm::u32 abo;

    grm::u32 count;

    void draw(int instanced);

    void update(Matrix4x4 *data, int size);
};

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

    std::vector<Material *> materialList;

    int meshID;
};

struct BatchingRendererInstanced
{
    std::vector<Transform *> transformlist;

    std::vector<Material *> materialList;

    int meshID;
};

struct GpuMesh
{
    std::vector<MeshData> m_meshdata;
    std::vector<grr::gShader *> m_shaderlist;

    void draw_mesh(int id, int instanced)
    {
        return m_meshdata[id < m_meshdata.size() ? id : 0].draw(instanced);
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

    void set_shader(int id, grr::gShader *shader)
    {
        m_shaderlist[id] = shader;
    }
};

class Batching
{
public:
    static Batching *Create(const std::string &name, std::vector<Mesh> *meshList);

    static Batching *Find(const std::string &name);

    static void Render();

    Batching(std::vector<Mesh> *meshList);

    void insert(Transform *transform, const std::vector<Material *> &materialList);

    void erase(Transform *transform);

private:
    std::vector<Mesh> *m_meshlist;

    std::vector<RenderEntity> m_renderdata[3];

    std::unique_ptr<BatchingRenderer> m_firstrenderer;

    std::unique_ptr<BatchingRendererInstanced> m_dynamicinstancedrenderer;

    std::unique_ptr<GpuMesh> m_gpuMesh[3];

    std::size_t m_drawcount;

    void render();

    int create_gpu_mesh(int meshID);

    void create_shader(int meshID, const std::vector<Material *> &materialList);

    void build(GpuMesh *gpuMesh, int meshID);

protected:
    static std::unordered_map<std::string, std::unique_ptr<Batching>> mBatcheInstances;

    static std::vector<Batching *> mBatcheList;
};
*/










