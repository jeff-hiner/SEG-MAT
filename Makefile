# SEG-MAT Build Makefile
# Usage:
#   make          - Build WASM with -O2
#   make native   - Build native CLI executable
#   make OPT=-O0  - Build without optimizations (for debugging)
#   make -j4      - Parallel build
#   make clean    - Remove build artifacts

# Use bash shell for consistent behavior
SHELL := /usr/bin/bash

EMSDK ?= /c/Users/prelu/git/emsdk
EMCC = $(EMSDK)/upstream/emscripten/em++.bat

# Native compiler (auto-detect: prefer g++ on Windows/MSYS2)
NATIVE_CXX ?= g++

# WASM Compiler settings
CXX = $(EMCC)
# EIGEN_NO_IO: Disable iostream includes in Eigen (reduces C++ static init overhead)
# GDIAM_QUIET: Disable debug output in gdiam.cpp (also removes iostream dependency)
# CGAL_NO_IOSTREAM: Disable iostream includes in CGAL (avoids C++ static init in WASM)
# CGAL_DISABLE_ROUNDING_MATH_CHECK: WASM doesn't support FPU rounding mode control
# -Werror: Treat warnings as errors to keep the build clean
CXXFLAGS_WASM = -std=c++17 -Wall -Wextra -Werror -msimd128 -DEIGEN_NO_IO -DGDIAM_QUIET -DCGAL_NO_IOSTREAM -DCGAL_DISABLE_ROUNDING_MATH_CHECK
# Native: relax warnings (third-party code has GCC 13 warnings)
CXXFLAGS_NATIVE = -std=c++17 -Wall -mavx2 -DGDIAM_QUIET -Wno-maybe-uninitialized -Wno-unused-variable
OPT ?= -O2

# Include paths
INCLUDES = -Isrc -Isrc/emd -Isrc/graphcut -Isrc/ombb -Iinclude

# Linker flags for WASM
# STANDALONE_WASM: Emit standard WASM without Emscripten JS runtime dependencies
# --no-entry: Library mode (no main function required)
# STACK_SIZE: Default 64KB is too small - emd.cpp:russel() allocates ~82KB on stack
# STACK_OVERFLOW_CHECK=2: Runtime stack checks
LDFLAGS = -sWASM=1 \
          -sSTANDALONE_WASM \
          --no-entry \
          -sEXPORTED_FUNCTIONS="['_wasm_malloc','_wasm_free','_segmat_segment']" \
          -sALLOW_MEMORY_GROWTH=1 \
          -sINITIAL_MEMORY=134217728 \
          -sSTACK_SIZE=2097152 \
          -sSTACK_OVERFLOW_CHECK=2

# Output
TARGET = segmat_buffer.js
TARGET_WASM = segmat_buffer.wasm
TARGET_NATIVE = segmat_native

# Build directory
BUILDDIR = build
BUILDDIR_NATIVE = build_native

# Source files (WASM)
SRCS = src/Decomposer.cpp \
       src/MAT-geometry.cpp \
       src/MAT-main.cpp \
       src/emd/emd.cpp \
       src/graphcut/GCoptimization.cpp \
       src/graphcut/LinkedBlockList.cpp \
       src/graphcut/graph.cpp \
       src/graphcut/maxflow.cpp \
       src/ombb/gdiam.cpp \
       segmat_wasm_api.cpp

# Source files (Native CLI)
SRCS_NATIVE = src/Decomposer.cpp \
              src/MAT-geometry.cpp \
              src/MAT-main.cpp \
              src/emd/emd.cpp \
              src/graphcut/GCoptimization.cpp \
              src/graphcut/LinkedBlockList.cpp \
              src/graphcut/graph.cpp \
              src/graphcut/maxflow.cpp \
              src/ombb/gdiam.cpp \
              src/main.cpp

# Object files in build directory
OBJS = $(patsubst %.cpp,$(BUILDDIR)/%.o,$(SRCS))
OBJS_NATIVE = $(patsubst %.cpp,$(BUILDDIR_NATIVE)/%.o,$(SRCS_NATIVE))

# Default target (WASM)
all: $(TARGET)

# Link all objects into final output
$(TARGET): $(OBJS)
	@echo "=== Linking $(TARGET) ==="
	$(CXX) $(OPT) $(LDFLAGS) $^ -o $@
	@echo "=== Build complete ==="
	@ls -la $(TARGET) $(TARGET_WASM) 2>/dev/null || dir $(TARGET) $(TARGET_WASM) 2>NUL

# Compile each source file (WASM)
$(BUILDDIR)/%.o: %.cpp | $(BUILDDIR)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS_WASM) $(OPT) $(INCLUDES) -c $< -o $@

# Create build directory structure
$(BUILDDIR):
	@mkdir -p $(BUILDDIR)/src/emd $(BUILDDIR)/src/graphcut $(BUILDDIR)/src/ombb

# Clean build artifacts
clean:
	rm -rf $(BUILDDIR) $(BUILDDIR_NATIVE) $(TARGET) $(TARGET_WASM) $(TARGET_NATIVE)

# Rebuild from scratch
rebuild: clean all

# Show what would be built
info:
	@echo "Sources: $(SRCS)"
	@echo "Objects: $(OBJS)"
	@echo "Target: $(TARGET)"
	@echo "CXX: $(CXX)"
	@echo "CXXFLAGS_WASM: $(CXXFLAGS_WASM) $(OPT)"

#==============================================================================
# Native build targets
#==============================================================================

# Build native CLI executable
native: $(TARGET_NATIVE)

# Link native executable
$(TARGET_NATIVE): $(OBJS_NATIVE)
	@echo "=== Linking $(TARGET_NATIVE) ==="
	$(NATIVE_CXX) $(CXXFLAGS_NATIVE) $(OPT) $^ -o $@
	@echo "=== Native build complete ==="
	@ls -la $(TARGET_NATIVE) 2>/dev/null || dir $(TARGET_NATIVE) 2>NUL

# Compile native objects
$(BUILDDIR_NATIVE)/%.o: %.cpp | $(BUILDDIR_NATIVE)
	@mkdir -p $(dir $@)
	$(NATIVE_CXX) $(CXXFLAGS_NATIVE) $(OPT) $(INCLUDES) -c $< -o $@

# Create native build directory
$(BUILDDIR_NATIVE):
	@mkdir -p $(BUILDDIR_NATIVE)/src/emd $(BUILDDIR_NATIVE)/src/graphcut $(BUILDDIR_NATIVE)/src/ombb

# Rebuild native
rebuild-native: clean native

.PHONY: all clean rebuild info native rebuild-native
