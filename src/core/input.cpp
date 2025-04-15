#include "gr-base/core/input.hpp"

bool (*Input::IsKeyPress)(int key);

void (*Input::SetCursorPos)(const Vector2 &pos);

Vector2 (*Input::GetCursorPos)();

bool (*Input::IsMousePress)(int button);



