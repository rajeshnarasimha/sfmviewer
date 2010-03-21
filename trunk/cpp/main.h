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
#include "render.h"

namespace sfmviewer {

	// the pointer of the application
	QApplication *app;

	// the pointer of main window
	SFMViewer *window;

	// the pointer of opengl canvas
	GLCanvas *canvas;

	// the width of the window
	int width = 1024;

	// the height of the window
	int height = 768;

	// the setup function called by users
	void setup();

	// the user's draw function
	void draw();
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

  // create a new window and its opengl canvas
	app = new QApplication(argc, argv);
  window = new SFMViewer("SFMViewer");
  canvas = window->canvas();

  // initialize the window
  window->setSize(width, height);
  window->setDrawFunc(draw);
  setup();
  window->show();
  return app->exec();
}
