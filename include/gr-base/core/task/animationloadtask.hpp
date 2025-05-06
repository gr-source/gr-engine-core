#pragma once

#include <functional>
#include <string>
#include <vector>

#include "gr-base/core/animation/default.hpp"

class AnimationLoadTask
{
public:
    AnimationLoadTask(const std::string &path, std::function<void (Animation *)> func = nullptr);

    void addFunction(std::function<void (Animation *)> func);

private:
    std::vector<std::function<void (Animation *)>> m_funlist;

    void invoke(Animation *animation);

    void upload(Animation *animation, std::string path);

protected:
    void doInBackground(std::string path);
};



