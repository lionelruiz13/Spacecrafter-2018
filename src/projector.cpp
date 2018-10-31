/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2003 Fabien Chereau
 * Copyright (C) 2009 Digitalis Education Solutions, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * Spacecrafter is a free open project of of LSS team
 * See the TRADEMARKS file for free open project usage requirements.
 *
 */

#include <GL/glew.h>

#include <iostream>
#include <cstdio>
#include "projector.hpp"
#include "s_font.hpp"

#include <unistd.h>
#include <fcntl.h>
#include "fmath.hpp"
// #include "spacecrafter.hpp"

using namespace std;


Projector::Projector(const Vec4i& viewport, double _fov)
	:fov(1.0), min_fov(0.0001), max_fov(100),
	 zNear(0.1), zFar(10000),
	 vec_viewport(viewport),
	 flag_auto_zoom(0), 
	 viewport_fov_diameter(std::min<int>(viewport[2],viewport[3]))
{
	viewport_radius = -1;  // unset value DIGITALIS
	//~ flip_horz = 1.0;
	//~ flip_vert = 1.0;

	setViewport(viewport);
	setFov(_fov);

	mat_projection.set(1., 0., 0., 0.,
	                   0., 1., 0., 0.,
	                   0., 0., -1, 0.,
	                   0., 0., 0., 1.);
	min_fov = 0.0001;
	max_fov = 359.9;
	setFov(_fov);
}

Projector::~Projector()
{
}

// Init the viewing matrix, setting the field of view, the clipping planes, and screen ratio
// The function is a reimplementation of gluPerspective
void Projector::initProjectMatrix(void)
{
}

void Projector::setViewport(int x, int y, int w, int h)
{
	vec_viewport[0] = x;
	vec_viewport[1] = y;
	vec_viewport[2] = w;
	vec_viewport[3] = h;

	viewport_fov_diameter = std::min<int>(w,h);

	// may differ from viewport used, so store
	displayW = w;
	displayH = h;
	// cout << "CALLED NORMAL set viewport\n\n";

	viewport_center.set(vec_viewport[0]+vec_viewport[2]/2,vec_viewport[1]+vec_viewport[3]/2,0);
	glViewport(x, y, w, h);
	initProjectMatrix();
}

// allow truncated fisheye projection easily
// This method IS NOT AWARE of projectorConfiguration value
void Projector::setDiskViewport(int cx, int cy, int screenW, int screenH, int radius)
{

	// may differ from viewport used, so store
	displayW = screenW;
	displayH = screenH;

	if (cx< 0 ) vec_viewport[0] = 0;
	else vec_viewport[0] = cx-radius;

	if (cy-radius < 0) vec_viewport[1] = 0;
	else vec_viewport[1] = cy-radius;

	if (2*radius > screenW) vec_viewport[2] = screenW;
	else vec_viewport[2] = 2*radius;

	if (2*radius > screenH) vec_viewport[3] = screenH;
	else vec_viewport[3] = 2*radius;

	  //~ cout << "CALLED DISK set viewport\n\n";
	  //~ cout << "viewport " << vec_viewport[0] << " " << vec_viewport[1] << " " << vec_viewport[2] << " " << vec_viewport[3] << endl;
	  //~ cout << "set center " << cx << " with offset " << center_horizontal_offset << ", " << cy << endl;

	viewport_center.set(cx, cy, 0);
	viewport_radius = radius;

	glViewport(vec_viewport[0], vec_viewport[1], vec_viewport[2], vec_viewport[3]);
	initProjectMatrix();
}


void Projector::setFov(double f)
{
	if (f>max_fov) f = max_fov;
	if (f<min_fov) f = min_fov;

	fov = f;
	initProjectMatrix();

	fisheye_scale_factor = 1.0/fov*180./C_PI*2;
}

void Projector::setMaxFov(double max)
{
	if (fov > max) setFov(max);
	max_fov = max;
}


