#include "gr-base/core/task/filmboxmanager.hpp"

#include <imgui.h>

#include <filesystem>
#include <mutex>

// ********** public ********** //
std::mutex FilmboxManager::mutex;

// ********** public ********** //
FilmboxManager::FilmboxManager()
{}

FilmboxManager::~FilmboxManager()
{
    clear();
}

bool FilmboxManager::remove_task(const std::string &path)
{
    std::unique_lock<std::mutex> lock(mutex);
    auto it = filmboxtaskmap.find(path);
    if (it == filmboxtaskmap.end())
    {
        return false;
    }

    taskcount--;

    tasklist.erase(std::remove(tasklist.begin(), tasklist.end(), &(*it->second)), tasklist.end());

    taskpointertoname.erase(&(*it->second));

    filmboxtaskmap.erase(it);

    return true;
}

bool FilmboxManager::remove_task(const FilmboxTask *task)
{
    std::unique_lock<std::mutex> lock(mutex);
    auto it = taskpointertoname.find(task);
    if (it == taskpointertoname.end())
    {
        return false;
    }

    taskcount--;

    tasklist.erase(std::remove(tasklist.begin(), tasklist.end(), it->first), tasklist.end());

    filmboxtaskmap.erase(it->second);

    taskpointertoname.erase(it);

    return true;
}

void FilmboxManager::render()
{
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
}

bool FilmboxManager::has_task() const
{
    return taskcount > 0;
}

void FilmboxManager::clear()
{
    std::lock_guard<std::mutex> lock(mutex);

    taskcount = 0;

    tasklist.clear();

    taskpointertoname.clear();

    filmboxtaskmap.clear();
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

    taskpointertoname.emplace(&(*filmboxtaskmap[path]), path);

    tasklist.push_back(&(*filmboxtaskmap[path]));

    taskcount++;
}






