/*
 * SFMViewer.h
 *
 *   Created on: Mar 18, 2010
 *       Author: nikai
 *  Description: the base class for the viewer window
 */

#pragma once

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

		protected:
			void keyPressEvent(QKeyEvent *event);

		private:
			GLCanvas *glWidget;
	};

} // namespace sfmviewer
