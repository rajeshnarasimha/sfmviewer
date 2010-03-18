/*
 * SFMViewer.cpp
 *
 *   Created on: Mar 18, 2010
 *       Author: nikai
 *  Description: the base class for the viewer window
 */

#include <QtCore> // for layout
#include <QtGui>

#include "SFMViewer.h"

using namespace std;

namespace sfmviewer {

	/* ************************************************************************* */
	SFMViewer::SFMViewer(const string& title, QWidget *parent) :
			QWidget(parent)
	{
			glWidget = new GLCanvas;

			QHBoxLayout *mainLayout = new QHBoxLayout;
			mainLayout->addWidget(glWidget);
			setLayout(mainLayout);

			setWindowTitle(QString::fromStdString(title));
	}

	/* ************************************************************************* */
	SFMViewer::~SFMViewer() {

	}

	/* ************************************************************************* */
	void SFMViewer::setSize(int width, int height) {
//		setSizeHint(width,height);
		glWidget->setSizeHint(width, height);
	}

	/* ************************************************************************* */
	void SFMViewer::keyPressEvent(QKeyEvent *e)
	{
			if (e->key() == Qt::Key_Escape)
					close();
			else
					QWidget::keyPressEvent(e);
	}

} // namespace sfmviewer
