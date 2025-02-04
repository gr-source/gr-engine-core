#include "gr-base/core/window.hpp"

#include <GLFW/glfw3.h>

gVector2<int> Window::windowsize = {0, 0};

void Window::SetWindowSize(const gVector2<int> &size) {
    windowsize = size;

    glfwSetWindowSize(glfwGetCurrentContext(), size.x, size.y);
}

gVector2<int> Window::GetWindowSize() {
    glfwGetWindowSize(glfwGetCurrentContext(), &windowsize.x, &windowsize.y);
    return windowsize;
}

void Window::SetWindowTitle(const char *title) {
    glfwSetWindowTitle(glfwGetCurrentContext(), title);
}
