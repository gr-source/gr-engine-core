#pragma once

#include <gr-math/gmath.hpp>

class Window
{
public:
    static void (*SetVSync)(bool value);

    static bool (*GetEnableVSync)();

    static void (*SetWindowSize)(const Vector2 &size);

    static Vector2 (*GetWindowSize)();

    static void (*SetWindowTitle)(const char *title);
};

