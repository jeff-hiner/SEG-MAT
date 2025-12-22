# SEG-MAT WASM Build Makefile
# Usage:
#   make          - Build with -O2
#   make OPT=-O0  - Build without optimizations (for debugging)
#   make -j4      - Parallel build
#   make clean    - Remove build artifacts

# Use bash shell for consistent behavior
SHELL := /usr/bin/bash

EMSDK ?= /c/Users/prelu/git/emsdk
EMCC = $(EMSDK)/upstream/emscripten/em++.bat

# Compiler settings
CXX = $(EMCC)
# EIGEN_NO_IO: Disable iostream includes in Eigen (reduces C++ static init overhead)
# GDIAM_QUIET: Disable debug output in gdiam.cpp (also removes iostream dependency)
# CGAL_NO_IOSTREAM: Disable iostream includes in CGAL (avoids C++ static init in WASM)
# CGAL_DISABLE_ROUNDING_MATH_CHECK: WASM doesn't support FPU rounding mode control
# -Werror: Treat warnings as errors to keep the build clean
CXXFLAGS = -std=c++17 -Wall -Wextra -Werror -DEIGEN_NO_IO -DGDIAM_QUIET -DCGAL_NO_IOSTREAM -DCGAL_DISABLE_ROUNDING_MATH_CHECK
OPT ?= -O2

# Include paths
INCLUDES = -Isrc -Isrc/emd -Isrc/graphcut -Isrc/ombb -Iinclude

# Linker flags for WASM
# STACK_SIZE: Default 64KB is too small - emd.cpp:russel() allocates ~82KB on stack
# STACK_OVERFLOW_CHECK=2: Runtime stack checks (requires calling __wasm_call_ctors + emscripten_stack_init)
LDFLAGS = -sWASM=1 \
          -sEXPORTED_FUNCTIONS="['_wasm_malloc','_wasm_free','_segmat_segment']" \
          -sEXPORTED_RUNTIME_METHODS="['ccall','cwrap']" \
          -sALLOW_MEMORY_GROWTH=0 \
          -sINITIAL_MEMORY=134217728 \
          -sSTACK_SIZE=2097152 \
          -sSTACK_OVERFLOW_CHECK=2

# Output
TARGET = segmat_buffer.js
TARGET_WASM = segmat_buffer.wasm

# Build directory
BUILDDIR = build

# Source files
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

# Object files in build directory
OBJS = $(patsubst %.cpp,$(BUILDDIR)/%.o,$(SRCS))

# Default target
all: $(TARGET)

# Link all objects into final output
$(TARGET): $(OBJS)
	@echo "=== Linking $(TARGET) ==="
	$(CXX) $(OPT) $(LDFLAGS) $^ -o $@
	@echo "=== Build complete ==="
	@ls -la $(TARGET) $(TARGET_WASM) 2>/dev/null || dir $(TARGET) $(TARGET_WASM) 2>NUL

# Compile each source file
$(BUILDDIR)/%.o: %.cpp | $(BUILDDIR)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(OPT) $(INCLUDES) -c $< -o $@

# Create build directory structure
$(BUILDDIR):
	@mkdir -p $(BUILDDIR)/src/emd $(BUILDDIR)/src/graphcut $(BUILDDIR)/src/ombb

# Clean build artifacts
clean:
	rm -rf $(BUILDDIR) $(TARGET) $(TARGET_WASM)

# Rebuild from scratch
rebuild: clean all

# Show what would be built
info:
	@echo "Sources: $(SRCS)"
	@echo "Objects: $(OBJS)"
	@echo "Target: $(TARGET)"
	@echo "CXX: $(CXX)"
	@echo "CXXFLAGS: $(CXXFLAGS) $(OPT)"

.PHONY: all clean rebuild info
