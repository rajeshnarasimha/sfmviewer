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
#include "trackball.h"
#include "bunny.h"

using namespace std;
using namespace gtsam;

#define SFM_POINT_COLOR          1.0f, 1.0f, 1.0f, 1.0f
#define SFM_CAMERA_COLOR  240.f/255.f, 140.0f/255.f, 24.0f/255.f,  1.0f

#define DRAWONERECT(X1,Y1,Z1,X2,Y2,Z2,X3,Y3,Z3,X4,Y4,Z4) \
	glEnable(GL_BLEND); \
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA); \
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
		glPointSize(1.0);

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
    	glColor4f(color.r, color.g, color.b, color.alpha);
    	DRAWONERECT(pv[1].X, pv[1].Y, pv[1].Z, pv[2].X, pv[2].Y, pv[2].Z,
    			pv[3].X, pv[3].Y, pv[3].Z, pv[4].X, pv[4].Y, pv[4].Z);
    }
		glDisable(GL_BLEND);
	}

	/* ************************************************************************* */
	void drawCameras(const vector<CameraVertices>& cameras, const vector<SFMColor>& cameraColors, const bool fill) {

		GLfloat linewidth = 1;
		for (size_t i=0; i<cameras.size(); i++) {
			if (cameraColors.empty())
				drawCamera(cameras[i].v, default_camera_color, linewidth, fill);
			else
				drawCamera(cameras[i].v, cameraColors[i], linewidth, fill);
		}
	}

	/* ************************************************************************* */
	void drawRGBCamera(const Pose3& pose, const GLfloat linewidth, const float scale) {
		Matrix r = pose.rotation().matrix();
		r = r * scale;
		float xr = pose.x()+r(0,0), yr = pose.y()+r(1,0), zr = pose.z()+r(2,0);
		float xg = pose.x()+r(0,1), yg = pose.y()+r(1,1), zg = pose.z()+r(2,1);
		float xb = pose.x()+r(0,2), yb = pose.y()+r(1,2), zb = pose.z()+r(2,2);
		drawOneLine(pose.x(), pose.y(), pose.z(), xr, yr, zr, SFMColor(1.0, 0.0, 0.0, 1.), linewidth); // r
		drawOneLine(pose.x(), pose.y(), pose.z(), xg, yg, zg, SFMColor(0.0, 1.0, 0.0, 1.), linewidth); // g
		drawOneLine(pose.x(), pose.y(), pose.z(), xb, yb, zb, SFMColor(0.0, 0.0, 1.0, 1.), linewidth); // b
	}

	/* ************************************************************************* */
	void drawRGBCameras(const vector<Pose3>& poses, const GLfloat linewidth, const float scale) {
		BOOST_FOREACH(const Pose3& pose, poses)
			drawRGBCamera(pose, linewidth, scale);
	}

	/* ************************************************************************* */
	CameraVertices calcCameraVertices(const SimpleCamera& camera, const int img_w, const int img_h, const float scale)
	{
		CameraVertices cam_vertices;

		// the first point is the optical center
		cam_vertices.v[0].X = camera.pose().x();
		cam_vertices.v[0].Y = camera.pose().y();
		cam_vertices.v[0].Z = camera.pose().z();

		// generate four vertex corners
		vector<Point2> corners;
		corners.push_back(Point2(0.f,img_h-1));
		corners.push_back(Point2(img_w-1,img_h-1));
		corners.push_back(Point2(img_w-1,0.f));
		corners.push_back(Point2(0.f,0.f));
		for(int j=1; j<=4; j++) {
			Point3 tmp = camera.backproject(corners[j-1], scale);
			cam_vertices.v[j].X = tmp.x();
			cam_vertices.v[j].Y = tmp.y();
			cam_vertices.v[j].Z = tmp.z();
		}

		return cam_vertices;
	}

	GLuint loadThumbnailTexture(const QImage& image) {
		GLuint texID;
		glEnable(GL_TEXTURE_2D);
		glGenTextures(1, &texID);
		glBindTexture(GL_TEXTURE_2D, texID);
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_REPEAT );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_REPEAT );
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image.width(), image.height(), 0, GL_BGRA, GL_UNSIGNED_BYTE, image.bits());
		glDisable(GL_TEXTURE_2D);
		return texID;
	}


	/* ************************************************************************* */
	void drawThumbnail(const GLuint texID, const QSize& size, const QRectF& rect, const SFMColor& color) {
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		gluOrtho2D(0,(GLint)size.width(), 0, (GLint)size.height());

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// draw the textures
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, texID);
		glBegin(GL_QUADS);
		glColor4f(1.0f, 1.0f, 1.0f,  0.75f);
		glTexCoord2f(0.0f, 0.0f); glVertex2f(rect.right(), rect.bottom());
		glTexCoord2f(1.0f, 0.0f); glVertex2f(rect.left(),  rect.bottom());
		glTexCoord2f(1.0f, 1.0f); glVertex2f(rect.left(),  rect.top());
		glTexCoord2f(0.0f, 1.0f); glVertex2f(rect.right(), rect.top());
		glEnd();

		// draw the frame
  	glColor4f(color.r, color.g, color.b, color.alpha);
		glLineWidth(3.);
		glBegin(GL_LINES);
  	glVertex2f(rect.left() -1, rect.top()-1);    glVertex2f(rect.left() -1, rect.bottom()+1);
  	glVertex2f(rect.left() -1, rect.bottom()+1); glVertex2f(rect.right()+1, rect.bottom()+1);
  	glVertex2f(rect.right()+1, rect.bottom()+1); glVertex2f(rect.right()+1, rect.top()-1);
  	glVertex2f(rect.right()+1, rect.top()-1);    glVertex2f(rect.left() -1, rect.top()-1);
  	glEnd();

		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);

		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
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
	void drawCameraCircle() {
		int numCameras = 20;
		float orbit_center_x = 0.;
		float orbit_center_z = 0.;
		float orbit_height = 0.;
		float orbit_radius = 50.;
		int img_w = 1600, img_h = 1600;
		float scale = 7.;
		Cal3_S2 k(120., img_w, img_h);

		vector<Pose3> poses;
		vector<CameraVertices> cameras;
		for(int i=0; i<numCameras; i++) {

			// compute the rotation
			float angle = -M_PI_2 + (float)i / numCameras * M_PI * 2;
			float theta = -(M_PI_4 + angle / 2);
			float q1[4] = {0., sin(theta), 0., cos(theta)};
			float beta = -M_PI_4;
			float q2[4] = {sin(beta), 0., 0., cos(beta)};
			float q[4];
			add_quats(q2, q1, q);
			float r[3][3];
			build_rotmatrix(r, q);

			// compute the translation
			float x = cos(angle) * orbit_radius + orbit_center_x;
			float z = sin(angle) * orbit_radius + orbit_center_z;
			float trans[3] = {x, orbit_height, z};

			Pose3 pose(Rot3(r[0][0], r[0][1], r[0][2],
											r[1][0], r[1][1], r[1][2],
											r[2][0], r[2][1], r[2][2]),
					Point3(trans[0], trans[1], trans[2]));
			poses.push_back(pose);
			cameras.push_back(calcCameraVertices(SimpleCamera(k, pose), img_w, img_h, scale));
		}
		drawRGBCameras(poses, 1., scale);
		drawCameras(cameras);
	}

} // namespace sfmviewer