StelGeom::ConvexS Projector::unprojectViewport(void) const
{
	// This is quite ugly, but already better than nothing.
	// In fact this function should have different implementations
	// for the different mapping types. And maskType, viewport_fov_diameter,
	// viewport_center, viewport_xywh must be taken into account, too.
	// Last not least all halfplanes n*x>d really should have d<=0
	// or at least very small d/n.length().
	if ((fov < 90) && fov < 360.0) {
		Vec3d e0,e1,e2,e3;
		bool ok;
		if (fov >= 120.0) {
			unprojectJ2000(viewport_center[0],viewport_center[1],e0);
			StelGeom::ConvexS rval(1);
			rval[0].n = e0;
			rval[0].d = (fov<360.0) ? cos(fov*(C_PI/360.0)) : -1.0;
			return rval;
		}
		ok  = unprojectJ2000(viewport_center[0] - 0.5*viewport_fov_diameter, viewport_center[1] - 0.5*viewport_fov_diameter,e0);
		ok &= unprojectJ2000(viewport_center[0] + 0.5*viewport_fov_diameter, viewport_center[1] + 0.5*viewport_fov_diameter,e2);
		//~ if (needGlFrontFaceCW()) {
			//~ ok &= unprojectJ2000(viewport_center[0] - 0.5*viewport_fov_diameter, viewport_center[1] + 0.5*viewport_fov_diameter,e3);
			//~ ok &= unprojectJ2000(viewport_center[0] + 0.5*viewport_fov_diameter, viewport_center[1] - 0.5*viewport_fov_diameter,e1);
		//~ } else {
			ok &= unprojectJ2000(viewport_center[0] - 0.5*viewport_fov_diameter, viewport_center[1] + 0.5*viewport_fov_diameter,e1);
			ok &= unprojectJ2000(viewport_center[0] + 0.5*viewport_fov_diameter, viewport_center[1] - 0.5*viewport_fov_diameter,e3);
		//~ }

		if (ok) {
			StelGeom::HalfSpace h0(e0^e1);
			StelGeom::HalfSpace h1(e1^e2);
			StelGeom::HalfSpace h2(e2^e3);
			StelGeom::HalfSpace h3(e3^e0);
			if (h0.contains(e2) && h0.contains(e3) &&
			        h1.contains(e3) && h1.contains(e0) &&
			        h2.contains(e0) && h2.contains(e1) &&
			        h3.contains(e1) && h3.contains(e2)) {
				StelGeom::ConvexS rval(4);
				rval[0] = h0;
				rval[1] = h1;
				rval[2] = h2;
				rval[3] = h3;
				return rval;
			} else {
				Vec3d middle;
				if (unprojectJ2000(vec_viewport[0]+0.5*vec_viewport[2], vec_viewport[1]+0.5*vec_viewport[3],middle)) {
					double d = middle*e0;
					double h = middle*e1;
					if (d > h) d = h;
					h = middle*e2;
					if (d > h) d = h;
					h = middle*e3;
					if (d > h) d = h;
					StelGeom::ConvexS rval(1);
					rval[0].n = middle;
					rval[0].d = d;
					return rval;
				}
			}
		}
	}
	StelGeom::ConvexS rval(1);
	rval[0].n = Vec3d(1.0,0.0,0.0);
	rval[0].d = -2.0;
	return rval;
}

void Projector::setClippingPlanes(double znear, double zfar)
{
	zNear = znear;
	zFar = zfar;
	initProjectMatrix();
}


bool Projector::checkInViewport(const Vec3d& pos) const
{
	return 	(pos[1]>vec_viewport[1] && pos[1]<(vec_viewport[1] + vec_viewport[3]) &&
	         pos[0]>vec_viewport[0] && pos[0]<(vec_viewport[0] + vec_viewport[2]));
}

// to support large object check
bool Projector::checkInMask(const Vec3d& pos, const int object_pixel_radius) const
{
	float radius = sqrt( powf(pos[0]-viewport_center[0], 2) + powf(pos[1]-viewport_center[1], 2));
	return 	(radius - 1.75 * object_pixel_radius <= viewport_radius);  // 1.75 is safety factor
}



void Projector::changeFov(double deltaFov)
{
	if (deltaFov) setFov(fov+deltaFov);
}


bool Projector::projectCustom(const Vec3d &v,Vec3d &win, const Mat4d &mat) const
{
	win[0] = mat.r[0]*v[0] + mat.r[4]*v[1] +  mat.r[8]*v[2] + mat.r[12];
	win[1] = mat.r[1]*v[0] + mat.r[5]*v[1] +  mat.r[9]*v[2] + mat.r[13];
	win[2] = mat.r[2]*v[0] + mat.r[6]*v[1] + mat.r[10]*v[2] + mat.r[14];
	const double depth = win.length();
	const double rq1 = win[0]*win[0]+win[1]*win[1];

	if (rq1 <= 0 ) {
		if (win[2] < 0.0) {
			win[0] = viewport_center[0];
			win[1] = viewport_center[1];
			win[2] = 1.0;
			return true;
		}
		win[0] = viewport_center[0];
		win[1] = viewport_center[1];
		win[2] = -1e99;
		return false;
	}

	const double oneoverh = 1.0/sqrt(rq1);

	const double a = C_PI_2 + atan(win[2]*oneoverh);

	double f = a * fisheye_scale_factor;
	f *= viewport_radius * oneoverh;

	win[0] = viewport_center[0] + win[0] * f;
	win[1] = viewport_center[1] + win[1] * f;

	win[2] = (fabs(depth) - zNear) / (zFar-zNear);
	return (a<0.9*C_PI) ? true : false;

}

bool Projector::projectCustomFixedFov(const Vec3d &v,Vec3d &win, const Mat4d &mat) const
{
	win[0] = mat.r[0]*v[0] + mat.r[4]*v[1] +  mat.r[8]*v[2] + mat.r[12];
	win[1] = mat.r[1]*v[0] + mat.r[5]*v[1] +  mat.r[9]*v[2] + mat.r[13];
	const double depth = win[2] = mat.r[2]*v[0] + mat.r[6]*v[1] + mat.r[10]*v[2] + mat.r[14];
	const double rq1 = win[0]*win[0]+win[1]*win[1];

	if (rq1 <= 0 ) {
		if (win[2] < 0.0) {
			win[0] = viewport_center[0];
			win[1] = viewport_center[1];
			win[2] = 1.0;
			return true;
		}
		win[0] = viewport_center[0];
		win[1] = viewport_center[1];
		win[2] = -1e99;
		return false;
	}

	const double oneoverh = 1.0/sqrt(rq1);

	const double a = C_PI_2 + atan(win[2]*oneoverh);

	// TODO this is not exact, should use init fov
	double f = a / C_PI_2; // *fov_scale;

	f *= viewport_radius * oneoverh;

	win[0] = viewport_center[0] + /*shear_horz * */ win[0] * f;
	win[1] = viewport_center[1] + win[1] * f;

	win[2] = (fabs(depth) - zNear) / (zFar-zNear);
	return (a<0.9*C_PI) ? true : false;

}


