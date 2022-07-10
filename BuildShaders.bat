@ECHO OFF
echo Building Shaders...
PUSHD "Sources/Shaders"

set GLSLC=%VULKAN_SDK%\Bin\glslc.exe
set DXC=%VULKAN_SDK%\Bin\dxc.exe

set OutDir="../../Intermediate/Shaders/"

if not exist %OutDir% mkdir %OutDir%

GLSLC -g -O0 -fshader-stage=vert TempShader.cshadervert -o "%OutDir%/Vert.spv"
GLSLC -g -O0 -fshader-stage=frag TempShader.cshaderfrag -o "%OutDir%/Frag.spv"

GLSLC -g -O0 -fshader-stage=vert Fullscreen.cshadervert -o "%OutDir%/FullscreenVert.spv"

GLSLC -g -O0 -fshader-stage=frag Checkerboard.cshaderfrag -o "%OutDir%/TexFillFrag.spv"
GLSLC -g -O0 -fshader-stage=frag Procedural.cshaderfrag -o "%OutDir%/TexFill2Frag.spv"

rem DXC -g -spirv -E=main -fspv-target-env=vulkan1.3 -fspv-reduce-load-size -fvk-use-dx-position-w  TempShader.cshadervert -Fo ..\..\Intermediate\Shaders\Vert.spv
rem GLSLC -S TempShader.cshadervert -o vert.asm
rem GLSLC -S TempShader.cshaderfrag -o frag.asm

echo ...Done
pause

POPD