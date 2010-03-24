/*
 * common.h
 *
 *   Created on: Mar 23, 2010
 *       Author: nikai
 *  Description:
 */

#pragma once

#include <string>
#include <gl/gl.h>

/**
 * print functions
 */
void print2DArray(const float m[4][4], const std::string& str);
void print1DArray(const float m[16], const std::string& str);
void printModelViewMatrix();
void printProjectionatrix();
