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

//	Class which compute and display the daylight sky color using openGL
//	the sky is computed with the Skylight class.

#include <GL/glew.h>
#include <string>
#include "atmosphere.hpp"
// #include "spacecrafter.hpp"
#include "utility.hpp"
#include "navigator.hpp"
#include "projector.hpp"
#include "tone_reproductor.hpp"
#include "fmath.hpp"
#include "space_date.hpp"
#include "event_manager.hpp"


#define SKY_RESOLUTION 48

#define NB_LUM ((SKY_RESOLUTION+1) * (SKY_RESOLUTION+1))



using namespace std;

Atmosphere::Atmosphere() : /*tab_sky(NULL),*/ world_adaptation_luminance(0.f), atm_intensity(0),
	lightPollutionLuminance(0), cor_optoma(0)
{
	// Create the vector array used to store the sky color on the full field of view
	tab_sky = new Vec3f*[SKY_RESOLUTION+1];
	for (int k=0; k<SKY_RESOLUTION+1 ; k++) {
		tab_sky[k] = new Vec3f[SKY_RESOLUTION+1];
	}
	setFaderDuration(0.f);
	createShader();
}

Atmosphere::~Atmosphere()
{
	for (int k=0; k<SKY_RESOLUTION+1 ; k++) {
		if (tab_sky[k]) delete [] tab_sky[k];
	}
	if (tab_sky) delete [] tab_sky;
	dataColor.clear();
	dataPos.clear();
	deleteShader();
}


//! Define whether to display atmosphere
void Atmosphere::setFlagShow(bool b) {
	fader = b;
	Event* event = new Event(Event::E_ATMOSPHERE, fader);
	EventManager::getInstance()->queue(event);
}

void Atmosphere::initGridViewport(const Projector *prj)
{
	stepX = (float)prj->getViewportWidth() / SKY_RESOLUTION;
	stepY = (float)prj->getViewportHeight() / SKY_RESOLUTION;
	viewport_left = (float)prj->getViewportPosX();
	viewport_bottom = (float)prj->getViewportPosY();
}

//initialise la grille des points pour le calcul de l'atmosphere
void Atmosphere::initGridPos()
{
	for (int y=0; y<SKY_RESOLUTION; y++) {
		for (int x=0; x<SKY_RESOLUTION+1; x++) {
			dataPos.push_back( viewport_left+x*stepX );
			dataPos.push_back( viewport_bottom+y*stepY );
			dataPos.push_back( viewport_left+x*stepX );
			dataPos.push_back( viewport_bottom+(y+1)*stepY );
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER,atmosphere.pos);
	glBufferData(GL_ARRAY_BUFFER,sizeof(float)*dataPos.size(), dataPos.data(),GL_STATIC_DRAW);

	dataPos.clear();
}

void Atmosphere::createShader()
{
	shaderAtmosphere= new shaderProgram();
	shaderAtmosphere->init("atmosphere.vert", "", "", "","atmosphere.frag");

	glGenBuffers(1,&atmosphere.color);
	glGenBuffers(1,&atmosphere.pos);

	glGenVertexArrays(1,&atmosphere.vao);
	glBindVertexArray(atmosphere.vao);
	glBindBuffer (GL_ARRAY_BUFFER, atmosphere.pos);
	glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,0,NULL);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
}

void Atmosphere::deleteShader()
{
	if (shaderAtmosphere)
		delete shaderAtmosphere;
	shaderAtmosphere=nullptr;

	glDeleteBuffers(1,&atmosphere.pos);
	glDeleteBuffers(1,&atmosphere.color);
	glDeleteVertexArrays(1,&atmosphere.vao);
}

