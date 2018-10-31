/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2017 Immersive Adventure
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

#include "orbit_2d.hpp"
#include "body_bigbody.hpp"
#include "perf_debug.hpp"
#include "file_path.hpp"
#include "trail.hpp"
#include "axis.hpp"
#include "hints.hpp"
#include "orbit_2d.hpp"
#include "halo.hpp"
#include "projector.hpp"
#include "navigator.hpp"
#include "log.hpp"
#include "ring.hpp"

using namespace std;

BigBody::BigBody(Body *parent,
                 const string& englishName,
                 BODY_TYPE _typePlanet,
                 bool flagHalo,
                 double radius,
                 double oblateness,
                 BodyColor* _myColor,
                 float _sol_local_day,
                 float albedo,
                 const string& tex_map_name,
                 Orbit *orbit,
                 bool close_orbit,
                 ObjL* _currentObj,
                 //~ bool _hidden,
                 //~ bool _deleteable,
                 double orbit_bounding_radius,
                 const string& tex_norm_name,
                 const string& tex_night_name,
                 const string& tex_specular_name,
                 const string& tex_cloud_name,
                 const string& tex_norm_cloud_name
                 ) :
	Body(parent,
	     englishName,
	     _typePlanet,
	     flagHalo,
	     radius,
	     oblateness,
	     _myColor,
	     _sol_local_day,
	     albedo,
	     tex_map_name,
	     orbit,
	     close_orbit,
	     _currentObj,
	     //~ _hidden,
	     //~ _deleteable,
	     orbit_bounding_radius,
	     tex_norm_name),
	rings(NULL), tex_night(nullptr), tex_specular(nullptr), tex_cloud(nullptr), tex_shadow_cloud(nullptr), tex_norm_cloud(nullptr)
{
	if (tex_night_name != "") {  // prépare au night_shader
		tex_night = new s_texture(FilePath(tex_night_name,FilePath::TFP::TEXTURE).toString(), TEX_LOAD_TYPE_PNG_SOLID_REPEAT, 1);
		tex_specular = new s_texture(FilePath(tex_specular_name,FilePath::TFP::TEXTURE).toString(), TEX_LOAD_TYPE_PNG_SOLID_REPEAT);
	}

	if(tex_cloud_name != "") {
		// Try to use cloud texture in any event, even if can not use shader
		tex_cloud = new s_texture(FilePath(tex_cloud_name,FilePath::TFP::TEXTURE).toString(), TEX_LOAD_TYPE_PNG_ALPHA, true);

		if(tex_norm_cloud_name=="") {
			Log.write("No cloud normal texture defined for " + englishName, cLog::LOG_TYPE::L_ERROR);
		} else {
			tex_norm_cloud = new s_texture(FilePath(tex_norm_cloud_name,FilePath::TFP::TEXTURE).toString(), TEX_LOAD_TYPE_PNG_SOLID_REPEAT, true);
		}
	}

	trail = new Trail(this,1460);
	orbitPlot = new Orbit2D(this);
	selectShader();
}

BigBody::~BigBody()
{
	if (rings) delete rings;
	rings = nullptr;

	if (tex_night) delete tex_night;
	tex_night = nullptr;
	if (tex_specular) delete tex_specular;
	tex_specular = nullptr;
	if (tex_cloud) delete tex_cloud;
	tex_cloud = nullptr;
	if (tex_norm_cloud) delete tex_norm_cloud;
	tex_norm_cloud = nullptr;

	if (trail) delete trail;
	trail = nullptr;
	if (orbitPlot) delete orbitPlot;
	orbitPlot = nullptr;
}

