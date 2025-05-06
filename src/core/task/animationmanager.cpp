#include "gr-base/core/task/animationmanager.hpp"
#include "gr-base/core/animation/animation.hpp"
#include "gr-base/core/console.hpp"
#include "gr-base/core/file/file.hpp"
#include <memory>

// ********** protected ********** //
std::mutex AnimationManager::mutex;

// ********** public ********** //
AnimationManager::AnimationManager()
{}

Animation *AnimationManager::create(const std::string &name)
{
    std::unique_lock<std::mutex> lock(mutex);

    auto &&[it, created] = animation_map.emplace(name, std::make_unique<Animation>());
    if (created)
    {
        animation_name_map.emplace(&(*it->second), name);
    }

    return &(*it->second);
}

bool AnimationManager::remove_task(const std::string &name)
{
    std::lock_guard<std::mutex> lock(mutex);
    auto it = animationloadtask_map.find(name);
    if (it == animationloadtask_map.end())
    {
        return false;
    }
    animationloadtask_map.erase(it);
    return true;
}

Animation *AnimationManager::find(const std::string &name) const
{
    std::lock_guard<std::mutex> lock(mutex);
    auto it = animation_map.find(name);
    if (it == animation_map.end())
    {
        return nullptr;
    }
    return &(*it->second);
}

const char *AnimationManager::get_name(const Animation *animation) const
{
    std::lock_guard<std::mutex> lock(mutex);
    auto it = animation_name_map.find(animation);
    if (it == animation_name_map.end())
    {
        return nullptr;
    }
    return it->second.c_str();
}

// ********** private ********** //
void AnimationManager::load(const std::string &path, std::function<void (Animation *)> func)
{
    std::unique_lock<std::mutex> lock(mutex);
    // check if has been loading.
    {
        auto it = animationloadtask_map.find(path);
        if (it != animationloadtask_map.end())
        {
            if (func == nullptr)
            {
                return;
            }

            return it->second->addFunction(func);
        }
    }

    // check has been loaded
    {
        auto it = animation_map.find(path);
        if (it != animation_map.end())
        {
            if (func == nullptr)
            {
                return;
            }

            return func(&(*it->second));
        }
    }

    if (!gfile::exists(path))
    {
        return Console::LogError("[ Animation ] File do not exists: %s", path.c_str());
    }

    animationloadtask_map.emplace(path, std::make_unique<AnimationLoadTask>(path, func));
}



