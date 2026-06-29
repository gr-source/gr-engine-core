#include "gr-base/managers/assetsManager.hpp"

#include "gr-base/asset/materialasset.hpp"
#include "gr-base/asset/shaderasset.hpp"

#include "gr-base/core/binary/gHeader.hpp"
#include "gr-base/core/file/file.hpp"

#include "gr-base/core/animation/animation.hpp"

#include "gr-base/core/console.hpp"
#include "gr-base/core/assetDB.hpp"

#include "gr-base/core/file/fileStream.hpp"
#include "gr-base/core/geometry/default.hpp"

#include "gr-base/core/logger.hpp"
#include "gr-base/core/scene.hpp"
#include "gr-base/core/thread/taskmanager.hpp"

#include "gr-base/externals/hash.hpp"
#include "gr-base/managers/meshmanager.hpp"
#include "gr-base/managers/shaderprefabmanager.hpp"
#include "gr-base/task/modelloadtask.hpp"
#include <exception>
#include <stdexcept>

#include "gr-base/managers/materialmanager.hpp"

AssetsManager* AssetsManager::s_AssetsManager = nullptr;

void AssetsManager::Initialize()
{
    if (s_AssetsManager == nullptr)
        s_AssetsManager = new AssetsManager();

    logger::info("[AssetsManager] Initialize.");
}

void AssetsManager::Shutdown()
{
    if (s_AssetsManager != nullptr)
        delete s_AssetsManager;

    s_AssetsManager = nullptr;

    logger::info("[AssetsManager] Shutdown.");
}

std::string AssetsManager::GetRefTextureName(gr::gTexture *texture)
{
    auto name = s_AssetsManager->mTextureManager.getname(texture);
    if (name == nullptr)
        throw std::runtime_error("texture has not reference.");
    return name;
}

/*
gr::Shader *AssetsManager::CreateShader(const std::string& name)
{
    return s_AssetsManager->mShaderManager.create(name);
}

gr::Shader *AssetsManager::FindShader(const std::string &name)
{
    return s_AssetsManager->mShaderManager.find(name);
}

std::string AssetsManager::GetShaderName(gr::Shader *shader)
{
    auto name = s_AssetsManager->mShaderManager.get_shader_name(shader);
    if (name == nullptr)
        throw std::runtime_error("[ AssetsManager.Shader ] Fail to get shader name, pointer is not valid.");
    return name;
}

void AssetsManager::RemoveShader(gr::Shader *shader)
{
    return s_AssetsManager->mShaderManager.destroy(shader);
}
*/

gr::gTexture *AssetsManager::CreateTexture(const std::string &name)
{
    return s_AssetsManager->mTextureManager.create(name);
}

// ********** Mesh ********** //
MeshHandle AssetsManager::LoadMesh(const std::string& name)
{
    return LoadMesh(engine::hash::fnv1_str(name.c_str()));
}

MeshHandle AssetsManager::LoadMesh(AssetHash hash)
{
    MeshHandle h = FindMesh(hash);
    if (h != HANDLE_INVALID)
        return h;

    Mesh mesh = LoadMeshFromFile(AssetDB::Resolve(hash));

    h = CreateMesh(hash);
    *GetMesh(h) = std::move(mesh);

    return h;
}

size_t AssetsManager::GetMeshCount()
{
    return s_AssetsManager->mMeshManager->GetCount();
}

#include "gr-base/asset/meshasset.hpp"

static AABB CalculateBounds(Mesh* mesh)
{
    if (!mesh->numVertices)
        return { Vector3::zero, Vector3::zero };

    AABB bounds;

    bounds.min.x = mesh->vertices[0].position.x;
    bounds.min.y = mesh->vertices[0].position.y;
    bounds.min.z = mesh->vertices[0].position.z;

    bounds.max.x = mesh->vertices[0].position.x;
    bounds.max.y = mesh->vertices[0].position.y;
    bounds.max.z = mesh->vertices[0].position.z;

    for (size_t i=0;i<mesh->numVertices;i++)
    {
        const auto& vertex = mesh->vertices[i];
        const auto& p = vertex.position;

        bounds.min.x = std::min(bounds.min.x, p.x);
        bounds.min.y = std::min(bounds.min.y, p.y);
        bounds.min.z = std::min(bounds.min.z, p.z);

        bounds.max.x = std::max(bounds.max.x, p.x);
        bounds.max.y = std::max(bounds.max.y, p.y);

        bounds.max.z = std::max(bounds.max.z, p.z);
    }

    return bounds;
}

