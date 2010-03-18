/*
 * GLCanvas.h
 *
 *   Created on: Mar 18, 2010
 *       Author: nikai
 *  Description: an OpenGL canvas
 */

#include <iostream>
#include <math.h>
#include <QtGui>
#include <QtOpenGL>


#include "GLCanvas.h"
#include "bunny.h"
#include "trackball.h"

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif

#define SFM_POINT_COLOR          1.0f, 1.0f, 1.0f, 1.0f
#define SFM_BACKGROUND_COLOR     0.0f, 0.0f, 0.0f, 1.0f

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

using namespace std;

/* ************************************************************************* */
GLCanvas::GLCanvas(QWidget *parent)
: QGLWidget(QGLFormat(QGL::SampleBuffers), parent)
{
	qtGreen = QColor::fromCmykF(0.40, 0.0, 1.0, 0.0);
	qtPurple = QColor::fromCmykF(0.39, 0.39, 0.0, 0.0);

	// load bunny points
	Vertex v;
	for(int i=0;i<bunny_nr_vertex;i++)
	{
		v.X = (GLfloat)bunny_vertices[i][0];
		v.Y = -(GLfloat)bunny_vertices[i][1];
		v.Z = (GLfloat)bunny_vertices[i][2];
		structure_.push_back(v);
	}
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

	// initial view port
	viewPort_.m_shift[0]    =  0.0f;
	viewPort_.m_shift[1]    =  0.0f;
	viewPort_.m_shift[2]    =  -5.0f;
	trackball(viewPort_.m_quat, 0.0f, 0.0f, 0.0f, 0.0f);
}

/* ************************************************************************* */
void GLCanvas::paintGL()
{
	glClearColor(SFM_BACKGROUND_COLOR);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Transformations
	glMatrixMode( GL_MODELVIEW);
	glLoadIdentity();
	GLfloat trackball[4][4];
	build_tran_matrix(trackball, viewPort_.m_quat, viewPort_.m_shift[0],
			viewPort_.m_shift[1], viewPort_.m_shift[2]);
	glMultMatrixf((GLfloat*) trackball);
	glMultMatrixf(m_convert);

	DrawStructure();

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
void GLCanvas::mousePressEvent(QMouseEvent *event)
{
	lastPos_ = event->pos();
}

/* ************************************************************************* */
void GLCanvas::mouseMoveEvent(QMouseEvent *event)
{
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
}

/* ************************************************************************* */
void GLCanvas::DrawStructure()
{
	if (!structure_.empty())
	{
		glColor4f(SFM_POINT_COLOR);
		glEnableClientState( GL_VERTEX_ARRAY);
		glVertexPointer(3, GL_FLOAT, 0, (GLvoid*) &structure_[0]);
		glDrawArrays(GL_POINTS, 0, structure_.size());
		glDisableClientState(GL_VERTEX_ARRAY);
	}
}

#include "GLCanvas.moc"
