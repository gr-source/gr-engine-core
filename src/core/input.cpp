#include "gr-base/core/input.hpp"

bool (*Input::IsKeyPress)(int key);

void (*Input::SetCursorPos)(const Vector2 &pos);

Vector2 (*Input::GetCursorPos)(void);

bool (*Input::IsMousePress)(int button);

Vector2 (*Input::GetMouseDelta)(void);

Vector2 (*Input::GetMouseScroll)(void);



