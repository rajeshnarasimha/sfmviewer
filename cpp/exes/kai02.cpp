/*
 * kai02.cpp
 *
 *   Created on: Mar 20, 2010
 *       Author: nikai
 *  Description: the viewer for the visibility
 */

#include <fstream>
#include <boost/foreach.hpp>
#include <QDir>

#include "main.h"
#include "trackball.h"

using namespace std;
using namespace gtsam;
using namespace sfmviewer;

#define LINESIZE 81920
static const string filename = "/Users/nikai/borg/sfmviewer/data/StPeter.txt";
static const string visibility_filename = "/Users/nikai/borg/sfmviewer/data/StPeter_visibility.txt";
static const float slow_motion = 2.2;

/**
 * 3D world and the visibilities
 */
static vector<Vertex> structure;             // 3d points
static vector<SFMColor> pointColors;         // the colors of 3d points
static vector<CameraVertices> cameras;       // 3d cameras
static vector<SFMColor> cameraColors;        // the colors of 3d cameras
static map<int, vector<int> > visibileFeatures; // the indices of visible features
static map<int, vector<int> > neighborCameras;  // the indices of neighbor cameras
static const SFMColor camera_color(0., 1., 0., 1.);

/**
 * camera motions
 */
static float orbit_center_x = 0.;     // the center of the orbit
static float orbit_center_z = 200.;   // the center of the orbit
static float orbit_radius = 300.0;    // the radius of the camera's orbit circle
static float orbit_height = -200.0;   // the height of the orbit
static int orbit_segments = 720;      // how many steps to traverse the orbit
static int orbit_step = 250;          // the current step in the orbit

/**
 * information about the current frame
 */
static size_t step = 0;
static size_t step_size = 1;
static vector<SFMColor> pointColorsNow;
static vector<SFMColor> cameraColorsNow;

/**
 * thumbnails
 */
static vector<string> thumbnailNames;
static GLuint queryTexID = 0;
static vector<GLuint> nnTexIDs;
static float window_scale = 1;
static int thumbnail_width = window_scale * 175, thumbnail_height = window_scale *  117;
static int thumbnail_space = window_scale * 28;

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
			r = 0.5; g = 0.5; b = 0.5;
			pointColors.push_back(SFMColor(r, g, b, 1.0));
		}

		// load 3D camera
		if (tag == "POSE3") {
			double x, y, z, r11, r12, r13, r21, r22, r23, r31, r32, r33;
			is >> x >> y >> z
				 >> r11 >> r21 >> r31
				 >> r12 >> r22 >> r32
				 >> r13 >> r23 >> r33;
			Pose3 pose(Rot3(r11, r12, r13, r21, r22, r23, r31, r32, r33), Point3(x, y, z));
			SimpleCamera camera(Cal3_S2(120., 1600, 1600), pose);
			cameras.push_back(calcCameraVertices(camera, 1600, 1600, 7.0));
			cameraColors.push_back(SFMColor(camera_color.r, camera_color.g, camera_color.b, 0.2));
		}

		is.ignore(LINESIZE, '\n');
	}
	is.close();
	cout << "loaded " << structure.size() << " points and " << cameras.size() << " cameras" << endl;
	cout.flush();

	QDir dir(QString("/Users/nikai/borg/visibility/video/images"));
	QStringList nameFilters;
	nameFilters << "*.png";
	QStringList thumbnailNames_ = dir.entryList(nameFilters, QDir::NoFilter, QDir::Name);
	BOOST_FOREACH (const QString& name, thumbnailNames_) {
		thumbnailNames.push_back((dir.absolutePath() + "/" + name).toStdString());
	}
	cout << "loaded " << thumbnailNames.size() << " thumbnails" << endl;
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
void nextVisibility() {
	// change camera colors
	cameraColorsNow = cameraColors;
	if (step >=0 && step < neighborCameras.size()) {
		cameraColorsNow[step] = SFMColor(1.0, 0.0, 0.0, 1.0);
		BOOST_FOREACH(const int& i, neighborCameras.find(step)->second)
			cameraColorsNow[i].alpha = 1.0;
	}

	// change point colors
	pointColorsNow = pointColors;
	if (step >=0 && step < visibileFeatures.size()) {
		BOOST_FOREACH(const int& i, visibileFeatures.find(step)->second) {
			pointColorsNow[i].r     = 1.0;
			pointColorsNow[i].g     = 0.55;
			pointColorsNow[i].b     = 0.15;
			pointColorsNow[i].alpha = 1.0;
		}
	}

	// load thumbnails
	if (queryTexID!=0) glDeleteTextures(1, &queryTexID);
	BOOST_FOREACH(const GLuint& id, nnTexIDs)
		glDeleteTextures(1, &id);
	nnTexIDs.clear();

	QImage image(QString::fromStdString(thumbnailNames[step]));
	if (image.width() != 128 || image.height() != 128) image = image.scaled(QSize(128, 128));
	queryTexID = loadThumbnailTexture(image);

	if (step >=0 && step < neighborCameras.size()) {
		vector<int> nns = neighborCameras.find(step)->second;
		size_t numNN = nns.size() > 4 ? 4 : nns.size();
		for (size_t i=0; i<numNN; i++) {
			QImage image(QString::fromStdString(thumbnailNames[nns[i]]));
			nnTexIDs.push_back(loadThumbnailTexture(image));
		}
	}

	// update opengla canvas
	canvas->updateGL();

	// quit if all the data has been processed
	if (step + step_size >= visibileFeatures.size())
		app->quit();

	// find the next frame that has visibility information
	while(true) {
		step >= visibileFeatures.size() ? step = 0 : step += step_size;
		if (visibileFeatures.find(step) != visibileFeatures.end())
			break;
	}
}

