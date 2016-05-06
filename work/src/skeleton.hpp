//---------------------------------------------------------------------------
//
// Copyright (c) 2016 Taehyun Rhee, Joshua Scott, Ben Allen
//
// This software is provided 'as-is' for assignment of COMP308 in ECS,
// Victoria University of Wellington, without any express or implied warranty. 
// In no event will the authors be held liable for any damages arising from
// the use of this software.
//
// The contents of this file may not be copied or duplicated in any form
// without the prior permission of its owner.
//
//----------------------------------------------------------------------------

#pragma once

#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "cgra_math.hpp"
#include "opengl.hpp"
#include "geometry.hpp"



// Needed for Completion/Challenge
// We use bitmasking to work out the Degrees of Freedom
// To work out if a bone b has a y-axis dof, simply:
//     if (b.freedom & dof_ry) {...}
//
// To add and subtract degrees of freedom, respectively:
//     b.freedom |= dof_rx
//     b.freedom ^= dof_rx
using dof_set = unsigned int;

enum dof {
	dof_none = 0,
	dof_rx = 1,
	dof_ry = 2,
	dof_rz = 4,
	dof_root = 8 // Root has 6, 3 translation and 3 rotation
};


// Type to represent a bone
struct bone {
	std::string name;
	float length = 0;             // Length of the bone
	cgra::vec3 boneDir;           // Direction of the bone
	cgra::vec3 basisRot;          // Euler angle rotations for the bone basis
	dof_set freedom = dof_none;   // Degrees of freedom for the joint rotation
	std::vector<bone *> children; // Pointers to bone children

	// Completion and Challenge
	cgra::vec3 rotation;          // Rotation of joint in the basis (degrees)

	// Challenge
	cgra::vec3 translation;       // Translation (Only for the Root)
	cgra::vec3 rotation_max;      // Maximum value for rotation for this joint (degrees)
	cgra::vec3 rotation_min;      // Minimum value for rotation for this joint (degrees)

	std::vector< std::vector<double> > rotations = std::vector< std::vector<double>>();  // vector for each frame which contains the roations for that frame
	std::vector< std::vector<double> > poses = std::vector< std::vector<double>>(); // vector for each pose which contains the rotations for that frame
	std::vector<double> color = std::vector<double>{ 0.0,0.0,0.0 };
	int id;
};


class Skeleton {

private:
	std::vector<bone> m_bones;
	// Helper method
	int findBone(std::string);
	// Reading code
	void readASF(std::string);
	void readHeading(std::string, std::ifstream&);
	void readBone(std::ifstream&);
	void readHierarchy(std::ifstream&);
	void renderBone(bone *);
	void renderPoints();
	void renderSpline();
	cgra::vec3 catmull(float p, int i);
	int selected = -1;
	bool isSelected(bone *);
	int selectedAxis = 0;
	Geometry* geometry;
	std::vector<cgra::vec2> travelPoints = std::vector<cgra::vec2>();
	float currentLerpTime = 0.1;

public:

	Skeleton(std::string);
	float height(cgra::vec2 vec);
	void renderSkeleton();
	void readAMC(std::string);
	int rate = 1;
	int pose = -1;
	std::vector<cgra::vec3> m_points;
	bool color = false;
	void setSelected(int);
	int getSelected();
	void swapAxis();
	void applyRotation(int);
	bone* getBone();
	int frame = 0;
	bool animateMode = false;
	bool splineMode = false;
	bool keyframeMode = true;
	void saveFrame();
	void nextFrame();
	void previousFrame();
	void newPose();

};
