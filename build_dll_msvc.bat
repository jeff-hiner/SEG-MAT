@echo off
setlocal

REM SEG-MAT DLL Build with MSVC
REM Builds a shared library with the same API as the WASM module

set "VSDIR=C:\Program Files\Microsoft Visual Studio\2022\Community"
set "SRCDIR=%~dp0"

call "%VSDIR%\VC\Auxiliary\Build\vcvarsall.bat" x64 >nul 2>&1
if errorlevel 1 (
    echo ERROR: Could not initialize Visual Studio environment
    exit /b 1
)

cd /d "%SRCDIR%"

echo Building SEG-MAT DLL with MSVC...

cl /nologo /EHsc /O2 /arch:AVX2 /std:c++17 /DGDIAM_QUIET /LD ^
    /I src /I src/emd /I src/graphcut /I src/ombb /I include ^
    src\Decomposer.cpp ^
    src\MAT-geometry.cpp ^
    src\MAT-main.cpp ^
    src\emd\emd.cpp ^
    src\graphcut\GCoptimization.cpp ^
    src\graphcut\LinkedBlockList.cpp ^
    src\graphcut\graph.cpp ^
    src\graphcut\maxflow.cpp ^
    src\ombb\gdiam.cpp ^
    segmat_wasm_api.cpp ^
    /Fe:segmat.dll ^
    /link /EXPORT:wasm_malloc /EXPORT:wasm_free /EXPORT:segmat_segment

if errorlevel 1 (
    echo BUILD FAILED
    exit /b 1
)

del *.obj *.exp >nul 2>&1
echo BUILD SUCCESS: segmat.dll
dir /b segmat.dll segmat.lib

endlocal
