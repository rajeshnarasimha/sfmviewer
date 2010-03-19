/*
 * GLCanvas.h
 *
 *   Created on: Mar 18, 2010
 *       Author: nikai
 *  Description: an OpenGL canvas
 */

#include <stdexcept>
#include <iostream>
#include <math.h>
#include <QtGui>
#include <QtOpenGL>
#include <boost/foreach.hpp>

#include "GLCanvas.h"
#include "bunny.h"
#include "trackball.h"

#define SFM_POINT_COLOR          1.0f, 1.0f, 1.0f, 1.0f
#define SFM_BACKGROUND_COLOR     0.0f, 0.0f, 0.0f, 1.0f
#define SFM_CAMERA_COLOR  240.f/255.f, 140.0f/255.f, 24.0f/255.f,  1.0f

#define DRAWONERECT(X1,Y1,Z1,X2,Y2,Z2,X3,Y3,Z3,X4,Y4,Z4) glColor4f(SFM_CAMERA_COLOR); glEnable(GL_BLEND); glBlendFunc(/*GL_ONE_MINUS*/GL_SRC_COLOR,GL_DST_COLOR/*GL_ONE_MINUS_DST_COLOR*/); glBegin(GL_POLYGON);\
	glVertex3f(X1,Y1,Z1); glVertex3f(X2,Y2,Z2); glVertex3f(X3,Y3,Z3); glVertex3f(X4,Y4,Z4); glEnd(); glDisable(GL_BLEND);

using namespace std;

namespace sfmviewer {

	/* ************************************************************************* */
	// the conversion matrix from OpenGL default coordinate system
	//  to the camera coordiante system:
	// [ 1  0  0  0] * [ x ] = [ x ]
	//   0 -1  0  0      y      -y
	//   0  0 -1  0      z      -z
	//   0  0  0  1      1       1
	const GLfloat m_convert[16] = {
			1.f,  0.f,  0.f, 0.f,
			0.f, -1.f,  0.f, 0.f,
			0.f,  0.f, -1.f, 0.f,
			0.f,  0.f,  0.f, 1.f};

	/* ************************************************************************* */
	GLCanvas::GLCanvas(QWidget *parent)
	: QGLWidget(QGLFormat(QGL::SampleBuffers), parent)
	{
		// initial view port
		viewPort_.m_shift[0]    =  0.0f;
		viewPort_.m_shift[1]    =  0.0f;
		viewPort_.m_shift[2]    =  -5.0f;
		trackball(viewPort_.m_quat, 0.0f, 0.0f, 0.0f, 0.0f);
	}

	/* ************************************************************************* */
	GLCanvas::~GLCanvas()
	{
	}

	/* ************************************************************************* */
	QSize GLCanvas::minimumSizeHint() const
	{
		return QSize(50, 50);
	}

	/* ************************************************************************* */
	QSize GLCanvas::sizeHint() const {
		return QSize(hintWidth_, hintHeight_);
	}

	/* ************************************************************************* */
	void GLCanvas::setSizeHint(int w, int h) {
		hintWidth_ = w;
		hintHeight_ = h;
		updateGeometry();
	}

	/* ************************************************************************* */
	void GLCanvas::drawBunny() {
		vector<Vertex> structure;
		Vertex v;
		for(int i=0;i<bunny_nr_vertex;i++)
		{
			v.X = (GLfloat)bunny_vertices[i][0];
			v.Y = -(GLfloat)bunny_vertices[i][1];
			v.Z = (GLfloat)bunny_vertices[i][2];
			structure.push_back(v);
		}
		drawStructure(structure);
	}

	/* ************************************************************************* */
	void GLCanvas::initializeGL()
	{
		/* remove back faces */
		glDisable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
		/* speedups */
		glEnable(GL_DITHER);
		glShadeModel(GL_SMOOTH);
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
		glHint(GL_POLYGON_SMOOTH_HINT, GL_FASTEST);
	}

