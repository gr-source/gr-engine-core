#include "gr-base/render/pipeline/renderGraph.hpp"

#include "gr-base/render/pipeline/renderPass.hpp"

void RenderGraph::Execute()
{
    for (auto& pass : passes)
        pass->Execute();
}

