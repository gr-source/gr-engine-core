#pragma once

#include <gr-math/gmath.hpp>

class Display {
public:
    static void SetDisplayRect(const Rect &rect);

    static Rect GetDisplayRect();

private:
    static Rect displayRect;
};


