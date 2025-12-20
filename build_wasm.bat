@echo off
setlocal

set "EMSDK=C:\Users\prelu\git\emsdk"
set "PATH=%EMSDK%\upstream\emscripten;%PATH%"

echo.
echo === Testing SEG-MAT WASM compilation ===
echo.

cd /d "%~dp0"

em++ -O2 -std=c++14 -s WASM=1 ^
    -I src ^
    -I src/emd ^
    -I src/graphcut ^
    -I src/ombb ^
    -I include ^
    src/Decomposer.cpp ^
    src/MAT-geometry.cpp ^
    src/MAT-main.cpp ^
    src/main.cpp ^
    src/emd/emd.cpp ^
    src/graphcut/GCoptimization.cpp ^
    src/graphcut/LinkedBlockList.cpp ^
    src/graphcut/graph.cpp ^
    src/graphcut/maxflow.cpp ^
    src/ombb/gdiam.cpp ^
    -o segmat.js

if %ERRORLEVEL% EQU 0 (
    echo.
    echo === WASM compilation successful! ===
    echo Output: segmat.js and segmat.wasm
) else (
    echo.
    echo === WASM compilation FAILED ===
)

endlocal