void Atmosphere::computeColor(double JD, Vec3d sunPos, Vec3d moonPos, float moon_phase,
                               const ToneReproductor * eye, const Projector* prj,  const std::string &planetName,
                               float latitude, float altitude, float temperature, float relative_humidity)
{
	float min_mw_lum = 0.13;

	// no need to calculate if not visible
	if (!fader.getInterstate()) {
		atm_intensity = 0;
		world_adaptation_luminance = 3.75f + lightPollutionLuminance;
		milkyway_adaptation_luminance = min_mw_lum;  // brighter than without atm, since no drawing addition of atm brightness
		return;
	} else {
		atm_intensity = fader.getInterstate();
	}

	skylight_struct2 b2;

	// these are for radii
	double sun_angular_size = atan(696000./AU/sunPos.length());
	double moon_angular_size = atan(1738./AU/moonPos.length());

	double touch_angle = sun_angular_size + moon_angular_size;
	double dark_angle = moon_angular_size - sun_angular_size;

	sunPos.normalize();
	moonPos.normalize();

	// determine luminance falloff during solar eclipses
	double separation_angle = acos( sunPos.dot( moonPos ));  // angle between them
	//	printf("touch at %f\tnow at %f (%f)\n", touch_angle, separation_angle, separation_angle/touch_angle);

	// bright stars should be visible at total eclipse
	// because of above issues, this algorithm darkens more quickly than reality
	if ( separation_angle < touch_angle) {
		float min;
		if (dark_angle < 0) {
			// annular eclipse
			float asun = sun_angular_size*sun_angular_size;
			min = (asun - moon_angular_size*moon_angular_size)/asun;  // minimum proportion of sun uncovered
			dark_angle *= -1;
		} else min = 0.001; // so bright stars show up at total eclipse

		if (separation_angle < dark_angle) atm_intensity = min;
		else atm_intensity *= min + (1.-min)*(separation_angle-dark_angle)/(touch_angle-dark_angle);
		//		printf("atm int %f (min %f)\n", atm_intensity, min);
	}

	float sun_pos[3];
	sun_pos[0] = sunPos[0];
	sun_pos[1] = sunPos[1];
	sun_pos[2] = sunPos[2];

	float moon_pos[3];
	moon_pos[0] = moonPos[0];
	moon_pos[1] = moonPos[1];
	moon_pos[2] = moonPos[2];

	sky.setParamsv(sun_pos, 5.f, planetName);

	skyb.setLoc(latitude * C_PI/180., altitude, temperature, relative_humidity);
	skyb.setSunMoon(moon_pos[2], sun_pos[2], cor_optoma);

	// Calculate the date from the julian day.
	ln_date date;
	SpaceDate::JulianToDate(JD, &date);

	skyb.setDate(date.years, date.months, moon_phase);

	Vec3d point(1., 0., 0.);

	// Variables used to compute the average sky luminance
	double sum_lum = 0.;

	// Compute the sky color for every point above the ground
	for (int x=0; x<SKY_RESOLUTION+1; x++) {
		for (int y=0; y<SKY_RESOLUTION+1; y++) {
			prj->unprojectLocal((double)viewport_left+x*stepX, (double)viewport_bottom+y*stepY,point);
			point.normalize();

			if (point[2]<=0) {
				point[2] = -point[2];
				// The sky below the ground is the symetric of the one above :
				// it looks nice and gives proper values for brightness estimation
			}

			b2.pos[0] = point[0];
			b2.pos[1] = point[1];
			b2.pos[2] = point[2];

			// Use the Skylight model for the color
			sky.get_xyY_Valuev(b2);

			// Use the Skybright.cpp 's models for brightness which gives better results.
			b2.color[2] = skyb.getLuminance(moon_pos[0]*b2.pos[0]+moon_pos[1]*b2.pos[1]+
			                                 moon_pos[2]*b2.pos[2], sun_pos[0]*b2.pos[0]+sun_pos[1]*b2.pos[1]+
			                                 sun_pos[2]*b2.pos[2], b2.pos[2],cor_optoma);

			sum_lum+=b2.color[2];
			eye->xyY_to_RGB(b2.color);
			tab_sky[x][y].set(atm_intensity*b2.color[0],atm_intensity*b2.color[1],atm_intensity*b2.color[2]);
		}
	}

	world_adaptation_luminance = 3.75f + lightPollutionLuminance + 3.5*sum_lum/NB_LUM*atm_intensity;
	milkyway_adaptation_luminance = min_mw_lum*(1-atm_intensity) + 30*sum_lum/NB_LUM*atm_intensity;
	sum_lum = 0.f;
}


void Atmosphere::fillOutDataColor()
{
	dataColor.clear();
	for (int y=0; y<SKY_RESOLUTION; y++) {
		for (int x=0; x<SKY_RESOLUTION+1; x++) {
			dataColor.push_back(tab_sky[x][y][0]);
			dataColor.push_back(tab_sky[x][y][1]);
			dataColor.push_back(tab_sky[x][y][2]);
			//~ glColor3fv(tab_sky[x][y]);
			//~ glVertexi((int)(viewport_left+x*stepX),(int)(view_bottom+y*stepY));
			dataColor.push_back(tab_sky[x][y+1][0]);
			dataColor.push_back(tab_sky[x][y+1][1]);
			dataColor.push_back(tab_sky[x][y+1][2]);
			//~ glColor3fv(tab_sky[x][y+1]);
			//~ glVertexi((int)(viewport_left+x*stepX),(int)(view_bottom+(y+1)*stepY));
		}
	}
}

void Atmosphere::draw(const Projector* prj, const std::string &planetName)
{
	if (!fader.getInterstate())
		return;

	StateGL::BlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);

	fillOutDataColor();

	glBindVertexArray(atmosphere.vao);
	glBindBuffer (GL_ARRAY_BUFFER, atmosphere.color);
	glBufferData(GL_ARRAY_BUFFER,sizeof(float)*dataColor.size(),dataColor.data(),GL_DYNAMIC_DRAW);
	glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,0,NULL);
	StateGL::enable(GL_BLEND);

	shaderAtmosphere->use();
	for (int y=0; y<SKY_RESOLUTION; y++) {
		glDrawArrays(GL_TRIANGLE_STRIP,y*(SKY_RESOLUTION+1)*2,(SKY_RESOLUTION+1)*2);
	}
	shaderAtmosphere->unuse();
}
