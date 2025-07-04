#include "gr-base/core/window.hpp"

void (*Window::SetWindowSize)(const gVector2<int> &size);

gVector2<int> (*Window::GetWindowSize)();

void (*Window::SetWindowTitle)(const char *title);