void Projector::unproject(double x, double y, const Mat4d& m, Vec3d& v) const
{
	double d = getViewportRadius();

	//	printf("unproject x,y: %f, %f   cx,cy: %f, %f\n", x, y, center[0], center[1]);
	v[0] = (x - viewport_center[0]); //shear_horz;
	v[1] = y - viewport_center[1];
	v[2] = 0;

	double length = v.length()/d;

	//  printf("viewport radius = %f, length = %f \n", d, length);

	double angle_center = length * fov/2*C_PI/180;
	double r = sin(angle_center);

	if (length!=0) {
		v.normalize();
		v*= r;
		v[2] = sqrt(1.-(v[0]*v[0]+v[1]*v[1]));
	} else {
		v.set(0.,0.,1.);
	}

	if (angle_center>C_PI_2) v[2] = -v[2];

	v.transfo4d(m);
}



// Set the standard modelview matrices used for projection
void Projector::setModelViewMatrices(	const Mat4d& _mat_earth_equ_to_eye,
                                        const Mat4d& _mat_earth_equ_to_eye_fixed,
                                        const Mat4d& _mat_helio_to_eye,
                                        const Mat4d& _mat_local_to_eye,
                                        const Mat4d& _mat_j2000_to_eye,
                                        const Mat4d& _mat_dome,
                                        const Mat4d& _mat_dome_fixed)
{
	mat_earth_equ_to_eye = _mat_earth_equ_to_eye;
	mat_earth_equ_to_eye_fixed = _mat_earth_equ_to_eye_fixed;
	mat_j2000_to_eye = _mat_j2000_to_eye;
	mat_helio_to_eye = _mat_helio_to_eye;
	mat_local_to_eye = _mat_local_to_eye;
	mat_dome = _mat_dome;
	mat_dome_fixed = _mat_dome_fixed;

	inv_mat_earth_equ_to_eye = (mat_projection*mat_earth_equ_to_eye).fastInverse();
	inv_mat_earth_equ_to_eye_fixed = (mat_projection*mat_earth_equ_to_eye).fastInverse();
	inv_mat_j2000_to_eye = (mat_projection*mat_j2000_to_eye).fastInverse();
	inv_mat_helio_to_eye = (mat_projection*mat_helio_to_eye).fastInverse();
	inv_mat_local_to_eye = (mat_projection*mat_local_to_eye).fastInverse();
	inv_mat_dome = (mat_projection*mat_dome).fastInverse();
	inv_mat_dome_fixed = (mat_projection*mat_dome_fixed).fastInverse();
}

// Update auto_zoom if activated
void Projector::updateAutoZoom(int delta_time, bool manual_zoom)
{
	if (flag_auto_zoom) {
		// Use a smooth function
		double c;

// - manual zoom out (semi auto actually) requires fast at start to be smooth
		if ( manual_zoom || zoom_move.start > zoom_move.aim ) {
			// slow down as approach final view
			c = 1 - (1-zoom_move.coef)*(1-zoom_move.coef)*(1-zoom_move.coef);
		} else {
			// speed up as leave zoom target
			c = (zoom_move.coef)*(zoom_move.coef)*(zoom_move.coef);
		}

		setFov(zoom_move.start + (zoom_move.aim - zoom_move.start) * c);
		zoom_move.coef+=zoom_move.speed*delta_time;
		if (zoom_move.coef>=1.) {
			flag_auto_zoom = 0;
			setFov(zoom_move.aim);
		}
	}
}

// Zoom to the given field of view
void Projector::zoomTo(double aim_fov, float move_duration)
{

	if ( flag_auto_zoom && fabs(zoom_move.aim - aim_fov) <= .0000001f ) {
		//    cout << "Already zooming here\n";
		return;  // already zooming to this fov!
	}

	zoom_move.aim=aim_fov;
	zoom_move.start=fov;
	zoom_move.speed=1.f/(move_duration*1000);
	zoom_move.coef=0.;
	flag_auto_zoom = true;
}

//! Override glVertex3f
//! Here is the main trick for texturing in fisheye mode : The trick is to compute the
//! new coordinate in orthographic projection which will simulate the fisheye projection.
void Projector::sVertex3(double x, double y, double z, const Mat4d& mat) const
{
	Vec3d win;
	Vec3d v(x,y,z);
	projectCustom(v, win, mat);

	// Can be optimized by avoiding matrix inversion if it's always the same
	gluUnProject(win[0],win[1],win[2],mat,mat_projection,vec_viewport,&v[0],&v[1],&v[2]);
	//~ cout << "out " << v[0] << " " << v[1] << " " << v[2] << endl;
	glVertex3dv(v);
}

