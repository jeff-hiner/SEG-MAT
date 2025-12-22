@echo off
setlocal

REM Build SEG-MAT WASM using Make
REM Usage:
REM   build_wasm.bat           - Build with -O2
REM   build_wasm.bat OPT=-O0   - Build without optimizations (for debugging)
REM   build_wasm.bat -j4       - Parallel build
REM   build_wasm.bat clean     - Remove build artifacts

cd /d "%~dp0"

make %*

endlocal
