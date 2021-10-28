#pragma once

#include <Eigen/Sparse>
#include <igl/edge_lengths.h>
#include <igl/face_areas.h>
#include <igl/dihedral_angles.h>
#include <igl/volume.h>
#include "dihedral_sine.hpp"
#include <algorithm>
#include <set>

int set2ind(std::set<int> s) {
	// This is to match edge indices to their corresponding index in the output of 
	// edge_lengths or dihedral_angles
	if (s.find(3) != s.end() && s.find(0) != s.end()) return 0;
	if (s.find(3) != s.end() && s.find(1) != s.end()) return 1;
	if (s.find(3) != s.end() && s.find(2) != s.end()) return 2;
	if (s.find(1) != s.end() && s.find(2) != s.end()) return 3;
	if (s.find(2) != s.end() && s.find(0) != s.end()) return 4;
	if (s.find(0) != s.end() && s.find(1) != s.end()) return 5;
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
	Eigen::VectorXd volumes;
	volumes.resize(F.rows());

	igl::volume(edgelengths, volumes);

	// Face Areas
	Eigen::MatrixXd faceareas;
	faceareas.resize(F.rows(), 4);

	igl::face_areas(V, F, faceareas);

	// Sins
	Eigen::Matrix<double, Eigen::Dynamic, 6> sins;
	sins.resize(F.rows(), 6);

	dihedral_sine(volumes, faceareas, edgelengths, sins);

	/* Implement your code here. */
	for (int tetind = 0; tetind < F.rows(); tetind++) {
		// Loop over edges
		for (int i = 0; i < 3; i++)
			for (int j = i + 1; j < 4; j++) {
				// i and j are endpoints of edge in tet space
				// These edges all exist so this is case 1
				std::vector<int> currinds({ i, j });

				// Find opposite edge
				std::vector<int> opp;
				for (int k = 0; k < 4; k++) {
					if (std::find(currinds.begin(), currinds.end(), k) == currinds.end()) opp.push_back(k);
				}
				
				// Compute this element of the sum and insert
				std::set<int> e;
				e.insert(opp[0]);
				e.insert(opp[1]);
				unsigned colind = set2ind(e);

				// Compute cotangent
				double element = (1.f / 6.f) * edgelengths(tetind, colind) * (coses(tetind, colind) / sins(tetind, colind));
				Eigen::Triplet<double> trip1(F(tetind, i), F(tetind,j), element);
				Eigen::Triplet<double> trip2(F(tetind, i), F(tetind,i), -element);
				Eigen::Triplet<double> trip3(F(tetind, j), F(tetind,j), -element);
				IJV.push_back(trip1);
				IJV.push_back(trip2);
				IJV.push_back(trip3);
			}
	}
	// Set From Triplets Sums all Triplets with the same indices
	L.setFromTriplets(IJV.begin(), IJV.end());
}