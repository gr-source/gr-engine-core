#include "gr-base/core/window.hpp"

void (*Window::SetWindowSize)(const Vector2 &size);

Vector2 (*Window::GetWindowSize)();

void (*Window::SetWindowTitle)(const char *title);



