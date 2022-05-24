/* date = May 22nd 2022 1:31 pm */

#ifndef WINDOW_H
#define WINDOW_H

#include "defines.h"
#include "base/str.h"
#include <GLFW/glfw3.h>

typedef struct W_Window {
    GLFWwindow* handle;
    u32 width;
    u32 height;
    string title;
} W_Window;

void Window_Init(W_Window* window);
b8   Window_IsOpen(W_Window* window);
void Window_PollEvents(W_Window* window);
void Window_SwapBuffers(W_Window* window);
void Window_Free(W_Window* window);

#endif //WINDOW_H
