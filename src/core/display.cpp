#include "core/display.hpp"

Rect Display::displayRect;

void Display::SetDisplayRect(const Rect &rect) {
    displayRect = rect;
}

Rect Display::GetDisplayRect() {
    return displayRect;
}