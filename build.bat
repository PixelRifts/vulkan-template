@ECHO off
REM build script for engine
SetLocal EnableDelayedExpansion

REM Get's list of all C files
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

ECHO "Building %assembly%%..."
clang %c_filenames% %compiler_flags% %wexcludes% -o ./bin/%assembly%.exe %defines% %include_flags% %linker_flags%
