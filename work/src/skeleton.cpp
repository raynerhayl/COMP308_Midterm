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

#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <stdexcept>

#include "cgra_geometry.hpp"
#include "cgra_math.hpp"
#include "opengl.hpp"
#include "skeleton.hpp"
#include "geometry.hpp"

using namespace std;
using namespace cgra;


Skeleton::Skeleton(string filename) {
	geometry = new Geometry();
	bone b = bone();
	b.name = "root";
	b.freedom |= dof_rx;
	b.freedom |= dof_ry;
	b.freedom |= dof_rz;
	b.freedom |= dof_root;
	m_bones.push_back(b);
	readASF(filename);
}

float Skeleton::height(vec2 vec) {
	vec2 heightVec = cgra::sin(vec);
	return heightVec.x + heightVec.y;
}

//-------------------------------------------------------------
// [Assignment 2] :
// You may need to revise this function for Completion/Challenge
//-------------------------------------------------------------
void Skeleton::renderSkeleton() {

	glPushMatrix();

	if (color == true) {
		glBegin(GL_LINES);
		glColor3f(1, 1, 1);
		for (int i = 0; i < 1000; i++) {
			glLineWidth(4);
			glVertex3f(-1000, i*0.05 - 10, 0);
			glVertex3f(1000, i*0.05 - 10, 0);

			glVertex3f(i*0.05 - 10, -1000, 0);
			glVertex3f(i*0.05 - 10, 1000, 0);
		}
		glEnd();
	}

	if (splineMode) {
		renderPoints();
		renderSpline();
		frame = frame + 1;
		if (frame >= travelPoints.size()) {
			frame = 0;
		}
	}
	else if (animateMode) {

		renderBone(&m_bones[0]);

		currentLerpTime += 0.01;

		if (currentLerpTime > 1.0) {
			frame = frame + rate;
			currentLerpTime = 0;
		} 

		if (frame == m_bones[0].rotations.size()) {
			frame = 0;
		}
	}
	else if (keyframeMode) {
		renderBone(&m_bones[0]);

	}

	if (m_points.size() > 0) {
		if (m_points.size() >= 4) {
			glTranslatef(travelPoints[frame].x, travelPoints[frame].y, 0);
		}
		else {
			glTranslatef(m_points[0].x, m_points[0].y, 0);
		}
		glScalef(0.005, 0.005, 0.005);
		geometry->renderGeometry();

	}


	// Clean up
	glPopMatrix();
}



void Skeleton::renderPoints() {
	for (int i = 0; i < m_points.size(); i++) {
		glPushMatrix(); {
			if (i == 0 || i == m_points.size() - 1) {
				glColor3f(0, 0, 1);
			}
			else {
				glColor3f(0, 0, 0);
			}
			glTranslatef(m_points[i].x, m_points[i].y, m_points[i].z);
			cgraSphere(0.01);
		}glPopMatrix();
	}
}

void Skeleton::renderSpline() {
	travelPoints = std::vector<cgra::vec2>();
	if (m_points.size() >= 4) {
		glLineWidth(5);
		glBegin(GL_LINES);
		glColor3f(1, 0, 0);
		for (int i = 0; i + 3 < m_points.size(); i++) {
			float t = 0;
			while (t < 1) {
				vec3 point1 = catmull(t, i);
				t = t + 0.01;
				vec3 point2 = catmull(t, i);
				glVertex3f(point1.x, point1.y, point1.z);
				glVertex3f(point2.x, point2.y, point2.z);
				travelPoints.push_back(vec2(point1.x, point1.y));
				travelPoints.push_back(vec2(point2.x, point2.y));
			}
		}
		glEnd();
		glLineWidth(1);
	}
}

vec3 Skeleton::catmull(float p, int i) {
	return 0.5*((2 * m_points[i + 1]) +
		(m_points[i + 2] - m_points[i])*p +
		(2 * m_points[i] - 5 * m_points[i + 1] + 4 * m_points[i + 2] - m_points[i + 3])*(p*p) +
		(3 * m_points[i + 1] - m_points[i + 0] - 3 * m_points[i + 2] + m_points[i + 3])*(p*p*p));
}

