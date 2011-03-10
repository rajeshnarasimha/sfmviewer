/*
 * render.cpp
 *
 *   Created on: Mar 19, 2010
 *       Author: nikai
 *  Description: the rendering functions for different elements
 */

#include <stdexcept>
#include <boost/foreach.hpp>

#include "render.h"
#include "trackball.h"
#include "bunny.h"

using namespace std;

#define SFM_POINT_COLOR          0.0f, 0.0f, 0.0f, 1.0f
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
					throw std::runtime_error("DrawStructure: no. of colors != no. of points");
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
	void drawCameras(const vector<CameraVertices>& cameras, const SFMColor& color, const bool fill) {
		GLfloat linewidth = 1;
		for (size_t i=0; i<cameras.size(); i++) {
				drawCamera(cameras[i].v, color, linewidth, fill);
		}
	}

	/* ************************************************************************* */
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
} // namespace sfmviewer
