/*
 * common.cpp
 *
 *   Created on: Mar 23, 2010
 *       Author: nikai
 *  Description: utility function
 */
#include <iostream>
#include "common.h"

using namespace std;

/* ************************************************************************* */
void print2DArray(const float m[4][4], const std::string& str) {
	cout << str << endl;
	cout << m[0][0] << " " << m[0][1] << " " << m[0][2] << " " << m[0][3] << endl;
	cout << m[1][0] << " " << m[1][1] << " " << m[1][2] << " " << m[1][3] << endl;
	cout << m[2][0] << " " << m[2][1] << " " << m[2][2] << " " << m[2][3] << endl;
	cout << m[3][0] << " " << m[3][1] << " " << m[3][2] << " " << m[3][3] << endl;
}

void print1DArray(const float m[16], const string& str) {
	cout << str << endl;
	cout << m[0] << " " << m[4] << " " << m[8]  << " " << m[12] << endl;
	cout << m[1] << " " << m[5] << " " << m[9]  << " " << m[13] << endl;
	cout << m[2] << " " << m[6] << " " << m[10] << " " << m[14] << endl;
	cout << m[3] << " " << m[7] << " " << m[11] << " " << m[15] << endl;
}

void printModelViewMatrix() {
	float m[16];
	glGetFloatv(GL_MODELVIEW, m);
	print1DArray(m, "ModelView");
}

void printProjectionatrix() {
	float m[16];
	glGetFloatv(GL_PROJECTION, m);
	print1DArray(m, "Projection");
}
