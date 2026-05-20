#pragma once

#include "gr-base/core/default.hpp"
#include <gr-render/shader.hpp>

class RenderSystem
{
public:
    static void Update(Scene* scene);

private:
    static void Setuplight(gr::Shader* shader);
};