//-------------------------------------------------------------
// [Assignment 2] :
// Should render each bone by drawing the axes, and the bone
// then translating to the end of the bone and drawing each
// of it's children. Remember to use the basis rotations
// before drawing the axes (and for Completion, rotation).
// The actual bone is NOT drawn with the basis rotation.
//
// Should not draw the root bone (because it has zero length)
// but should go on to draw it's children
//-------------------------------------------------------------
void Skeleton::renderBone(bone *b) {
	double R = 0.015;
	glPushMatrix();
	{


		if (color == false) { // set color to unique color for color picking.
			int r = (b->id & 0x000000FF) >> 0;
			int g = (b->id & 0x0000FF00) >> 8;
			int bl = (b->id & 0x00FF0000) >> 16;

			glColor3f(r / 255.0, g / 255.0, bl / 255.0);
		}
		else if (isSelected(b)) {
			glColor3f(1, 0, 1); // Joint
		}
		else {
			glColor3f(0, 1, 1); // Joint
		}

		cgraSphere(1.2*R);

		glRotatef(b->basisRot.z, 0, 0, 1);
		glRotatef(b->basisRot.y, 0, 1, 0);
		glRotatef(b->basisRot.x, 1, 0, 0); // basis rotations

		if (color == true && isSelected(b)) { //axis can get in the way of color picking
			glPushMatrix(); // Z-axis
			{
				glColor3f(0, 0, 1);
				if (selectedAxis == 0) {
					glColor3f(0.5, 0.5, 1);
				}
				cgraCylinder(0.3 * R, 0.3 * R, 4 * R);
				glTranslatef(0, 0, 4 * R);
				cgraCone(.5*R, .5*R);
			}glPopMatrix();

			glPushMatrix(); // Y-axis
			{
				glColor3f(0, 1, 0);
				if (selectedAxis == 1) {
					glColor3f(0.5, 1, 0.5);
				}
				glRotatef(-90, 1, 0, 0);
				cgraCylinder(0.3 * R, 0.3 * R, 4 * R);
				glTranslatef(0, 0, 4 * R);
				cgraCone(.5*R, .5*R);
			}glPopMatrix();

			glPushMatrix(); // Y-axis
			{

				glColor3f(1, 0, 0);
				if (selectedAxis == 2) {
					glColor3f(1, 0.5, 0.5);
				}
				glRotatef(90, 0, 1, 0);
				cgraCylinder(0.3 * R, 0.3 * R, 4 * R);
				glTranslatef(0, 0, 4 * R);
				cgraCone(.5*R, .5*R);
			}glPopMatrix();// X-axis
		}

		if (b->rotations.size() > 0) {

			if (b->rotations.size() > 1) {
				double xRot = b->rotations[frame][3];
				double yRot = b->rotations[frame][4];
				double zRot = b->rotations[frame][5];

				double xRot2 = 0;
				double yRot2 = 0;
				double zRot2 = 0;

				if (frame < b->rotations.size()-1) {
					xRot2 = b->rotations[frame+1][3];
					yRot2 = b->rotations[frame+1][4];
					zRot2 = b->rotations[frame+1][5];
				}
				else {
					xRot2 = b->rotations[0][3];
					yRot2 = b->rotations[0][4];
					zRot2 = b->rotations[0][5];
				}

				quat lerpRot = slerp(quat(zRot, yRot, xRot), quat(zRot2, yRot2, xRot2), currentLerpTime);
				//cout << lerpRot << endl;
				glMultMatrixf((GLfloat *)((mat4)lerpRot));
			}
			else {
				double xRot = b->rotations[frame][3];
				double yRot = b->rotations[frame][4];
				double zRot = b->rotations[frame][5];

				glRotatef(zRot, 0, 0, 1);
				glRotatef(yRot, 0, 1, 0);
				glRotatef(xRot, 1, 0, 0);
			}


		} //amc rotation

		glRotatef(-b->basisRot.x, 1, 0, 0);
		glRotatef(-b->basisRot.y, 0, 1, 0);
		glRotatef(-b->basisRot.z, 0, 0, 1); // inverse basis rotations

		if (b->length > 0) {
			glPushMatrix(); // Bone Segment
			{

				//glColor3f(0.8, 0.8, 0.8);

				double forwardAngle = (180 * acos(dot(vec3(0, 0, 1), b->boneDir))) / math::pi();
				vec3 axis = cross(vec3(0, 0, 1), b->boneDir);

				glRotatef(forwardAngle, axis.x, axis.y, axis.z);

				if (color == false) { // set color to unique color for color picking.
					int r = (b->id & 0x000000FF) >> 0;
					int g = (b->id & 0x0000FF00) >> 8;
					int bl = (b->id & 0x00FF0000) >> 16;

					glColor3f(r / 255.0, g / 255.0, bl / 255.0);
				}
				else {
					if (isSelected(b)) { // selected bone
						glColor3f(1, 1, 0);
					}
					else { // not selected bone
						glColor3f(0.8, 0.8, 0.8);
					}
				}
				cgraCylinder(R, R / 3, b->length);

			}glPopMatrix();

			glTranslatef(b->boneDir.x*b->length, b->boneDir.y*b->length, b->boneDir.z*b->length);
		}

		for (auto &c : b->children) {
			renderBone(c);
		}

	}glPopMatrix();
}

bool Skeleton::isSelected(bone *b) {
	return selected >= 0 && selected < m_bones.size() && m_bones[selected].name == b->name;
}

void Skeleton::setSelected(int index) {
	if (index > 0 && index < m_bones.size()) {
		selected = index;
	}
}



void Skeleton::applyRotation(int rot) {



	if (selected > 0 && selected < m_bones.size()) {
		bone* b = &m_bones[selected];
		//b->rotations[0][0] = b->rotations[0][0] + rot;
		//cout << b->rotations[0][0] << endl;
		cout << frame << endl;
		int index = 3;
		if (b->freedom & dof_rx) {
			if (b->name != "root") {
				if (selectedAxis == 0) {
					b->rotations[frame][index] = b->rotations[frame][index] + rot;
				}
				index = index + 1;
			}
		}

		if (b->freedom & dof_ry) {
			if (b->name != "root") {

				if (selectedAxis == 1) {
					b->rotations[frame][index] = b->rotations[frame][index] + rot;
				}
				index = index + 1;
			}
		}

		if (b->freedom & dof_rz) {
			if (b->name != "root") {
				if (selectedAxis == 2) {
					b->rotations[frame][index] = b->rotations[frame][index] + rot;
				}
				index = index + 1;
			}
		}

	}
}

bone* Skeleton::getBone() {
	if (selected > 0 && selected < m_bones.size()) {
		return &m_bones[selected];
	}
	else {
		return nullptr;
	}
}

