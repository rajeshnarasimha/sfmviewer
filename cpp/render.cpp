/*
 * render.cpp
 *
 *   Created on: Mar 19, 2010
 *       Author: nikai
 *  Description: the rendering functions for different elements
 */

#include <boost/foreach.hpp>
#include <gtsam/Matrix.h>

#include "render.h"
#include "bunny.h"

using namespace std;

#define SFM_POINT_COLOR          1.0f, 1.0f, 1.0f, 1.0f
#define SFM_CAMERA_COLOR  240.f/255.f, 140.0f/255.f, 24.0f/255.f,  1.0f

#define DRAWONERECT(X1,Y1,Z1,X2,Y2,Z2,X3,Y3,Z3,X4,Y4,Z4) \
	glColor4f(SFM_CAMERA_COLOR); \
	glEnable(GL_BLEND); \
	glBlendFunc(GL_SRC_COLOR,GL_DST_COLOR); \
	glBegin(GL_POLYGON);\
	glVertex3f(X1,Y1,Z1); \
	glVertex3f(X2,Y2,Z2); \
	glVertex3f(X3,Y3,Z3); \
	glVertex3f(X4,Y4,Z4); \
	glEnd(); \
	glDisable(GL_BLEND);

namespace sfmviewer {
	/* ************************************************************************* */
	void drawStructure(const vector<Vertex>& structure,
			const vector<SFMColor>& pointColors) {

		// enable blending
		glEnable( GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// point rendering setting
		glEnable( GL_POINT_SMOOTH);
		glPointSize(2.0);

		if (!structure.empty()) {
			// set points to draw
			glEnableClientState( GL_VERTEX_ARRAY);
			glVertexPointer(3, GL_FLOAT, 0, (GLvoid*) &structure[0]);

			// set colors if available
			if (!pointColors.empty()) {
				if (pointColors.size() != structure.size())
					throw runtime_error("DrawStructure: no. of colors != no. of points");
				glEnableClientState( GL_COLOR_ARRAY);
				glColorPointer(4, GL_FLOAT, 0, (GLvoid*) &pointColors[0]);
			} else
				glColor4f(SFM_POINT_COLOR);

			// draw the points
			glDrawArrays(GL_POINTS, 0, structure.size());
			glDisableClientState(GL_VERTEX_ARRAY);
		}

		glDisable(GL_BLEND);
	}

	/* ************************************************************************* */
	void drawBunny() {
		vector<Vertex> structure;
		Vertex v;
		for (int i = 0; i < bunny_nr_vertex; i++) {
			v.X = (GLfloat) bunny_vertices[i][0];
			v.Y = -(GLfloat) bunny_vertices[i][1];
			v.Z = (GLfloat) bunny_vertices[i][2];
			structure.push_back(v);
		}
		drawStructure(structure);
	}

	/* ************************************************************************* */
	inline void drawOneLine(GLfloat X1, GLfloat Y1, GLfloat Z1, GLfloat X2,
			GLfloat Y2, GLfloat Z2, const SFMColor& color, GLfloat linewidth = 1) {
		glColor4f(color.r, color.g, color.b, color.alpha);
		glLineWidth(linewidth);
		glBegin( GL_LINES);
		glVertex3f(X1, Y1, Z1);
		glVertex3f(X2, Y2, Z2);
		glEnd();
	}

	/* ************************************************************************* */
	void drawCamera(const Vertex* pv, const SFMColor& color, const GLfloat linewidth, bool fill) {
		// enable blending
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    drawOneLine(pv[0].X,pv[0].Y,pv[0].Z, pv[1].X,pv[1].Y,pv[1].Z, color, linewidth);
    drawOneLine(pv[0].X,pv[0].Y,pv[0].Z, pv[2].X,pv[2].Y,pv[2].Z, color, linewidth);
    drawOneLine(pv[0].X,pv[0].Y,pv[0].Z, pv[3].X,pv[3].Y,pv[3].Z, color, linewidth);
    drawOneLine(pv[0].X,pv[0].Y,pv[0].Z, pv[4].X,pv[4].Y,pv[4].Z, color, linewidth);

    drawOneLine(pv[1].X,pv[1].Y,pv[1].Z, pv[2].X,pv[2].Y,pv[2].Z, color, linewidth);
    drawOneLine(pv[2].X,pv[2].Y,pv[2].Z, pv[3].X,pv[3].Y,pv[3].Z, color, linewidth);
    drawOneLine(pv[3].X,pv[3].Y,pv[3].Z, pv[4].X,pv[4].Y,pv[4].Z, color, linewidth);
    drawOneLine(pv[4].X,pv[4].Y,pv[4].Z, pv[1].X,pv[1].Y,pv[1].Z, color, linewidth);

    if (fill) {
        DRAWONERECT(pv[1].X, pv[1].Y, pv[1].Z, pv[2].X, pv[2].Y, pv[2].Z,
                    pv[3].X, pv[3].Y, pv[3].Z, pv[4].X, pv[4].Y, pv[4].Z);
    }
		glDisable(GL_BLEND);
	}

	/* ************************************************************************* */
	void drawCameras(const vector<CameraVertices>& cameras, const vector<SFMColor>& cameraColors) {

		GLfloat linewidth = 1;
		for (size_t i=0; i<cameras.size(); i++) {
			if (cameraColors.empty())
				drawCamera(cameras[i].v, default_camera_color, linewidth, true);
			else
				drawCamera(cameras[i].v, cameraColors[i], linewidth, true);
		}
	}

	/* ************************************************************************* */
	void drawRGBCamera(const gtsam::Pose3& pose, const GLfloat linewidth, const float alpha) {
		Matrix r = pose.rotation().matrix();
		drawOneLine(pose.x(), pose.y(), pose.z(), pose.x()+r(0,0), pose.y()+r(1,0), pose.z()+r(2,0), SFMColor(1.0, 0.0, 0.0, alpha), linewidth); // r
		drawOneLine(pose.x(), pose.y(), pose.z(), pose.x()+r(0,1), pose.y()+r(1,1), pose.z()+r(2,1), SFMColor(0.0, 1.0, 0.0, alpha), linewidth); // g
		drawOneLine(pose.x(), pose.y(), pose.z(), pose.x()+r(0,2), pose.y()+r(1,2), pose.z()+r(2,2), SFMColor(0.0, 0.0, 1.0, alpha), linewidth); // b
	}

	/* ************************************************************************* */
	void drawRGBCameras(const vector<gtsam::Pose3>& poses, const GLfloat linewidth, const float alpha) {
		BOOST_FOREACH(const gtsam::Pose3& pose, poses)
			drawRGBCamera(pose, linewidth, alpha);
	}

	/* ************************************************************************* */
	CameraVertices calcCameraVertices(const gtsam::SimpleCamera& camera, const int img_w, const int img_h)
	{
		CameraVertices cam_vertices;

		// the first point is the optical center
		cam_vertices.v[0].X = camera.pose().x();
		cam_vertices.v[0].Y = camera.pose().y();
		cam_vertices.v[0].Z = camera.pose().z();

		// generate four vertex corners
		vector<gtsam::Point2> corners;
		corners.push_back(gtsam::Point2(0.f,img_h-1));
		corners.push_back(gtsam::Point2(img_w-1,img_h-1));
		corners.push_back(gtsam::Point2(img_w-1,0.f));
		corners.push_back(gtsam::Point2(0.f,0.f));
		for(int j=1; j<=4; j++) {
			gtsam::Point3 tmp = camera.backproject(corners[j-1], 1.);
			cam_vertices.v[j].X = tmp.x();
			cam_vertices.v[j].Y = tmp.y();
			cam_vertices.v[j].Z = tmp.z();
		}

		return cam_vertices;
	}
} // namespace sfmviewer
