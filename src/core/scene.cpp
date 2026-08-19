#include "gr-base/core/scene.hpp"

#include "gr-base/core/console.hpp"
#include "gr-base/core/gameObject.hpp"
#include "gr-base/core/default.hpp"
#include "gr-base/core/logger.hpp"
#include "gr-base/core/system.hpp"

#include "gr-base/ecs/componentRegistry.hpp"
#include "gr-base/ecs/systemRegistry.hpp"

#include "gr-base/script/meshFilter.hpp"
#include "gr-base/script/meshRender.hpp"
#include "gr-base/spatial/bvhtree.hpp"

#include "gr-base/externals/handle.h"
#include "gr-base/managers/sceneManager.hpp"

#include "gr-base/script/relationship.hpp"
#include "gr-base/script/transform.hpp"
#include "gr-base/script/name.hpp"

#include <algorithm>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <utility>

// public
Scene* Scene::GetCurrent()
{
    return SceneManager::GetCurrent();
}

Scene::Scene() :
    m_root(INVALID_ENTITY)
{
    logger::info("[ Scene ] Create '%p'.", this);

    m_root = world.Create();
    world.AddComponent<Relationship>(m_root);
}

Scene::~Scene()
{
    logger::info("[ Scene ] Destroy '%p'.", this);

    for (auto& id : m_systemIDs)
    {
        void* system = m_typeIDToSystem[id];

        auto info = SystemRegistry::GetSystemByID(id);

        ((void (*)(void*))(info->destructor))(system);

        free(system);
    }
}

Entity Scene::create(const char* name)
{
    Entity e = world.Create();

    world.AddComponent<TransformLocal>(e);
    world.AddComponent<TransformWorld>(e);

    if (name != nullptr)
        world.AddComponent<Name>(e, strdup(name));
    
    SetParent(m_root, e);

    return e;
}

Entity Scene::create(Entity other)
{
    if (other == INVALID_ENTITY)
        return INVALID_ENTITY;

    GameObject go(this, create(nullptr));
    const GameObject otherGO(this, other);
    // m_isStatic = otherGO->m_isStatic;

    auto otherTrans = otherGO.GetComponent<TransformLocal>();

    auto myTrans = go.GetComponent<TransformLocal>();
    myTrans->position = otherTrans->position;
    myTrans->rotation = otherTrans->rotation;
    myTrans->scale = otherTrans->scale;

    MarkChanged<TransformWorld>(go.entity);

    auto otherRel = otherGO.GetComponent<Relationship>();
    if (otherRel != nullptr)
    {
        Entity sibling = otherRel->firstChild;
        while (sibling != INVALID_ENTITY)
        {
            Entity newChild = create(sibling);
            SetParent(go.entity, newChild);

            auto siblingRel = world.GetComponent<Relationship>(sibling);
            if (siblingRel == nullptr)
                break;
            sibling = siblingRel->nextSibling;
        }
    }

    auto rel = go.GetComponent<Relationship>();
    if (rel == nullptr || rel->parent == INVALID_ENTITY)
        SetParent(m_root, go.entity);

    for (auto& typeID : world.GetAllComponentType(other))
    {
        auto type = ComponentRegistry::GetComponentByID(typeID);

        if (go.HasComponentByID(typeID))
            continue;

        void* src = otherGO.GetComponentByID(typeID);
        void* dst = go.AddComponentByID(typeID);

        if (type->isTriviallyCopyable)
        {
            memcpy(dst, src, type->stride);
        } else
        {
            // o component foi inicializado em AddComponent
            ((void(*)(void*))(type->destructor))(dst);

            // Reinicializa com os dados novos copiados
            ((void(*)(void*, const void*))(type->copy))(dst, src);
        }
    }

    if (otherGO.HasComponent<RenderBounds>())
    {
        auto otherRenderBounds = otherGO.GetComponent<RenderBounds>();
        if (otherRenderBounds->id != UINT32_MAX)
        {
            auto renderBounds = go.GetOrAddComponent<RenderBounds>();
            renderBounds->localBounds = otherRenderBounds->localBounds;
            renderBounds->id = bvhtree.insert(go.entity, otherRenderBounds->localBounds);
        }
    }

    return go.entity;
}