void Skeleton::swapAxis() {
	selectedAxis = selectedAxis + 1;
	if (selectedAxis > 2) {
		selectedAxis = 0;
	}
}

void Skeleton::saveFrame() {
	for (int i = 0; i < m_bones.size(); i++) {
		vector<double> frameV = vector<double>();
		for (int j = 0; j < m_bones[i].rotations[frame].size(); j++) {
			frameV.push_back(m_bones[i].rotations[frame][j]);
		}
		m_bones[i].rotations.push_back(frameV);
	}
	frame = frame + 1;
}

void Skeleton::nextFrame() {
	frame = frame + 1;
	if (frame >= m_bones[0].rotations.size()) {
		frame = m_bones[0].rotations.size() - 1;
	}
}

void Skeleton::previousFrame() {
	frame = frame - 1;
	if (frame < 0) {
		frame = 0;
	}
}

void Skeleton::newPose() {
	for (int i = 0; i < m_bones.size(); i++) {
		bone* b = &m_bones[i];
		std::vector<double> frame = std::vector<double>{ 0,0,0,0,0,0 };
		b->rotations.push_back(frame);
	}

}

// Helper method for retreiving and trimming the next line in a file.
// You should not need to modify this method.
namespace {
	string nextLineTrimmed(istream &file) {
		// Pull out line from file
		string line;
		getline(file, line);
		// Remove leading and trailing whitespace and comments
		size_t i = line.find_first_not_of(" \t\r\n");
		if (i != string::npos) {
			if (line[i] != '#') {
				return line.substr(i, line.find_last_not_of(" \t\r\n") - i + 1);
			}
		}
		return "";
	}
}


int Skeleton::findBone(string name) {
	for (size_t i = 0; i < m_bones.size(); i++)
		if (m_bones[i].name == name)
			return i;
	return -1;
}


void Skeleton::readASF(string filename) {

	ifstream file(filename);

	if (!file.is_open()) {
		cerr << "Failed to open file " << filename << endl;
		throw runtime_error("Error :: could not open file.");
	}

	cout << "Reading file" << filename << endl;

	// good() means that failbit, badbit and eofbit are all not set
	while (file.good()) {

		// Pull out line from file
		string line = nextLineTrimmed(file);

		// Check if it is a comment or just empty
		if (line.empty() || line[0] == '#')
			continue;
		else if (line[0] == ':') {
			// Line starts with a ':' character so it must be a header
			readHeading(line, file);
		}
		else {
			// Would normally error here, but becuase we don't parse
			// every header entirely we will leave this blank.
		}
	}

	// Assign ids to bone segments, should be done elsewhere
	for (int i = 0; i < m_bones.size(); i++) {
		m_bones[i].id = i;
	}

	//cout << "Completed reading skeleton file" << endl;
}


void Skeleton::readHeading(string headerline, ifstream &file) {

	string head;
	istringstream lineStream(headerline);
	lineStream >> head; // get the first token from the stream

						// remove the ':' from the header name
	if (head[0] == ':')
		head = head.substr(1);

	if (lineStream.fail() || head.empty()) {
		cerr << "Could not get heading name from\"" << headerline << "\", all is lost" << endl;
		throw runtime_error("Error :: could not parse .asf file.");
	}

	if (head == "version") {
		//version string - must be 1.10
		string version;
		lineStream >> version;
		if (lineStream.fail() || version != "1.10") {
			cerr << "Invalid version: \"" << version << "\" must be 1.10" << endl;
			throw runtime_error("Error :: invalid .asf version.");
		}
	}
	else if (head == "name") {
		// This allows the skeleton to be called something
		// other than the file name. We don't actually care
		// what the name is, so can ignore this whole section
	}
	else if (head == "documentation") {
		// Documentation section has no meaningful information
		// only of use if you want to copy the file. So we skip it
	}
	else if (head == "units") {
		// Has factors for the units to be able to model the
		// real person, these must be parsed correctly. Only
		// really need to check if deg or rad, but that is 
		// not needed for this assignment.

		// We are going to assume that the units:length feild
		// is 0.45, and that the angles are in degrees
	}
	else if (head == "root") {
		// Read in information about root. Let's just assume
		// it'll be at the origin for this assignment.
	}
	else if (head == "bonedata") {
		// Read in each bone until we get to the
		// end of the file or a new header
		string line = nextLineTrimmed(file);
		while (file.good() && !line.empty()) {
			if (line[0] == ':') {
				// finished our reading of bones
				// read next header and return
				return readHeading(line, file);
			}
			else if (line == "begin") {
				// Read the bone data
				readBone(file);
			}
			else {
				cerr << "Expected 'begin' in bone data, found \"" << line << "\"";
				throw runtime_error("Error :: could not parse .asf file.");
			}
			line = nextLineTrimmed(file);
		}
	}
	else if (head == "hierarchy") {
		// Description of how the bones fit together
		// Read in each line until we get to the
		// end of the file or a new header
		string line = nextLineTrimmed(file);
		while (file.good() && !line.empty()) {
			if (line[0] == ':') {
				// finished our reading of bones
				// read next header and return
				return readHeading(line, file);
			}
			else if (line == "begin") {
				// Read the entire hierarchy
				readHierarchy(file);
			}
			else {
				cerr << "Expected 'begin' in hierarchy, found \"" << line << "\"";
				throw runtime_error("Error :: could not parse .asf file.");
			}
			line = nextLineTrimmed(file);
		}
	}
	else {
		// Would normally error here, but becuase we don't parse
		// every header entirely we will leave this blank.
	}
}


