#include "gr-base/managers/rendermanager.hpp"

#include "gr-base/core/console.hpp"
#include "gr-base/core/culling/frustum.hpp"

#include "gr-base/core/debugdraw.hpp"
#include "gr-base/core/mainthreaddispatcher.hpp"
#include "gr-base/core/status.hpp"
#include "gr-base/ecs/behaviorStorage.hpp"
#include "gr-base/managers/bvhtreemanager.hpp"
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
#include <cfloat>
#include <cstddef>
#include <gr-math/boundbox.hpp>
#include <gr-math/gmath.hpp>

RenderManager* RenderManager::s_RenderManager = nullptr;

void RenderManager::SetInstance(RenderManager* rendermanager)
{
    s_RenderManager = rendermanager;
}

void RenderManager::Insert(GameObjectHandle id)
{
    s_RenderManager->insert(id);
}

void RenderManager::Remove(GameObjectHandle id)
{
    s_RenderManager->remove(id);
}

void RenderManager::SwitchTo(GameObjectHandle handle, bool isStatic)
{
    s_RenderManager->switchTo(handle, isStatic);
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
void RenderManager::insert(GameObjectHandle gameObjectID)
{
    if (m_objectInOctree.find(gameObjectID) != m_objectInOctree.end())
        return;

    auto gameObject = GameObject::Get(gameObjectID);
    if (gameObject == nullptr)
        return;

    auto lodComponent = gameObject->getComponent<LODComponent>();
    auto meshFilter = gameObject->getComponent<MeshFilter>();
    auto meshRender = gameObject->getComponent<MeshRender>();

    if (meshFilter == nullptr || meshRender == nullptr)
        return Console::LogError("[ RenderManager ] This %s don't has 'MeshFilter' or 'MeshRender'!", gameObject->getName().c_str());

    auto materials = meshRender->getMaterialIDs();
    auto meshes = meshFilter->getMesh();

    if (materials.size() != meshes.size())
        return Console::LogError("[ MeshRender ] Num meshes/materials incompative.");

    auto transform = gameObject->GetTransform();

    Vector3 point = transform->getPosition();

    bool isStatic = gameObject->GetIsStatic();

    array* batchPerLOD = NULL;
    array* batchIDS = NULL;

    Boundbox boundbox;
    boundbox.min = { FLT_MAX, FLT_MAX, FLT_MAX };
    boundbox.max = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

    if (lodComponent != nullptr)
    {
        size_t length = meshes.size();
        auto lodMeshes = lodComponent->meshesLOD;

        batchPerLOD = array_create(length, sizeof(array*));
        array** vector = reinterpret_cast<array**>(batchPerLOD->vector);

        for (size_t lod = 0; lod < lodMeshes.size(); lod++)
        {
            auto& meshes = lodMeshes[lod].meshes;
            size_t meshCount = meshes.size();

            vector[lod] = array_create(meshCount, sizeof(BatchID));

            BatchID* vectorn = reinterpret_cast<BatchID*>(vector[lod]->vector);

            for (size_t m = 0; m < meshCount; m++)
                vectorn[m] = GetBatchID(boundbox, meshes[m], materials[m]);
        }
    } else
    {
        size_t length = meshes.size();

        batchIDS = array_create(length, sizeof(BatchID));
        BatchID* vector = reinterpret_cast<BatchID*>(batchIDS->vector);

        for (size_t i=0;i<length;i++)
            vector[i] = GetBatchID(boundbox, meshes[i], materials[i]);
    }

    auto matrix = transform->getGlobalMatrix();

    Vector4 min = matrix * Vector4{boundbox.min.x, boundbox.min.y, boundbox.min.z, 1.0f};
    Vector4 max = matrix * Vector4{boundbox.max.x, boundbox.max.y, boundbox.max.z, 1.0f};

    Boundbox globalBounds = Boundbox{Vector3{min.x, min.y, min.z}, Vector3{max.x, max.y, max.z}};

    // Vector3 scale = transform->getLocalScale();

    // std::cout << gameObject->getName() << ": " << point.x << " " << point.y << " " << point.z << std::endl;

    NodeData data{gameObjectID, batchIDS, globalBounds, batchPerLOD};

    NodeID nodeID = InvalidID;
    if (isStatic)
        nodeID = OctreeManager::InsertGlobal(data);
    else
    {
        nodeID = BVHTreeManager::Insert(boundbox, new NodeData(data));
    }

    NodeInfo info{};
    info.id = nodeID;
    info.type = isStatic ? 0 : 1;

    transform->isDirty = true;

    m_objectInOctree.emplace(gameObjectID, info);
}

BatchID RenderManager::GetBatchID(Boundbox& boundbox, MeshID meshID, MaterialID materialID) const
{
    size_t key = 0;
    key ^= std::hash<size_t>{}(meshID) + 0x9e3779b9 + (key << 6) + (key >> 2);
    key ^= std::hash<size_t>{}(materialID) + 0x9e3779b9 + (key << 6) + (key >> 2);

    BatchID batchID = BatchingManager::Find(key);
    if (batchID == InvalidBatch)
        batchID = BatchingManager::Create(key, meshID, materialID);

    auto mesh = AssetsManager::GetMesh(meshID);

    Boundbox meshBox = mesh->getBoundbox(); 

    boundbox = Math::Encapsulate(boundbox, meshBox);

    return batchID;
}

void RenderManager::remove(GameObjectHandle handle)
{
    auto it = m_objectInOctree.find(handle);
    if (it == m_objectInOctree.end())
        return;

    if (it->second.type == 0)
    {
        if (!OctreeManager::Remove(it->second.id, handle))
            return;
    } else
    {
        BVHTreeManager::Remove(it->second.id);
    }

    m_objectInOctree.erase(it);
}

void RenderManager::switchTo(GameObjectHandle handle, bool isStatic)
{
    RenderManager::Remove(handle);
    RenderManager::Insert(handle);
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
        if (!Frustum::IsCubeInFrustumAABB(node->boundary.min, node->boundary.max))
            return true;

        auto cameraTransform = camera->GetGameObject()->GetTransform();

        auto cameraPosition = cameraTransform->getPosition();

        for (int i=0;i<node->count;i++)
        {
            auto& data = node->data[i];

            auto gameObject = GameObject::Get(data.gameObjectID);

            auto lodComponent = gameObject->getComponent<LODComponent>();
            auto transform = gameObject->GetTransform();

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

    AsyncBVHTree(camera);

    // std::cout << "=====" << std::endl;
    BVHTreeManager::For_each([camera = std::forward<const Camera*>(camera)](NodeID id, BVHNode* node)
            {
                if (node->isLeaf())
                {
                    NodeData data = *static_cast<NodeData*>(node->data);

                    auto gameObject = GameObject::Get(data.gameObjectID);

                    auto transform = gameObject->GetTransform();

                    auto boundary = node->boundary;

                    if (!Frustum::IsCubeInFrustumAABB(boundary.min, boundary.max))
                        return true;

                    auto cameraTransform = camera->GetGameObject()->GetTransform();

                    auto cameraPosition = cameraTransform->getPosition();

                    {
                        auto lodComponent = gameObject->getComponent<LODComponent>();
                        auto transform = gameObject->GetTransform();

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
                }
                return false;
            });
}

void RenderManager::AsyncBVHTree(const Camera* camera)
{
    std::vector<NodeID> nodes;
    BVHTreeManager::For_each([&](NodeID id, BVHNode* node)
    {
        if (node->isLeaf())
        {
            NodeData data = *static_cast<NodeData*>(node->data);

            auto gameObject = GameObject::Get(data.gameObjectID);

            auto transform = gameObject->GetTransform();

            if (transform->isDirty)
            {
                auto matrix = transform->getGlobalMatrix();

                auto boundary = node->localBoundary;

                Vector4 min = matrix * Vector4{boundary.min.x, boundary.min.y, boundary.min.z, 1.0f};
                Vector4 max = matrix * Vector4{boundary.max.x, boundary.max.y, boundary.max.z, 1.0f};

                node->boundary = Boundbox{Vector3{min.x, min.y, min.z}, Vector3{max.x, max.y, max.z}};

                nodes.push_back(node->parent);

                transform->isDirty = false;
                return true;
            }
        }
        return false;
    });

    for (auto node : nodes)
        BVHTreeManager::SyncHeirarchy(node);


    if (!nodes.empty())
        AsyncBVHTree(camera);

    // nodes.clear();
}

void RenderManager::render()
{
    Status::BatchDrawCallCount = 0;
    Status::BatchRenderCount = 0;

    // BatchingManager::Render();
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


