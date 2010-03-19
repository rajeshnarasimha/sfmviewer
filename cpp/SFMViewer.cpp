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
	SFMViewer::SFMViewer(const std::string& title, QMainWindow *parent) :
					QMainWindow(parent)
	{
		// set window title
		setWindowTitle(QString::fromStdString(title));

		// set up menus
		QMenu *helpMenu = new QMenu(tr("Help"), this);
		menuBar()->addMenu(helpMenu);
		helpMenu->addAction(tr("About"), this, SLOT(about()));

		// enable status bar
		statusBar();

		// create opengl canvas
		glWidget = new GLCanvas(this);
		setCentralWidget(glWidget);

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


	/* ************************************************************************* */
	void SFMViewer::about() {
		QMessageBox::about(this, tr("About"), tr("SFMViewer by Kai Ni\n\n"
				"nikai97@gmail.com"));

	}

#include "SFMViewer.moc"

} // namespace sfmviewer