Vec3d Projector::sVertex3v(double x, double y, double z, const Mat4d& mat) const
{
	Vec3d win;
	Vec3d v(x,y,z);
	projectCustom(v, win, mat);

	// Can be optimized by avoiding matrix inversion if it's always the same
	gluUnProject(win[0],win[1],win[2],mat,mat_projection,vec_viewport,&v[0],&v[1],&v[2]);
	// cout << "out " << v[0] << " " << v[1] << " " << v[2] << endl;
	return v;
}

//~ // work of Jerome Lartillot
//~ void Projector::sModel3D(model3D *model ,GLdouble radius, const Mat4d& mat) const
//~ {
//~ glPushMatrix();
//~ glLoadMatrixd(mat);

//~ static Vec3d w;
//~ GLfloat x, y, z;

//~ for (size_t i = 0; i < model->shapes.size(); i++) {
//~ glBegin(GL_TRIANGLES);

//~ //  printf("model sharpes.mesh.indices.size %li\n",model->shapes[i].mesh.indices.size());
//~ for(size_t v = 0; v < model->shapes[i].mesh.indices.size() / 3; v++) {
//~ int i1=model->shapes[i].mesh.indices[3*v+0],
//~ i2=model->shapes[i].mesh.indices[3*v+1],
//~ i3=model->shapes[i].mesh.indices[3*v+2];

//~ glTexCoord2f(model->shapes[i].mesh.texcoords[2*i1+0], model->shapes[i].mesh.texcoords[2*i1+1]);
//~ x = model->shapes[i].mesh.positions[3*i1+0];
//~ y = model->shapes[i].mesh.positions[3*i1+1];
//~ z = model->shapes[i].mesh.positions[3*i1+2];

//~ glNormal3f(x, y, z);
//~ glColor3f(x * radius,y * radius,z * radius);
//~ w = sVertex3v(x * radius, y * radius, z * radius, mat);
//~ glVertex3dv(w);

//~ glTexCoord2f(model->shapes[i].mesh.texcoords[2*i2+0], model->shapes[i].mesh.texcoords[2*i2+1]);
//~ x = model->shapes[i].mesh.positions[3*i2+0];
//~ y = model->shapes[i].mesh.positions[3*i2+1];
//~ z = model->shapes[i].mesh.positions[3*i2+2];

//~ glNormal3f(x, y, z);
//~ glColor3f(x * radius,y * radius,z * radius);
//~ w=sVertex3v(x * radius, y * radius, z * radius, mat);
//~ glVertex3dv(w);

//~ glTexCoord2f(model->shapes[i].mesh.texcoords[2*i3+0], model->shapes[i].mesh.texcoords[2*i3+1]);
//~ x = model->shapes[i].mesh.positions[3*i3+0];
//~ y = model->shapes[i].mesh.positions[3*i3+1];
//~ z = model->shapes[i].mesh.positions[3*i3+2];

//~ glNormal3f(x, y, z);
//~ glColor3f(x * radius,y * radius,z * radius);
//~ w=sVertex3v(x * radius, y * radius, z * radius, mat);
//~ glVertex3dv(w);
//~ }
//~ glEnd();
//~ }

//~ glPopMatrix();
//~ }

//~ //return GLfoat's for vbo usage
//~ void Projector::getSphereBody(GLint slices, GLint stacks, GLfloat * dataTexture, GLfloat * dataPosition)
//~ {
//~ unsigned int indiceNormal=0;
//~ unsigned int indiceTexture=0;
//~ unsigned int indicePosition=0;
//~ static Vec3d v;

//~ GLfloat x, y, z;
//~ GLfloat s, t;
//~ GLint i, j;

//~ t=1.0;

//~ const GLfloat drho = C_PI / (GLfloat) stacks;
//~ double cos_sin_rho[2*(stacks+1)];
//~ double *cos_sin_rho_p = cos_sin_rho;
//~ for (i = 0; i <= stacks; i++) {
//~ double rho = i * drho;
//~ *cos_sin_rho_p++ = cos(rho);
//~ *cos_sin_rho_p++ = sin(rho);
//~ }

//~ const GLfloat dtheta = 2.0 * C_PI / (GLfloat) slices;
//~ double cos_sin_theta[2*(slices+1)];
//~ double *cos_sin_theta_p = cos_sin_theta;
//~ for (i = 0; i <= slices; i++) {
//~ double theta = (i == slices) ? 0.0 : i * dtheta;
//~ *cos_sin_theta_p++ = cos(theta);
//~ *cos_sin_theta_p++ = sin(theta);
//~ }

//~ // texturing: s goes from 0.0/0.25/0.5/0.75/1.0 at +y/+x/-y/-x/+y axis
//~ // t goes from -1.0/+1.0 at z = -radius/+radius (linear along longitudes)
//~ // cannot use triangle fan on texturing (s coord. at top/bottom tip varies)
//~ const GLfloat ds = 1.0 / slices;
//~ const GLfloat dt = 1.0 / stacks;

//~ // draw intermediate as quad strips
//~ for (i = 0,cos_sin_rho_p = cos_sin_rho; i < stacks; i++,cos_sin_rho_p+=2) {
//~ s = 0.0;
//~ for (j = 0,cos_sin_theta_p = cos_sin_theta; j <= slices; j++,cos_sin_theta_p+=2) {
//~ x = -cos_sin_theta_p[1] * cos_sin_rho_p[1];
//~ y = cos_sin_theta_p[0] * cos_sin_rho_p[1];
//~ z = 1.0 * cos_sin_rho_p[0];

