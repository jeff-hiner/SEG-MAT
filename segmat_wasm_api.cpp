// SEG-MAT WASM Buffer API Implementation
#include "segmat_wasm_api.h"
#include "src/Decomposer.h"
#include <cstdlib>
#include <cstring>
#include <new>
#include <set>

// Memory management exports
extern "C" {

void* wasm_malloc(size_t size) {
    return malloc(size);
}

void wasm_free(void* ptr) {
    free(ptr);
}

} // extern "C"

// Create error result
static void* create_error_result(int32_t error_code) {
    void* result = wasm_malloc(12);  // Header only: error, face_count, label_count
    if (result) {
        int32_t* header = static_cast<int32_t*>(result);
        header[0] = error_code;
        header[1] = 0;  // face_count
        header[2] = 0;  // label_count
    }
    return result;
}

extern "C" {

void* segmat_segment_buffer(
    // Surface mesh
    int32_t mesh_vertex_count,
    int32_t mesh_face_count,
    const float* mesh_vertices,
    const int32_t* mesh_faces,

    // Base MAT
    int32_t base_mat_vertex_count,
    int32_t base_mat_edge_count,
    int32_t base_mat_face_count,
    const float* base_mat_vertices,
    const float* base_mat_radii,
    const int32_t* base_mat_edges,
    const int32_t* base_mat_faces,

    // Structure MAT
    int32_t struct_mat_vertex_count,
    int32_t struct_mat_edge_count,
    int32_t struct_mat_face_count,
    const float* struct_mat_vertices,
    const float* struct_mat_radii,
    const int32_t* struct_mat_edges,
    const int32_t* struct_mat_faces,

    // Parameters
    float growing_threshold,
    float min_region)
{
    // Validate mesh inputs
    if (mesh_vertex_count <= 0 || mesh_face_count <= 0 ||
        !mesh_vertices || !mesh_faces) {
        return create_error_result(SEGMAT_ERR_INVALID);
    }

    // Validate base MAT inputs
    if (base_mat_vertex_count <= 0 || !base_mat_vertices || !base_mat_radii) {
        return create_error_result(SEGMAT_ERR_INVALID);
    }

    // Validate struct MAT inputs (can be empty/null for no structural decomposition)
    // We'll allow empty struct MAT

    void* result = nullptr;

    try {
        // Create mesh from buffers
        Mesh mesh = Decomposer::createMeshFromBuffers(
            mesh_vertices, mesh_vertex_count,
            mesh_faces, mesh_face_count);

        if (mesh.number_of_faces() == 0) {
            return create_error_result(SEGMAT_ERR_TOPOLOGY);
        }

        // Create structure MAT
        MAT smat;
        if (struct_mat_vertex_count > 0 && struct_mat_vertices && struct_mat_radii) {
            smat = MAT(
                struct_mat_vertices,
                struct_mat_radii,
                struct_mat_vertex_count,
                struct_mat_edges,
                struct_mat_edge_count,
                struct_mat_faces,
                struct_mat_face_count,
                mesh);
        }

        // Create base MAT
        MAT mat(
            base_mat_vertices,
            base_mat_radii,
            base_mat_vertex_count,
            base_mat_edges,
            base_mat_edge_count,
            base_mat_faces,
            base_mat_face_count,
            mesh);

        if (mat.points.size() == 0) {
            return create_error_result(SEGMAT_ERR_INVALID);
        }

        // Perform segmentation
        Decomposer solver;
        solver.decompose3Dshape(mat, smat, mesh, growing_threshold, min_region);
        solver.transfer_MAT_mesh(mat, mesh, 0.3f);

        // Get results
        const vector<int>& labels = solver.final_facelabel;
        int32_t face_count = static_cast<int32_t>(labels.size());

        // Count distinct labels
        std::set<int> unique_labels(labels.begin(), labels.end());
        int32_t label_count = static_cast<int32_t>(unique_labels.size());

        // Allocate result buffer
        size_t header_size = 3 * sizeof(int32_t);
        size_t labels_size = face_count * sizeof(int32_t);
        size_t total_size = header_size + labels_size;

        result = wasm_malloc(total_size);
        if (!result) {
            return create_error_result(SEGMAT_ERR_ALLOC);
        }

        // Write header
        int32_t* header = static_cast<int32_t*>(result);
        header[0] = SEGMAT_SUCCESS;
        header[1] = face_count;
        header[2] = label_count;

        // Write labels
        int32_t* out_labels = header + 3;
        for (int32_t i = 0; i < face_count; i++) {
            out_labels[i] = static_cast<int32_t>(labels[i]);
        }

        return result;

    } catch (const std::exception& e) {
        if (result) wasm_free(result);
        return create_error_result(SEGMAT_ERR_SEGMENT);
    } catch (...) {
        if (result) wasm_free(result);
        return create_error_result(SEGMAT_ERR_SEGMENT);
    }
}

// Accessor functions for result buffer
int32_t segmat_result_error_code(void* result) {
    if (!result) return SEGMAT_ERR_INVALID;
    return static_cast<int32_t*>(result)[0];
}

int32_t segmat_result_face_count(void* result) {
    if (!result) return 0;
    return static_cast<int32_t*>(result)[1];
}

int32_t segmat_result_label_count(void* result) {
    if (!result) return 0;
    return static_cast<int32_t*>(result)[2];
}

const int32_t* segmat_result_labels(void* result) {
    if (!result) return nullptr;
    int32_t* header = static_cast<int32_t*>(result);
    if (header[0] != SEGMAT_SUCCESS) return nullptr;
    return header + 3;
}

} // extern "C"
