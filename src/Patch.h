#pragma once

#include "common_include.h"

class Patch
{
public:
	vector<Point> points;
	vector<Face> faces;
	map<double, int> feature_map;
	vector<feature_t> feature_value;  // Use vector for automatic memory management
	vector<float> feature_weight;     // Use vector for automatic memory management
	bool valid = true;
	int seednode = 0;

	// Default constructor, copy, move all handled automatically by compiler
	// because all members are now RAII types (vectors, map)

	Point computeCentroid()
	{

		double sumx = 0, sumy = 0, sumz = 0;
		for (size_t i = 0; i < points.size(); i++)
		{
			sumx += points[i].x(); sumy += points[i].y(); sumz += points[i].z();
		}
		Point p(sumx / (float)points.size(), sumy / (float)points.size(), sumz / (float)points.size());
		return p;
	}
};
