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

#include "trackball.h"

namespace sfmviewer {

	// callback function types
	typedef boost::function<void ()> Callback;

	class GLCanvas : public QGLWidget
	{
		Q_OBJECT

	public:

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
		void setDrawFunc(const Callback& fun_draw) { fun_draw_ = fun_draw; }

		// set the current opengl camera pose
		void setGLPose(const QuatPose& pose) { glPose_ = pose; }

		void setGLPoseTop(const QuatPose& pose) { glPoseTop_ = pose; }

		// set the refresh interval
		void setRefreshInterval(int msec);

		// set callback function for the local timer events
		int addTimer(const Callback& fun_timer, int msec);

	protected:
		// intialize the opengl canvas
		void initializeGL();

		// paint function
		void paintGL();

		// callback when the window is resized
		void resizeGL(int width, int height);

		// record the last mouse position
		void mousePressEvent(QMouseEvent *event);

		// mouse click and drag
		void mouseMoveEvent(QMouseEvent *event);

		// create actions for events
		void createActions();

		// context menu event
		void contextMenuEvent(QContextMenuEvent *event);

		// repsond to the timer events
		void timerEvent(QTimerEvent *event);

	private slots:

		// change to the top view
		void changeTopView();

	private:
		// the last position of mouse clicks
		QPoint lastPos_;

		int hintWidth_;
		int hintHeight_;

		// the pose of the current opengl camera
		QuatPose glPose_;

		// the pose when the opengl camera is at top
		QuatPose glPoseTop_;

		// the pointers of callback functions
		Callback fun_draw_;

		// the action to change mouse speed
		QAction* changeTopViewAct;

		// the timer to force to refresh
		QTimer *timerRefresh_;

		// timer identifiers and their corresponding callback functions
		std::map<int, Callback> timer_callbacks_;
	};
} // namespace sfmviewer
