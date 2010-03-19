/*
 * render.h
 *
 *   Created on: Mar 19, 2010
 *       Author: nikai
 *  Description: the rendering functions for different elements
 */

#pragma once

#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <gtsam/SimpleCamera.h>

namespace sfmviewer {

  // the data structure for 3D points
	struct Vertex{
		GLfloat X,Y,Z;
		Vertex(GLfloat X0, GLfloat Y0, GLfloat Z0) : X(X0), Y(Y0), Z(Z0) {}
		Vertex() {}
	};

	// the data structure for the colors of 3D points
	struct VertexColor{
		unsigned char r,g,b;
		VertexColor(unsigned char r0, unsigned char g0, unsigned char b0) : r(r0), g(g0), b(b0) {}
	};

	// a camera is composed of five vertices
	struct CameraVertices{
		Vertex v[5];
	};

	// draw the 3D structure
	void drawStructure(const std::vector<Vertex>& structure,
			const std::vector<VertexColor>& pointColors = std::vector<VertexColor>());

	// draw the bunny example
	void drawBunny();

	// draw cameras
	void drawCameras(const std::vector<CameraVertices>& cameras);

	// backproject four corners of the image to the system coordinate
	CameraVertices calcCameraVertices(const gtsam::SimpleCamera& camera, const int img_w = 800, const int img_h = 800);

} // namespace sfmviewer
