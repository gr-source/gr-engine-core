#pragma once

#include <string>

#include "gr-base/core/animation/default.hpp"

class AnimationSaveTask
{
public:
    AnimationSaveTask(std::string path, const Animation *animation);

private:

protected:
    void doInBackground(std::string path, const Animation *animation);
};

