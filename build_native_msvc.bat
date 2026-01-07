@echo off
setlocal

REM SEG-MAT Native Build with MSVC
REM Run this from any directory - it handles paths internally

set "VSDIR=C:\Program Files\Microsoft Visual Studio\2022\Community"
set "SRCDIR=%~dp0"

REM Initialize VS environment
call "%VSDIR%\VC\Auxiliary\Build\vcvarsall.bat" x64 >nul 2>&1
if errorlevel 1 (
    echo ERROR: Could not initialize Visual Studio environment
    exit /b 1
)

REM Change to source directory
cd /d "%SRCDIR%"

echo Building SEG-MAT native with MSVC...
echo Source: %SRCDIR%

cl /nologo /EHsc /O2 /std:c++17 /DGDIAM_QUIET ^
    /I src /I src\emd /I src\graphcut /I src\ombb /I include ^
    src\Decomposer.cpp ^
    src\MAT-geometry.cpp ^
    src\MAT-main.cpp ^
    src\emd\emd.cpp ^
    src\graphcut\GCoptimization.cpp ^
    src\graphcut\LinkedBlockList.cpp ^
    src\graphcut\graph.cpp ^
    src\graphcut\maxflow.cpp ^
    src\ombb\gdiam.cpp ^
    src\main.cpp ^
    /Fe:segmat_native.exe

if errorlevel 1 (
    echo BUILD FAILED
    exit /b 1
)

del *.obj >nul 2>&1
echo BUILD SUCCESS: segmat_native.exe
dir /b segmat_native.exe

endlocal