void Skeleton::readBone(ifstream &file) {
	// Create the bone to add the data to
	bone b;

	string line = nextLineTrimmed(file);
	while (file.good()) {
		if (line == "end") {
			// End of the data for this bone
			// Push the bone into the vector
			m_bones.push_back(b);

			return;
		}
		else {

			string head;
			istringstream lineStream(line);
			lineStream >> head; // Get the first token

			if (head == "name") {
				// Name of the bone
				lineStream >> b.name;
			}
			else if (head == "direction") {
				// Direction of the bone
				lineStream >> b.boneDir.x >> b.boneDir.y >> b.boneDir.z;
				b.boneDir = normalize(b.boneDir); // Normalize here for consistency
			}
			else if (head == "length") {
				// Length of the bone
				float length;
				lineStream >> length;
				length *= (1.0 / 0.45);  // scale by 1/0.45 to get actual measurements
				length *= 0.0254;      // convert from inches to meters
				b.length = length;
			}
			else if (head == "dof") {
				// Degrees of Freedom of the joint (rotation)
				while (lineStream.good()) {
					string dofString;
					lineStream >> dofString;
					if (!dofString.empty()) {
						// Parse each dof string
						if (dofString == "rx") b.freedom |= dof_rx;
						else if (dofString == "ry") b.freedom |= dof_ry;
						else if (dofString == "rz") b.freedom |= dof_rz;
						else throw runtime_error("Error :: could not parse .asf file.");
					}
				}
			}
			else if (head == "axis") {
				// Basis rotations 
				lineStream >> b.basisRot.x >> b.basisRot.y >> b.basisRot.z;
			}
			else if (head == "limits") {
				// Limits for each of the DOF
				// Assumes dof has been read first
				// You can optionally fill this method out
			}

			// Because we've tried to parse numerical values
			// check if we've failed at any point
			if (lineStream.fail()) {
				cerr << "Unable to parse \"" << line << "\"";
				throw runtime_error("Error :: could not parse .asf file.");
			}
		}

		// Get the next line
		line = nextLineTrimmed(file);
	}

	cerr << "Expected end in bonedata, found \"" << line << "\"";
	throw runtime_error("Error :: could not parse .asf file.");
}


void Skeleton::readHierarchy(ifstream &file) {
	string line = nextLineTrimmed(file);
	while (file.good()) {
		if (line == "end") {
			// End of hierarchy
			return;
		}
		else if (!line.empty()) {
			// Read the parent node
			string parentName;
			istringstream lineStream(line);
			lineStream >> parentName;

			// Find the parent bone and have a pointer to it
			int parentIndex = findBone(parentName);

			if (parentIndex < 0) {
				cerr << "Expected a valid parent bone name, found \"" << parentName << "\"" << endl;
				throw runtime_error("Error :: could not parse .asf file.");
			}

			//Read the connections
			string childName;
			lineStream >> childName;
			while (!lineStream.fail() && !childName.empty()) {

				int childIndex = findBone(childName);

				if (childIndex < 0) {
					cerr << "Expected a valid child bone name, found \"" << childName << "\"" << endl;
					throw runtime_error("Error :: could not parse .asf file.");
				}

				// Set a POINTER to the child to be recorded in the parents
				m_bones[parentIndex].children.push_back(&m_bones[childIndex]);

				// Get the next child
				lineStream >> childName;
			}
		}
		line = nextLineTrimmed(file);
	}
	cerr << "Expected end in bonedata, found \"" << line << "\"";
	throw runtime_error("Error :: could not parse .asf file.");
}



//-------------------------------------------------------------
// [Assignment 2] :
// Complete the following method to load data from an *.amc file
//-------------------------------------------------------------
void Skeleton::readAMC(string filename) {
	ifstream file(filename);
	if (!file.is_open()) {
		cerr << "Failed to open file " << filename << endl;
		throw runtime_error("Error :: could not open file.");
	}

	cout << "Reading file" << filename << endl;

	// good() means that failbit, badbit and eofbit are all not set
	while (file.good()) {

		// Pull out line from file
		string line = nextLineTrimmed(file);
		istringstream lineStream(line);

		// Check if it is a comment or just empty
		if (line.empty() || line[0] == '#' || line[0] == ':') {
			continue;
		}
		else {
			string name;
			lineStream >> name;
			for (int i = 0; i < m_bones.size(); i++) {
				if (m_bones[i].name == name) {
					string token;
					vector<double> tempFrame = vector<double>();
					while (lineStream.fail() == false) {
						double angle;
						lineStream >> token;
						angle = stod(token);
						tempFrame.push_back(angle);

					}
					vector<double> frame = vector<double>{ 0, 0, 0, 0, 0, 0 };
					if (m_bones[i].freedom & dof_root || m_bones[i].freedom == 0) {
						frame = tempFrame;
						//cout << m_bones[i].name << endl;
					}
					else {
						int index = 0;
						if (m_bones[i].freedom & dof_rx) {
							frame[3] = tempFrame[index];
							index = index + 1;
						}

						if (m_bones[i].freedom & dof_ry) {
							frame[4] = tempFrame[index];
							index = index + 1;
						}

						if (m_bones[i].freedom & dof_rz) {
							frame[5] = tempFrame[index];
							index = index + 1;
						}
					}
					m_bones[i].rotations.push_back(frame);
				}
			}
		}

	}

	cout << "Completed reading AMC file" << endl;
}