void BigBody::selectShader ()
{
	//~ bool useShaderNormal = true;

	if (tex_night) { //night Shader
		myShader = SHADER_NIGHT;
		myShaderProg = shaderNight;
		//~ useShaderNormal = false;
		return;
	}

	if (tex_norm) { //bump Shader
		myShader = SHADER_BUMP;
		myShaderProg = shaderBump;
		//~ useShaderNormal = false;
		return;
	}

	if (rings) { //ringed Shader
		myShader = SHADER_RINGED;
		myShaderProg = shaderRinged;
		//~ useShaderNormal = false;
		return;
	}

	//~ if (useShaderNormal) {
		myShader = SHADER_NORMAL;
		myShaderProg = shaderNormal;
	//~ }
}

float BigBody::getOnScreenSize(const Projector* prj, const Navigator * nav, bool orb_only)
{
	double rad;
	if (rings && !orb_only) rad = rings->getOuterRadius();
	else
		rad = radius;

	return atanf(rad*2.f/getEarthEquPos(nav).length())*180./C_PI/prj->getFov()*prj->getViewportHeight();
}

void BigBody::removeSatellite(Body *planet)
{
	list<Body *>::iterator iter;
	for (iter=satellites.begin(); iter != satellites.end(); iter++) {
		if ( (*iter) == planet ) {
			satellites.erase(iter);
			break;
		}
	}
}

double BigBody::calculateBoundingRadius()
{
	// TODO: only have boundary function for elliptical orbits right now
	//~ if ( satellites.empty() ) {
		//~ boundingRadius = -1;
		//~ return -1;  // special case, not visible
	//~ }

	double d = radius.final();

	if (rings) d = rings->getOuterRadius();

	list<Body *>::const_iterator iter;
	list<Body *>::const_iterator end = satellites.end();
	//	cout << englishName << endl;

	double r;
	for ( iter=satellites.begin(); iter != end; iter++) {

		r = (*iter)->getBoundingRadius();
		//		cout << "\t" << (*iter)->getEnglishName() << " : " << r << endl;
		if ( r > d ) d = r;
	}

	boundingRadius = d;
	return boundingRadius;
}
/*
void BigBody::drawGL(Projector* prj, const Navigator* nav, const Observer* observatory, const ToneReproductor* eye, bool stencil, bool depthTest, bool drawHomePlanet, bool selected)
{
	////AutoPerfDebug apd(&pd, "BigBody::drawGL$"); //Debug
	if (hidden)
		return;
	//on ne dessine pas une planete sur laquel on se trouve
	if (!drawHomePlanet && observatory->isOnBody(this)) {
		if (rings)
			rings->draw(prj,mat,1000.0,lightDirection,eye_planet,radius);
		return ; // 0;
	}

	orbitPlot->drawOrbit(nav,prj,parent_mat);

	if(trail != nullptr)
		trail->drawTrail(nav, prj);

	if (ang_dist>0.25)// {
		//~ if (ang_dist>1.f)
			//~ ang_dist = 1.f;
		hints->drawHints(nav, prj);
	//~ }

	if (depthTest)
		StateGL::enable(GL_DEPTH_TEST);

	if (screen_sz > 1 && isVisible) {  // huge improvement in performance

		if (rings) {
			StateGL::enable(GL_DEPTH_TEST);
			axis->drawAxis(prj, mat);
			drawBody(prj, nav, mat, screen_sz);
			rings->draw(prj,mat,screen_sz,lightDirection,eye_planet,radius);
		} else {
			if (stencil)
				StateGL::enable(GL_STENCIL_TEST);

			axis->drawAxis(prj, mat);
			drawBody(prj, nav, mat, screen_sz);
			if (stencil)
				StateGL::disable(GL_STENCIL_TEST);
		}
	}

	if (isVisible && flagHalo && this->getSphereScale()<10.0) {
		// Workaround for depth buffer precision and near planets
		StateGL::disable(GL_DEPTH_TEST);
		halo->drawHalo(nav, prj, eye);
	}

	StateGL::disable(GL_DEPTH_TEST);
}*/