Mesh AssetsManager::LoadMeshFromFile(const std::string& file)
{
    MeshAsset asset;
    if (!LoadFromFile(file + HEADER_MESH_EXTENSION, HEADER_MESH_SIGN, [&](Stream* stream)
            {
                return asset.deserialize(stream);
            }))
        throw std::runtime_error("[ AssetsManager ] Mesh load failed: " + file + HEADER_MESH_EXTENSION);

    auto& submeshes = asset.submeshes;
    auto& vertices = asset.vertices;
    auto& indices = asset.indices;

    Mesh mesh;
    {
        mesh.vertices = (Vertex3D*)malloc(vertices.size() * sizeof(Vertex3D));
        mesh.numVertices = static_cast<uint32_t>(vertices.size());

        memcpy(mesh.vertices, vertices.data(), vertices.size() * sizeof(Vertex3D));
    }

    {
        mesh.indices = (uint32_t*)malloc(indices.size() * sizeof(uint32_t));
        mesh.numIndices = static_cast<uint32_t>(indices.size());

        memcpy(mesh.indices, indices.data(), indices.size() * sizeof(uint32_t));
    }

    mesh.submeshes = (Submesh*)malloc(submeshes.size() * sizeof(Submesh));
    mesh.numSubmeshes = static_cast<uint32_t>(submeshes.size());

    for (size_t i=0;i<mesh.numSubmeshes;i++)
    {
        auto& assetSubmesh = submeshes[i];

        auto& submesh = mesh.submeshes[i];
        submesh.indexOffset = assetSubmesh.indexOffset;
        submesh.indexCount = assetSubmesh.indexCount;
    }

    mesh.localBounds = CalculateBounds(&mesh);

    return mesh;
}

MeshHandle AssetsManager::CreateMesh(AssetHash hash)
{
    return s_AssetsManager->mMeshManager->create(hash);
}

MeshHandle AssetsManager::FindMesh(AssetHash hash)
{
    return s_AssetsManager->mMeshManager->find(hash);
}

void AssetsManager::RemoveMesh(MeshHandle handle)
{
    s_AssetsManager->mMeshManager->remove(handle);
}

Mesh *AssetsManager::GetMesh(MeshHandle h)
{
    return s_AssetsManager->mMeshManager->get(h);
}

/*
void AssetsManager::SetAnimationList(const std::string &path, const std::function<void (Animation *)> func)
{
    std::lock_guard<std::mutex> lock(mAnimationMutex);
    
    // check task is processing
    {
        auto it = mAnimationLoadTask.find(path);
        if (it != mAnimationLoadTask.end())
        {
            if (func == nullptr)
                return;

            return it->second->addFunction(func);
        }
    }

    // check animation has be loaded
    {
        auto it = mAnimationList.find(path);
        if (it != mAnimationList.end())
        {
            if (func == nullptr)
                return;
            
            return func(&(*it->second));
        }
    }

    if (!File::Exists(path))
    {
        return Console::LogError("[ AssetsManager::Animation ] Fail to load, file do not found: %s", path.c_str());
    }

    mAnimationLoadTask.emplace(path, std::make_unique<AnimationLoadTask>(path, func));
}

Animation *AssetsManager::CreateAnimation(const std::string &name)
{
    return mAnimationManager.create(name);
}

Animation *AssetsManager::FindAnimation(const std::string &name)
{
    return mAnimationManager.find(name);
}

std::string AssetsManager::GetAnimationName(const Animation *animation)
{
    auto name = mAnimationManager.get_name(animation);
    return name == nullptr ? "" : name;
}
*/
/*
std::vector<gr::gShader *> AssetsManager::GetShaderList(AssetShader_ type, ShaderOptimize_ optimize)
{
    return s_AssetsManager->mShaderManager.get_shader(type, optimize);
}
*/

/*
std::vector<gr::Shader *> AssetsManager::GetShaderList(ShaderType type)
{
    return s_AssetsManager->mShaderManager.get_shader(type);
}
*/

MaterialHandle AssetsManager::CreateMaterial(AssetHash hash)
{
    return s_AssetsManager->mMaterialManager->create(hash);
}

Material *AssetsManager::GetMaterial(MaterialHandle h)
{
    return s_AssetsManager->mMaterialManager->get(h);
}

MaterialHandle AssetsManager::FindMaterial(AssetHash hash)
{
    return s_AssetsManager->mMaterialManager->find(hash);
}

