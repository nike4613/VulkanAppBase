@echo off

set CALL="%VULKAN_SDK%\Bin\glslangValidator.exe"

%CALL% -V shader.frag
%CALL% -V shader.vert