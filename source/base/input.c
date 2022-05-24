#include "input.h"
#include <string.h>
#include <stdio.h>

typedef struct I_InputState {
    GLFWwindow* window;
    u8 key_states[350];
    u8 button_states[8];
    
    f32 mouse_x;
    f32 mouse_y;
    f32 mouse_scrollx;
    f32 mouse_scrolly;
    f32 mouse_absscrollx;
    f32 mouse_absscrolly;
    f32 mouse_recordedx;
    f32 mouse_recordedy;
} I_InputState;
static I_InputState _state;

static void I_KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key < 0 || key >= 350) return;
    
    switch (action) {
        case GLFW_PRESS: {
            _state.key_states[key] |= 0b00000001;
        } break;
        
        case GLFW_RELEASE: {
            _state.key_states[key] |= 0b00000010;
        } break;
        
        case GLFW_REPEAT: {
            _state.key_states[key] |= 0b00000100;
        } break;
    }
}

static void I_ButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button < 0 || button >= 8) return;
    switch (action) {
        case GLFW_PRESS: {
            _state.button_states[button] |= 0b00000001;
            _state.mouse_recordedx = _state.mouse_x;
            _state.mouse_recordedy = _state.mouse_y;
        } break;
        case GLFW_RELEASE: {
            _state.button_states[button] |= 0b00000010;
            _state.mouse_recordedx = _state.mouse_x;
            _state.mouse_recordedy = _state.mouse_y;
        } break;
    }
}

void I_CursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    _state.mouse_x = (f32)xpos;
    _state.mouse_y = (f32)ypos;
}

void I_ScrollCallback(GLFWwindow* window, double xscroll, double yscroll) {
    _state.mouse_scrollx = (f32)xscroll;
    _state.mouse_scrolly = (f32)yscroll;
    _state.mouse_absscrollx += (f32)xscroll;
    _state.mouse_absscrolly += (f32)yscroll;
}

b32 I_Init(GLFWwindow* window) {
    _state.window = window;
    glfwSetKeyCallback(_state.window, I_KeyCallback);
    glfwSetMouseButtonCallback(_state.window, I_ButtonCallback);
    glfwSetCursorPosCallback(_state.window, I_CursorPosCallback);
    glfwSetScrollCallback(_state.window, I_ScrollCallback);
    return true;
}

void I_Reset() {
    memset(_state.key_states, 0, 350 * sizeof(u8));
    memset(_state.button_states, 0, 8 * sizeof(u8));
    _state.mouse_scrollx = 0;
    _state.mouse_scrolly = 0;
}

b32 I_Key(i32 key) { return glfwGetKey(_state.window, key) != GLFW_RELEASE; }
b32 I_KeyPressed(i32 key) { return (_state.key_states[key] & 0b00000001) != 0; }
b32 I_KeyReleased(i32 key) { return (_state.key_states[key] & 0b00000010) != 0; }
b32 I_KeyHeld(i32 key) { return (_state.key_states[key] & 0b00000100) != 0; }
b32 I_Button(i32 button) { return glfwGetMouseButton(_state.window, button) != GLFW_RELEASE; }
b32 I_ButtonPressed(i32 button) { return (_state.button_states[button] & 0b00000001) != 0; }
b32 I_ButtonReleased(i32 button) { return (_state.button_states[button] & 0b00000010) != 0; }
f32 I_GetMouseX() { return _state.mouse_x; }
f32 I_GetMouseY() { return _state.mouse_y; }
f32 I_GetMouseScrollX() { return _state.mouse_scrollx; }
f32 I_GetMouseScrollY() { return _state.mouse_scrolly; }
f32 I_GetMouseAbsoluteScrollX() { return _state.mouse_absscrollx; }
f32 I_GetMouseAbsoluteScrollY() { return _state.mouse_absscrolly; }
f32 I_GetMouseDX() { return _state.mouse_x - _state.mouse_recordedx; }
f32 I_GetMouseDY() { return _state.mouse_y - _state.mouse_recordedy; }
f32 I_GetMouseRecordedX() { return _state.mouse_recordedx; }
f32 I_GetMouseRecordedY() { return _state.mouse_recordedy; }