void BigBody::drawBody(const Projector* prj, const Navigator * nav, const Mat4d& mat, float screen_sz)
{	
	//~ AutoPerfDebug apd(&pd, "BigBody::drawBody$"); //Debug
	glEnable(GL_TEXTURE_2D);
	StateGL::enable(GL_CULL_FACE);
	StateGL::disable(GL_BLEND);

	glBindTexture(GL_TEXTURE_2D, tex_map->getID());

	myShaderProg->use();

	//load specific values for shader
	switch (myShader) {

		case SHADER_BUMP:
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, tex_map->getID());
			//~ myShaderProg->setUniform("mapTexture",0);

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, tex_norm->getID());
			//~ myShaderProg->setUniform("normalTexture",1);

			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, tex_eclipse_map->getID());
			//~ myShaderProg->setUniform("shadowTexture",2);
			break;

		case SHADER_NIGHT :
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, tex_map->getID());

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, tex_eclipse_map->getID());

			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, tex_night->getID());

			if (tex_norm) {
				glActiveTexture(GL_TEXTURE6);
				glBindTexture(GL_TEXTURE_2D, tex_norm->getID());
			}

			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, tex_specular->getID());

			if(tex_cloud) {
				glActiveTexture(GL_TEXTURE4);
				glBindTexture(GL_TEXTURE_2D, tex_cloud->getID());
				myShaderProg->setUniform("CloudTexture",4);

				glActiveTexture(GL_TEXTURE5);
				glBindTexture(GL_TEXTURE_2D, tex_norm_cloud->getID());
				myShaderProg->setUniform("CloudNormalTexture",5);
			}
			if ((Body::flagClouds && tex_norm_cloud)==1)
				myShaderProg->setUniform("Clouds",1);
			else
				myShaderProg->setUniform("Clouds",0);
			break;

		case SHADER_RINGED :
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, tex_map->getID());

			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, tex_eclipse_map->getID());

			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, (rings->getTexID()));

			myShaderProg->setUniform("RingInnerRadius",rings->getInnerRadius());
			myShaderProg->setUniform("RingOuterRadius",rings->getOuterRadius());

			break;

		case SHADER_NORMAL :
		default: //shader normal
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, tex_map->getID());

			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, tex_eclipse_map->getID());
			break;
	}

	//paramétrage des matrices pour opengl4
	Mat4f proj = prj->getMatProjection().convert();
	Mat4f matrix=mat.convert();
	matrix = matrix * Mat4f::zrotation(C_PI/180*(axis_rotation + 90));

	Mat4f inv_matrix = matrix.inverse();
	myShaderProg->setUniform("ModelViewProjectionMatrix",proj*matrix);
	myShaderProg->setUniform("inverseModelViewProjectionMatrix",(proj*matrix).inverse());
	myShaderProg->setUniform("ModelViewMatrix",matrix);
	myShaderProg->setUniform("clipping_fov",prj->getClippingFov());
	myShaderProg->setUniform("planetScaledRadius",radius);

	if ( myShader == SHADER_RINGED )
		myShaderProg->setUniform("ModelViewMatrixInverse", inv_matrix);

	if ( myShader == SHADER_NIGHT ) {
		myShaderProg->setUniform("ViewProjection", proj*view);
		myShaderProg->setUniform("Model", model);
	}


	//paramètres commun aux shaders sauf Sun
	myShaderProg->setUniform("planetRadius",initialRadius);
	myShaderProg->setUniform("planetOneMinusOblateness",one_minus_oblateness);
	//utilisable si on est pas le soleil
	myShaderProg->setUniform("ModelViewMatrix",matrix);
	myShaderProg->setUniform("NormalMatrix", inv_matrix.transpose());

	int index=1;
	myShaderProg->setUniform("LightPosition",eye_sun);
	myShaderProg->setUniform("SunHalfAngle",sun_half_angle);

	double length;
	double moonDotLight;
	Vec3f tmp= v3fNull;
	Vec3f tmp2(0.4, 0.12, 0.0);

	if (myShader == SHADER_BUMP) {
		if(getEnglishName() == "Moon") {
			myShaderProg->setUniform("UmbraColor",tmp2);
		} else
			myShaderProg->setUniform("UmbraColor",tmp);
	}

	Vec3d planet_helio = get_heliocentric_ecliptic_pos();
	Vec3d light = -planet_helio;
	light.normalize();

	list<Body*>::iterator iter;
	for(iter=satellites.begin(); iter!=satellites.end() && index <= 4; iter++) {
		tmp2 = (*iter)->get_heliocentric_ecliptic_pos() - planet_helio;
		length = tmp2.length();
		tmp2.normalize();
		moonDotLight = tmp2.dot(light);
		if(moonDotLight > 0 && length*sin(acos(moonDotLight)) <= radius + 2*(*iter)->getRadius()) {
			tmp = nav->getHelioToEyeMat() * (*iter)->get_heliocentric_ecliptic_pos();

			if (index==1) {
				myShaderProg->setUniform("MoonPosition1",tmp);
				myShaderProg->setUniform("MoonRadius1",(*iter)->getRadius()/(*iter)->getSphereScale());
			} else if (index==2) {
				myShaderProg->setUniform("MoonPosition2",tmp);
				myShaderProg->setUniform("MoonRadius2",(*iter)->getRadius()/(*iter)->getSphereScale());
			} else if (index==3) {
				myShaderProg->setUniform("MoonPosition3",tmp);
				myShaderProg->setUniform("MoonRadius3",(*iter)->getRadius()/(*iter)->getSphereScale());
			} else if (index==4) {
				myShaderProg->setUniform("MoonPosition4",tmp);
				myShaderProg->setUniform("MoonRadius4",(*iter)->getRadius()/(*iter)->getSphereScale());
			}

			index++;
		}
	}

	// clear any leftover values
	for(; index<=4; index++) {
		if (index==1) // No moon data
			myShaderProg->setUniform("MoonRadius1",0.0);
		if (index==2)
			myShaderProg->setUniform("MoonRadius2",0.0);
		if (index==3)
			myShaderProg->setUniform("MoonRadius3",0.0);
		if (index==4)
			myShaderProg->setUniform("MoonRadius4",0.0);
	}

	//~ if( this->getEnglishName() != "Earth")
	if ( myShader == SHADER_NIGHT )
		currentObj->draw(screen_sz, GL_TRIANGLES);
	else
		currentObj->draw(screen_sz);


	myShaderProg->use();
	glActiveTexture(GL_TEXTURE0);

	StateGL::disable(GL_CULL_FACE);
}

void BigBody::setSphereScale(float s, bool initial_scale)
{
	radius = initialRadius * s;
	if (initial_scale)
		initialScale = s;
	if (rings!=nullptr)
		rings->multiplyRadius(s);

	calculateBoundingRadius();
	updateBoundingRadii();
}

void BigBody::update(int delta_time, const Navigator* nav, const TimeMgr* timeMgr)
{
	Body::update(delta_time, nav, timeMgr);
	if (radius.isScaling() && rings!=nullptr)
		rings->multiplyRadius(radius/initialRadius);
}


void BigBody::drawRings(const Projector* prj,const Mat4d& mat,double screen_sz, Vec3f& _lightDirection, Vec3f& _planetPosition, float planetRadius){
	
	rings->draw(prj,mat,screen_sz,_lightDirection,_planetPosition,planetRadius);
	
}

void BigBody::drawHalo(const Navigator* nav, const Projector* prj, const ToneReproductor* eye){
	
	if (isVisible && flags.flag_halo && this->getOnScreenSize(prj, nav) < 10 && this->getSphereScale()<10.0) {
		// Workaround for depth buffer precision and near planets
		StateGL::disable(GL_DEPTH_TEST);
		halo->drawHalo(nav, prj, eye);
	}
	
}
