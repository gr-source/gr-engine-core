#include "gr-base/render/pipeline/scenePass.hpp"
#include "gr-base/core/renderer.hpp"

void ScenePass::Execute()
{
    Renderer::Render();
}
