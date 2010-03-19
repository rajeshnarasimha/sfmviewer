/*
 * SFMViewer.h
 *
 *   Created on: Mar 18, 2010
 *       Author: nikai
 *  Description: the base class for the viewer window
 */

#pragma once

#include <gtsam/Point3.h>
#include "GLCanvas.h"

QT_BEGIN_NAMESPACE
class QSlider;
QT_END_NAMESPACE
class GLCanvas;

namespace sfmviewer {

	class SFMViewer : public QWidget
	{

		public:
			SFMViewer(const std::string& title = "SFMViewer", QWidget *parent = 0);
			~SFMViewer();

			// set the window size
			void setSize(int width, int height);

			// set callback function for the opengl canvas
			void setCallback (const GLCanvas::Callback& fun_draw) { glWidget->setCallback(fun_draw); }

			// return the canvas pointer
			GLCanvas* canvas() { return glWidget; }

		protected:

			void keyPressEvent(QKeyEvent *event);

		private:
			GLCanvas *glWidget;
	};

} // namespace sfmviewer
