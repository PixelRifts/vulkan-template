/* date = April 5th 2022 6:25 pm */

#ifdef PLUGIN
#define GLFW_DLL
#endif
#include <GLFW/glfw3.h>
#include <defines.h>

#ifndef INPUT_H
#define INPUT_H

b32 I_Init(GLFWwindow* window);
void I_Reset();

b32 I_Key(i32 key);
b32 I_KeyPressed(i32 key);
b32 I_KeyReleased(i32 key);
b32 I_KeyHeld(i32 key);
b32 I_Button(i32 button);
b32 I_ButtonPressed(i32 button);
b32 I_ButtonReleased(i32 button);
f32 I_GetMouseX();
f32 I_GetMouseY();
f32 I_GetMouseScrollX();
f32 I_GetMouseScrollY();
f32 I_GetMouseAbsoluteScrollX();
f32 I_GetMouseAbsoluteScrollY();
f32 I_GetMouseDX();
f32 I_GetMouseDY();
f32 I_GetMouseRecordedX();
f32 I_GetMouseRecordedY();

#endif //INPUT_H
