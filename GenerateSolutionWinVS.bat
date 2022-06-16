@ECHO OFF
echo Creating Cyclone Projects..
PUSHD "Sources/Build/"
premake5.exe --file=premake5.lua vs2019
POPD