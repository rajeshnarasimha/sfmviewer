/*
 * configdialog.h
 *
 *   Created on: Mar 20, 2010
 *       Author: nikai
 *  Description: the config dialog
 */

#pragma once

#include <QDialog>

namespace sfmviewer {

	// a label that contains a float variable
	class SliderLabel : public QLabel{
		Q_OBJECT

	private:
		QString label_;

	public:
		SliderLabel(const QString& label, int pos) : QLabel(label.arg(pos)), label_(label) { }

	public slots:
		void setPos(int pos) { setText(label_.arg(pos)); }
	};

	// the interface tab
	class InterfaceTab : public QWidget {
		Q_OBJECT

	private:
		QSlider *speedSlider;

	public:
		InterfaceTab(QWidget *parent = 0);
		int getMouseSpeed() { return speedSlider->value(); }
	};

	// configuration dialog
	class ConfigDialog : public QDialog
	{
		Q_OBJECT

	public:
		ConfigDialog(QWidget *parent = 0);

		// get the speed factor of mouse operations
		float getMouseSpeed() const { return interfaceTab->getMouseSpeed(); }

	private:
		QTabWidget *tabWidget;
		InterfaceTab *interfaceTab;
		QDialogButtonBox *buttonBox;
	};

} // namespace sfmviewer