size_t AssetsManager::GetMaterialCount()
{
    return s_AssetsManager->mMaterialManager->GetCount();
}

usize AssetsManager::GetTextureCount()
{
    return s_AssetsManager->mTextureManager.getcount();
}

const std::vector<ShaderPrefab*>& AssetsManager::GetAllShader3D()
{
    return s_AssetsManager->mShaderPrefabManager->GetAllShader3d();
}

void AssetsManager::RemoveTask(const std::string &name)
{
    // Check if task on the texture
    if (s_AssetsManager->mTextureManager.remove_task(name))
        return;

    /*
    if (s_AssetsManager->mShaderManager.remove_task(name))
        return;
    */
}

MaterialHandle AssetsManager::LoadMaterial(const std::string& name)
{
    try
    {
        return LoadMaterialFromFile(AssetDB::Resolve(engine::hash::fnv1_str(name.c_str())));
    } catch (const std::exception& e)
    {
        throw e;
    }

    return ID_INVALID;
}

MaterialHandle AssetsManager::LoadMaterial(Hash64 hash)
{
    try
    {
        return LoadMaterialFromFile(AssetDB::Resolve(hash));
    } catch (const std::exception& e)
    {
        throw std::runtime_error(e.what());
    }

    return ID_INVALID;
}

MaterialHandle AssetsManager::LoadMaterialFromFile(const std::string& file)
{
    MaterialAsset asset;
    if (!LoadFromFile(file + HEADER_MATERIAL_EXTENSION, HEADER_MATERIAL_SIGN, [&](Stream* stream)
                {
                    return asset.deserialize(stream);
                }))
        return HANDLE_INVALID;

    ShaderPrefab* prefab = s_AssetsManager->mShaderPrefabManager->find(engine::hash::fnv1_str(asset.GetShader()));
    if (prefab == nullptr)
        prefab = LoadShader(asset.GetShader());

    MaterialHandle h = CreateMaterial(asset.hash);

    auto material = GetMaterial(h);
    material->prefab = prefab;

    for (auto&& [name, buffer] : asset.GetOverrides())
        material->Set(name, buffer.data(), buffer.size());

    return h;
}

ShaderPrefab* AssetsManager::FindShader(const std::string &name)
{
    return FindShader(engine::hash::fnv1_str(name.c_str()));
}

ShaderPrefab* AssetsManager::FindShader(Hash64 hash)
{
    return s_AssetsManager->mShaderPrefabManager->find(hash);
}

ShaderPrefab* AssetsManager::LoadShader(const std::string& name)
{
    return LoadShaderFromFile(AssetDB::Resolve(engine::hash::fnv1_str(name.c_str())));
}

ShaderPrefab* AssetsManager::LoadShader(Hash64 hash)
{
    return LoadShaderFromFile(AssetDB::Resolve(hash));
}

ShaderPrefab* AssetsManager::LoadShaderFromFile(const std::string& file)
{
    ShaderAsset asset;
    if (!LoadFromFile(file + HEADER_SHADER_EXTENSION, HEADER_SHADER_SIGN, [&](Stream* stream)
                {
                    return asset.deserialize(stream);
                }))
        return nullptr;

    return s_AssetsManager->mShaderPrefabManager->create(&asset);
}

// private
AssetsManager::AssetsManager() : mMaterialManager(new MaterialManager()), mMeshManager(new MeshManager()), mShaderPrefabManager(new ShaderPrefabManager())
{}

AssetsManager::~AssetsManager()
{
    delete mMaterialManager;

    delete mMeshManager;

    delete mShaderPrefabManager;
}

bool AssetsManager::LoadFromFile(const std::string& file, Hash64 sign, std::function<bool (Stream*)> func)
{
    FileStream stream(file, StreamType::INPUT);

    FileHeader header{};
    if (!stream.read(&header, sizeof(FileHeader)))
        return false;

    if (header.sign != sign)
        return false;

    return func(&stream);
}

void AssetsManager::loadGameObject(const std::string &name, std::function<void(Entity)> func)
{
    loadGameObjectFromFile(AssetDB::Resolve(engine::hash::fnv1_str(name.c_str())) + HEADER_MODEL_EXTENSION, func);
}

void AssetsManager::loadGameObjectFromFile(const std::string &file, std::function<void(Entity)> func)
{
    auto scene = Scene::GetCurrent();
    if (scene != nullptr)
        TaskManager::Add<ModelLoadTask>(file, scene, func);
}





