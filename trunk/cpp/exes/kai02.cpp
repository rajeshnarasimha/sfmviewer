/*
 * kai02.cpp
 *
 *   Created on: Mar 20, 2010
 *       Author: nikai
 *  Description: the viewer for the visibility
 */

#include <fstream>
#include <boost/foreach.hpp>

#include "main.h"
#include "trackball.h"

using namespace std;
using namespace gtsam;
using namespace sfmviewer;

#define LINESIZE 81920
static const string filename = "/Users/nikai/borg/sfmviewer/data/StPeter.txt";
static const string visibility_filename = "/Users/nikai/borg/sfmviewer/data/StPeter_visibility.txt";

/**
 * 3D world and the visibilities
 */
static vector<Vertex> structure;             // 3d points
static vector<SFMColor> pointColors;         // the colors of 3d points
static vector<CameraVertices> cameras;       // 3d cameras
static vector<SFMColor> cameraColors;        // the colors of 3d cameras
static map<int, vector<int> > visibileFeatures; // the indices of visible features
static map<int, vector<int> > neighborCameras;  // the indices of neighbor cameras

/**
 * information about the current frame
 */
static size_t step = 0;
static vector<SFMColor> pointColorsNow;
static vector<SFMColor> cameraColorsNow;

/* ************************************************************************* */
void load3D() {
	ifstream is(filename.c_str());
	string tag;
	while (is) {
		is >> tag;

		// load 3D points
		if (tag == "POINT3") {
			double x, y, z, r, g, b;
			is >> x >> y >> z >> r >> g >> b;
			structure.push_back(Vertex(x,y,z));
			pointColors.push_back(SFMColor(r, g, b, 0.2));
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
			cameraColors.push_back(SFMColor(default_camera_color.r, default_camera_color.g, default_camera_color.b, 0.2));
		}

		is.ignore(LINESIZE, '\n');
	}
	is.close();
	cout << "loaded " << structure.size() << " points and " << cameras.size() << " cameras" << endl;
	cout.flush();
}

/* ************************************************************************* */
void loadVisibility() {
	ifstream is(visibility_filename.c_str());
	while (is) {
		int id, numFeatures, numCameras;
		is >> id >> numFeatures >> numCameras;

		// load the indices of visible features
		int index;
		vector<int> featureIndices;
		for (int i=0; i<numFeatures; i++) {
			is >> index;
			featureIndices.push_back(index-1);
		}
		visibileFeatures.insert(make_pair(id-1, featureIndices));

		// load the indices of neighbor cameras
		vector<int> cameraIndices;
		for (int i=0; i<numCameras; i++) {
			is >> index;
			cameraIndices.push_back(index-1);
		}
		neighborCameras.insert(make_pair(id-1, cameraIndices));

		is.ignore(LINESIZE, '\n');
	}
	is.close();
	cout << "loaded " << visibileFeatures.size() << " frames of visibilities" << endl;
	cout.flush();
}

/* ************************************************************************* */
// show the visibility of the next frame
void advanceFrame() {
	// change point colors
	pointColorsNow = pointColors;
	if (step >=0 && step < visibileFeatures.size()) {
		BOOST_FOREACH(const int& i, visibileFeatures.find(step)->second)
			pointColorsNow[i].alpha = 1.0;
	}

	// change camera colors
	cameraColorsNow = cameraColors;
	if (step >=0 && step < neighborCameras.size()) {
		cameraColorsNow[step] = SFMColor(1.0, 0.0, 0.0, 1.0);
		BOOST_FOREACH(const int& i, neighborCameras.find(step)->second)
				cameraColorsNow[i].alpha = 1.0;
	}

	canvas->updateGL();

	// find the next frame that has visibility information
	while(true) {
		step == visibileFeatures.size() ? step = 0 : step++;
		if (visibileFeatures.find(step) != visibileFeatures.end())
			break;
	}
}

/* ************************************************************************* */
static float orbit_center_x = 0.;    // the center of the orbit
static float orbit_center_z = 200.;  // the center of the orbit
static float orbit_radius = 50.0;   // the radius of the camera's orbit circle
static float orbit_height = -100.0;   // the height of the orbit
static int orbit_segments = 50;     // how many steps to traverse the orbit
static int orbit_step = 0;           // the current step in the orbit


