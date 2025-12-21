@echo off
setlocal

set "EMSDK=C:\Users\prelu\git\emsdk"
set "PATH=%EMSDK%\upstream\emscripten;%PATH%"

echo.
echo === Building SEG-MAT WASM Buffer API ===
echo.

cd /d "%~dp0"

em++ -O2 -std=c++14 -s WASM=1 ^
    -s EXPORTED_FUNCTIONS="['_wasm_malloc','_wasm_free','_segmat_segment']" ^
    -s EXPORTED_RUNTIME_METHODS="['ccall','cwrap']" ^
    -s ALLOW_MEMORY_GROWTH=0 ^
    -s INITIAL_MEMORY=134217728 ^
    -I src ^
    -I src/emd ^
    -I src/graphcut ^
    -I src/ombb ^
    -I include ^
    src/Decomposer.cpp ^
    src/MAT-geometry.cpp ^
    src/MAT-main.cpp ^
    src/emd/emd.cpp ^
    src/graphcut/GCoptimization.cpp ^
    src/graphcut/LinkedBlockList.cpp ^
    src/graphcut/graph.cpp ^
    src/graphcut/maxflow.cpp ^
    src/ombb/gdiam.cpp ^
    segmat_wasm_api.cpp ^
    -o segmat_buffer.js

if %ERRORLEVEL% EQU 0 (
    echo.
    echo === WASM compilation successful! ===
    echo Output: segmat_buffer.js and segmat_buffer.wasm
    dir segmat_buffer.*
) else (
    echo.
    echo === WASM compilation FAILED ===
)

endlocal