//~ glTexCoord2f(s, t);
//~ dataTexture[indiceTexture]=s; indiceTexture++;
//~ dataTexture[indiceTexture]=t; indiceTexture++;
//~ glNormal3f(x, y, z);
//~ dataNormal[indiceNormal]=x; indiceNormal++;
//~ dataNormal[indiceNormal]=y; indiceNormal++;
//~ dataNormal[indiceNormal]=z; indiceNormal++;
//~ glColor3f(x * radius,y * radius,z * one_minus_oblateness * radius);
//~ v = sVertex3v(x * radius, y * radius, z * one_minus_oblateness * radius, mat);
//~ glVertex3dv(v);
//~ dataPosition[indicePosition]=x; indicePosition++;
//~ dataPosition[indicePosition]=y; indicePosition++;
//~ dataPosition[indicePosition]=z; indicePosition++;

//~ x = -cos_sin_theta_p[1] * cos_sin_rho_p[3];
//~ y = cos_sin_theta_p[0] * cos_sin_rho_p[3];
//~ z = 1.0 * cos_sin_rho_p[2];

//~ glTexCoord2f(s, t - dt);
//~ dataTexture[indiceTexture]=s; indiceTexture++;
//~ dataTexture[indiceTexture]=t; indiceTexture++;

//~ glNormal3f(x, y, z);
//~ dataNormal[indiceNormal]=x; indiceNormal++;
//~ dataNormal[indiceNormal]=y; indiceNormal++;
//~ dataNormal[indiceNormal]=z; indiceNormal++;

//~ glColor3f(x * radius,y * radius,z * one_minus_oblateness * radius);
//~ v= sVertex3v(x * radius, y * radius, z * one_minus_oblateness * radius, mat);
//~ glVertex3dv(v);
//~ dataPosition[indicePosition]=x; indicePosition++;
//~ dataPosition[indicePosition]=y; indicePosition++;
//~ dataPosition[indicePosition]=z; indicePosition++;

//~ s += ds;
//~ }
//~ t -= dt;
//~ }
//~ }



//~ // Reimplementation of gluSphere : glu is overrided for non standard projection
//~ void Projector::sSphereShader(GLdouble radius, GLdouble scaledRadius, GLdouble one_minus_oblateness, GLint slices, GLint stacks, const Mat4d& mat) const
//~ {
//~ cout << "draw sphere" << endl;
//~ glPushMatrix();
//~ glLoadMatrixd(mat);
//~ static Vec3d v;

//~ GLfloat x, y, z;
//~ GLfloat s, t;
//~ GLint i, j;

//~ t=1.0;


//~ const GLfloat drho = C_PI / (GLfloat) stacks;
//~ double cos_sin_rho[2*(stacks+1)];
//~ double *cos_sin_rho_p = cos_sin_rho;
//~ for (i = 0; i <= stacks; i++) {
//~ double rho = i * drho;
//~ *cos_sin_rho_p++ = cos(rho);
//~ *cos_sin_rho_p++ = sin(rho);
//~ }

//~ const GLfloat dtheta = 2.0 * C_PI / (GLfloat) slices;
//~ double cos_sin_theta[2*(slices+1)];
//~ double *cos_sin_theta_p = cos_sin_theta;
//~ for (i = 0; i <= slices; i++) {
//~ double theta = (i == slices) ? 0.0 : i * dtheta;
//~ *cos_sin_theta_p++ = cos(theta);
//~ *cos_sin_theta_p++ = sin(theta);
//~ }

// texturing: s goes from 0.0/0.25/0.5/0.75/1.0 at +y/+x/-y/-x/+y axis
// t goes from -1.0/+1.0 at z = -radius/+radius (linear along longitudes)
// cannot use triangle fan on texturing (s coord. at top/bottom tip varies)
//~ const GLfloat ds = 1.0 / slices;
//~ const GLfloat dt = 1.0 / stacks;

//~ // draw intermediate as quad strips
//~ for (i = 0,cos_sin_rho_p = cos_sin_rho; i < stacks; i++,cos_sin_rho_p+=2) {
//~ glBegin(GL_TRIANGLE_STRIP);
//~ s = 0.0;
//~ for (j = 0,cos_sin_theta_p = cos_sin_theta; j <= slices; j++,cos_sin_theta_p+=2) {
//~ x = -cos_sin_theta_p[1] * cos_sin_rho_p[1];
//~ y = cos_sin_theta_p[0] * cos_sin_rho_p[1];
//~ z = 1.0 * cos_sin_rho_p[0];

//~ glTexCoord2f(s, t);
//~ glNormal3f(x, y, z);
//~ glColor3f(x * radius,y * radius,z * one_minus_oblateness * radius);
//~ v = sVertex3v(x * scaledRadius, y * scaledRadius, z * one_minus_oblateness * scaledRadius, mat);
//~ glVertex3dv(v);

//~ x = -cos_sin_theta_p[1] * cos_sin_rho_p[3];
//~ y = cos_sin_theta_p[0] * cos_sin_rho_p[3];
//~ z = 1.0 * cos_sin_rho_p[2];

