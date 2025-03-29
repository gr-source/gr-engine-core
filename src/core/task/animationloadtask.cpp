#include "gr-base/core/task/animationloadtask.hpp"

#include "BulletCollision/NarrowPhaseCollision/btPersistentManifold.h"
#include "gr-base/core/animation/animation.hpp"
#include "gr-base/core/assetsManager.hpp"
#include "gr-base/core/event.hpp"
#include "gr-base/core/task/animationsavetask.hpp"
#include "gr-base/core/threadpool.hpp"
#include "gr-base/core/console.hpp"

#include "gr-base/core/binary/gHeader.hpp"
#include <cstring>

AnimationLoadTask::AnimationLoadTask(const std::string &path, std::function<void (Animation *)> func)
{
    if (func != nullptr)
        m_funlist.push_back(func);

    Threadpool::AddTask(&AnimationLoadTask::doInBackground, this, path);
}

void AnimationLoadTask::addFunction(std::function<void (Animation *)> func)
{
    return m_funlist.push_back(func);
}

void AnimationLoadTask::invoke(Animation *animation)
{
    for (auto func : m_funlist)
    {
        func(animation);
    }
}

void AnimationLoadTask::postExecute(Animation *animation, std::string path)
{
    invoke(animation);

    AssetsManager::RemoveTask(path);

    Console::LogInfo("[ AnimationTask ] Animation loaded %s.", animation->getName().c_str());
}

void AnimationLoadTask::doInBackground(std::string path)
{
    Stream stream(path, std::ios_base::in | std::ios_base::binary);

    {
        FileHeader fileHeader{{'g', 'r', 'a', 'n'}, 1.0f};

        if (!stream.read(reinterpret_cast<char *>(&fileHeader), sizeof(FileHeader)))
        {
            stream.close();
            Console::LogError("[ AnimationTask ] Fail to load animation 1-%s.", path.c_str());
            return AssetsManager::RemoveTask(path);
        }

        if (std::strcmp(fileHeader.signature, "gran") != 0)
        {
            stream.close();
            Console::LogError("[ AnimationTask ] Failt to load animation, binary is not valid %s.", path.c_str());
            return AssetsManager::RemoveTask(path);
        }
    }

    auto animation = AssetsManager::CreateAnimation(path);

    if (!animation->deserialize(&stream))
    {
        stream.close();
        Console::LogError("[ AnimationTask ] Fail to load animation 2-%s.", path.c_str());
        return AssetsManager::RemoveTask(path);
    }

    stream.close();

    Event::Add(&AnimationLoadTask::postExecute, this, animation, path);
}







