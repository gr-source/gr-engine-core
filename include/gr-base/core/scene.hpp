#pragma once

#include "gr-base/core/gameObject.hpp"
#include "gr-base/core/default.hpp"

#include "gr-base/core/system.hpp"
#include "gr-base/ecs/default.hpp"

#include "gr-base/managers/default.hpp"

#include <unordered_set>
#include <vector>
#include <string>
#include <memory>

enum class StageType
{
    INPUT,
    SIMULATION,
    RENDER,
    COUNT
};

class Scene
{
public:
    static Scene* GetCurrent();

    bool hierarchyDirty;

    Scene();
    ~Scene();

    inline Entity GetRoot() const
    {
        return m_root;
    }

    Entity create(const char* name);

    Entity create(Entity other);

    inline GameObject Wrap(Entity e)
    {
        return {this, e};
    }

    void destroy(Entity e);

    void select(Handle id);

    void deselect(Handle id);

    void deselect_all();

    bool has_object_selected() const;

    bool object_has_been_selected(Entity id) const;

    std::vector<Handle> get_object_selected() const;

    void SetName(const std::string& name);

    std::string GetName() const;

    inline World* GetWorld() const
    {
        return m_world;
    }

    size_t GetEntityCount() const;

    void MarkChildrenDirty(Entity entity);

    void Unlink(Entity child);

    void SetParent(Entity parent, Entity child);

    inline void SetCamera(Entity entity)
    {
        m_camera = entity;
    }

    inline Entity GetCamera() const
    {
        return m_camera;
    }

    std::string GetEntityName(Entity entity);

    template <typename T, typename... Args>
    void RegistrySystem(StageType stage, Args&&... args);

    void Update();

private:
    std::unordered_set<Handle> u_object_has_been_selected;

    std::vector<Entity> v_object_selected_list;

    std::vector<std::unique_ptr<System>> m_systems[static_cast<int>(StageType::COUNT)];

    World* m_world;

    std::string m_name;

    Entity m_root;

    Entity m_camera;
};


template <typename T, typename... Args>
void Scene::RegistrySystem(StageType stage, Args&&... args)
{
    m_systems[static_cast<int>(stage)].push_back(std::make_unique<T>(std::forward<Args>(args)...));
}