//~ glTexCoord2f(s, t - dt);
//~ glNormal3f(x, y, z);
//~ glColor3f(x * radius,y * radius,z * one_minus_oblateness * radius);
//~ v = sVertex3v(x * scaledRadius, y * scaledRadius, z * one_minus_oblateness * scaledRadius, mat);
//~ glVertex3dv(v);

//~ s += ds;
//~ }
//~ glEnd();
//~ t -= dt;
//~ }
//~ glPopMatrix();
//~ }

//~ //! save on spherical texture sizes since a lot is transparent
//~ //! Draw only a partial sphere with top and/or bottom missing

//~ //! bottom_altitude is angle above (- for below) horizon for bottom of texture
//~ //! top_altitude is altitude angle for top of texture both are in degrees

//~ void Projector::sPartialSphere(GLdouble radius, GLdouble one_minus_oblateness,
//~ GLint slices, GLint stacks,
//~ const Mat4d& mat, int orient_inside,
//~ double bottom_altitude, double top_altitude ) const
//~ {
//~ glPushMatrix();
//~ glLoadMatrixd(mat);

//~ double bottom = C_PI / 180. * bottom_altitude;
//~ double angular_height = C_PI / 180. * top_altitude - bottom;

//~ GLfloat x, y, z;
//~ GLfloat s, t;
//~ GLint i, j;
//~ GLfloat nsign;

//~ if (orient_inside) {
//~ nsign = -1.0;
//~ t=0.0; // from inside texture is reversed
//~ } else {
//~ nsign = 1.0;
//~ t=1.0;
//~ }

//~ const GLfloat drho = angular_height / (GLfloat) stacks;
//~ double cos_sin_rho[2*(stacks+1)];
//~ double *cos_sin_rho_p = cos_sin_rho;
//~ for (i = 0; i <= stacks; i++) {
//~ double rho = C_PI_2 + bottom + i * drho;
//~ *cos_sin_rho_p++ = cos(rho);
//~ *cos_sin_rho_p++ = sin(rho);
//~ }

//~ const GLfloat dtheta = 2.0 * C_PI / (GLfloat) slices;
//~ double cos_sin_theta[2*(slices+1)];
//~ double *cos_sin_theta_p = cos_sin_theta;
//~ for (i = 0; i <= slices; i++) {
//~ double theta = (i == slices) ? 0.0 : i * dtheta;
//~ *cos_sin_theta_p++ = cos(theta);
//~ *cos_sin_theta_p++ = sin(theta);
//~ }

//~ // texturing: s goes from 0.0/0.25/0.5/0.75/1.0 at +y/+x/-y/-x/+y axis
//~ // t goes from -1.0/+1.0 at z = -radius/+radius (linear along longitudes)
//~ // cannot use triangle fan on texturing (s coord. at top/bottom tip varies)
//~ const GLfloat ds = 1.0 / slices;
//~ const GLfloat dt = nsign / stacks; // from inside texture is reversed


//~ // draw intermediate  as quad strips
//~ for (i = 0,cos_sin_rho_p = cos_sin_rho; i < stacks; i++,cos_sin_rho_p+=2) {
//~ glBegin(GL_QUAD_STRIP);
//~ s = 0.0;
//~ for (j = 0,cos_sin_theta_p = cos_sin_theta; j <= slices;
//~ j++,cos_sin_theta_p+=2) {
//~ x = -cos_sin_theta_p[1] * cos_sin_rho_p[1];
//~ y = cos_sin_theta_p[0] * cos_sin_rho_p[1];
//~ z = nsign * cos_sin_rho_p[0];

//~ glTexCoord2f(s, t);
//~ sVertex3(x * radius, y * radius, z * one_minus_oblateness * radius, mat);

//~ x = -cos_sin_theta_p[1] * cos_sin_rho_p[3];
//~ y = cos_sin_theta_p[0] * cos_sin_rho_p[3];
//~ z = nsign * cos_sin_rho_p[2];

//~ glTexCoord2f(s, t - dt);
//~ sVertex3(x * radius, y * radius, z * one_minus_oblateness * radius, mat);
//~ s += ds;
//~ }
//~ glEnd();
//~ t -= dt;
//~ }
//~ glPopMatrix();
//~ }

//~ //! Reimplementation of gluCylinder : glu is overrided for non standard projection
//~ void Projector::sCylinder(GLdouble radius, GLdouble height, GLint slices, GLint stacks, const Mat4d& mat) const
//~ {
//~ glPushMatrix();
//~ glLoadMatrixd(mat);

//~ static GLdouble da, r, dz;
//~ static GLfloat z ;
//~ static GLint i, j;

//~ glCullFace(GL_FRONT);

//~ da = 2.0 * C_PI / slices;
//~ dz = height / stacks;

//~ GLfloat ds = 1.0 / slices;
//~ GLfloat dt = 1.0 / stacks;
//~ GLfloat t = 0.0;
//~ z = 0.0;
//~ r = radius;
//~ for (j = 0; j < stacks; j++) {
//~ GLfloat s = 0.0;
//~ glBegin(GL_QUAD_STRIP);
//~ for (i = 0; i <= slices; i++) {
//~ GLfloat x, y;
//~ if (i == slices) {
//~ x = sinf(0.0);
//~ y = cosf(0.0);
//~ } else {
//~ x = sinf(i * da);
//~ y = cosf(i * da);
//~ }
//~ glNormal3f(x , y , 0);
//~ glTexCoord2f(s, t);
//~ sVertex3(x * r, y * r, z, mat);
//~ glNormal3f(x , y , 0);
//~ glTexCoord2f(s, t + dt);
//~ sVertex3(x * r, y * r, z + dz, mat);
//~ s += ds;
//~ }			/* for slices */
//~ glEnd();
//~ t += dt;
//~ z += dz;
//~ }				/* for stacks */

