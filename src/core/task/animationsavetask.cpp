#include "gr-base/core/task/animationsavetask.hpp"

#include "gr-base/core/animation/animation.hpp"

#include "gr-base/core/console.hpp"
#include "gr-base/core/file/stream.hpp"

#include "gr-base/core/binary/gHeader.hpp"

#include <filesystem>
#include <ios>

AnimationSaveTask::AnimationSaveTask(std::string path, const Animation *animation)
{
    if (std::filesystem::is_directory(path))
    {
        path += path[path.size()] == '/' ? (animation->getName() + ".anm") : ("/" + animation->getName() + ".anm");
    }

    doInBackground(path, animation);
}

void AnimationSaveTask::doInBackground(std::string path, const Animation *animation)
{
    auto name = animation->getName();

    Stream stream(path, std::ios_base::out | std::ios_base::binary);

    {
        FileHeader fileHeader{{'g', 'r', 'a', 'n'}, 1.0f};

        if (!stream.writeS<FileHeader>(&fileHeader, 1, false))
        {
            stream.close();
            return Console::LogError("[ AnimationTask ] Fail to save animation 1-%s.", name.c_str());
        }
    }

    if (!animation->serialize(&stream))
    {
        stream.close();
        return Console::LogError("[ AnimationTask ] Fail to save animation 2-%s.", name.c_str());
    }

    stream.flush();

    stream.close();

    Console::LogInfo("[ AnimationTask ] Animation saved %s.", name.c_str());
}




