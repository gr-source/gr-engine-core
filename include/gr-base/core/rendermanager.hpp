#pragma once

#include "gr-base/script/default.hpp"

#include <gr-render/gCommon.h>

class RenderManager
{
public:
    static void PreRender(const Camera *camera);

    static void Render();

    static void PosRender();

private:
    static RenderManager &GetInstance();

    void preRender(const Camera *camera);

    void render();

    void posRender();

    void setuplight(grr::gShader *shader);
};