	/* ************************************************************************* */
	void GLCanvas::paintGL()
	{
		// Transformations
		glMatrixMode( GL_MODELVIEW);
		glLoadIdentity();
		GLfloat trackball[4][4];
		build_tran_matrix(trackball, viewPort_.m_quat, viewPort_.m_shift[0],
				viewPort_.m_shift[1], viewPort_.m_shift[2]);
		glMultMatrixf((GLfloat*) trackball);
		glMultMatrixf(m_convert);

		// background
		glClearColor(SFM_BACKGROUND_COLOR);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		fun_draw_(*this);

		glFlush();
	}

	/* ************************************************************************* */
	void GLCanvas::resizeGL(int width, int height)
	{
		glViewport(0, 0, (GLint)width, (GLint)height);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(60.0, (GLfloat)width/height, 0.001, 1000.0); // the 3rd parameter
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
	}

	/* ************************************************************************* */
	void GLCanvas::mousePressEvent(QMouseEvent *event) {
		lastPos_ = event->pos();
	}

	/* ************************************************************************* */
	void GLCanvas::mouseMoveEvent(QMouseEvent *event) {
		float m_shift_step =  0.01f;

		// left click to rotate
		if (event->buttons() & Qt::LeftButton)
		{
			QSize ws = size();
			float spin_quat[4];
			trackball(spin_quat,
					 (2.0*lastPos_.x() - ws.width())  / ws.width(),
					-(2.0*lastPos_.y() - ws.height()) / ws.height(),
					 (2.0*event->x()   - ws.width())  / ws.width(),
					-(2.0*event->y()   - ws.height()) / ws.height());
			add_quats(spin_quat, viewPort_.m_quat, viewPort_.m_quat);
		}
		// right click to translate
		else if (event->buttons() & Qt::RightButton)
		{
			float dx = event->x()-lastPos_.x();
			float dy = lastPos_.y()-event->y();
			viewPort_.m_shift[0] += dx*m_shift_step;
			viewPort_.m_shift[1] += dy*m_shift_step;
		}
		// middle click to zoom in and zoom out
		else if (event->buttons() & Qt::MidButton)
		{
			float dx = event->x()-lastPos_.x();
			float dy = lastPos_.y()-event->y();
			if(fabs(dy)>fabs(dx))
				viewPort_.m_shift[2] += (dy/fabs(dy))*m_shift_step*10;
		}
		lastPos_ = event->pos();
		updateGL();

//		cout << viewPort_.m_shift[0] << ", " << viewPort_.m_shift[1] << ", " << viewPort_.m_shift[2] << ", "
//         << viewPort_.m_quat[0] << ", " << viewPort_.m_quat[1] << ", " << viewPort_.m_quat[2] << ", " << viewPort_.m_quat[3] << endl;
//		cout.flush();
	}

	/* ************************************************************************* */
	void drawStructure(const vector<Vertex>& structure, const vector<VertexColor>& pointColors) {

		if (!structure.empty())
		{
			// set default color
			glColor4f(SFM_POINT_COLOR);

			// set points to draw
			glEnableClientState( GL_VERTEX_ARRAY);
			glVertexPointer(3, GL_FLOAT, 0, (GLvoid*) &structure[0]);

			// set colors if available
			if (!pointColors.empty()) {
				if (pointColors.size() != structure.size())
					throw runtime_error("DrawStructure: no. of colors != no. of points");
				glEnableClientState(GL_COLOR_ARRAY);
				glColorPointer(3,GL_UNSIGNED_BYTE, 0, (GLvoid*)&pointColors[0]);
			}

			// draw the points
			glDrawArrays(GL_POINTS, 0, structure.size());
			glDisableClientState(GL_VERTEX_ARRAY);
		}
	}

