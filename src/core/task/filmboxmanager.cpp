#include "gr-base/core/task/filmboxmanager.hpp"
#include "gr-base/core/task/runtime/filmboxtask.hpp"
#include "gr-base/core/thread/taskmanager.hpp"

#include <imgui.h>

#include <filesystem>
#include <mutex>

// ********** public ********** //
FilmboxManager::FilmboxManager()
{}

FilmboxManager::~FilmboxManager()
{}

std::shared_ptr<TaskInfo> FilmboxManager::create_task_load(const std::string &path, std::function<void ()> func)
{
    std::unique_lock<std::mutex> lock(mutex);
    auto it = m_filmbox_task_map.find(path);
    if (it != m_filmbox_task_map.end())
    {
        return it->second;
    }

    auto info = TaskManager::Add<FilmboxTask>(path);

    m_filmbox_task_map.emplace(path, info);

    return info;
}

bool FilmboxManager::remove_task(const std::string &path)
{
    std::lock_guard<std::mutex> lock(mutex);
    auto it = m_filmbox_task_map.find(path);
    if (it == m_filmbox_task_map.end())
    {
        return false;
    }
    m_filmbox_task_map.erase(it);
    return true;
}

void FilmboxManager::render()
{
    /*
    auto avail = ImGui::GetContentRegionAvail();

    std::unique_lock<std::mutex> lock(mutex);
    for (auto &&[name, task] : filmboxtaskmap)
    {
        lock.unlock();

        std::filesystem::path path = name;

        if (ImGui::TreeNode(path.filename().c_str()))
        {
            task->render();

            ImGui::TreePop();
        }

        lock.lock();
    }

    if (ImGui::Button("Save", ImVec2(avail.x, 0.0f)))
    {
        for (auto task : tasklist)
        {
            lock.unlock();

            task->save();
            // apartir daqui o ponteiro de 'task' e invalido

            lock.lock();
        }
    }
    */
}





