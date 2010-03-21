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
		BOOST_FOREACH(const int& i, neighborCameras.find(step)->second)
				cameraColorsNow[i].alpha = 1.0;
	}

	canvas->update();

	// find the next frame that has visibility information
	while(true) {
		step == visibileFeatures.size() ? step = 0 : step++;
		if (visibileFeatures.find(step) != visibileFeatures.end())
			break;
	}
}

/* ************************************************************************* */
void sfmviewer::setup()
{
	// load 3d
	load3D();

	// load the visibility file
	loadVisibility();

	// set the default view port for St. Peter
	ViewPort viewport(-1.93, -57.07, -200, -0.292426, 0.0699906, 0.0326308, 0.953166);
	canvas->setViewPort(viewport);

	// set up the timer for pluging in the visibility data
	pointColorsNow = pointColors;
	cameraColorsNow = cameraColors;
	window->setTimerFunc(advanceFrame);
	window->startTimer(200);
}

/* ************************************************************************* */
void sfmviewer::draw() {
	// draw inactive world
	drawStructure(structure, pointColorsNow);
	drawCameras(cameras, cameraColorsNow);
}

