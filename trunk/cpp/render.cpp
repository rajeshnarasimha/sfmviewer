/*
 * render.cpp
 *
 *   Created on: Mar 19, 2010
 *       Author: nikai
 *  Description: the rendering functions for different elements
 */

#include <boost/foreach.hpp>

#include "render.h"
#include "bunny.h"

using namespace std;

#define SFM_POINT_COLOR          1.0f, 1.0f, 1.0f, 1.0f
#define SFM_CAMERA_COLOR  240.f/255.f, 140.0f/255.f, 24.0f/255.f,  1.0f

#define DRAWONERECT(X1,Y1,Z1,X2,Y2,Z2,X3,Y3,Z3,X4,Y4,Z4) glColor4f(SFM_CAMERA_COLOR); glEnable(GL_BLEND); glBlendFunc(/*GL_ONE_MINUS*/GL_SRC_COLOR,GL_DST_COLOR/*GL_ONE_MINUS_DST_COLOR*/); glBegin(GL_POLYGON);\
	glVertex3f(X1,Y1,Z1); glVertex3f(X2,Y2,Z2); glVertex3f(X3,Y3,Z3); glVertex3f(X4,Y4,Z4); glEnd(); glDisable(GL_BLEND);

namespace sfmviewer {
	/* ************************************************************************* */
	void drawStructure(const vector<Vertex>& structure, const vector<VertexColor>& pointColors) {

		if (!structure.empty())
		{
			// set default color
			glColor4f(SFM_POINT_COLOR);

			// set points to draw
			glEnableClientState( GL_VERTEX_ARRAY);
			glVertexPointer(3, GL_FLOAT, 0, (GLvoid*) &structure[0]);

			// set colors if available
			if (!pointColors.empty()) {
				if (pointColors.size() != structure.size())
					throw runtime_error("DrawStructure: no. of colors != no. of points");
				glEnableClientState(GL_COLOR_ARRAY);
				glColorPointer(3,GL_UNSIGNED_BYTE, 0, (GLvoid*)&pointColors[0]);
			}

			// draw the points
			glDrawArrays(GL_POINTS, 0, structure.size());
			glDisableClientState(GL_VERTEX_ARRAY);
		}
	}

	/* ************************************************************************* */
	void drawBunny() {
		vector<Vertex> structure;
		Vertex v;
		for(int i=0;i<bunny_nr_vertex;i++)
		{
			v.X = (GLfloat)bunny_vertices[i][0];
			v.Y = -(GLfloat)bunny_vertices[i][1];
			v.Z = (GLfloat)bunny_vertices[i][2];
			structure.push_back(v);
		}
		drawStructure(structure);
	}

	/* ************************************************************************* */
	inline void DrawOneLine(GLfloat X1, GLfloat Y1, GLfloat Z1,
			GLfloat X2, GLfloat Y2, GLfloat Z2,
			float r, float g, float b, float alpha, GLfloat linewidth = 1)
	{
	    glColor4f(r,g,b,alpha);
	    glLineWidth(linewidth);
	    glBegin(GL_LINES);
	    glVertex3f(X1,Y1,Z1);
	    glVertex3f(X2,Y2,Z2);
	    glEnd();
	}

	/* ************************************************************************* */
	void drawCamera(const Vertex* pv, const float r, const float g, const float b, const float alpha, const GLfloat linewidth, bool fill) {
    DrawOneLine(pv[0].X,pv[0].Y,pv[0].Z, pv[1].X,pv[1].Y,pv[1].Z, r, g, b, alpha, linewidth);
    DrawOneLine(pv[0].X,pv[0].Y,pv[0].Z, pv[2].X,pv[2].Y,pv[2].Z, r, g, b, alpha, linewidth);
    DrawOneLine(pv[0].X,pv[0].Y,pv[0].Z, pv[3].X,pv[3].Y,pv[3].Z, r, g, b, alpha, linewidth);
    DrawOneLine(pv[0].X,pv[0].Y,pv[0].Z, pv[4].X,pv[4].Y,pv[4].Z, r, g, b, alpha, linewidth);

    DrawOneLine(pv[1].X,pv[1].Y,pv[1].Z, pv[2].X,pv[2].Y,pv[2].Z, r, g, b, alpha, linewidth);
    DrawOneLine(pv[2].X,pv[2].Y,pv[2].Z, pv[3].X,pv[3].Y,pv[3].Z, r, g, b, alpha, linewidth);
    DrawOneLine(pv[3].X,pv[3].Y,pv[3].Z, pv[4].X,pv[4].Y,pv[4].Z, r, g, b, alpha, linewidth);
    DrawOneLine(pv[4].X,pv[4].Y,pv[4].Z, pv[1].X,pv[1].Y,pv[1].Z, r, g, b, alpha, linewidth);

    if (fill) {
        DRAWONERECT(pv[1].X, pv[1].Y, pv[1].Z, pv[2].X, pv[2].Y, pv[2].Z,
                    pv[3].X, pv[3].Y, pv[3].Z, pv[4].X, pv[4].Y, pv[4].Z);
    }
	}

	/* ************************************************************************* */
	void drawCameras(const vector<CameraVertices>& cameras) {

		const float r = 240.f/255.f, g = 140.0f/255.f, b = 24.0f/255.f, alpha = 1.f;
		GLfloat linewidth = 1;
		BOOST_FOREACH(const CameraVertices& camera, cameras)
			drawCamera(camera.v, r, g, b, alpha, linewidth, true);
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
