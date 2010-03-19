/*
 * GLCanvas.h
 *
 *   Created on: Mar 18, 2010
 *       Author: nikai
 *  Description: an OpenGL canvas
 */

#pragma once

#include <boost/function.hpp>
#include <boost/optional.hpp>
#include <QGLWidget>
#include <gtsam/SimpleCamera.h>

namespace sfmviewer {

	// the data structure for the view port
	struct ViewPort {
		float m_shift[3];  // translation
		float m_quat[4];   // orientation in quaternion format
		ViewPort() {}
		ViewPort(float x, float y, float z, float q1, float q2, float q3, float q4) {
			m_shift[0] = x; m_shift[1] = y; m_shift[2] = z;
			m_quat[0] = q1; m_quat[1] = q2; m_quat[2] = q3; m_quat[3] = q4;
		}

	};

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

	class GLCanvas : public QGLWidget
	{
		Q_OBJECT

	public:

		// callback function types
		typedef boost::function<void (GLCanvas&)> Callback;

		// constructor
		GLCanvas(QWidget *parent = 0);

		// destructor
		~GLCanvas();

		// return the minimal size of the window
		QSize minimumSizeHint() const;

		// Used by windowing system, uses globals set by size
		QSize sizeHint() const;

		// Called from window.cpp to set default size
		void setSizeHint(int w, int h);

		// set up users' callback functions
		void setCallback(const Callback& fun_draw) { fun_draw_ = fun_draw; }

		void setViewPort(const ViewPort& port) { viewPort_ = port; }

	protected:
		// intialize the opengl canvas
		void initializeGL();

		// paint function
		void paintGL();

		// callback when the window is resized
		void resizeGL(int width, int height);

		// load the bunny example
		void drawBunny();

		// record the last mouse position
		void mousePressEvent(QMouseEvent *event);

		// mouse click and drag
		void mouseMoveEvent(QMouseEvent *event);

	private:
		QPoint lastPos_;

		int hintWidth_;
		int hintHeight_;

		// the current opengl viewport
		ViewPort viewPort_;

		// the pointers of callback functions
		Callback fun_draw_;
	};

	// draw the 3D structure
	void drawStructure(const std::vector<Vertex>& structure,
			const std::vector<VertexColor>& pointColors = std::vector<VertexColor>());

	// draw cameras
	void drawCameras(const std::vector<CameraVertices>& cameras);

	// backproject four corners of the image to the system coordinate
	CameraVertices calcCameraVertices(const gtsam::SimpleCamera& camera, const int img_w = 800, const int img_h = 800);

} // namespace sfmviewer