ViewPort Pose2ViewPort(const float q[4], const float trans[3]) {
	ViewPort port;
	return port;
}

static vector<Pose3> camerasOrbit;
void computeOrbitCameras() {
	for(int i=0; i<orbit_segments; i++) {
		float angle = -M_PI_2 + (float)i / orbit_segments * M_PI * 2;

		float theta = -(M_PI_4 + angle / 2);
		float q1[4] = {0., sin(theta), 0., cos(theta)};
		float beta = -M_PI_4 * 0.6;
		float q2[4] = {sin(beta), 0., 0., cos(beta)};
		float q[4];
		add_quats(q2, q1, q);
		float r[4][4];
		build_rotmatrix(r, q);

		// compute the translation in the new rotated coordinate system
		float x = cos(angle) * orbit_radius + orbit_center_x;
		float z = sin(angle) * orbit_radius + orbit_center_z;
		float trans[3] = {x, orbit_height, z};

		Pose3 pose(Rot3(r[0][0], r[0][1], r[0][2],
				r[1][0], r[1][1], r[1][2],
				r[2][0], r[2][1], r[2][2]),
				Point3(trans[0], trans[1], trans[2]));
		camerasOrbit.push_back(pose);
	}
	cout << camerasOrbit.size() << " cameras in the orbit" << endl;
}

// move the camera around an orbit
void moveCamera() {
	// the current position on the orbit
	int segment = orbit_step % orbit_segments;
	float angle = -M_PI_2 + (float)segment / orbit_segments * M_PI * 2;

	// compute the rotation of the current point on the orbit, refer to quaternions.lyx
	//float beta = atan2(orbit_height, orbit_radius) / 2;
	float theta = -(M_PI_4 + angle / 2);
	float beta = 0.05;
	float q1[4] = {0., sin(theta), 0., cos(theta)};
	float q2[4] = {sin(beta), 0., 0., cos(beta)};
	float q[4];
	add_quats(q1, q2, q);

	// compute the translation in the new rotated coordinate system
	float x = cos(angle) * orbit_radius + orbit_center_x;
	float z = sin(angle) * orbit_radius + orbit_center_z;
	float trans[3] = {x, orbit_height, z}, transOrbit[3];
	transformByRotation(trans, q, transOrbit);

	// set up the new viewport
	//ViewPort viewport(transOrbit[0], transOrbit[1], transOrbit[2], q[0], q[1], q[2], q[3]);
	ViewPort viewport(0, 0, 0, q[0], q[1], q[2], q[3]);
	canvas->setViewPort(viewport);
	canvas->updateGL();
	orbit_step ++;
}

/* ************************************************************************* */
void sfmviewer::setup()
{
	// load 3d
	load3D();

	// load the visibility file
	loadVisibility();

	// set the default view port for St. Peter
//	ViewPort viewport(-1.93, -57.07, -200, -0.292426, 0.0699906, 0.0326308, 0.953166);
	ViewPort viewport(-31, -94, 91, 0., 0., 0., 1.);
	canvas->setViewPort(viewport);

	// set the top view port
	float trans[3] = {0., -300., 200.}, transTop[3];
	float q[4] = {-1./sqrt(2.), 0., 0., 1./sqrt(2.)}; // top view
	transformByRotation(trans, q, transTop);
	canvas->setTopViewPort(ViewPort(transTop[0], transTop[1], transTop[2], q[0], q[1], q[2], q[3]));

	// set up the timer for pluging in the visibility data
	pointColorsNow = pointColors;
	cameraColorsNow = cameraColors;
	canvas->addTimer(advanceFrame, 200);

	// set up the timer for moving the opengl camera
	//canvas->addTimer(moveCamera, 10);
	computeOrbitCameras();
}

/* ************************************************************************* */
void sfmviewer::draw() {
	// draw inactive world
	drawStructure(structure, pointColorsNow);
	drawCameras(cameras, cameraColorsNow);
	//drawCameras(camerasOrbit);
	drawRGBCameras(camerasOrbit, 3.0);
}

