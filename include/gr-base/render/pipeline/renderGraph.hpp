#pragma once

#include <memory>
#include <vector>

class IRenderPass;

class RenderGraph
{
public:
    template <typename T, typename... Args>
    void add(Args&&... args);

    void Execute();

private:
    std::vector<std::shared_ptr<IRenderPass>> passes;
};

template <typename T, typename... Args>
inline void RenderGraph::add(Args&&... args)
{
    passes.push_back(std::make_shared<T>(std::forward<Args>(args)...));
}


