// SEG-MAT WASM Buffer API
// C-compatible interface for WebAssembly module
#ifndef SEGMAT_WASM_API_H
#define SEGMAT_WASM_API_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Error codes
#define SEGMAT_SUCCESS          0
#define SEGMAT_ERR_ALLOC       -1
#define SEGMAT_ERR_INVALID     -2
#define SEGMAT_ERR_TOPOLOGY    -3
#define SEGMAT_ERR_SEGMENT     -4

// Result header structure (followed by variable-length data)
// Memory layout:
//   [0-3]   error_code (int32)
//   [4-7]   face_count (int32)
//   [8-11]  label_count (int32)
//   [12...] face_labels (int32[face_count])

// Memory management - must be exported for wasmer-python
void* wasm_malloc(size_t size);
void wasm_free(void* ptr);

// Main segmentation function
// Returns pointer to result buffer (caller must wasm_free)
// Returns NULL on critical failure
void* segmat_segment_buffer(
    // Surface mesh
    int32_t mesh_vertex_count,
    int32_t mesh_face_count,
    const float* mesh_vertices,     // [mesh_vertex_count * 3]
    const int32_t* mesh_faces,      // [mesh_face_count * 3]

    // Base MAT (detailed)
    int32_t base_mat_vertex_count,
    int32_t base_mat_edge_count,
    int32_t base_mat_face_count,
    const float* base_mat_vertices, // [base_mat_vertex_count * 3]
    const float* base_mat_radii,    // [base_mat_vertex_count]
    const int32_t* base_mat_edges,  // [base_mat_edge_count * 2]
    const int32_t* base_mat_faces,  // [base_mat_face_count * 3]

    // Structure MAT (coarse)
    int32_t struct_mat_vertex_count,
    int32_t struct_mat_edge_count,
    int32_t struct_mat_face_count,
    const float* struct_mat_vertices,
    const float* struct_mat_radii,
    const int32_t* struct_mat_edges,
    const int32_t* struct_mat_faces,

    // Parameters
    float growing_threshold,        // default 0.015
    float min_region                // default 0.002
);

// Get values from result buffer (for convenience)
int32_t segmat_result_error_code(void* result);
int32_t segmat_result_face_count(void* result);
int32_t segmat_result_label_count(void* result);
const int32_t* segmat_result_labels(void* result);

#ifdef __cplusplus
}
#endif

#endif // SEGMAT_WASM_API_H
