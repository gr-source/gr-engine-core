#pragma once

#include "gr-base/core/task/runtime/filmboxtask.hpp"

#include <atomic>
#include <unordered_set>
#include <vector>

class FilmboxManager
{
public:
    FilmboxManager();
    ~FilmboxManager();

    std::shared_ptr<TaskInfo> create_task_load(const std::string &path, std::function<void ()> func);

    bool remove_task(const std::string &name);

    void render();

    void create_save(const std::string &name, std::vector<GameObject *> root, std::vector<std::unique_ptr<GameObject>> gameObjectList);

private:
    std::vector<std::tuple<std::string, std::vector<GameObject *>, std::vector<std::unique_ptr<GameObject>>>> resultTask;

    std::unordered_set<std::string> taskResultMap;
    
    std::mutex mutex;

    void GameObjectSave(const GameObject *go);
};








