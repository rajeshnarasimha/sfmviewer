/*
 * kai01.cpp
 *
 *   Created on: Mar 18, 2010
 *       Author: nikai
 *  Description: the most simple viewer
 */
#include <fstream>
#include <gtsam/geometry/SimpleCamera.h>

#include "render-inl.h"
#include "main.h"

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
	bool firstCamera = true;
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

			if (firstCamera) {	camera.calibration().print("calibration"); firstCamera = false; }
			cameras.push_back(calcCameraVertices<SimpleCamera, Point2, Point3>(camera, 1600, 1600, 7.));
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

	// set the default camera pose for St. Peter
	canvas->setGLPose(QuatPose(119., -257., -100., -0.341, -0.223, -0.081, 0.909));
	//canvas->setGLPose(QuatPose(0., 0., 0., 0., 0., 0., 1.));

	// set the top view camera pose
	canvas->setGLPoseTop(QuatPose(0., -500., 200., -1./sqrt(2.), 0., 0., 1./sqrt(2.)));
}

void sfmviewer::draw() {
	drawStructure(structure, pointColors);
	drawCameras(cameras);
//	drawCameraCircle();
}
