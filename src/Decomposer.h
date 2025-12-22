#pragma once
#include "MAT.h"
#include "ombb/gdiam.hpp"
#include <cstdint>

class Decomposer
{
public:
	vector<int> final_facelabel;
	Mesh readMesh(string path);

	// Buffer-based mesh creation for WASM interface
	static Mesh createMeshFromBuffers(
		const float* vertices,      // [vertex_count * 3] - x,y,z interleaved
		int32_t vertex_count,
		const int32_t* faces,       // [face_count * 3] - v0,v1,v2 triplets
		int32_t face_count
	);
	void decompose3Dshape(MAT& mat, MAT& smat, Mesh& mesh, float growing_threshold, float min_region);
	void transfer_MAT_mesh(MAT& mat, Mesh& mesh, float weight);
	void primitiveAbstraction(Mesh& mesh, vector<vector<float>> colors, string outputpath);
	void saveColoredMesh(Mesh& mesh, vector<vector<float>> colors, string outputpath);
	void saveSegResult(Mesh& mesh, string outputpath);

private:
	double compute_face_angle(Face& f1, Face& f2);
	double compute_bend_angle_degree(Face& f1, Face& f2);
	bool check_concave(Face& f1, Face& f2, Edge& sharededge);
	bool checkTriangleConnect(Face& f1, Face& f2, Edge& sharededge);
	vector<Eigen::Vector3d> box_fitting(gdiam_real  * points, int  num);
	vector<Eigen::Vector3d> computeOrientedBox(std::vector<Eigen::Vector3d>& vertices);

};