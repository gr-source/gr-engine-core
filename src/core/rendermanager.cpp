#include "gr-base/core/rendermanager.hpp"

#include "gr-base/core/culling/frustum.hpp"

#include "gr-base/core/assetsManager.hpp"

#include "gr-base/core/geometry/batchingmanager.hpp"
#include "gr-base/script/camera.hpp"
#include "gr-base/script/light.hpp"

// public
void RenderManager::PreRender(const Camera *camera)
{
    GetInstance().preRender(camera);
}

void RenderManager::Render()
{
    GetInstance().render();
}

void RenderManager::PosRender()
{
    GetInstance().posRender();
}

// private
RenderManager &RenderManager::GetInstance()
{
    static RenderManager instance;
    return instance;
}

void RenderManager::preRender(const Camera *camera)
{
    auto projection = camera->getPerspesctive();
    auto view = camera->getViewmatrix();
    auto position = camera->getPosition();

    auto list = AssetsManager::GetShaderList(ShaderType::SHADER3D);
    for (auto &shader : list)
    {
        if (!shader || !shader->isValid())
        {
            continue;
        }

        shader->setUniform("u_projection", &projection);
        shader->setUniform("u_view",       &view);
        shader->setUniform("u_eye",        &position);

        setuplight(shader);
    }

    Frustum::CalculateFrustum(projection * view);
}

void RenderManager::render()
{
    BatchingManager::Render();
}

void RenderManager::posRender()
{}

void RenderManager::setuplight(grr::gShader *shader)
{
    auto lights = Light::GetListLight();

    int numLight = static_cast<int>(lights.size());

    if (!shader->setUniform("numLights", &numLight))
        return;

    if (lights.empty())
        return;

    for (std::size_t i={};i<lights.size();i++)
    {
        auto &light = lights[i];

        auto position   = light->getPosition();
        auto forward    = light->getForward();
        auto color      = light->getColor();
        auto type       = light->getType();

        // auto attenuation = Vector3{light->getAttenuationConstant(), light->getAttenuationLinear(), light->getAttenuationQuadractc()};
        // auto params      = Vector3{light->getInnecone(), light->getOutercone(), (float)light->getType()};
        // auto projection  = light->getProjetion();
        
        shader->setUniform(("lights[" + std::to_string(i) + "].position").c_str(),    &position);
        shader->setUniform(("lights[" + std::to_string(i) + "].direction").c_str(),   &forward);
        shader->setUniform(("lights[" + std::to_string(i) + "].color").c_str(),       &color);
        // shader->setUniform(("lights[" + std::to_string(i) + "].attenuation").c_str(), &attenuation);
        // shader->setUniform(("lights[" + std::to_string(i) + "].params").c_str(),      &params);
        shader->setUniform(("lights[" + std::to_string(i) + "].type").c_str(),       &type);
    }
}


