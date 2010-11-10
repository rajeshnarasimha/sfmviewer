/*
 * SFMViewer.h
 *
 *   Created on: Mar 18, 2010
 *       Author: nikai
 *  Description: the base class for the viewer window
 */

#pragma once

#include <QMainWindow>
#include "GLCanvas.h"

QT_BEGIN_NAMESPACE
class QSlider;
QT_END_NAMESPACE

QT_FORWARD_DECLARE_CLASS(QMenu)

namespace sfmviewer {

	class SFMViewer : public QMainWindow
	{
		Q_OBJECT

	public:
		SFMViewer(const std::string& title = "SFMViewer", QMainWindow *parent = 0);
		~SFMViewer();

		// set the window size
		void setSize(int width, int height);

		// set callback function for the opengl canvas
		void setDrawFunc (const Callback& fun_draw) { glCanvas->setDrawFunc(fun_draw); }

		// return the canvas pointer
		GLCanvas* canvas() { return glCanvas; }

	private slots:
		// show the about dialog
		void about();

		// show the config dialog
		void config();

	protected:

		// repsond the key press events
		void keyPressEvent(QKeyEvent *event);

	private:

		// the pointer of the opengl canvas
		GLCanvas *glCanvas;

		// the callback function handle for timer events
		Callback fun_timer_;
	};

} // namespace sfmviewer
