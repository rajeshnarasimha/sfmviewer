/*
 * main.h
 *
 *   Created on: Mar 18, 2010
 *       Author: nikai
 *  Description: a header file to create the boring main function automatically
 */

#pragma once

#ifdef __APPLE__
#include <ApplicationServices/ApplicationServices.h>
#endif

#include <QApplication>
#include <iostream>

#include "SFMViewer.h"

namespace sfmviewer {
	// the pointer of main window
	SFMViewer *window;

	// the width of the window
	int width = 1024;

	// the height of the window
	int height = 768;

	// the setup function called by users
	void setup(SFMViewer& window);

	// the user's draw function
	void draw (GLCanvas& canvas);
}

using namespace sfmviewer;

int main(int argc, char *argv[])
{
#ifdef __APPLE__
  ProcessSerialNumber PSN;
  GetCurrentProcess(&PSN);
  TransformProcessType(&PSN,kProcessTransformToForegroundApplication);
  SetFrontProcess(&PSN);
#endif

	QApplication app(argc, argv);
  window = new SFMViewer("SFMViewer");
  window->setSize(width, height);
  window->setCallback(draw);
  setup(*window);
  window->show();
  return app.exec();
}
