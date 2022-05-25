@ECHO off
REM build script for engine
SetLocal EnableDelayedExpansion


REM Preprocess compile shaders
pushd res
ECHO     Building shaders...
glslc -fshader-stage=vertex basic.vert.glsl -o basic.vert.spv
glslc -fshader-stage=fragment basic.frag.glsl -o basic.frag.spv
ECHO Built Shaders "basic"
popd

REM Gets list of all C files
SET c_filenames= 
FOR /R %%f in (*.c) do (
	SET c_filenames=!c_filenames! %%f
)

SET assembly=vulkan
SET compiler_flags=-Wvarargs -Wall -Werror
SET wexcludes=-Wno-unused-function
SET include_flags=-Isource -I%VULKAN_SDK%/Include -Ithird_party/Include
SET linker_flags=-luser32 -lgdi32 -lshell32 -lmsvcrt -lvulkan-1 -lglfw3 -L%VULKAN_SDK%/Lib -Lthird_party/Libs -g
SET defines=-D_CRT_SECURE_NO_WARNINGS -DDEBUG=1

ECHO     Building %assembly%...
clang %c_filenames% %compiler_flags% %wexcludes% -o ./bin/%assembly%.exe %defines% %include_flags% %linker_flags%
