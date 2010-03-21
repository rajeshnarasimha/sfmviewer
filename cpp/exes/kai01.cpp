/*
 * kai01.cpp
 *
 *   Created on: Mar 18, 2010
 *       Author: nikai
 *  Description: the most simple viewer
 */
#include <fstream>

#include "main.h"
#include "trackball.h"

using namespace std;
using namespace gtsam;
using namespace sfmviewer;

#define LINESIZE 81920
static string filename = "/Users/nikai/borg/sfmviewer/data/StPeter.txt";

static vector<Vertex> structure;             // 3d points
static vector<SFMColor> pointColors;      // the colors of 3d points
static vector<CameraVertices> cameras;       // 3d cameras

void load3d() {
	// load the files
	ifstream is(filename.c_str());
	string tag;
	while (is) {
		is >> tag;

		// load 3D points
		if (tag == "POINT3") {
			double x, y, z, r, g, b;
			is >> x >> y >> z >> r >> g >> b;
			structure.push_back(Vertex(x,y,z));
			pointColors.push_back(SFMColor(r, g, b, 1.0));
		}

		// load 3D camera
		if (tag == "POSE3") {
			double x, y, z, r11, r12, r13, r21, r22, r23, r31, r32, r33;
			is >> x >> y >> z
			>> r11 >> r12 >> r13
			>> r21 >> r22 >> r23
			>> r31 >> r32 >> r33;
			Pose3 pose(Rot3(r11, r12, r13, r21, r22, r23, r31, r32, r33), Point3(x, y, z));
			SimpleCamera camera(Cal3_S2(120., 1600, 1600), pose);
			cameras.push_back(calcCameraVertices(camera, 1600, 1600));
		}

		is.ignore(LINESIZE, '\n');
	}
	is.close();
	cout << "loaded " << structure.size() << " points and " << cameras.size() << " cameras" << endl;
	cout.flush();
}

void sfmviewer::setup()
{
	load3d();

	// set the default view port for St. Peter
	ViewPort viewport(-1.93, -57.07, -200, -0.292426, 0.0699906, 0.0326308, 0.953166);
	canvas->setViewPort(viewport);
}

void sfmviewer::draw() {
	drawStructure(structure, pointColors);
	drawCameras(cameras);
}
