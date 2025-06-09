#include "gr-base/core/task/filmboxtask.hpp"

#include "gr-base/core/assetsManager.hpp"
#include "gr-base/core/threadpool.hpp"
#include "gr-base/core/console.hpp"
#include "gr-base/core/event.hpp"

#include "gr-base/core/filmbox/filmbox.hpp"

#include "gr-base/ecs/gameObject.hpp"
#include "gr-base/script/transform.hpp"
#include "imgui.h"

// ********** public ********** //
FilmboxTask::FilmboxTask(const std::string &path, std::function<void ()> func) : m_name(path), bcanrender(false)
{
    add_function(func);

    Threadpool::AddTask(&FilmboxTask::doInBackground, this);
}

FilmboxTask::~FilmboxTask()
{
    invoke();
}

void FilmboxTask::add_function(std::function<void ()> func)
{
    if (func == nullptr)
    {
        return;
    }

    funclist.push_back(func);
}

void FilmboxTask::render()
{
    if (!bcanrender)
    {
        return;
    }

    auto avail = ImGui::GetContentRegionAvail();

    if (ImGui::CollapsingHeader("Node"))
    {
        for (auto go : m_objectroot)
        {
            render_hierarchy(go);
        }
    }

    if (ImGui::CollapsingHeader("Animation"))
    {}

    if (ImGui::Button("Save", ImVec2(avail.x, 0.0f)))
    {}
}

void FilmboxTask::render_hierarchy(GameObject *go)
{
    auto transform = go->getTransform();

    if (ImGui::TreeNodeEx(go->getName().c_str(), transform->hasChildren() ? ImGuiTreeNodeFlags_None : ImGuiTreeNodeFlags_Leaf))
    {
        for (auto children : transform->getChildren())
        {
            render_hierarchy(children->getGameObject());
        }
        ImGui::TreePop();
    }
}

// ********** private ********** //
void FilmboxTask::invoke()
{
    for (auto &func : funclist)
    {
        func();
    }
}

void FilmboxTask::doInBackground()
{
    Filmbox filmbox;
    try
    {
        filmbox.load(m_name);
    }
    catch (const std::exception &e)
    {
        AssetsManager::RemoveTask(m_name);

        return Console::LogError("[ FilmboxTask ] %s.", e.what());
    }

    m_scene = filmbox.get_scene();

    load_object(m_scene.get());

    bcanrender = true;
}

void FilmboxTask::load_object(const FbxScene *fbxscene)
{
    auto &root = fbxscene->getRootNode();
    
    for (auto &node : root.getChildren())
    {
        m_objectroot.push_back(create_from(node));
    }
}

GameObject *FilmboxTask::create_from(const FbxNode &node, Transform *parent)
{
    auto go = &(*m_objectlist.emplace_back(std::make_unique<GameObject>(node.getName())));
    
    auto transform = go->getTransform();
    if (parent != nullptr)
    {
        transform->setParent(parent);
    }

    for (auto &children : node.getChildren())
    {
        auto child = create_from(children, transform);
        if (child == nullptr)
        {
            break;
        }
    }
    return go;
}



