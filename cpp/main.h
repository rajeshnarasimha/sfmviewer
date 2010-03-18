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
	SFMViewer *window;
	int width = 1024;
	int height = 768;
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
  window = new sfmviewer::SFMViewer("SFMViewer");
  window->setSize(width, height);
  window->show();
  return app.exec();
}