//~ glPopMatrix();
//~ glCullFace(GL_BACK);
//~ }

//~ // Draw a disk with a special texturing mode having texture center at disk center
//~ void Projector::sDisk(GLdouble radius, GLint slices, GLint stacks,
//~ const Mat4d& mat, int orient_inside) const
//~ {
//~ glPushMatrix();
//~ glLoadMatrixd(mat);

//~ GLfloat r, dr, theta, dtheta;
//~ GLfloat x, y;
//~ GLint j;
//~ GLfloat nsign;

//~ if (orient_inside) nsign = -1.0;
//~ else nsign = 1.0;

//~ dr = radius / (GLfloat) stacks;
//~ dtheta = 2.0 * C_PI / (GLfloat) slices;
//~ if (slices < 0) slices = -slices;

//~ // draw intermediate stacks as quad strips
//~ for (r = 0; r < radius; r+=dr) {
//~ glBegin(GL_TRIANGLE_STRIP);
//~ for (j = 0; j <= slices; j++) {
//~ theta = (j == slices) ? 0.0 : j * dtheta;
//~ x = r*cos(theta);
//~ y = r*sin(theta);
//~ glNormal3f(0, 0, nsign);
//~ glTexCoord2f(0.5+x/2/radius, 0.5+y/2/radius);
//~ sVertex3(x, y, 0, mat);
//~ x = (r+dr)*cos(theta);
//~ y = (r+dr)*sin(theta);
//~ glNormal3f(0, 0, nsign);
//~ glTexCoord2f(0.5+x/2/radius, 0.5+y/2/radius);
//~ sVertex3(x, y, 0, mat);
//~ }
//~ glEnd();
//~ }
//~ glPopMatrix();
//~ }

//~ void Projector::sRing(GLdouble r_min, GLdouble r_max, GLint slices, GLint stacks, const Mat4d& mat, bool h) const
//~ {
//~ glPushMatrix();
//~ glLoadMatrixd(mat);

//~ double theta;
//~ double x,y;
//~ int j;

//~ const double dr = (r_max-r_min) / stacks;
//~ const double dtheta = 2.0 * C_PI / slices*(1-2*h);
//~ //if (slices < 0) slices = -slices;
//~ double cos_sin_theta[2*(slices+1)];
//~ double *cos_sin_theta_p = cos_sin_theta;
//~ for (j = 0; j <= slices; j++) {
//~ theta = (j == slices) ? 0.0 : j * dtheta;
//~ *cos_sin_theta_p++ = cos(theta);
//~ *cos_sin_theta_p++ = sin(theta);
//~ }

//~ //int t=0;

//~ // draw intermediate stacks as quad strips
//~ for (double r = r_min; r < r_max; r+=dr) {
//~ const double tex_r0 = (r-r_min)/(r_max-r_min);
//~ const double tex_r1 = (r+dr-r_min)/(r_max-r_min);
//~ glBegin(GL_TRIANGLE_STRIP);
//~ for (j=0,cos_sin_theta_p=cos_sin_theta; j<=slices; j++,cos_sin_theta_p+=2) {
//~ theta = (j == slices) ? 0.0 : j * dtheta;

//~ x = r*cos_sin_theta_p[0];
//~ y = r*cos_sin_theta_p[1];
//~ //glNormal3d(0, 0, 1);
//~ glTexCoord2d(tex_r0, 0.5);
//~ glColor3f(x,y,0);
//~ sVertex3(x, y, 0, mat);

//~ x = (r+dr)*cos_sin_theta_p[0];
//~ y = (r+dr)*cos_sin_theta_p[1];
//~ //glNormal3d(0, 0, 1);
//~ glTexCoord2d(tex_r1, 0.5);
//~ glColor3f(x,y,0);
//~ sVertex3(x, y, 0, mat);

//~ //t=t+2;
//~ }
//~ glEnd();
//~ }
//~ //printf("nombre de points calculés : %i \n", t);
//~ glPopMatrix();
//~ }

//~ static inline void sSphereMapTexCoordFast(double rho_div_fov, double costheta, double sintheta)
//~ {
//~ if (rho_div_fov>0.5) rho_div_fov=0.5;
//~ glTexCoord2d(0.5 + rho_div_fov * costheta,
//~ 0.5 + rho_div_fov * sintheta);
//~ }

//~ void Projector::sSphere_map(GLdouble radius, GLint slices, GLint stacks,
//~ const Mat4d& mat, double texture_fov,
//~ int orient_inside) const
//~ {
//~ glPushMatrix();
//~ glLoadMatrixd(mat);

//~ double rho,x,y,z;
//~ int i, j;
//~ const double nsign = orient_inside?-1:1;

