#pragma once

#include "gr-base/render/postProcessPass.hpp"

#include <vector>
#include <memory>

class PostProcessPipeline
{
public:
    template <typename T, typename... Args>
    void add(Args&&... args);

private:
    std::vector<std::shared_ptr<IPostProcessPass>> passes;

    void submit(std::shared_ptr<IPostProcessPass> pass);
};


template <typename T, typename... Args>
inline void PostProcessPipeline::add(Args&&... args)
{
    submit(std::make_shared<T>(std::forward<Args>(args)...));
}