/* ************************************************************************* */
ViewPort computeOrbitCamera(const int segment) {
	float angle = -M_PI_2 + (float)segment / orbit_segments * M_PI * 2;

	// compute the rotation of the current point on the orbit, refer to quaternions.lyx
	//float beta = atan2(orbit_height, orbit_radius) / 2;
	float theta = -(M_PI_4 + angle / 2);
	float beta = -M_PI_4 * 0.4;
	float q1[4] = {0., sin(theta), 0., cos(theta)};
	float q2[4] = {sin(beta), 0., 0., cos(beta)};
	float q[4];
	add_quats(q2, q1, q);

	// compute the translation in the new rotated coordinate system
	float x = cos(angle) * orbit_radius + orbit_center_x;
	float z = sin(angle) * orbit_radius + orbit_center_z;
	float trans[3] = {x, orbit_height, z};

	return ViewPort(trans[0], trans[1], trans[2], q[0], q[1], q[2], q[3]);
}

// move the camera around an orbit
void moveCamera() {
	// the current position on the orbit
	int segment = orbit_step % orbit_segments;
	canvas->setViewPort(computeOrbitCamera(segment));
	canvas->updateGL();
	orbit_step ++;
}

/* ************************************************************************* */
void sfmviewer::setup()
{
	// set window size
	width = 1024;
	height = 768;

	// load 3d
	load3D();

	// load the visibility file
	loadVisibility();

	// set the default view port for St. Peter
//	canvas->setViewPort(ViewPort(119., -257., -100., -0.341, -0.223, -0.081, 0.909));
	canvas->setViewPort(computeOrbitCamera(orbit_step));

	// set the top view port
	canvas->setTopViewPort(ViewPort(0., -500., 200., -1./sqrt(2.), 0., 0., 1./sqrt(2.)));

	// set up the timer for pluging in the visibility data
	pointColorsNow = pointColors;
	cameraColorsNow = cameraColors;
	canvas->addTimer(nextVisibility, slow_motion * 100);

	// set up the timer for moving the opengl camera
	canvas->addTimer(moveCamera, slow_motion * 60);
}

/* ************************************************************************* */
void sfmviewer::draw() {
	// draw inactive world
	drawStructure(structure, pointColorsNow);
	drawCameras(cameras, cameraColorsNow, false);
//	drawCameraCircle();

	int left = window_scale * 17;
	if (!nnTexIDs.empty()) {
		drawThumbnail(queryTexID, canvas->size(), QRectF(left, 10., thumbnail_width, thumbnail_height), SFMColor(1.,0.,0.,1.));
		BOOST_FOREACH(const GLuint& id, nnTexIDs) {
			left += thumbnail_width + thumbnail_space;
			drawThumbnail(id, canvas->size(), QRectF(left, 10., thumbnail_width, thumbnail_height), SFMColor(0.,1.,0.,1.));
		}
	}
}

