#pragma once

#include "gr-base/core/task/runtime/filmboxtask.hpp"

#include <atomic>

class FilmboxManager
{
public:
    FilmboxManager();
    ~FilmboxManager();

    std::shared_ptr<TaskInfo> create_task_load(const std::string &path, std::function<void ()> func);

    bool remove_task(const std::string &name);

    void render();

private:
    std::unordered_map<std::string, std::shared_ptr<TaskInfo>> m_filmbox_task_map;

    std::unordered_map<const FilmboxTask *, const std::string> taskpointertoname;

    std::vector<FilmboxTask *> tasklist;
    
    std::mutex mutex;
};








