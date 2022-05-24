#include "window.h"

void Window_Init(W_Window* window) {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window->handle = glfwCreateWindow(1080, 720, "Proc", nullptr, nullptr);
}

b8 Window_IsOpen(W_Window* window) {
    return !glfwWindowShouldClose(window->handle);
}

void Window_PollEvents(W_Window* window) {
    glfwPollEvents();
}

void Window_SwapBuffers(W_Window* window) {
    glfwSwapBuffers(window->handle);
}

void Window_Free(W_Window* window) {
    glfwDestroyWindow(window->handle);
    glfwTerminate();
}
