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

#define SFM_BACKGROUND_COLOR     1.0f, 1.0f, 1.0f, 1.0f

using namespace std;

namespace sfmviewer {

	/* ************************************************************************* */
	// the conversion matrix from OpenGL default coordinate system
	//  to the camera coordiante system:
	// [ 1  0  0  0] * [ x ] = [ x ]
	//   0 -1  0  0      y      -y
	//   0  0 -1  0      z      -z
	//   0  0  0  1      1       1
	const GLfloat m_convert[4][4] = {
			{1.,  0.,  0., 0.},
			{0., -1.,  0., 0.},
			{0.,	0., -1., 0.},
			{0.,  0.,  0., 1.}};

	/* ************************************************************************* */
	GLCanvas::GLCanvas(QWidget *parent) :
		QGLWidget(QGLFormat(QGL::SampleBuffers), parent) {
		// initial camera pose
		glPose_.m_shift[0] = 0.0f;
		glPose_.m_shift[1] = 0.0f;
		glPose_.m_shift[2] = -5.0f;
		trackball(glPose_.m_quat, 0.0f, 0.0f, 0.0f, 0.0f);

		// create various actions
		createActions();

		// the timer to force to refresh
		timerRefresh_ = new QTimer(this);
		connect(timerRefresh_, SIGNAL(timeout()), this, SLOT(updateGL()));

		// set the camera pose of the top view
		glPoseTop_ = QuatPose(0., -500., 0., -1./sqrt(2.), 0., 0., 1./sqrt(2.));
	}

	/* ************************************************************************* */
	void GLCanvas::setRefreshInterval(int msec) {
		msec > 0 ? timerRefresh_->start(msec) : timerRefresh_->stop();
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
		glEnable( GL_CULL_FACE);
		glEnable( GL_DEPTH_TEST);
		glEnable(GL_MULTISAMPLE);

		// speedups
		glEnable(GL_DITHER);
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
		glHint(GL_POLYGON_SMOOTH_HINT, GL_FASTEST);
		glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
		glEnable( GL_POINT_SMOOTH);

		glClearColor(SFM_BACKGROUND_COLOR);
	}

	/* ************************************************************************* */
	void GLCanvas::paintGL() {

		// Transformations
		glMatrixMode( GL_MODELVIEW);
		glLoadIdentity();
		GLfloat prj[4][4];
		build_tran_matrix(glPose_, prj);
		glMultTransposeMatrixf((GLfloat*)m_convert); // second, convert the camera coordinates to the opengl camera coordinates
		glMultTransposeMatrixf((GLfloat*)prj);       // first, project the points in the world coordinates to the camera coorrdinates

		// background
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		fun_draw_();

		glFlush();
	}

	/* ************************************************************************* */
	void GLCanvas::resizeGL(int width, int height) {
		// the calibration matrix only depends on the window aspect ratio
		glMatrixMode( GL_PROJECTION);

    // set the viewport size
    glViewport(0, 0, width, height);

		glLoadIdentity();
		gluPerspective(60.0, (GLfloat) width / height, 0.01, 5000.0); // the 3rd parameter
	}

	/* ************************************************************************* */
	void GLCanvas::mousePressEvent(QMouseEvent *event) {
		lastPos_ = event->pos();
	}

	/* ************************************************************************* */
	void GLCanvas::mouseMoveEvent(QMouseEvent *event) {
		float m_shift_step = 0.01f;

		float scale = 10;

		// left click to rotate
		if ((event->buttons() & Qt::LeftButton) && !(event->modifiers() & Qt::ControlModifier)
				&& !(event->modifiers() & Qt::AltModifier)) {
			QSize ws = size();
			float spin_quat[4];
			trackball(spin_quat, (2.0 * lastPos_.x() - ws.width())  / ws.width(),
					                -(2.0 * lastPos_.y() - ws.height()) / ws.height(),
					                 (2.0 * event->x()   - ws.width())  / ws.width(),
  				                -(2.0 * event->y()   - ws.height())	/ ws.height());
			add_quats(spin_quat, glPose_.m_quat, glPose_.m_quat);
		}

		// right click or command + left click to translate
		else if ((event->buttons() & Qt::RightButton) ||
				(event->buttons() & Qt::LeftButton) && (event->modifiers() & Qt::ControlModifier)) {
			float dx = event->x() - lastPos_.x();
			float dy = lastPos_.y() - event->y();
			float r[3][3];
			build_rotmatrix(r, glPose_.m_quat);
			glPose_.m_shift[0] += (r[0][0] * dx + r[0][1] * dy) * m_shift_step * scale;
			glPose_.m_shift[1] += (r[1][0] * dx + r[1][1] * dy) * m_shift_step * scale;
			glPose_.m_shift[2] += (r[2][0] * dx + r[2][1] * dy) * m_shift_step * scale;
		}

		// middle click to zoom in and zoom out
		else if ((event->buttons() & Qt::MidButton) ||
				(event->buttons() & Qt::LeftButton) && (event->modifiers() & Qt::AltModifier)) {
			float dx = event->x() - lastPos_.x();
			float dy = lastPos_.y() - event->y();
			if (fabs(dy) > fabs(dx)) {
				float r[3][3];
				build_rotmatrix(r, glPose_.m_quat);
				float delta = dy / fabs(dy);
				glPose_.m_shift[0] += r[0][2] * delta * m_shift_step * scale;
				glPose_.m_shift[1] += r[1][2] * delta * m_shift_step * scale;
				glPose_.m_shift[2] += r[2][2] * delta * m_shift_step * scale;
			}
		}
		lastPos_ = event->pos();
		updateGL();

		// update status bar
		if (parentWidget()) {
			stringstream portMsg;
			portMsg << glPose_.m_shift[0] << ", " << glPose_.m_shift[1] << ", "
							<< glPose_.m_shift[2] << ", " << glPose_.m_quat[0] << ", "
							<< glPose_.m_quat[1] << ", " << glPose_.m_quat[2] << ", "
							<< glPose_.m_quat[3] << endl;
			// TODO: check whether the parent is actually a QMainWindow
			((QMainWindow*) parentWidget())->statusBar()->showMessage(
					QString::fromStdString(portMsg.str()));
		}
	}

	/* ************************************************************************* */
	void GLCanvas::createActions() {
		changeTopViewAct = new QAction(tr("&Top view"), this);
		changeTopViewAct->setStatusTip(tr("Change to the top view"));
		connect(changeTopViewAct, SIGNAL(triggered()), this, SLOT(changeTopView()));
	}

	/* ************************************************************************* */
	void GLCanvas::changeTopView() {
		glPose_ = glPoseTop_;
		updateGL();
	}

	/* ************************************************************************* */
	void GLCanvas::contextMenuEvent(QContextMenuEvent *event) {
		if (event->modifiers() & Qt::MetaModifier) { // it requires control click
			QMenu menu(this);
			menu.addAction(changeTopViewAct);
			menu.exec(event->globalPos());
		}
	}

	/* ************************************************************************* */
	int GLCanvas::addTimer(const Callback& fun_timer, int msec) {
		int identifier = startTimer(msec);
		timer_callbacks_.insert(make_pair(identifier, fun_timer));
		return identifier;
	}

	/* ************************************************************************* */
	void GLCanvas::timerEvent(QTimerEvent *event)
	{
		map<int, Callback>::const_iterator it = timer_callbacks_.find(event->timerId());
		if (it == timer_callbacks_.end())
			throw runtime_error("GLCanvas::timerEvent: invalid timer id!");
		it->second();
	}
#include "GLCanvas.moc"

} // namespace sfmviewer
