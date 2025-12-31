#include "gr-base/core/task/runtime/animationloadtask.hpp"

#include "gr-base/core/task/runtime/animationsavetask.hpp"

#include "gr-base/core/animation/animation.hpp"

#include "gr-base/managers/assetsManager.hpp"

#include "gr-base/core/mainthreaddispatcher.hpp"

#include "gr-base/core/thread/threadpool.hpp"

#include "gr-base/core/console.hpp"

#include "gr-base/core/binary/gHeader.hpp"
#include <cstring>

AnimationLoadTask::AnimationLoadTask(const std::string &path, std::function<void (Animation *)> func) : m_animation(nullptr)
{
    if (func != nullptr)
        m_funlist.push_back(func);

    Threadpool::AddTask(&AnimationLoadTask::doInBackground, this, path);
}

AnimationLoadTask::~AnimationLoadTask()
{
    if (m_animation == nullptr)
    {
        return;
    }

    invoke(m_animation);
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

void AnimationLoadTask::upload(std::string path)
{
}

void AnimationLoadTask::doInBackground(std::string path)
{
    Stream stream(path, std::ios_base::in | std::ios_base::binary);

    {
        FileHeader fileHeader;
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

    /*
    m_animation = AssetsManager::CreateAnimation(path);

    if (!m_animation->deserialize(&stream))
    {
        stream.close();
        Console::LogError("[ AnimationTask ] Fail to load animation 2-%s.", path.c_str());
        return AssetsManager::RemoveTask(path);
    }
    */

    stream.close();

    AssetsManager::RemoveTask(path);

    Console::LogInfo("[ AnimationTask ] Animation loaded %s.", m_animation->getName().c_str());
}







