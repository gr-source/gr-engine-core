#include "gr-base/managers/rendermanager.hpp"

#include "gr-base/core/console.hpp"
#include "gr-base/core/culling/frustum.hpp"

#include "gr-base/core/debugdraw.hpp"
#include "gr-base/core/mainthreaddispatcher.hpp"
#include "gr-base/ecs/behaviorStorage.hpp"
#include "gr-base/managers/octreemanager.hpp"

#include "gr-base/core/gameObject.hpp"
#include "gr-base/core/geometry/default.hpp"
#include "gr-base/core/scene.hpp"
#include "gr-base/managers/batchingmanager.hpp"
#include "gr-base/managers/assetsManager.hpp"

#include "gr-base/managers/sceneManager.hpp"
#include "gr-base/script/LODComponent.hpp"
#include "gr-base/script/meshRender.hpp"
#include "gr-base/script/meshFilter.hpp"
#include "gr-base/script/transform.hpp"
#include "gr-base/script/camera.hpp"
#include "gr-base/script/light.hpp"
#include <gr-math/gmath.hpp>

RenderManager* RenderManager::s_RenderManager = nullptr;

void RenderManager::Initialize()
{
    if (s_RenderManager == nullptr)
        s_RenderManager = new RenderManager();

    BatchingManager::Initialize();
    // OctreeManager::Initialize();
    DebugDraw::Initialize();
}

void RenderManager::Shutdown()
{
    if (s_RenderManager != nullptr)
        delete s_RenderManager;
    s_RenderManager = nullptr;

    BatchingManager::Shutdown();
    // OctreeManager::Shutdown();
    DebugDraw::Shutdown();
}

void RenderManager::Insert(GameObjectID id)
{
    s_RenderManager->insert(id);
}

void RenderManager::Remove(GameObjectID id)
{
    s_RenderManager->remove(id);
}

void RenderManager::MarkDirt(GameObjectID id, const Vector3& position)
{
    s_RenderManager->markDirt(id, position);
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
    if (m_objectInOctree.find(gameObjectID) != m_objectInOctree.end())
        return;

    auto gameObject = GameObject::Get(gameObjectID);

    auto lodComponent = gameObject->getComponent<LODComponent>();
    auto meshFilter = gameObject->getComponent<MeshFilter>();
    auto meshRender = gameObject->getComponent<MeshRender>();

    if (meshFilter == nullptr || meshRender == nullptr)
        return Console::LogError("[ RenderManager ] This %s don't has 'MeshFilter' or 'MeshRender'!", gameObject->getName().c_str());

    auto materials = meshRender->getMaterialIDs();
    auto meshes = meshFilter->getMesh();

    if (materials.size() != meshes.size())
        return Console::LogError("[ MeshRender ] Num meshes/materials incompative.");

    auto transform = gameObject->getTransform();

    if (lodComponent != nullptr)
    {
        size_t length = meshes.size();
        auto lodMeshes = lodComponent->meshesLOD;

        array* batchPerLOD = array_create(length, sizeof(array*));
        array** vector = reinterpret_cast<array**>(batchPerLOD->vector);

        for (size_t lod = 0; lod < lodMeshes.size(); lod++)
        {
            auto& meshes = lodMeshes[lod].meshes;
            size_t meshCount = meshes.size();

            vector[lod] = array_create(meshCount, sizeof(BatchID));

            BatchID* vectorn = reinterpret_cast<BatchID*>(vector[lod]->vector);

            for (size_t m = 0; m < meshCount; m++)
                vectorn[m] = GetBatchID(meshes[m], materials[m]);
        }

        m_objectInOctree[gameObjectID] = OctreeManager::InsertGlobal({gameObjectID, nullptr, transform->getPosition(), batchPerLOD});
    } else
    {
        size_t length = meshes.size();

        array* batchIDS = array_create(length, sizeof(BatchID));
        BatchID* vector = reinterpret_cast<BatchID*>(batchIDS->vector);

        for (size_t i=0;i<length;i++)
            vector[i] = GetBatchID(meshes[i], materials[i]);

        m_objectInOctree[gameObjectID] = OctreeManager::InsertGlobal({gameObjectID, batchIDS, transform->getPosition(), nullptr});
    }
}