void Scene::destroy(Entity entity)
{
    if (world.HasComponent<RenderBounds>(entity))
    {
        auto renderBounds = world.GetComponent<RenderBounds>(entity);
        if (renderBounds->id != UINT32_MAX)
            bvhtree.remove(renderBounds->id);
    }

    Unlink(entity);

    world.Destroy(entity);
}

// ********** public ********** //
void Scene::SetName(const std::string& name)
{
    m_name = name;
}

std::string Scene::GetName() const
{
    return m_name;
}

size_t Scene::GetEntityCount()
{
    return world.GetEntityCount();
}

Entity Scene::GetRoot() const
{
    return m_root;
}

void Scene::Unlink(Entity entity)
{
    auto rel = world.GetComponent<Relationship>(entity);
    if (rel == nullptr)
        return;

    Entity parent = rel->parent;

    if (parent != INVALID_ENTITY)
    {
        auto parentRel = world.GetComponent<Relationship>(parent);

        // se era o primeiro filho
        if (parentRel->firstChild == entity)
            parentRel->firstChild = rel->nextSibling;

        // conecta anterior -> próximo
        if (rel->prevSibling != INVALID_ENTITY)
        {
            auto prevRel = world.GetComponent<Relationship>(rel->prevSibling);

            prevRel->nextSibling = rel->nextSibling;
        }

        // conecta próximo -> anterior
        if (rel->nextSibling != INVALID_ENTITY)
        {
            auto nextRel = world.GetComponent<Relationship>(rel->nextSibling);

            nextRel->prevSibling = rel->prevSibling;
        }
    }

    rel->parent       = INVALID_ENTITY;
    rel->prevSibling  = INVALID_ENTITY;
    rel->nextSibling  = INVALID_ENTITY;

    hierarchySync[writeIndex] = true;
}

void Scene::SetParent(Entity parentEntity, Entity childEntity)
{
    if (parentEntity == childEntity)
        return;

    GameObject parent(this, parentEntity);
    GameObject child(this, childEntity);

    auto childRel  = child.GetOrAddComponent<Relationship>();
    auto parentRel = parent.GetOrAddComponent<Relationship>();

    if (childRel->parent != INVALID_ENTITY)
        Unlink(child.entity);

    childRel->parent = parent.entity;
    childRel->nextSibling = INVALID_ENTITY;
    childRel->prevSibling = INVALID_ENTITY;

    // Caso seja o primeiro filho
    if (parentRel->firstChild == INVALID_ENTITY)
    {
        parentRel->firstChild = child.entity;
    } else
    {
        Entity oldFirst = parentRel->firstChild;

        auto oldFirstRel = world.GetComponent<Relationship>(oldFirst);

        // novo filho aponta pro antigo primeiro
        childRel->nextSibling = oldFirst;

        // antigo primeiro aponta de volta
        oldFirstRel->prevSibling = child.entity;

        // parent agora aponta pro novo
        parentRel->firstChild = child.entity;
    }

    hierarchySync[writeIndex] = true;

    MarkChanged<TransformWorld>(child.entity);
}

void Scene::SetCamera(Entity entity)
{
    currentCamera = entity;
}

Entity Scene::GetCurrentCamera() const
{
    return currentCamera != INVALID_ENTITY ? currentCamera : mainCamera;
}

void Scene::SetMainCamera(Entity entity)
{
    mainCamera = entity;
}

Entity Scene::GetMainCamera() const
{
    return mainCamera;
}

bool Scene::CanSyncHierarchyUI() const
{
    return hierarchySync[readIndex];
}

void Scene::GetRootEntities(std::vector<Entity>& entities) const
{
    auto rel = world.GetComponent<Relationship>(m_root);

    Entity child = rel->firstChild;
    while (child != INVALID_ENTITY)
    {
        entities.push_back(child);

        auto rel = world.GetComponent<Relationship>(child);
        if (rel == nullptr)
            break;
        child = rel->nextSibling;
    }
}