//~ const double drho = C_PI / stacks;
//~ double cos_sin_rho[2*(stacks+1)];
//~ double *cos_sin_rho_p = cos_sin_rho;
//~ for (i = 0; i <= stacks; i++) {
//~ const double rho = i * drho;
//~ *cos_sin_rho_p++ = cos(rho);
//~ *cos_sin_rho_p++ = sin(rho);
//~ }

//~ const double dtheta = 2.0 * C_PI / slices;
//~ double cos_sin_theta[2*(slices+1)];
//~ double *cos_sin_theta_p = cos_sin_theta;
//~ for (i = 0; i <= slices; i++) {
//~ const double theta = (i == slices) ? 0.0 : i * dtheta;
//~ *cos_sin_theta_p++ = cos(theta);
//~ *cos_sin_theta_p++ = sin(theta);
//~ }

//~ // texturing: s goes from 0.0/0.25/0.5/0.75/1.0 at +y/+x/-y/-x/+y axis
//~ // t goes from -1.0/+1.0 at z = -radius/+radius (linear along longitudes)
//~ // cannot use triangle fan on texturing (s coord. at top/bottom tip varies)
//~ const int imax = stacks;

//~ // draw intermediate stacks as quad strips
//~ if (!orient_inside) {
//~ for (i = 0,cos_sin_rho_p=cos_sin_rho,rho=0.0; i < imax; ++i,cos_sin_rho_p+=2,rho+=drho) {
//~ glBegin(GL_QUAD_STRIP);
//~ for (j=0,cos_sin_theta_p=cos_sin_theta; j<=slices; ++j,cos_sin_theta_p+=2) {
//~ x = -cos_sin_theta_p[1] * cos_sin_rho_p[1];
//~ y = cos_sin_theta_p[0] * cos_sin_rho_p[1];
//~ z = cos_sin_rho_p[0];
//~ glNormal3d(x * nsign, y * nsign, z * nsign);
//~ sSphereMapTexCoordFast(rho/texture_fov, cos_sin_theta_p[0], cos_sin_theta_p[1]);
//~ sVertex3(x * radius, y * radius, z * radius, mat);

//~ x = -cos_sin_theta_p[1] * cos_sin_rho_p[3];
//~ y = cos_sin_theta_p[0] * cos_sin_rho_p[3];
//~ z = cos_sin_rho_p[2];
//~ glNormal3d(x * nsign, y * nsign, z * nsign);
//~ sSphereMapTexCoordFast((rho + drho)/texture_fov, cos_sin_theta_p[0], cos_sin_theta_p[1]);
//~ sVertex3(x * radius, y * radius, z * radius, mat);
//~ }
//~ glEnd();
//~ }
//~ } else {
//~ for (i = 0,cos_sin_rho_p=cos_sin_rho,rho=0.0; i < imax; ++i,cos_sin_rho_p+=2,rho+=drho) {
//~ glBegin(GL_QUAD_STRIP);
//~ for (j=0,cos_sin_theta_p=cos_sin_theta; j<=slices; ++j,cos_sin_theta_p+=2) {
//~ x = -cos_sin_theta_p[1] * cos_sin_rho_p[3];
//~ y = cos_sin_theta_p[0] * cos_sin_rho_p[3];
//~ z = cos_sin_rho_p[2];
//~ glNormal3d(x * nsign, y * nsign, z * nsign);
//~ sSphereMapTexCoordFast((rho + drho)/texture_fov,cos_sin_theta_p[0],-cos_sin_theta_p[1]);
//~ sVertex3(x * radius, y * radius, z * radius, mat);

//~ x = -cos_sin_theta_p[1] * cos_sin_rho_p[1];
//~ y = cos_sin_theta_p[0] * cos_sin_rho_p[1];
//~ z = cos_sin_rho_p[0];
//~ glNormal3d(x * nsign, y * nsign, z * nsign);
//~ sSphereMapTexCoordFast(rho/texture_fov, cos_sin_theta_p[0], -cos_sin_theta_p[1]);
//~ sVertex3(x * radius, y * radius, z * radius, mat);
//~ }
//~ glEnd();
//~ }
//~ }
//~ glPopMatrix();
//~ }

void Projector::printGravity180(s_font* font, float x, float y, const string& str, Vec4f Color, bool speed_optimize, float xshift, float yshift) const
{
	static float dx, dy, d, theta, psi;

	// ASSUME disk viewport
	dx = x - viewport_center[0];
	dy = y - viewport_center[1];
	d = sqrt(dx*dx + dy*dy);

	// If the text is too far away to be visible in the screen return
	if (d>myMax(vec_viewport[3], vec_viewport[2])*2) return;


	theta = C_PI + atan2f(dx, dy - 1);
	psi = atan2f((float)font->getStrLen(str)/str.length(),d + 1) * 180./C_PI;

	if (psi>5) psi = 5;

	Mat4f MVP = getMatProjectionOrtho2D();
	Mat4f TRANSFO = Mat4f::translation( Vec3f(x,y,0) );
	TRANSFO = TRANSFO*Mat4f::rotation( Vec3f(0,0,-1), -theta);
	TRANSFO = TRANSFO*Mat4f::translation( Vec3f(xshift, -yshift, 0) );
	TRANSFO = TRANSFO*Mat4f::scaling( Vec3f(1, -1, 1) );

	font->print(0, 0, str, Color, MVP*TRANSFO ,0, speed_optimize);  // ASSUME speed optimized strings should be cached
}

