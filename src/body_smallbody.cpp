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

#include "body_smallbody.hpp"
#include "perf_debug.hpp"
#include "trail.hpp"
#include "axis.hpp"
#include "orbit_2d.hpp"
#include "hints.hpp"
#include "halo.hpp"
#include "projector.hpp"
#include "navigator.hpp"

#include "ring.hpp"


using namespace std;

SmallBody::SmallBody(Body *parent,
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
                     const string& tex_norm_name):
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
	     tex_norm_name)
{
	if (_typePlanet == COMET)
		trail = new Trail(this,2920);
	else
		trail = new Trail(this, 60);

	selectShader();
	orbitPlot = new Orbit2D(this);
}

SmallBody::~SmallBody()
{
	if (trail) delete trail;
	trail = nullptr;
	if (orbitPlot) delete orbitPlot;
	orbitPlot = nullptr;
}

void SmallBody::selectShader ()
{

	bool useShaderNormal = true;

	if (tex_norm) { //bump Shader
		myShader = SHADER_BUMP;
		myShaderProg = shaderBump;
		useShaderNormal = false;
	}

	if (useShaderNormal) {
		myShader = SHADER_NORMAL;
		myShaderProg = shaderNormal;
	}
}
/*
void SmallBody::drawGL(Projector* prj, const Navigator* nav, const Observer* observatory, const ToneReproductor* eye, bool stencil, bool depthTest, bool drawHomePlanet, bool selected)
{
	if (hidden)
		return;
	//on ne dessine pas une planete sur laquel on se trouve
	if (!drawHomePlanet && observatory->isOnBody(this)) {
		return ;
	}

	orbitPlot->drawOrbit(nav, prj, parent_mat);
	
	if(trail != nullptr)	
		trail->drawTrail(nav, prj);

	if (ang_dist>0.25) //{
		//~ if (ang_dist>1.f) ang_dist = 1.f;
		hints->drawHints(nav, prj);
	//~ }

	if (depthTest)
		StateGL::enable(GL_DEPTH_TEST);

	if (screen_sz > 1 && isVisible) {  // huge improvement in performance
		if (stencil)
			StateGL::enable(GL_STENCIL_TEST);
		axis->drawAxis(prj, mat);
		drawBody(prj, nav, mat, screen_sz);
		if (stencil)
			StateGL::disable(GL_STENCIL_TEST);
	}

	if (isVisible && flagHalo) {
		StateGL::disable(GL_DEPTH_TEST);
		halo->drawHalo(nav, prj, eye);
	}

	StateGL::disable(GL_DEPTH_TEST);
}*/

void SmallBody::drawBody(const Projector* prj, const Navigator * nav, const Mat4d& mat, float screen_sz)
{
	//~ AutoPerfDebug apd(&pd, "SmallBody::drawBody$"); //Debug
	StateGL::enable(GL_CULL_FACE);
	//~ StateGL::disable(GL_CULL_FACE);
	StateGL::disable(GL_BLEND);

	glBindTexture(GL_TEXTURE_2D, tex_map->getID());

	myShaderProg->use();

	//load specific values for shader
	switch (myShader) {

		case SHADER_BUMP:
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, tex_map->getID());
			myShaderProg->setUniform("mapTexture",0);

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, tex_norm->getID());
			myShaderProg->setUniform("normalTexture",1);

			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, tex_eclipse_map->getID());
			myShaderProg->setUniform("shadowTexture",2);
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

	//paramètres commun aux shaders
	myShaderProg->setUniform("planetRadius",initialRadius);
	myShaderProg->setUniform("planetOneMinusOblateness",one_minus_oblateness);
	myShaderProg->setUniform("ModelViewMatrix",matrix);
	myShaderProg->setUniform("NormalMatrix", inv_matrix.transpose());

	int index=1;
	myShaderProg->setUniform("LightPosition",eye_sun);
	myShaderProg->setUniform("SunHalfAngle",sun_half_angle);

	Vec3f tmp= v3fNull;
	Vec3f tmp2(0.4, 0.12, 0.0);

	if (myShader == SHADER_BUMP)
		myShaderProg->setUniform("UmbraColor",tmp);

	Vec3d planet_helio = get_heliocentric_ecliptic_pos();
	Vec3d light = -planet_helio;
	light.normalize();

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

	currentObj->draw(screen_sz);

	myShaderProg->unuse();
	glActiveTexture(GL_TEXTURE0);
	StateGL::disable(GL_CULL_FACE);
}
