#pragma once

#include "gr-base/core/task/runtime/filmboxtask.hpp"

#include <unordered_map>
#include <mutex>
#include <vector>

class FilmboxManager
{
protected:
    static std::mutex mutex;

public:
    FilmboxManager();
    ~FilmboxManager();

    template <typename ...Args>
    void load(const std::string &path, Args&&... args);

    bool remove_task(const std::string &name);

    bool remove_task(const FilmboxTask *task);

    void render();

    bool has_task() const;

    void clear();

private:
    std::unordered_map<std::string, std::unique_ptr<FilmboxTask>> filmboxtaskmap;

    std::unordered_map<const FilmboxTask *, const std::string> taskpointertoname;

    std::vector<FilmboxTask *> tasklist;

    std::atomic<std::uint32_t> taskcount;

    void load(const std::string &path, std::function<void ()> func);
};

template <typename ...Args>
inline void FilmboxManager::load(const std::string &path, Args &&...args)
{
    std::function<void ()> func = nullptr;
    if constexpr (sizeof...(Args) > 0)
    {
        func = std::bind(std::forward<Args>(args)...);
    }

    return load(path, func);
}







