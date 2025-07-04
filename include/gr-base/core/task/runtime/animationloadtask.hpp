#pragma once

#include <functional>
#include <string>
#include <vector>

#include "gr-base/core/animation/default.hpp"

class AnimationLoadTask
{
public:
    AnimationLoadTask(const std::string &path, std::function<void (Animation *)> func = nullptr);
    ~AnimationLoadTask();

    void addFunction(std::function<void (Animation *)> func);

private:
    std::vector<std::function<void (Animation *)>> m_funlist;

    Animation *m_animation;

    void invoke(Animation *animation);

    void upload(std::string path);

protected:
    void doInBackground(std::string path);
};



