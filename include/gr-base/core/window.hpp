#pragma once

#include <gr-math/gmath.hpp>

class Window {
public:
    static void SetWindowSize(const gVector2<int> &size);

    static gVector2<int> GetWindowSize();

    static void SetWindowTitle(const char *title);

private:
    static gVector2<int> windowsize;
};

