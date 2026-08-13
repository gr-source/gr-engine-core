#include "gr-base/render/postProcessPipeline.hpp"
#include "gr-base/core/console.hpp"
#include <exception>

void PostProcessPipeline::submit(std::shared_ptr<IPostProcessPass> pass)
{
    try
    {
        pass->Initialize();

        passes.push_back(pass);
    } catch (const std::exception& e)
    {
        Console::LogDefault("{FF0000}[ PostProcessPipeline ] {FFFFFF}%s", e.what());
    }
}