// YOUR CODE GOES HERE
// ...
//
////---------------------------------------------------------------------------
////
//// Copyright (c) 2016 Taehyun Rhee, Joshua Scott, Ben Allen
////
//// This software is provided 'as-is' for assignment of COMP308 in ECS,
//// Victoria University of Wellington, without any express or implied warranty. 
//// In no event will the authors be held liable for any damages arising from
//// the use of this software.
////
//// The contents of this file may not be copied or duplicated in any form
//// without the prior permission of its owner.
////
////----------------------------------------------------------------------------
//
//#include <cmath>
//#include <iostream>
//#include <fstream>
//#include <sstream>
//#include <string>
//#include <stdexcept>
//
//#include "cgra_geometry.hpp"
//#include "cgra_math.hpp"
//#include "opengl.hpp"
//#include "skeleton.hpp"
//
//using namespace std;
//using namespace cgra;
//
//
//Skeleton::Skeleton(string filename) {
//	bone b = bone();
//	b.name = "root";
//	b.freedom |= dof_rx;
//	b.freedom |= dof_ry;
//	b.freedom |= dof_rz;
//	b.freedom |= dof_root;
//	m_bones.push_back(b);
//	readASF(filename);
//}
//
//float Skeleton::height(vec2 vec) {
//	vec2 heightVec = cgra::sin(vec);
//	return heightVec.x + heightVec.y;
//}
//
////-------------------------------------------------------------
//// [Assignment 2] :
//// You may need to revise this function for Completion/Challenge
////-------------------------------------------------------------
//void Skeleton::renderSkeleton() {
//
//	glPushMatrix();
//
//	if (color == true) {
//		renderPoints();
//
//		renderSpline();
//
//
//		glBegin(GL_LINES);
//		glColor3f(1, 1, 1);
//		for (int i = 0; i < 1000; i++) {
//			glLineWidth(4);
//			glVertex3f(-1000, i*0.05 - 10, 0);
//			glVertex3f(1000, i*0.05 - 10, 0);
//
//			glVertex3f(i*0.05 - 10, -1000, 0);
//			glVertex3f(i*0.05 - 10, 1000, 0);
//		}
//		glEnd();
//	}
//
//	renderBone(&m_bones[0]);
//
//
//	//frame = frame + rate;
//
//	if (frame == m_bones[0].rotations.size()) {
//		frame = 0;
//	}
//
//	// Clean up
//	glPopMatrix();
//}
//
//
//void Skeleton::renderPoints() {
//	for (int i = 0; i < m_points.size(); i++) {
//		glPushMatrix(); {
//			if (i == 0 || i == m_points.size() - 1) {
//				glColor3f(0, 0, 1);
//			}
//			else {
//				glColor3f(0, 0, 0);
//			}
//			glTranslatef(m_points[i].x, m_points[i].y, m_points[i].z);
//			cgraSphere(0.01);
//		}glPopMatrix();
//	}
//}
//
//void Skeleton::renderSpline() {
//	if (m_points.size() >= 4) {
//		glLineWidth(5);
//		glBegin(GL_LINES);
//		glColor3f(1, 0, 0);
//		for (int i = 0; i + 3 < m_points.size(); i++) {
//			float t = 0;
//			while (t < 1) {
//				vec3 point1 = catmull(t, i);
//				t = t + 0.01;
//				vec3 point2 = catmull(t, i);
//				glVertex3f(point1.x, point1.y, point1.z);
//				glVertex3f(point2.x, point2.y, point2.z);
//			}
//		}
//		glEnd();
//		glLineWidth(1);
//	}
//}
//
//vec3 Skeleton::catmull(float p, int i) {
//	return 0.5*((2 * m_points[i + 1]) +
//		(m_points[i + 2] - m_points[i])*p +
//		(2 * m_points[i] - 5 * m_points[i + 1] + 4 * m_points[i + 2] - m_points[i + 3])*(p*p) +
//		(3 * m_points[i + 1] - m_points[i + 0] - 3 * m_points[i + 2] + m_points[i + 3])*(p*p*p));
//}
//
////-------------------------------------------------------------
//// [Assignment 2] :
//// Should render each bone by drawing the axes, and the bone
//// then translating to the end of the bone and drawing each
//// of it's children. Remember to use the basis rotations
//// before drawing the axes (and for Completion, rotation).
//// The actual bone is NOT drawn with the basis rotation.
////
//// Should not draw the root bone (because it has zero length)
//// but should go on to draw it's children
////-------------------------------------------------------------
//void Skeleton::renderBone(bone *b) {
//	double R = 0.015;
//	glPushMatrix();
//	{
//
//		glColor3f(0, 1, 1); // Joint
//		cgraSphere(1.2*R);
//
//		glRotatef(b->basisRot.z, 0, 0, 1);
//		glRotatef(b->basisRot.y, 0, 1, 0);
//		glRotatef(b->basisRot.x, 1, 0, 0); // basis rotations
//
//		if (color == true && isSelected(b)) { //axis can get in the way of color picking
//			glPushMatrix(); // Z-axis
//			{
//				glColor3f(0, 0, 1);
//				if (selectedAxis == 0) {
//					glColor3f(0.5, 0.5, 1);
//				}
//				cgraCylinder(0.3 * R, 0.3 * R, 4 * R);
//				glTranslatef(0, 0, 4 * R);
//				cgraCone(.5*R, .5*R);
//			}glPopMatrix();
//
//			glPushMatrix(); // Y-axis
//			{
//				glColor3f(0, 1, 0);
//				if (selectedAxis == 1) {
//					glColor3f(0.5, 1, 0.5);
//				}
//				glRotatef(-90, 1, 0, 0);
//				cgraCylinder(0.3 * R, 0.3 * R, 4 * R);
//				glTranslatef(0, 0, 4 * R);
//				cgraCone(.5*R, .5*R);
//			}glPopMatrix();
//
//			glPushMatrix(); // Y-axis
//			{
//
//				glColor3f(1, 0, 0);
//				if (selectedAxis == 2) {
//					glColor3f(1, 0.5, 0.5);
//				}
//				glRotatef(90, 0, 1, 0);
//				cgraCylinder(0.3 * R, 0.3 * R, 4 * R);
//				glTranslatef(0, 0, 4 * R);
//				cgraCone(.5*R, .5*R);
//			}glPopMatrix();// X-axis
//		}
//
//		if (b->rotations.size() > 0) {
//
//			double xRot = 0;
//			double yRot = 0;
//			double zRot = 0;
//
//			int index = 0;
//			if (b->freedom & dof_rx) {
//				if (b->name != "root") {
//
//					xRot = b->rotations[frame][index];
//					index = index + 1;
//				}
//			}
//
//			if (b->freedom & dof_ry) {
//				if (b->name != "root") {
//
//					yRot = b->rotations[frame][index];
//					index = index + 1;
//				}
//			}
//
//			if (b->freedom & dof_rz) {
//				if (b->name != "root") {
//
//					zRot = b->rotations[frame][index];
//					index = index + 1;
//				}
//			}
//			if (b->freedom&dof_root) {
//				xRot = b->rotations[frame][3];
//				yRot = b->rotations[frame][4];
//				zRot = b->rotations[frame][5];
//
//			}
//
//			glRotatef(zRot, 0, 0, 1);
//			glRotatef(yRot, 0, 1, 0);
//			glRotatef(xRot, 1, 0, 0);
//
//		} //amc rotation
//
//		glRotatef(-b->basisRot.x, 1, 0, 0);
//		glRotatef(-b->basisRot.y, 0, 1, 0);
//		glRotatef(-b->basisRot.z, 0, 0, 1); // inverse basis rotations
//
//		if (b->length > 0) {
//			glPushMatrix(); // Bone Segment
//			{
//
//				//glColor3f(0.8, 0.8, 0.8);
//
//				double forwardAngle = (180 * acos(dot(vec3(0, 0, 1), b->boneDir))) / math::pi();
//				vec3 axis = cross(vec3(0, 0, 1), b->boneDir);
//
//				glRotatef(forwardAngle, axis.x, axis.y, axis.z);
//
//				if (color == false) { // set color to unique color for color picking.
//					int r = (b->id & 0x000000FF) >> 0;
//					int g = (b->id & 0x0000FF00) >> 8;
//					int bl = (b->id & 0x00FF0000) >> 16;
//
//					glColor3f(r / 255.0, g / 255.0, bl / 255.0);
//				}
//				else {
//					if (isSelected(b)) { // selected bone
//						glColor3f(1, 1, 0);
//					}
//					else { // not selected bone
//						glColor3f(0.8, 0.8, 0.8);
//					}
//				}
//				cgraCylinder(R, R / 3, b->length);
//
//			}glPopMatrix();
//
//			glTranslatef(b->boneDir.x*b->length, b->boneDir.y*b->length, b->boneDir.z*b->length);
//		}
//
//		for (auto &c : b->children) {
//			renderBone(c);
//		}
//
//	}glPopMatrix();
//}
//
//bool Skeleton::isSelected(bone *b) {
//	return selected >= 0 && selected < m_bones.size() && m_bones[selected].name == b->name;
//}
//
//void Skeleton::setSelected(int index) {
//	if (index > 0 && index < m_bones.size()) {
//		selected = index;
//	}
//}
//
//void Skeleton::applyRotation(int rot) {
//	if (selected > 0 && selected < m_bones.size()) {
//		bone* b = &m_bones[selected];
//		//b->rotations[0][0] = b->rotations[0][0] + rot;
//		//cout << b->rotations[0][0] << endl;
//
//		int index = 0;
//		if (b->freedom & dof_rx) {
//			if (b->name != "root") {
//				if (selectedAxis == 0) {
//					b->rotations[0][index] = b->rotations[0][index] + rot;
//				}
//				index = index + 1;
//			}
//		}
//
//		if (b->freedom & dof_ry) {
//			if (b->name != "root") {
//
//				if (selectedAxis == 1) {
//					b->rotations[0][index] = b->rotations[0][index] + rot;
//				}
//				index = index + 1;
//			}
//		}
//
//		if (b->freedom & dof_rz) {
//			if (b->name != "root") {
//				if (selectedAxis == 2) {
//					b->rotations[0][index] = b->rotations[0][index] + rot;
//				}
//				index = index + 1;
//			}
//		}
//
//	}
//}
//
//bone* Skeleton::getBone() {
//	if (selected > 0 && selected < m_bones.size()) {
//		return &m_bones[selected];
//	}
//	else {
//		return nullptr;
//	}
//}
//
//void Skeleton::swapAxis() {
//	selectedAxis = selectedAxis + 1;
//	if (selectedAxis > 2) {
//		selectedAxis = 0;
//	}
//}
//
//// Helper method for retreiving and trimming the next line in a file.
//// You should not need to modify this method.
//namespace {
//	string nextLineTrimmed(istream &file) {
//		// Pull out line from file
//		string line;
//		getline(file, line);
//		// Remove leading and trailing whitespace and comments
//		size_t i = line.find_first_not_of(" \t\r\n");
//		if (i != string::npos) {
//			if (line[i] != '#') {
//				return line.substr(i, line.find_last_not_of(" \t\r\n") - i + 1);
//			}
//		}
//		return "";
//	}
//}
//
//
//int Skeleton::findBone(string name) {
//	for (size_t i = 0; i < m_bones.size(); i++)
//		if (m_bones[i].name == name)
//			return i;
//	return -1;
//}
//
//
//void Skeleton::readASF(string filename) {
//
//	ifstream file(filename);
//
//	if (!file.is_open()) {
//		cerr << "Failed to open file " << filename << endl;
//		throw runtime_error("Error :: could not open file.");
//	}
//
//	cout << "Reading file" << filename << endl;
//
//	// good() means that failbit, badbit and eofbit are all not set
//	while (file.good()) {
//
//		// Pull out line from file
//		string line = nextLineTrimmed(file);
//
//		// Check if it is a comment or just empty
//		if (line.empty() || line[0] == '#')
//			continue;
//		else if (line[0] == ':') {
//			// Line starts with a ':' character so it must be a header
//			readHeading(line, file);
//		}
//		else {
//			// Would normally error here, but becuase we don't parse
//			// every header entirely we will leave this blank.
//		}
//	}
//
//	// Assign ids to bone segments, should be done elsewhere
//	for (int i = 0; i < m_bones.size(); i++) {
//		m_bones[i].id = i;
//	}
//
//	//cout << "Completed reading skeleton file" << endl;
//}
//
//
//void Skeleton::readHeading(string headerline, ifstream &file) {
//
//	string head;
//	istringstream lineStream(headerline);
//	lineStream >> head; // get the first token from the stream
//
//						// remove the ':' from the header name
//	if (head[0] == ':')
//		head = head.substr(1);
//
//	if (lineStream.fail() || head.empty()) {
//		cerr << "Could not get heading name from\"" << headerline << "\", all is lost" << endl;
//		throw runtime_error("Error :: could not parse .asf file.");
//	}
//
//	if (head == "version") {
//		//version string - must be 1.10
//		string version;
//		lineStream >> version;
//		if (lineStream.fail() || version != "1.10") {
//			cerr << "Invalid version: \"" << version << "\" must be 1.10" << endl;
//			throw runtime_error("Error :: invalid .asf version.");
//		}
//	}
//	else if (head == "name") {
//		// This allows the skeleton to be called something
//		// other than the file name. We don't actually care
//		// what the name is, so can ignore this whole section
//	}
//	else if (head == "documentation") {
//		// Documentation section has no meaningful information
//		// only of use if you want to copy the file. So we skip it
//	}
//	else if (head == "units") {
//		// Has factors for the units to be able to model the
//		// real person, these must be parsed correctly. Only
//		// really need to check if deg or rad, but that is 
//		// not needed for this assignment.
//
//		// We are going to assume that the units:length feild
//		// is 0.45, and that the angles are in degrees
//	}
//	else if (head == "root") {
//		// Read in information about root. Let's just assume
//		// it'll be at the origin for this assignment.
//	}
//	else if (head == "bonedata") {
//		// Read in each bone until we get to the
//		// end of the file or a new header
//		string line = nextLineTrimmed(file);
//		while (file.good() && !line.empty()) {
//			if (line[0] == ':') {
//				// finished our reading of bones
//				// read next header and return
//				return readHeading(line, file);
//			}
//			else if (line == "begin") {
//				// Read the bone data
//				readBone(file);
//			}
//			else {
//				cerr << "Expected 'begin' in bone data, found \"" << line << "\"";
//				throw runtime_error("Error :: could not parse .asf file.");
//			}
//			line = nextLineTrimmed(file);
//		}
//	}
//	else if (head == "hierarchy") {
//		// Description of how the bones fit together
//		// Read in each line until we get to the
//		// end of the file or a new header
//		string line = nextLineTrimmed(file);
//		while (file.good() && !line.empty()) {
//			if (line[0] == ':') {
//				// finished our reading of bones
//				// read next header and return
//				return readHeading(line, file);
//			}
//			else if (line == "begin") {
//				// Read the entire hierarchy
//				readHierarchy(file);
//			}
//			else {
//				cerr << "Expected 'begin' in hierarchy, found \"" << line << "\"";
//				throw runtime_error("Error :: could not parse .asf file.");
//			}
//			line = nextLineTrimmed(file);
//		}
//	}
//	else {
//		// Would normally error here, but becuase we don't parse
//		// every header entirely we will leave this blank.
//	}
//}
//
//
//void Skeleton::readBone(ifstream &file) {
//	// Create the bone to add the data to
//	bone b;
//
//	string line = nextLineTrimmed(file);
//	while (file.good()) {
//		if (line == "end") {
//			// End of the data for this bone
//			// Push the bone into the vector
//			m_bones.push_back(b);
//
//			return;
//		}
//		else {
//
//			string head;
//			istringstream lineStream(line);
//			lineStream >> head; // Get the first token
//
//			if (head == "name") {
//				// Name of the bone
//				lineStream >> b.name;
//			}
//			else if (head == "direction") {
//				// Direction of the bone
//				lineStream >> b.boneDir.x >> b.boneDir.y >> b.boneDir.z;
//				b.boneDir = normalize(b.boneDir); // Normalize here for consistency
//			}
//			else if (head == "length") {
//				// Length of the bone
//				float length;
//				lineStream >> length;
//				length *= (1.0 / 0.45);  // scale by 1/0.45 to get actual measurements
//				length *= 0.0254;      // convert from inches to meters
//				b.length = length;
//			}
//			else if (head == "dof") {
//				// Degrees of Freedom of the joint (rotation)
//				while (lineStream.good()) {
//					string dofString;
//					lineStream >> dofString;
//					if (!dofString.empty()) {
//						// Parse each dof string
//						if (dofString == "rx") b.freedom |= dof_rx;
//						else if (dofString == "ry") b.freedom |= dof_ry;
//						else if (dofString == "rz") b.freedom |= dof_rz;
//						else throw runtime_error("Error :: could not parse .asf file.");
//					}
//				}
//			}
//			else if (head == "axis") {
//				// Basis rotations 
//				lineStream >> b.basisRot.x >> b.basisRot.y >> b.basisRot.z;
//			}
//			else if (head == "limits") {
//				// Limits for each of the DOF
//				// Assumes dof has been read first
//				// You can optionally fill this method out
//			}
//
//			// Because we've tried to parse numerical values
//			// check if we've failed at any point
//			if (lineStream.fail()) {
//				cerr << "Unable to parse \"" << line << "\"";
//				throw runtime_error("Error :: could not parse .asf file.");
//			}
//		}
//
//		// Get the next line
//		line = nextLineTrimmed(file);
//	}
//
//	cerr << "Expected end in bonedata, found \"" << line << "\"";
//	throw runtime_error("Error :: could not parse .asf file.");
//}
//
//
//void Skeleton::readHierarchy(ifstream &file) {
//	string line = nextLineTrimmed(file);
//	while (file.good()) {
//		if (line == "end") {
//			// End of hierarchy
//			return;
//		}
//		else if (!line.empty()) {
//			// Read the parent node
//			string parentName;
//			istringstream lineStream(line);
//			lineStream >> parentName;
//
//			// Find the parent bone and have a pointer to it
//			int parentIndex = findBone(parentName);
//
//			if (parentIndex < 0) {
//				cerr << "Expected a valid parent bone name, found \"" << parentName << "\"" << endl;
//				throw runtime_error("Error :: could not parse .asf file.");
//			}
//
//			//Read the connections
//			string childName;
//			lineStream >> childName;
//			while (!lineStream.fail() && !childName.empty()) {
//
//				int childIndex = findBone(childName);
//
//				if (childIndex < 0) {
//					cerr << "Expected a valid child bone name, found \"" << childName << "\"" << endl;
//					throw runtime_error("Error :: could not parse .asf file.");
//				}
//
//				// Set a POINTER to the child to be recorded in the parents
//				m_bones[parentIndex].children.push_back(&m_bones[childIndex]);
//
//				// Get the next child
//				lineStream >> childName;
//			}
//		}
//		line = nextLineTrimmed(file);
//	}
//	cerr << "Expected end in bonedata, found \"" << line << "\"";
//	throw runtime_error("Error :: could not parse .asf file.");
//}
//
//
//
////-------------------------------------------------------------
//// [Assignment 2] :
//// Complete the following method to load data from an *.amc file
////-------------------------------------------------------------
//void Skeleton::readAMC(string filename) {
//	ifstream file(filename);
//	if (!file.is_open()) {
//		cerr << "Failed to open file " << filename << endl;
//		throw runtime_error("Error :: could not open file.");
//	}
//
//	cout << "Reading file" << filename << endl;
//
//	// good() means that failbit, badbit and eofbit are all not set
//	while (file.good()) {
//
//		// Pull out line from file
//		string line = nextLineTrimmed(file);
//		istringstream lineStream(line);
//
//		// Check if it is a comment or just empty
//		if (line.empty() || line[0] == '#' || line[0] == ':') {
//			continue;
//		}
//		else {
//			string name;
//			lineStream >> name;
//			for (int i = 0; i < m_bones.size(); i++) {
//				if (m_bones[i].name == name) {
//					string token;
//					vector<double> frame = vector<double>();
//					while (lineStream.fail() == false) {
//						double angle;
//						lineStream >> token;
//						angle = stod(token);
//
//
//						if (m_bones[i].freedom & dof_rx && frame.size() < 1) {
//							frame.push_back(angle);
//						}
//						else
//
//							if (m_bones[i].freedom & dof_ry && frame.size() < 2) {
//								frame.push_back(angle);
//							}
//							else
//
//
//								if (m_bones[i].freedom & dof_rz && frame.size() < 3) {
//									frame.push_back(angle);
//								}
//								else
//
//
//									if (m_bones[i].freedom & dof_root && frame.size() < 6) {
//										frame.push_back(angle);
//									}
//
//					}
//					m_bones[i].rotations.push_back(frame);
//				}
//			}
//		}
//
//	}
//
//	cout << "Completed reading AMC file" << endl;
//}
//
//// YOUR CODE GOES HERE
//// ...
//
//
//
//



