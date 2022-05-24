#include <GLFW/glfw3.h>

#include "defines.h"
#include "window.h"
#include "base/input.h"
#include "context.h"

int main() {
    M_ScratchInit();
    
    W_Window window = {0};
    V_VulkanContext context = {0};
    
    Window_Init(&window);
    I_Init(window.handle);
    
    Vulkan_Init(&window, &context, DEBUG);
    while (Window_IsOpen(&window)) {
        I_Reset();
        Window_PollEvents(&window);
        //Window_SwapBuffers(&window);
    }
    Vulkan_Free(&context, DEBUG);
    
    Window_Free(&window);
    
    M_ScratchFree();
}
