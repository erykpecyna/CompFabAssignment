#pragma once

#include <Eigen/Sparse>
#include <igl/edge_lengths.h>
#include <igl/face_areas.h>
#include <igl/dihedral_angles.h>
#include <igl/volume.h>
#include "dihedral_sine.hpp"
#include <algorithm>
#include <set>

unsigned set2ind(std::set<unsigned> s) {
	// This is to match edge indices to their corresponding index in the output of 
	// edge_lengths or dihedral_angles
	if (s.find(3) != s.end() && s.find(0) != s.end()) return 0;
	if (s.find(3) != s.end() && s.find(1) != s.end()) return 0;
	if (s.find(3) != s.end() && s.find(2) != s.end()) return 0;
	if (s.find(1) != s.end() && s.find(2) != s.end()) return 0;
	if (s.find(2) != s.end() && s.find(0) != s.end()) return 0;
	if (s.find(0) != s.end() && s.find(1) != s.end()) return 0;
}

// TODO: HW3
// Assignment 3, Part 3.2.
/* Implement your code here. */
// Implement the function to compute the cotangent laplacian matrix L 
// V is the vertex matrix of shape (n, 3), each row is the position of a vertex in the mesh
// F is the element index matrix of shape (m, 4), each row is the vertex indices of a tetrahedron
// L is the output cotangent laplacian matrix of shape (n, n), and it's a sparse matrix.
// Hints:
	// 1. For each tetrahedron, loop over each of its edge,
	//    consider which part of the L matrix this edge in this tetrahedron contributes to
	// 2. compute the cos and sin of the dihedral angle by the law of diehedral angles http://mathworld.wolfram.com/Tetrahedron.html
	//	  specifically, compute the sin and cos of dihedral angles from the edge lengths, face areas and tet volume
	// 3. build the triplets <row, col, value> in IJV
void cotangent_laplacian(
	const Eigen::MatrixXd& V, 
	const Eigen::MatrixXi& F, 
	Eigen::SparseMatrix<double>& L) 
{
	L.resize(V.rows(), V.rows());

	std::vector<Eigen::Triplet<double> > IJV;
	IJV.clear();

	// Angle and cos matrix
	Eigen::MatrixXd angles;
	angles.resize(F.rows(), 6);
	Eigen::MatrixXd coses;
	coses.resize(F.rows(), 6);

	igl::dihedral_angles(V, F, angles, coses);

	// Edge lengths
	Eigen::MatrixXd edgelengths;
	edgelengths.resize(F.rows(), 6);

	igl::edge_lengths(V, F, edgelengths);

	// Volumes
	Eigen::MatrixXd volumes;
	volumes.resize(F.rows(), 1);

	igl::volume(edgelengths, volumes);

	/* Implement your code here. */
	for (unsigned tetind = 0; tetind < F.rows(); tetind++) {
		// Loop over edges
		for (unsigned i = 0; i < 3; i++)
			for (unsigned j = i + 1; j < 4; j++) {
				// i and j are endpoints of edge in tet space
				// These edges all exist so this is case 1
				std::vector<unsigned> currinds({ i, j });

				// Find opposite edge
				std::vector<int> opp;
				for (unsigned k = 0; k < 4; k++) {
					if (std::find(currinds.begin(), currinds.end(), k) != currinds.end()) opp.push_back(k);
				}
				
				// Compute this element of the sum and insert
				std::set<unsigned> e;
				e.insert(opp[0]);
				e.insert(opp[1]);
				unsigned colind = set2ind(e);

				// Compute cotangent
				double element = (1.f / 6.f) * edgelengths(tetind, colind) * (coses(tetind, colind) / sin(angles(tetind, colind)));
				Eigen::Triplet<double> trip(i, j, element);
				Eigen::Triplet<double> trip(i, i, -element);
				Eigen::Triplet<double> trip(j, j, -element);
				IJV.push_back(trip);
			}
	}
	// Set From Triplets Sums all Triplets with the same indices
	L.setFromTriplets(IJV.begin(), IJV.end());
}