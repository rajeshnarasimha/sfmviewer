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
//#include <gtsam/geometry/SimpleCamera.h>
#include <QRectF>
#include <QImage>

namespace sfmviewer {

  // the data structure for 3D points
	struct Vertex{
		GLfloat X,Y,Z;
		Vertex(GLfloat X0, GLfloat Y0, GLfloat Z0) : X(X0), Y(Y0), Z(Z0) {}
		Vertex() {}
	};

	// the data structure for the colors of 3D points
	struct SFMColor{
		GLfloat r, g, b, alpha;
		SFMColor(GLfloat r0, GLfloat g0, GLfloat b0, GLfloat alpha0) : r(r0), g(g0), b(b0), alpha(alpha0) {}
	};

	const SFMColor default_camera_color(240.f/255.f, 140.0f/255.f, 24.0f/255.f, 1.f);

	// a camera is composed of five vertices
	struct CameraVertices{
		Vertex v[5];
	};

	// draw the 3D structure
	void drawStructure(const std::vector<Vertex>& structure,
			const std::vector<SFMColor>& pointColors = std::vector<SFMColor>());

	// draw cameras
	void drawCameras(const std::vector<CameraVertices>& cameras,
			const std::vector<SFMColor>& cameraColors = std::vector<SFMColor>(), const bool fill = true);

	// draw a rgb cameras
	template<class Pose3>
	void drawRGBCamera(const Pose3& pose, const GLfloat linewidth = 1.0, const float scale = 1.0);

	// draw rgb cameras
	template<class Pose3>
	void drawRGBCameras(const std::vector<Pose3>& poses, const GLfloat linewidth = 1.0, const float scale = 1.0);

	// backproject four corners of the image to the system coordinate
	template<class Camera>
	CameraVertices calcCameraVertices(const Camera& camera, const int img_w = 800, const int img_h = 800,
			const float scale = 1.0);

	// load a texture to opengl
	GLuint loadThumbnailTexture(const QImage& image);;

	// draw a texture specified by {texID} in the rectangle {rect}
	void drawThumbnail(const GLuint texID, const QSize& size, const QRectF& rect, const SFMColor& color);

	// draw the bunny example
	void drawBunny();

	// draw a circle of cameras;
	template<class Pose3, class Calibration>
	void drawCameraCircle();

} // namespace sfmviewer
