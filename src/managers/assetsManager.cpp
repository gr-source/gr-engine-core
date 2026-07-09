#include "gr-base/managers/assetsManager.hpp"

#include "gr-base/asset/materialasset.hpp"
#include "gr-base/asset/shaderasset.hpp"
#include "gr-base/asset/assetDB.hpp"

#include "gr-base/core/binary/gHeader.hpp"
#include "gr-base/core/file/file.hpp"

#include "gr-base/core/animation/animation.hpp"

#include "gr-base/core/console.hpp"

#include "gr-base/core/file/fileStream.hpp"
#include "gr-base/core/geometry/default.hpp"

#include "gr-base/core/logger.hpp"
#include "gr-base/core/scene.hpp"
#include "gr-base/core/thread/taskmanager.hpp"

#include "gr-base/externals/hash.hpp"

#include "gr-base/managers/shaderprefabmanager.hpp"
#include "gr-base/managers/materialmanager.hpp"
#include "gr-base/managers/textureManager.hpp"
#include "gr-base/managers/meshmanager.hpp"

#include "gr-base/task/modelloadtask.hpp"

#include <exception>
#include <stdexcept>

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

TextureHandle AssetsManager::LoadTexture(const std::string& name)
{
    return LoadTexture(engine::hash::fnv1_str(name));
}

TextureHandle AssetsManager::LoadTexture(AssetHash hash)
{
    TextureHandle h = s_AssetsManager->mTextureManager->find(hash);
    if (h != HANDLE_INVALID)
        return h;

    TextureAsset asset = ImportTextureAssetFromFile(AssetDB::Resolve(hash));

    h = CreateTexture(hash);

    auto texture = GetTexture(h);
    texture->width = asset.width;
    texture->height = asset.height;

    texture->filter = TEXTURE_FILTER_LINEAR;
    texture->wrap = TEXTURE_WRAP_REPEAT;
    texture->type = TEXTURE_TYPE_2D;

    switch (asset.channels)
    {
        case 1:
            texture->format = TEXTURE_FORMAT_RED;
            break;

        case 2:
            texture->format = asset.hdr ? TEXTURE_FORMAT_RG16F : TEXTURE_FORMAT_RG;
            break;

        case 3:
            texture->format = asset.hdr ? TEXTURE_FORMAT_RGB16F : TEXTURE_FORMAT_RGB;
            break;

        case 4:
            texture->format = asset.hdr ? TEXTURE_FORMAT_RGBA16F : TEXTURE_FORMAT_RGBA;
            break;
        default:
            throw std::runtime_error("Unsupported channel count: " + std::to_string(asset.channels));
    }

    gr::BindTexture(texture);
    gr::ApplyTextureSettings(texture);
    gr::ApplyTextureBuffer(texture, asset.raw.data());
    gr::UnbindTexture(texture);

    return h;
}

TextureHandle AssetsManager::CreateTexture(AssetHash hash)
{
    return s_AssetsManager->mTextureManager->create(hash);
}

gr::Texture* AssetsManager::GetTexture(TextureHandle h)
{
    return s_AssetsManager->mTextureManager->get(h);
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
        return { type_traits<Vector3>::zero, type_traits<Vector3>::zero };

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

MaterialHandle AssetsManager::FindMaterial(AssetHash hash)
{
    return s_AssetsManager->mMaterialManager->find(hash);
}

Material *AssetsManager::GetMaterial(MaterialHandle h)
{
    return s_AssetsManager->mMaterialManager->get(h);
}

size_t AssetsManager::GetMaterialCount()
{
    return s_AssetsManager->mMaterialManager->GetCount();
}

usize AssetsManager::GetTextureCount()
{
    return s_AssetsManager->mTextureManager->GetCount();
}

const std::vector<ShaderPrefab*>& AssetsManager::GetAllShader3D()
{
    return s_AssetsManager->mShaderPrefabManager->GetAllShader3d();
}

MaterialHandle AssetsManager::LoadMaterial(const std::string& name)
{
    return LoadMaterial(engine::hash::fnv1_str(name.c_str()));
}

MaterialHandle AssetsManager::LoadMaterial(AssetHash hash)
{
    MaterialHandle h = FindMaterial(hash);
    if (h != HANDLE_INVALID)
        return h;

    Material material = LoadMaterialFromFile(AssetDB::Resolve(hash));

    h = CreateMaterial(hash);
    *GetMaterial(h) = std::move(material);

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

TextureAsset AssetsManager::ImportTextureAssetFromFile(const std::string& file)
{
    TextureAsset asset;
    if (!LoadFromFile(file + HEADER_TEXTURE_EXTENSION, HEADER_TEXTURE_SIGN, [&](Stream* stream)
            {
                return asset.deserialize(stream);
            }))
        throw std::runtime_error("[ AssetsManager ] TextureAsset load failed: " + file + HEADER_TEXTURE_EXTENSION);

    return asset;
}

Material AssetsManager::LoadMaterialFromFile(const std::string& file)
{
    MaterialAsset asset;
    if (!LoadFromFile(file + HEADER_MATERIAL_EXTENSION, HEADER_MATERIAL_SIGN, [&](Stream* stream)
            {
                return asset.deserialize(stream);
            }))
        throw std::runtime_error("[ AssetsManager ] Material load failed: " + file + HEADER_MATERIAL_EXTENSION);

    ShaderPrefab* prefab = s_AssetsManager->mShaderPrefabManager->find(engine::hash::fnv1_str(asset.GetShader()));
    if (prefab == nullptr)
        prefab = LoadShader(asset.GetShader());

    Material material;
    material.prefab = prefab;

    for (auto&& [name, buffer] : asset.GetOverrides())
        material.Set(name, buffer.data(), buffer.size());

    return material;
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

// private
AssetsManager::AssetsManager() : mMaterialManager(new MaterialManager()), mMeshManager(new MeshManager()), mShaderPrefabManager(new ShaderPrefabManager()), mTextureManager(new TextureManager())
{}

AssetsManager::~AssetsManager()
{
    delete mMaterialManager;

    delete mMeshManager;

    delete mShaderPrefabManager;

    delete mTextureManager;
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





