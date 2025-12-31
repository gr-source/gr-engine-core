#pragma once

#include "gr-base/script/default.hpp"

#include <gr-render/gCommon.h>

class RenderManager
{
public:
    static void Initialize();

    static void Shutdown();

    static void PreRender(const Camera *camera);

    static void Render();

    static void PosRender();

private:
    static RenderManager* s_RenderManager;

    void preRender(const Camera *camera);

    void render();

    void posRender();

    void setuplight(grr::gShader *shader);
};


