/*
 * kai01.cpp
 *
 *   Created on: Mar 18, 2010
 *       Author: nikai
 *  Description: the most simpler viewer
 */
#include <fstream>

#include "main.h"
#include "trackball.h"


using namespace std;
using namespace gtsam;
using namespace sfmviewer;

#define LINESIZE 81920
static string filename = "/Users/nikai/borg/sfmviewer/data/StPeter.txt";

static vector<Vertex> structure;
static vector<VertexColor> pointColors;
static vector<CameraVertices> cameras;

void sfmviewer::setup(SFMViewer& window)
{
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
			pointColors.push_back(VertexColor((unsigned char)(r*255)
					,(unsigned char)(g*255),(unsigned char)(b*255)));
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

	// set view port
	ViewPort viewport(-1.93, -57.07, -200, -0.292426, 0.0699906, 0.0326308, 0.953166);
	//ViewPort viewport(-0., -5., -10., 0., 0., 0., 1.);
	window.canvas()->setViewPort(viewport);
}

void sfmviewer::draw(GLCanvas& canvas) {
	drawStructure(structure, pointColors);
	drawCameras(cameras);
}
