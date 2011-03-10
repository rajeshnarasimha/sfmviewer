/*
 * kai_bunny.cpp
 *
 *   Created on: Mar 10, 2011
 *       Author: nikai
 *  Description:
 */

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
using namespace sfmviewer;

static vector<Vertex> structure;             // 3d points
static vector<SFMColor> pointColors;      // the colors of 3d points
static vector<CameraVertices> cameras;       // 3d cameras

void load3d() {
}

void sfmviewer::setup()
{
	canvas->setGLPose(QuatPose(0., 0., -1., 0., 0., 0., 1.));
}

void sfmviewer::draw() {
	drawBunny();
}
