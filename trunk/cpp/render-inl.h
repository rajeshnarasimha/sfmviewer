/*
 * render-inl.h
 *
 *   Created on: Nov 9, 2010
 *       Author: nikai
 *  Description:
 */

#pragma once

#include <boost/foreach.hpp>
#include "render.h"
#include "trackball.h"


namespace sfmviewer {

	using namespace std;

	/* ************************************************************************* */
	template<class Pose3>
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
	template<class Pose3>
	void drawRGBCameras(const vector<Pose3>& poses, const GLfloat linewidth, const float scale) {
		BOOST_FOREACH(const Pose3& pose, poses)
			drawRGBCamera(pose, linewidth, scale);
	}

	/* ************************************************************************* */
	template<class Camera, class Point2, class Point3>
	CameraVertices calcCameraVertices(const Camera& camera, const int img_w, const int img_h, const float scale)
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

	/* ************************************************************************* */
	template<class Pose3, class Rot3, class Calibration, class Point3, class Point2>
	void drawCameraCircle() {
		int numCameras = 20;
		float orbit_center_x = 0.;
		float orbit_center_z = 0.;
		float orbit_height = 0.;
		float orbit_radius = 50.;
		int img_w = 1600, img_h = 1600;
		float scale = 7.;
		Calibration k(120., img_w, img_h);

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

} // namespace
