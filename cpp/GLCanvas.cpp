/*
 * GLCanvas.h
 *
 *   Created on: Mar 18, 2010
 *       Author: nikai
 *  Description: an OpenGL canvas
 */

#include <stdexcept>
#include <iostream>
#include <sstream>
#include <math.h>
#include <QtGui>
#include <QtOpenGL>

#include "GLCanvas.h"
#include "trackball.h"

#define SFM_BACKGROUND_COLOR     0.0f, 0.0f, 0.0f, 1.0f

using namespace std;

namespace sfmviewer {

	/* ************************************************************************* */
	// the conversion matrix from OpenGL default coordinate system
	//  to the camera coordiante system:
	// [ 1  0  0  0] * [ x ] = [ x ]
	//   0 -1  0  0      y      -y
	//   0  0 -1  0      z      -z
	//   0  0  0  1      1       1
	const GLfloat m_convert[16] = { 1.f, 0.f, 0.f, 0.f, 0.f, -1.f, 0.f, 0.f, 0.f,
			0.f, -1.f, 0.f, 0.f, 0.f, 0.f, 1.f };

	/* ************************************************************************* */
	GLCanvas::GLCanvas(QWidget *parent) :
		QGLWidget(QGLFormat(QGL::SampleBuffers), parent) {
		// initial view port
		viewPort_.m_shift[0] = 0.0f;
		viewPort_.m_shift[1] = 0.0f;
		viewPort_.m_shift[2] = -5.0f;
		trackball(viewPort_.m_quat, 0.0f, 0.0f, 0.0f, 0.0f);

		// create various actions
		createActions();

		// create the refresh timer
		timer_ = new QTimer(this);
		connect(timer_, SIGNAL(timeout()), this, SLOT(updateGL()));
	}

	/* ************************************************************************* */
	void GLCanvas::setRefreshInterval(int msec) {
		msec > 0 ? timer_->start(msec) : timer_->stop();
	}

	/* ************************************************************************* */
	GLCanvas::~GLCanvas() {
	}

	/* ************************************************************************* */
	QSize GLCanvas::minimumSizeHint() const {
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
	void GLCanvas::initializeGL() {
		// remove back faces
		glDisable( GL_CULL_FACE);
		glEnable( GL_DEPTH_TEST);

		// speedups
		glEnable(GL_DITHER);
		glShadeModel(GL_SMOOTH);
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
		glHint(GL_POLYGON_SMOOTH_HINT, GL_FASTEST);
	}

	/* ************************************************************************* */
	void GLCanvas::paintGL() {
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

		fun_draw_();

		glFlush();
	}

	/* ************************************************************************* */
	void GLCanvas::resizeGL(int width, int height) {
		glViewport(0, 0, (GLint) width, (GLint) height);
		glMatrixMode( GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(60.0, (GLfloat) width / height, 0.001, 1000.0); // the 3rd parameter
		glMatrixMode( GL_MODELVIEW);
		glLoadIdentity();
	}

	/* ************************************************************************* */
	void GLCanvas::mousePressEvent(QMouseEvent *event) {
		lastPos_ = event->pos();
	}

	/* ************************************************************************* */
	void GLCanvas::mouseMoveEvent(QMouseEvent *event) {
		float m_shift_step = 0.01f;

		// left click to rotate
		if (event->buttons() & Qt::LeftButton) {
			QSize ws = size();
			float spin_quat[4];
			trackball(spin_quat, (2.0 * lastPos_.x() - ws.width()) / ws.width(),
					-(2.0 * lastPos_.y() - ws.height()) / ws.height(), (2.0 * event->x()
							- ws.width()) / ws.width(), -(2.0 * event->y() - ws.height())
							/ ws.height());
			add_quats(spin_quat, viewPort_.m_quat, viewPort_.m_quat);
		}
		// right click to translate
		else if (event->buttons() & Qt::RightButton) {
			float dx = event->x() - lastPos_.x();
			float dy = lastPos_.y() - event->y();
			viewPort_.m_shift[0] += dx * m_shift_step;
			viewPort_.m_shift[1] += dy * m_shift_step;
		}
		// middle click to zoom in and zoom out
		else if (event->buttons() & Qt::MidButton) {
			float dx = event->x() - lastPos_.x();
			float dy = lastPos_.y() - event->y();
			if (fabs(dy) > fabs(dx))
				viewPort_.m_shift[2] += (dy / fabs(dy)) * m_shift_step * 100;
		}
		lastPos_ = event->pos();
		updateGL();

		if (parentWidget()) {
			stringstream portMsg;
			portMsg << viewPort_.m_shift[0] << ", " << viewPort_.m_shift[1] << ", "
					<< viewPort_.m_shift[2] << ", " << viewPort_.m_quat[0] << ", "
					<< viewPort_.m_quat[1] << ", " << viewPort_.m_quat[2] << ", "
					<< viewPort_.m_quat[3] << endl;
			// TODO: check whether the parent is actually a QMainWindow
			((QMainWindow*) parentWidget())->statusBar()->showMessage(
					QString::fromStdString(portMsg.str()));
		}
	}

	/* ************************************************************************* */
	void GLCanvas::createActions() {
		changeMouseSpeedAct = new QAction(tr("&Mouse speed"), this);
		changeMouseSpeedAct->setStatusTip(tr(
				"Change the speed of the mouse operations"));
		connect(changeMouseSpeedAct, SIGNAL(triggered()), this,
				SLOT(changeMouseSpeed()));
	}

	/* ************************************************************************* */
	void GLCanvas::changeMouseSpeed() {
		// create a dialog
		QDialog dlg(this);
		dlg.setModal(true);
		QPushButton *ok = new QPushButton("OK", &dlg);
		ok->setGeometry(10, 10, 100, 30);
		connect(ok, SIGNAL(clicked()), SLOT(accept()) );
		QPushButton *cancel = new QPushButton("Cancel", &dlg);
		cancel->setGeometry(10, 60, 100, 30);
		connect(cancel, SIGNAL(clicked()), SLOT(reject()) );

		if (dlg.exec()) {
			cout << "kai1" << endl;
		}
	}

	/* ************************************************************************* */
	void GLCanvas::contextMenuEvent(QContextMenuEvent *event) {
		if (event->modifiers() & Qt::MetaModifier) { // it requires control click
			QMenu menu(this);
			cout << "contextMenuEvent" << endl;
			menu.addAction(changeMouseSpeedAct);
			menu.exec(event->globalPos());
		}
	}

#include "GLCanvas.moc"

} // namespace sfmviewer
