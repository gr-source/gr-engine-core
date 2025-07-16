#include "gr-base/core/task/filmboxmanager.hpp"

#include "gr-base/core/thread/taskmanager.hpp"

#include "gr-base/core/file/file.hpp"


#include "gr-base/core/window.hpp"

#include "gr-base/core/task/runtime/gameobjectsavetask.hpp"
#include "gr-base/core/task/runtime/filmboxtask.hpp"

#include "gr-base/ecs/gameObject.hpp"
#include "gr-base/script/transform.hpp"

#include <imgui.h>

// ********** public ********** //
FilmboxManager::FilmboxManager()
{}

FilmboxManager::~FilmboxManager()
{}

std::shared_ptr<TaskInfo> FilmboxManager::create_task_load(const std::string &path, std::function<void ()> func)
{
    return TaskManager::Add<FilmboxTask>(path, this);
}

bool FilmboxManager::remove_task(const std::string &path)
{
    return false;
}

void FilmboxManager::render()
{
    if (resultTask.empty())
    {
        return;
    }

    bool isOpen = true;

    auto mainWindowSize = Window::GetWindowSize() / 2;
    auto beginSize = ImVec2{400, 300};

    ImGui::SetNextWindowSize(beginSize);
    ImGui::SetNextWindowPos(ImVec2{static_cast<float>(mainWindowSize.x), static_cast<float>(mainWindowSize.y)} - (beginSize / 2.0f));

    if (ImGui::Begin("FilmboxManager", &isOpen))
    {
        for (auto &&[name, root] : resultTask)
        {
            if (ImGui::CollapsingHeader(name.c_str()))
            {
                for (auto &go : root)
                    render_tree_node(go);
            }
        }

        auto avail = ImGui::GetContentRegionAvail();

        if (ImGui::Button("Save", ImVec2(avail.x, 0.0f)))
        {
            auto dir = File::GetAsset("/model");

            for (auto &&[name, root] : resultTask)
            {
                for (auto &go : root)
                    TaskManager::Add<GameObjectSaveTask>(dir, go, std::bind(&FilmboxManager::GameObjectSave, this, std::placeholders::_1));
            }
        }
    }
    ImGui::End();

    if (!isOpen)
    {
        for (auto &&[name, root] : resultTask)
        {
            for (auto &go : root)
                GameObject::Destroy(go);
        }
        resultTask.clear();

        taskResultMap.clear();
    }
}

void FilmboxManager::create_save(const std::string &name, std::vector<GameObject *> root)
{
    if (taskResultMap.find(name) != taskResultMap.end())
    {
        return;
    }

    resultTask.emplace_back(name, root);

    taskResultMap.emplace(name);
}

void FilmboxManager::render_tree_node(const GameObject *gameObject)
{
    auto transform = gameObject->getTransform();

    auto isOpen = ImGui::TreeNodeEx(gameObject->getName().c_str(), !transform->hasChildren() ? ImGuiTreeNodeFlags_Leaf : 0);
    if (!isOpen)
    {
        return;
    }

    for (auto &child : transform->getChildren())
        render_tree_node(child->getGameObject());

    ImGui::TreePop();
}

void FilmboxManager::GameObjectSave(GameObject *go)
{
    resultTask.erase(std::remove_if(resultTask.begin(), resultTask.end(),
                [go](auto &tuple) {
                    auto &rootList = std::get<1>(tuple);
                    rootList.erase(std::remove(rootList.begin(), rootList.end(), go), rootList.end());

                    return rootList.empty();
                }), resultTask.end());

    GameObject::Destroy(go);
}


