// SEG-MAT WASM Buffer API Implementation
// Caller-allocated buffer design - no WASM memory management needed

#include "segmat_wasm_api.h"
#include "src/Decomposer.h"
#include <cstdlib>
#include <cstring>
#include <new>
#include <set>

extern "C" {

// Memory management exports for Python wrapper to allocate WASM memory
void* wasm_malloc(size_t size) {
    return malloc(size);
}

void wasm_free(void* ptr) {
    free(ptr);
}

int32_t segmat_segment(
    const WasmMesh* mesh,
    const WasmMAT* base_mat,
    const WasmMAT* struct_mat,
    const SEGMATParams* params,
    Int32SliceMut* labels,
    int32_t* unique_label_count)
{
    // Validate null pointers
    if (!mesh || !base_mat || !params || !labels || !unique_label_count) {
        return SEGMAT_ERR_NULL_PTR;
    }

    // Validate mesh data
    size_t mesh_vertex_count = mesh->vertices.len;
    size_t mesh_face_count = mesh->faces.len;

    if (mesh_vertex_count == 0 || !mesh->vertices.ptr) {
        return SEGMAT_ERR_INVALID_SIZE;
    }
    if (mesh_face_count == 0 || !mesh->faces.ptr) {
        return SEGMAT_ERR_INVALID_SIZE;
    }

    // Validate base MAT data
    size_t base_vertex_count = base_mat->centers.len;
    size_t base_edge_count = base_mat->edges.len;
    size_t base_face_count = base_mat->faces.len;

    if (base_vertex_count == 0 || !base_mat->centers.ptr || !base_mat->radii.ptr) {
        return SEGMAT_ERR_INVALID_SIZE;
    }
    if (base_mat->radii.len != base_vertex_count) {
        return SEGMAT_ERR_INVALID_SIZE;
    }
    if (base_edge_count > 0 && !base_mat->edges.ptr) {
        return SEGMAT_ERR_INVALID_SIZE;
    }
    if (base_face_count > 0 && !base_mat->faces.ptr) {
        return SEGMAT_ERR_INVALID_SIZE;
    }

    // Validate output buffer capacity (must hold one label per mesh face)
    if (!labels->ptr || labels->len < mesh_face_count) {
        return SEGMAT_ERR_BUFFER_TOO_SMALL;
    }

    // Structure MAT can be empty (zero-length slices)
    size_t struct_vertex_count = struct_mat ? struct_mat->centers.len : 0;
    size_t struct_edge_count = struct_mat ? struct_mat->edges.len : 0;
    size_t struct_face_count = struct_mat ? struct_mat->faces.len : 0;

    // Validate struct MAT if present
    if (struct_vertex_count > 0) {
        if (!struct_mat->centers.ptr || !struct_mat->radii.ptr) {
            return SEGMAT_ERR_INVALID_SIZE;
        }
        if (struct_mat->radii.len != struct_vertex_count) {
            return SEGMAT_ERR_INVALID_SIZE;
        }
        if (struct_edge_count > 0 && !struct_mat->edges.ptr) {
            return SEGMAT_ERR_INVALID_SIZE;
        }
        if (struct_face_count > 0 && !struct_mat->faces.ptr) {
            return SEGMAT_ERR_INVALID_SIZE;
        }
    }

    try {
        // Create mesh from buffers
        Mesh cgal_mesh = Decomposer::createMeshFromBuffers(
            mesh->vertices.ptr,
            static_cast<int>(mesh_vertex_count),
            mesh->faces.ptr,
            static_cast<int>(mesh_face_count));

        if (cgal_mesh.number_of_faces() == 0) {
            return SEGMAT_ERR_TOPOLOGY;
        }

        // Create structure MAT (can be empty)
        MAT smat;
        if (struct_vertex_count > 0) {
            smat = MAT(
                struct_mat->centers.ptr,
                struct_mat->radii.ptr,
                static_cast<int>(struct_vertex_count),
                struct_mat->edges.ptr,
                static_cast<int>(struct_edge_count),
                struct_mat->faces.ptr,
                static_cast<int>(struct_face_count),
                cgal_mesh);
        }

        // Create base MAT
        MAT mat(
            base_mat->centers.ptr,
            base_mat->radii.ptr,
            static_cast<int>(base_vertex_count),
            base_mat->edges.ptr,
            static_cast<int>(base_edge_count),
            base_mat->faces.ptr,
            static_cast<int>(base_face_count),
            cgal_mesh);

        if (mat.points.size() == 0) {
            return SEGMAT_ERR_INVALID_SIZE;
        }

        // Perform segmentation
        Decomposer solver;
        solver.decompose3Dshape(mat, smat, cgal_mesh, params->growing_threshold, params->min_region);
        solver.transfer_MAT_mesh(mat, cgal_mesh, 0.3f);

        // Get results
        const vector<int>& result_labels = solver.final_facelabel;
        size_t result_count = result_labels.size();

        // Verify result count matches mesh faces
        if (result_count != mesh_face_count) {
            return SEGMAT_ERR_SEGMENT;
        }

        // Copy labels to caller-provided buffer
        for (size_t i = 0; i < result_count; i++) {
            labels->ptr[i] = static_cast<int32_t>(result_labels[i]);
        }

        // Update output length
        labels->len = result_count;

        // Count distinct labels
        std::set<int> unique_set(result_labels.begin(), result_labels.end());
        *unique_label_count = static_cast<int32_t>(unique_set.size());

        return SEGMAT_SUCCESS;

    } catch (const std::exception& e) {
        return SEGMAT_ERR_SEGMENT;
    } catch (...) {
        return SEGMAT_ERR_SEGMENT;
    }
}

} // extern "C"
