#pragma once

#include "gr-base/core/animation/animation.hpp"

#include "gr-base/core/task/animationloadtask.hpp"
#include "gr-base/core/task/animationsavetask.hpp"

#include <memory>
#include <mutex>
#include <unordered_map>

class AnimationManager
{
public:
    AnimationManager();

    template <typename ...Args>
    void load(const std::string &path, Args &&...args);

    bool remove_task(const std::string &name);

    Animation *create(const std::string &name);

    Animation *find(const std::string &name) const;

    const char *get_name(const Animation *animation) const;

private:
    std::unordered_map<std::string, std::unique_ptr<AnimationLoadTask>> animationloadtask_map;

    std::unordered_map<std::string, std::unique_ptr<Animation>> animation_map;

    std::unordered_map<const Animation *, std::string> animation_name_map;

    void load(const std::string &path, std::function<void (Animation *)> func);

protected:
    static std::mutex mutex;
};

template <typename ...Args>
inline void AnimationManager::load(const std::string &path, Args &&...args)
{
    std::function<void (Animation *)> func = nullptr;
    if constexpr (sizeof...(Args) > 0)
    {
        func = std::bind(std::forward<Args>(args)..., std::placeholders::_1);
    }

    return load(path, func);
}