void Scene::BeginFrame()
{
}

void Scene::EndFrame()
{
    std::swap(readIndex, writeIndex);

    for (auto& change : changes[writeIndex])
        change.entities.clear();

    currentFrame++;

    hierarchySync[writeIndex] = false;
}

// #define ENABLE_PROFILING

#ifdef ENABLE_PROFILING
#include "gr-base/scopeTimer.hpp"
#include "gr-base/profiler.hpp"
#endif

void Scene::Update()
{
#ifdef ENABLE_PROFILING
    for (auto& systemID : m_systemIDs)
    {
        auto info = SystemRegistry::GetSystemByID(systemID);

        ISystem* sys = m_typeIDToSystem[systemID];

        ScopeTimer scope;

        scope.start();

        sys->Update();

        double ms =
            scope.end();

        Profiler::Add("[ Update ] " + std::string(info->name), static_cast<float>(ms));
    }
#else // ENABLE_PROFILING
    for (auto& sys : m_systems)
        sys->Update();
#endif // ENABLE_PROFILING
}

void Scene::Render()
{
#ifdef ENABLE_PROFILING
    for (auto& systemID : m_systemIDs)
    {
        auto info = SystemRegistry::GetSystemByID(systemID);

        ISystem* sys = m_typeIDToSystem[systemID];

        ScopeTimer scope;

        scope.start();

        sys->Render();

        double ms =
            scope.end();

        Profiler::Add("[ Render ] " + std::string(info->name), static_cast<float>(ms));
    }
#else // ENABLE_PROFILING
    for (auto& sys : m_systems)
        sys->Render();
#endif // ENABLE_PROFILING

    // bvhtree.renderer();
}

void Scene::MarkChanged(TypeID typeID, Entity entity)
{
    if (typeID >= changes[writeIndex].size())
        changes[writeIndex].resize(typeID + 1);

    auto entityID = handle_get_first(entity);

    auto& change = changes[writeIndex][typeID];
    if (entityID >= change.frames.size())
        change.frames.resize(entityID + 1, 0);

    if (change.frames[entityID] != currentFrame)
    {
        change.entities.push_back(entity);

        change.frames[entityID] = currentFrame;
    }
}

bool Scene::Changed(TypeID typeID, std::vector<Entity>& entities) const
{
    if (typeID >= changes[readIndex].size() || changes[readIndex][typeID].entities.empty())
        return false;

    entities = changes[readIndex][typeID].entities;

    return true;
}

void Scene::AddSystemByID(TypeID systemID)
{
    auto info = SystemRegistry::GetSystemByID(systemID);
    if (info == nullptr)
        return;

    if (info->index >= m_typeIDToSystem.size())
        m_typeIDToSystem.resize(info->index + 1, nullptr);

    if (m_typeIDToSystem[info->index] != nullptr)
        return;

    ISystem* system = createSystem(info);
    if (system == nullptr)
        return;

    m_systemIDs.push_back(info->index);

    m_typeIDToSystem[info->index] = system;

    m_systems.push_back(system);

    sortSystems();
}

void Scene::DelSystemByID(TypeID systemID)
{
    auto info = SystemRegistry::GetSystemByID(systemID);
    if (info == nullptr)
        return;

    if (info->index >= m_typeIDToSystem.size())
        return;

    if (m_typeIDToSystem[info->index] == nullptr)
        return;

    m_systemIDs.erase(std::remove(m_systemIDs.begin(), m_systemIDs.end(), info->index), m_systemIDs.end());

    ISystem* system = m_typeIDToSystem[info->index];
    m_typeIDToSystem[info->index] = nullptr;

    m_systems.erase(std::remove(m_systems.begin(), m_systems.end(), system), m_systems.end());

    info->destructor(system);
    free(system);

    sortSystems();
}

std::vector<TypeID> Scene::GetSystemIDs() const
{
    return m_systemIDs;
}

