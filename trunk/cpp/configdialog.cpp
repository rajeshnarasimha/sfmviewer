/*
 * configdialog.cpp
 *
 *   Created on: Mar 20, 2010
 *       Author: nikai
 *  Description: the config dialog
 */

#include <QtGui>

#include "configdialog.h"

namespace sfmviewer {
	ConfigDialog::ConfigDialog(QWidget *parent) : QDialog(parent) {

		// create tabs
		tabWidget = new QTabWidget;
		interfaceTab = new InterfaceTab();
		tabWidget->addTab(interfaceTab, tr("Interface"));

		buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
				| QDialogButtonBox::Cancel);

		connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
		connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

		QVBoxLayout *mainLayout = new QVBoxLayout;
		mainLayout->addWidget(tabWidget);
		mainLayout->addWidget(buttonBox);
		setLayout(mainLayout);

		setWindowTitle(tr("Preferences"));
	}

	InterfaceTab::InterfaceTab(QWidget *parent) : QWidget(parent)
	{
		// the mouse speed
		int pos0 = 10;
		SliderLabel *speedLabel = new SliderLabel(tr("Mouse speed: %1"), pos0);
		speedSlider = new QSlider(Qt::Horizontal);
		speedSlider->setGeometry(50, 50, 130, 30);
		speedSlider->setValue(pos0);
		speedSlider->setMinimum(1);
		speedSlider->setMaximum(100);
		connect(speedSlider, SIGNAL(valueChanged(int)), speedLabel, SLOT(setPos(int)));

		QGridLayout *layout = new QGridLayout;
		layout->setColumnStretch(1, 1);
		layout->setColumnMinimumWidth(0, 150);
		layout->setColumnMinimumWidth(1, 250);
		layout->addWidget(speedLabel, 0, 0);
		layout->addWidget(speedSlider, 0, 1);
		setLayout(layout);
	}

#include "configdialog.moc"
}