	/* ************************************************************************* */
	inline void DrawOneLine(GLfloat X1, GLfloat Y1, GLfloat Z1,
			GLfloat X2, GLfloat Y2, GLfloat Z2,
			float r, float g, float b, float alpha, GLfloat linewidth = 1)
	{
	    glColor4f(r,g,b,alpha);
	    glLineWidth(linewidth);
	    glBegin(GL_LINES);
	    glVertex3f(X1,Y1,Z1);
	    glVertex3f(X2,Y2,Z2);
	    glEnd();
	}

	/* ************************************************************************* */
	void drawCamera(const Vertex* pv, const float r, const float g, const float b, const float alpha, const GLfloat linewidth, bool fill) {
    DrawOneLine(pv[0].X,pv[0].Y,pv[0].Z, pv[1].X,pv[1].Y,pv[1].Z, r, g, b, alpha, linewidth);
    DrawOneLine(pv[0].X,pv[0].Y,pv[0].Z, pv[2].X,pv[2].Y,pv[2].Z, r, g, b, alpha, linewidth);
    DrawOneLine(pv[0].X,pv[0].Y,pv[0].Z, pv[3].X,pv[3].Y,pv[3].Z, r, g, b, alpha, linewidth);
    DrawOneLine(pv[0].X,pv[0].Y,pv[0].Z, pv[4].X,pv[4].Y,pv[4].Z, r, g, b, alpha, linewidth);

    DrawOneLine(pv[1].X,pv[1].Y,pv[1].Z, pv[2].X,pv[2].Y,pv[2].Z, r, g, b, alpha, linewidth);
    DrawOneLine(pv[2].X,pv[2].Y,pv[2].Z, pv[3].X,pv[3].Y,pv[3].Z, r, g, b, alpha, linewidth);
    DrawOneLine(pv[3].X,pv[3].Y,pv[3].Z, pv[4].X,pv[4].Y,pv[4].Z, r, g, b, alpha, linewidth);
    DrawOneLine(pv[4].X,pv[4].Y,pv[4].Z, pv[1].X,pv[1].Y,pv[1].Z, r, g, b, alpha, linewidth);

    if (fill) {
        DRAWONERECT(pv[1].X, pv[1].Y, pv[1].Z, pv[2].X, pv[2].Y, pv[2].Z,
                    pv[3].X, pv[3].Y, pv[3].Z, pv[4].X, pv[4].Y, pv[4].Z);
    }
	}

	/* ************************************************************************* */
	void drawCameras(const vector<CameraVertices>& cameras) {

		const float r = 240.f/255.f, g = 140.0f/255.f,  b = 24.0f/255.f, alpha = 1.f;
		GLfloat linewidth = 1;
		BOOST_FOREACH(const CameraVertices& camera, cameras)
			drawCamera(camera.v, r, g, b, alpha, linewidth, true);
	}

	/* ************************************************************************* */
	CameraVertices calcCameraVertices(const gtsam::SimpleCamera& camera, const int img_w, const int img_h)
	{
		CameraVertices cam_vertices;

		// the first point is the optical center
		cam_vertices.v[0].X = camera.pose().x();
		cam_vertices.v[0].Y = camera.pose().y();
		cam_vertices.v[0].Z = camera.pose().z();

		// generate four vertex corners
		vector<gtsam::Point2> corners;
		corners.push_back(gtsam::Point2(0.f,img_h-1));
		corners.push_back(gtsam::Point2(img_w-1,img_h-1));
		corners.push_back(gtsam::Point2(img_w-1,0.f));
		corners.push_back(gtsam::Point2(0.f,0.f));
		for(int j=1; j<=4; j++) {
			gtsam::Point3 tmp = camera.backproject(corners[j-1], 1.);
			cam_vertices.v[j].X = tmp.x();
			cam_vertices.v[j].Y = tmp.y();
			cam_vertices.v[j].Z = tmp.z();
		}

		return cam_vertices;
	}

#include "GLCanvas.moc"

} // namespace sfmviewer
