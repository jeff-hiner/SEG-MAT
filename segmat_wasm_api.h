// SEG-MAT WASM Buffer API
// C-compatible interface for WebAssembly module
//
// Design: Caller allocates all buffers. Each array pointer is paired with
// its element count. Structs group related data logically.

#ifndef SEGMAT_WASM_API_H
#define SEGMAT_WASM_API_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============ Error codes ============

#define SEGMAT_SUCCESS            0
#define SEGMAT_ERR_NULL_PTR      -1
#define SEGMAT_ERR_INVALID_SIZE  -2
#define SEGMAT_ERR_TOPOLOGY      -3
#define SEGMAT_ERR_SEGMENT       -4
#define SEGMAT_ERR_BUFFER_TOO_SMALL -5

// ============ Slice types (ptr + len always paired) ============

typedef struct {
    const float* ptr;
    size_t len;  // element count (e.g., vertex count), not byte count
} FloatSlice;

typedef struct {
    const int32_t* ptr;
    size_t len;
} Int32Slice;

typedef struct {
    int32_t* ptr;
    size_t len;  // capacity on input, actual count on output
} Int32SliceMut;

// ============ Domain structures ============

// Triangle mesh (immutable input)
// Prefixed to avoid conflicts with internal C++ types
typedef struct {
    FloatSlice vertices;  // len = vertex_count, data is [x,y,z, x,y,z, ...] (stride 3)
    Int32Slice faces;     // len = face_count, data is [v0,v1,v2, ...] (stride 3)
} WasmMesh;

// Medial Axis Transform representation (immutable input)
// Prefixed to avoid conflicts with internal C++ types
typedef struct {
    FloatSlice centers;  // len = vertex_count, data is [x,y,z, x,y,z, ...] (stride 3)
    FloatSlice radii;    // len = vertex_count
    Int32Slice edges;    // len = edge_count, data is [v0,v1, v0,v1, ...] (stride 2)
    Int32Slice faces;    // len = face_count, data is [v0,v1,v2, ...] (stride 3)
} WasmMAT;

// ============ Parameters ============

typedef struct {
    float growing_threshold;  // Region growing threshold (default 0.015)
    float min_region;         // Minimum region size as fraction (default 0.002)
} SEGMATParams;

// ============ API ============

// Segment a mesh using MAT-based approach.
//
// Parameters:
//   mesh       - Input triangle mesh
//   base_mat   - Detailed base MAT
//   struct_mat - Coarse structure MAT (can have zero-length slices if unused)
//   params     - Segmentation parameters
//   labels     - Output buffer for face labels. Must have capacity >= mesh->faces.len
//                On success, labels->len is set to mesh->faces.len
//   unique_label_count - Output: number of unique segment labels
//
// Returns:
//   SEGMAT_SUCCESS on success
//   SEGMAT_ERR_NULL_PTR if any required pointer is NULL
//   SEGMAT_ERR_INVALID_SIZE if input sizes are inconsistent
//   SEGMAT_ERR_BUFFER_TOO_SMALL if labels buffer is too small
//   SEGMAT_ERR_TOPOLOGY on topology errors
//   SEGMAT_ERR_SEGMENT on segmentation failure
//
// Note: Output labels array size = mesh face count (exactly known)
//       Caller should allocate labels with capacity >= mesh->faces.len
int32_t segmat_segment(
    const WasmMesh* mesh,
    const WasmMAT* base_mat,
    const WasmMAT* struct_mat,
    const SEGMATParams* params,
    Int32SliceMut* labels,
    int32_t* unique_label_count
);

#ifdef __cplusplus
}
#endif

#endif // SEGMAT_WASM_API_H
