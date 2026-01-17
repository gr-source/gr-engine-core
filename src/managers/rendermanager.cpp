#include "gr-base/managers/rendermanager.hpp"

#include "gr-base/core/console.hpp"
#include "gr-base/core/culling/frustum.hpp"

#include "gr-base/core/culling/octreemanager.hpp"
#include "gr-base/core/gameObject.hpp"
#include "gr-base/managers/batchingmanager.hpp"
#include "gr-base/managers/assetsManager.hpp"

#include "gr-base/script/meshRender.hpp"
#include "gr-base/script/meshFilter.hpp"
#include "gr-base/script/transform.hpp"
#include "gr-base/script/camera.hpp"
#include "gr-base/script/light.hpp"

RenderManager* RenderManager::s_RenderManager = nullptr;

void RenderManager::Initialize()
{
    if (s_RenderManager == nullptr)
        s_RenderManager = new RenderManager();

    BatchingManager::Initialize();
}

void RenderManager::Shutdown()
{
    if (s_RenderManager != nullptr)
        delete s_RenderManager;
    s_RenderManager = nullptr;

    BatchingManager::Shutdown();
}

void RenderManager::Insert(GameObjectID id)
{
    s_RenderManager->insert(id);
}

void RenderManager::Remove(GameObjectID id)
{
    s_RenderManager->remove(id);
}

void RenderManager::PreRender(const Camera *camera)
{
    s_RenderManager->preRender(camera);
}

void RenderManager::Render()
{
    s_RenderManager->render();
}

void RenderManager::PosRender()
{
    s_RenderManager->posRender();
}

// private
void RenderManager::insert(GameObjectID gameObjectID)
{
    auto gameObject = GameObject::Get(gameObjectID);

    auto meshFilter = gameObject->getComponent<MeshFilter>();
    auto meshRender = gameObject->getComponent<MeshRender>();

    if (meshFilter == nullptr || meshRender == nullptr)
        return Console::LogError("[ RenderManager ] This %s don't has 'MeshFilter' or 'MeshRender'!", gameObject->getName().c_str());

    auto materials = meshRender->getMaterialIDs();
    auto meshes = meshFilter->getMesh();

    if (materials.size() != meshes.size())
        return Console::LogError("[ MeshRender ] Num meshes/materials incompative.");

    auto transform = gameObject->getTransform();

    size_t key = 0;

    size_t length = meshes.size();
    for (size_t i=0;i<length;i++)
    {
        MaterialID materialID = materials[i];
        MeshID meshID = meshes[i];

        key ^= std::hash<size_t>{}(meshID) + 0x9e3779b9 + (key << 6) + (key >> 2);
        key ^= std::hash<size_t>{}(materialID) + 0x9e3779b9 + (key << 6) + (key >> 2);

        auto batchID = BatchingManager::Find(key);
        if (batchID == InvalidBatch)
            batchID = BatchingManager::Create(key, meshID, materialID);

        OctreeManager::InsertGlobal({gameObjectID, batchID, transform->getPosition()});
    }
}

void RenderManager::remove(GameObjectID id)
{}

void RenderManager::preRender(const Camera *camera)
{
    auto projection = camera->getPerspesctive();
    auto view = camera->getViewmatrix();
    auto position = camera->getPosition();

    auto list = AssetsManager::GetShaderList(ShaderType::SHADER3D);
    for (auto &shader : list)
    {
        if (!shader || !shader->isValid())
        {
            continue;
        }

        shader->setUniform("u_projection", &projection);
        shader->setUniform("u_view",       &view);
        shader->setUniform("u_eye",        &position);

        setuplight(shader);
    }

    Frustum::CalculateFrustum(projection * view);

    OctreeManager::Foreach([](OctreeNode* node)
    {
        Vector3 position = node->position;
        float radius = node->radius;

        Vector3 min = position - Vector3{radius, radius, radius};
        Vector3 max = position + Vector3{radius, radius, radius};

        if (!Frustum::IsCubeInFrustumAABB(min, max))
            return false;

        for (int i=0;i<node->count;i++)
        {
            auto& data = node->data[i];

            auto gameObject = GameObject::Get(data.gameObjectID);
            auto transform = gameObject->getTransform();

            auto& batching = BatchingManager::Get(data.batchID);
            batching.insert(transform->getGlobalMatrix());
        }
        return true;
    });
}

void RenderManager::render()
{
    BatchingManager::Render();
}

void RenderManager::posRender()
{
}

void RenderManager::setuplight(grr::gShader *shader)
{
    auto lights = Light::GetListLight();

    int numLight = static_cast<int>(lights.size());

    if (!shader->setUniform("numLights", &numLight))
        return;

    if (lights.empty())
        return;

    for (std::size_t i={};i<lights.size();i++)
    {
        auto &light = lights[i];

        auto position   = light->getPosition();
        auto forward    = light->getForward();
        auto color      = light->getColor();
        auto type       = light->getType();

        // auto attenuation = Vector3{light->getAttenuationConstant(), light->getAttenuationLinear(), light->getAttenuationQuadractc()};
        // auto params      = Vector3{light->getInnecone(), light->getOutercone(), (float)light->getType()};
        // auto projection  = light->getProjetion();
        
        shader->setUniform(("lights[" + std::to_string(i) + "].position").c_str(),    &position);
        shader->setUniform(("lights[" + std::to_string(i) + "].direction").c_str(),   &forward);
        shader->setUniform(("lights[" + std::to_string(i) + "].color").c_str(),       &color);
        // shader->setUniform(("lights[" + std::to_string(i) + "].attenuation").c_str(), &attenuation);
        // shader->setUniform(("lights[" + std::to_string(i) + "].params").c_str(),      &params);
        shader->setUniform(("lights[" + std::to_string(i) + "].type").c_str(),       &type);
    }
}


