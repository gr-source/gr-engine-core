#include "gr-base/core/task/filmboxmanager.hpp"

#include <imgui.h>

#include <filesystem>

// ********** public ********** //
std::mutex FilmboxManager::mutex;

// ********** public ********** //
FilmboxManager::FilmboxManager()
{}

FilmboxManager::~FilmboxManager()
{}

bool FilmboxManager::remove_task(const std::string &path)
{
    std::lock_guard<std::mutex> lock(mutex);
    auto it = filmboxtaskmap.find(path);
    if (it == filmboxtaskmap.end())
    {
        return false;
    }

    taskcount--;

    filmboxtaskmap.erase(it);

    return true;
}

void FilmboxManager::render()
{
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
}

bool FilmboxManager::has_task() const
{
    return taskcount > 0;
}

// ********** private ********** //
void FilmboxManager::load(const std::string &path, std::function<void ()> func)
{
    std::unique_lock<std::mutex> lock(mutex);
    {
        auto it = filmboxtaskmap.find(path);
        if (it != filmboxtaskmap.end())
        {
            if (func == nullptr)
            {
                return;
            }

            return it->second->add_function(func);
        }
    }

    filmboxtaskmap.emplace(path, std::make_unique<FilmboxTask>(path, func));

    taskcount++;
}






