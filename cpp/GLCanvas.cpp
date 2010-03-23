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
	const GLfloat m_convert[4][4] = {
			{1.,  0.,  0., 0.},
			{0., -1.,  0., 0.},
			{0.,	0., -1., 0.},
			{0.,  0.,  0., 1.}};

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

		// the timer to force to refresh
		timerRefresh_ = new QTimer(this);
		connect(timerRefresh_, SIGNAL(timeout()), this, SLOT(updateGL()));

		// set the view port of the top view
		viewPortTop_ = ViewPort(0., -500., 0., -1./sqrt(2.), 0., 0., 1./sqrt(2.));
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
		glShadeModel(GL_FLAT);
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
		glHint(GL_POLYGON_SMOOTH_HINT, GL_FASTEST);

		glClearColor(SFM_BACKGROUND_COLOR);

//		GLfloat light_ambient[] = { 0.2, 0.2, 0.2, 1.0 };
//		GLfloat light_diffuse[] = { 0.7, 0.7, 0.7, 1.0 };
//		GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
//		GLfloat light_position[] = { -150.0, 500.0, -300.0, 0.0 };
//
//		glEnable(GL_LIGHTING);
//		glEnable(GL_LIGHT0);
//		glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
//		glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
//		glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
//		glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	}

	/* ************************************************************************* */
	void print2DArray(const float m[4][4], const string& str) {
		cout << str << endl;
		cout << m[0][0] << " " << m[0][1] << " " << m[0][2] << " " << m[0][3] << endl;
		cout << m[1][0] << " " << m[1][1] << " " << m[1][2] << " " << m[1][3] << endl;
		cout << m[2][0] << " " << m[2][1] << " " << m[2][2] << " " << m[2][3] << endl;
		cout << m[3][0] << " " << m[3][1] << " " << m[3][2] << " " << m[3][3] << endl;
	}

	void print1DArray(const float m[16], const string& str) {
		cout << str << endl;
		cout << m[0] << " " << m[4] << " " << m[8]  << " " << m[12] << endl;
		cout << m[1] << " " << m[5] << " " << m[9]  << " " << m[13] << endl;
		cout << m[2] << " " << m[6] << " " << m[10] << " " << m[14] << endl;
		cout << m[3] << " " << m[7] << " " << m[11] << " " << m[15] << endl;
	}

	void printModelViewMatrix() {
		float m[16];
		glGetFloatv(GL_MODELVIEW, m);
		print1DArray(m, "ModelView");
	}

	void printProjectionatrix() {
		float m[16];
		glGetFloatv(GL_PROJECTION, m);
		print1DArray(m, "Projection");
	}

	/* ************************************************************************* */
	void GLCanvas::paintGL() {

		// Transformations
		glMatrixMode( GL_MODELVIEW);
		glLoadIdentity();
		GLfloat prj[4][4];
		build_tran_matrix(viewPort_, prj);
		glMultTransposeMatrixf((GLfloat*)m_convert); // second, convert the camera coordinates to the opengl camera coordinates
//		printModelViewMatrix();
		glMultTransposeMatrixf((GLfloat*)prj);       // first, project the points in the world coordinates to the camera coorrdinates
//		print2DArray(prj, "trackball");
//		printModelViewMatrix();

		// background
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		fun_draw_();

//		glFlush();
		swapBuffers();
	}

	/* ************************************************************************* */
	void GLCanvas::resizeGL(int width, int height) {
		// the calibration matrix only depends on the window aspect ratio
		glMatrixMode( GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(60.0, (GLfloat) width / height, 0.00001, 1000.0); // the 3rd parameter
	}

	/* ************************************************************************* */
	void GLCanvas::mousePressEvent(QMouseEvent *event) {
		lastPos_ = event->pos();
	}

	/* ************************************************************************* */
	void GLCanvas::mouseMoveEvent(QMouseEvent *event) {
		float m_shift_step = 0.01f;

		// left click to rotate
		if ((event->buttons() & Qt::LeftButton) && !(event->modifiers() & Qt::ControlModifier)
				&& !(event->modifiers() & Qt::AltModifier)) {
			QSize ws = size();
			float spin_quat[4];
			trackball(spin_quat, (2.0 * lastPos_.x() - ws.width())  / ws.width(),
					                -(2.0 * lastPos_.y() - ws.height()) / ws.height(),
					                 (2.0 * event->x()   - ws.width())  / ws.width(),
  				                -(2.0 * event->y()   - ws.height())	/ ws.height());
			add_quats(spin_quat, viewPort_.m_quat, viewPort_.m_quat);
		}
		// right click or command + left click to translate
		else if ((event->buttons() & Qt::RightButton) ||
				(event->buttons() & Qt::LeftButton) && (event->modifiers() & Qt::ControlModifier)) {
			float dx = event->x() - lastPos_.x();
			float dy = lastPos_.y() - event->y();
			viewPort_.m_shift[0] += dx * m_shift_step * 100;
			viewPort_.m_shift[1] += dy * m_shift_step * 100;
		}
		// middle click to zoom in and zoom out
		else if ((event->buttons() & Qt::MidButton) ||
				(event->buttons() & Qt::LeftButton) && (event->modifiers() & Qt::AltModifier)) {
			float dx = event->x() - lastPos_.x();
			float dy = lastPos_.y() - event->y();
			if (fabs(dy) > fabs(dx))
				viewPort_.m_shift[2] += (dy / fabs(dy)) * m_shift_step * 100;
		}
		lastPos_ = event->pos();
		updateGL();

		// update status bar
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
		changeTopViewAct = new QAction(tr("&Top view"), this);
		changeTopViewAct->setStatusTip(tr("Change to the top view"));
		connect(changeTopViewAct, SIGNAL(triggered()), this, SLOT(changeTopView()));
	}

	/* ************************************************************************* */
	void GLCanvas::changeTopView() {
		viewPort_ = viewPortTop_;
		updateGL();
	}

	/* ************************************************************************* */
	void GLCanvas::contextMenuEvent(QContextMenuEvent *event) {
		if (event->modifiers() & Qt::MetaModifier) { // it requires control click
			QMenu menu(this);
			cout << "contextMenuEvent" << endl;
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
