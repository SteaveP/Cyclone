@ECHO OFF
echo Build
PUSHD "Build/"
premake5.exe --file=premake5.lua vs2019
POPD