BatchID RenderManager::GetBatchID(MeshID meshID, MaterialID materialID) const
{
    size_t key = 0;
    key ^= std::hash<size_t>{}(meshID) + 0x9e3779b9 + (key << 6) + (key >> 2);
    key ^= std::hash<size_t>{}(materialID) + 0x9e3779b9 + (key << 6) + (key >> 2);

    BatchID batchID = BatchingManager::Find(key);
    if (batchID == InvalidBatch)
        batchID = BatchingManager::Create(key, meshID, materialID);
    return batchID;
}

void RenderManager::remove(GameObjectID id)
{
    auto it = m_objectInOctree.find(id);
    if (it == m_objectInOctree.end())
        return;

    OctreeManager::Remove(it->second, id);

    m_objectInOctree.erase(it);
}

void RenderManager::markDirt(GameObjectID id, const Vector3& position)
{
    auto it = m_objectInOctree.find(id);
    if (it == m_objectInOctree.end())
        return;

    NodeID nodeID = OctreeManager::Get(id, position);
    if (nodeID == InvalidNode)
        return;

    if (!OctreeManager::Remove(nodeID, id))
        return;

    // std::cout << "Remove: " << id << " from: " << nodeID << std::endl;

    m_objectInOctree.erase(it);
}

size_t SelectLOD(float distance, const std::vector<LODMesh>& lods)
{
    for (size_t i = 0; i < lods.size(); i++)
    {
        if (distance < lods[i].minDistance)
            return i;
    }
    return lods.size() - 1;
}

void RenderManager::preRender(const Camera *camera)
{
    auto scene = Scene::GetCurrent();

    auto projection = camera->getPerspesctive();
    auto view = camera->getViewmatrix();
    auto position = camera->getPosition();

    auto list = AssetsManager::GetShaderList(ShaderType::SHADER3D);
    for (auto &shader : list)
    {
        if (!shader || !shader->isValid())
            continue;

        shader->setUniform("u_projection", &projection);
        shader->setUniform("u_view",       &view);
        shader->setUniform("u_eye",        &position);

        setuplight(scene, shader);
    }

    Frustum::CalculateFrustum(projection * view);

    OctreeManager::Foreach([camera = std::forward<const Camera*>(camera)](OctreeNode* node)
    {
        Vector3 position = node->position;
        float radius = node->radius;

        Vector3 min = position - Vector3{radius, radius, radius};
        Vector3 max = position + Vector3{radius, radius, radius};

        if (!Frustum::IsCubeInFrustumAABB(min, max))
            return true;

        auto cameraTransform = camera->GetGameObject()->getTransform();

        auto cameraPosition = cameraTransform->getPosition();

        for (int i=0;i<node->count;i++)
        {
            auto& data = node->data[i];

            auto gameObject = GameObject::Get(data.gameObjectID);
            auto lodComponent = gameObject->getComponent<LODComponent>();
            auto transform = gameObject->getTransform();

            auto objectPosition = transform->getPosition();

            auto model = transform->getGlobalMatrix();

            if (data.batchIDs != nullptr)
            {
                array* arr = data.batchIDs;

                BatchID* vector = reinterpret_cast<BatchID*>(arr->vector);
                for (BatchID id=0;id<arr->length;id++)
                {
                    auto& batching = BatchingManager::Get(vector[id]);
                    batching.insert(model);
                }
            } else
            {
                float distance = Math::distance(cameraPosition, objectPosition);

                size_t lodID = SelectLOD(distance, lodComponent->meshesLOD);

                array* batchArray = data.batchPerLOD;
                array** vector = reinterpret_cast<array**>(batchArray->vector);

                BatchID* nvector = reinterpret_cast<BatchID*>(vector[lodID]->vector);

                for (size_t m = 0; m < batchArray->length; m++)
                {
                    auto& batching = BatchingManager::Get(nvector[m]);
                    batching.insert(model);
                }
            }
        }
        return false;
    });
}

void RenderManager::render()
{
    BatchingManager::Render();
}

void RenderManager::posRender()
{
}

void RenderManager::setuplight(const Scene* scene, grr::gShader *shader)
{
    auto lights = scene->GetLights();

    int numLight = static_cast<int>(lights.size());

    if (!shader->setUniform("numLights", &numLight))
        return;

    if (lights.empty())
        return;

    for (std::size_t i={};i<lights.size();i++)
    {
        auto light = BehaviorStorage::GetComponent<Light>(lights[i]);

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