ISystem* Scene::GetSystemByID(TypeID id) const
{
    if (id >= m_typeIDToSystem.size())
        return nullptr;
    return m_typeIDToSystem[id];
}

ISystem* Scene::createSystem(const SystemInfo* info)
{
    ISystem* system = (ISystem*)malloc(info->stride);
    info->constructor(system, nullptr);
    system->scene = this;

    try
    {
        system->Init();
    } catch (const std::exception& e)
    {
        info->destructor(system);

        free(system);

        Console::LogError("[ Scene ] Fail in to create system='%s' error='%s'.", info->name, e.what());

        return nullptr;
    }

    return system;
}

void Scene::sortSystems()
{
    std::unordered_map<TypeID, std::vector<TypeID>> graph;

    std::unordered_map<TypeID, std::vector<TypeID>> readers;

    std::unordered_map<TypeID, std::vector<TypeID>> writers;

    for (TypeID systemID : m_systemIDs)
    {
        auto sys = m_typeIDToSystem[systemID];
        if (sys == nullptr)
            continue;

        Signature readMask = sys->GetRead();

        while (readMask != INVALID_SIGNATURE && readMask)

        {
            TypeID component =
                static_cast<TypeID>(__builtin_ctzll(readMask));

            readers[component].push_back(systemID);

            readMask &= (readMask - 1);
        }

        Signature writeMask = sys->GetWrite();

        while (writeMask != INVALID_SIGNATURE && writeMask)
        {
            TypeID component =
                static_cast<TypeID>(__builtin_ctzll(writeMask));

            writers[component].push_back(systemID);

            writeMask &= (writeMask - 1);
        }
    }

    for (const auto& [component, componentReaders] : readers)
    {
        auto itWriters = writers.find(component);

        if (itWriters == writers.end())
            continue;

        const auto& componentWriters = itWriters->second;

        for (TypeID reader : componentReaders)
        {
            for (TypeID writer : componentWriters)
            {
                if (writer == reader)
                    continue;

                graph[reader].push_back(writer);
            }
        }
    }

    for (const auto& [component, componentWriters] : writers)
    {
        for (size_t i = 0; i < componentWriters.size(); ++i)
        {
            TypeID first = componentWriters[i];

            for (size_t j = i + 1;
                 j < componentWriters.size();
                 ++j)
            {
                TypeID second = componentWriters[j];

                if (first == second)
                    continue;

                graph[second].push_back(first);
            }
        }
    }

    for (auto& [systemID, dependencies] : graph)
    {
        std::sort(
            dependencies.begin(),
            dependencies.end()
        );

        dependencies.erase(
            std::unique(
                dependencies.begin(),
                dependencies.end()
            ),
            dependencies.end()
        );
    }

    std::unordered_set<TypeID> visited;
    std::unordered_set<TypeID> temp;

    std::vector<TypeID> order;
    order.reserve(m_systemIDs.size());

    bool hasCycle = false;

    std::function<void(TypeID)> dfs =
        [&](TypeID node)
    {
        if (hasCycle)
            return;

        if (temp.count(node))
        {
            for (auto& t : temp)
                std::cout << SystemRegistry::GetSystemByID(t)->name << std::endl;
            hasCycle = true;
            return;
        }

        if (visited.count(node))
            return;

        temp.insert(node);

        auto it = graph.find(node);

        if (it != graph.end())
        {
            for (TypeID dependency : it->second)
            {
                dfs(dependency);

                if (hasCycle)
                    return;
            }
        }

        temp.erase(node);
        visited.insert(node);

        order.push_back(node);
    };

    for (TypeID systemID : m_systemIDs)
    {
        if (!visited.count(systemID))
            dfs(systemID);

        if (hasCycle)
            break;
    }


    if (hasCycle)
    {
        throw std::runtime_error(
            "Erro: ciclo detectado na DAG de sistemas!"
        );
    }

    for (size_t i = 0; i < order.size(); ++i)
    {
        TypeID systemID = order[i];

        m_systemIDs[i] = systemID;
        m_systems[i] = m_typeIDToSystem[systemID];
    }
}



