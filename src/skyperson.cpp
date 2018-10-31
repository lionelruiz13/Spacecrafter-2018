/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2002 Fabien Chereau
 * Copyright (C) 2009 Digitalis Education Solutions, Inc.
 * Copyright (C) 2013 of the LSS team
 * Copyright (C) 2014 of the LSS Team & Association Sirius
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
 * Spacecrafter is a free open project of the LSS team
 * See the TRADEMARKS file for free open project usage requirements.
 *
 */

#include "body.hpp"
#include "skyperson.hpp"
#include "s_texture.hpp"
#include "utility.hpp"
#include <string>
#include "log.hpp"
#include "app_settings.hpp"

#include "s_font.hpp"
#include "projector.hpp"
#include "navigator.hpp"
#include "tone_reproductor.hpp"
#include "translator.hpp"


using namespace std;

#define NB_MAX_POINTS 4194304

// -------------------- SKYLINE_PERSONAL  ---------------------------------------------

SkyPerson::SkyPerson(PERSON_TYPE _ptype)
{
	ptype= _ptype;
	createShader();
	switch(ptype) {
		case AL:
			proj_func = &Projector::projectLocal;
			break;
		case EQ:
			proj_func = &Projector::projectEarthEqu;
			break;
		default:
			proj_func = &Projector::projectLocal;
			break;
	}
	font = new s_font(12, AppSettings::Instance()->getUserDir()+"fonts/DejaVuSans.ttf");
}



SkyPerson::~SkyPerson()
{
	dataSky.clear();
	if (font) delete font;
	deleteShader();
}

void SkyPerson::createShader()
{
	shaderSkyPerson = new shaderProgram();
	shaderSkyPerson->init("person.vert","person.geom","person.frag");
	shaderSkyPerson->setUniformLocation("color");
	shaderSkyPerson->setUniformLocation("fader");
	shaderSkyPerson->setUniformLocation("Mat");

	glGenVertexArrays(1,&sData.vao);
	glBindVertexArray(sData.vao);
	glGenBuffers(1,&sData.pos);

	glEnableVertexAttribArray(0);
}

void SkyPerson::deleteShader()
{
	if(shaderSkyPerson!=nullptr) shaderSkyPerson=nullptr;

	glDeleteBuffers(1,&sData.pos);
	glDeleteVertexArrays(1,&sData.vao);
}

void SkyPerson::clear()
{
	dataSky.clear();
}

void SkyPerson::loadData(string filename)
{
	double alpha, delta,x,y,z;
	int nblines;
	Vec3f punts;
	clear();

	ifstream fichier(filename, ios::in);
	if(fichier) {
		fichier >> nblines;
		if (nblines>NB_MAX_POINTS)
			nblines= NB_MAX_POINTS;

		for(int i=0; i<nblines ; i++) {
			fichier >> alpha >> delta;
			Utility::spheToRect(alpha,delta, punts);
			// On Earth or AL		
			x=punts[0];
			y=punts[1];
			z=punts[2];
			// Elsewhere
			//x=punts[0];
			//y=punts[1]*cos(-23.43928*3.1415926/180.0)-punts[2]*sin(-23.43928*3.1415926/180.0);
			//z=punts[1]*sin(-23.43928*3.1415926/180.0)+punts[2]*cos(-23.43928*3.1415926/180.0);
			// End of test
			dataSky.push_back(x);
			dataSky.push_back(y);
			dataSky.push_back(z);
		}
		aperson=alpha;
		fichier.close();
	}

	//on charge les points dans un vbo
	glBindVertexArray(sData.vao);

	glBindBuffer(GL_ARRAY_BUFFER,sData.pos);
	glBufferData(GL_ARRAY_BUFFER,sizeof(float)*dataSky.size(),dataSky.data(),GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,NULL);
	//~ glEnableVertexAttribArray(0);
}

void SkyPerson::draw(const Projector *prj,const Navigator *nav)
{
	if (!fader.getInterstate()) return;

	StateGL::enable(GL_BLEND);
	StateGL::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Normal transparency mode

	shaderSkyPerson->use();
	shaderSkyPerson->setUniform("color", color);
	shaderSkyPerson->setUniform("fader", fader.getInterstate());

	if (ptype == AL)
		shaderSkyPerson->setUniform("Mat", prj->getMatLocalToEye() );
	else
		shaderSkyPerson->setUniform("Mat", prj->getMatEarthEquToEye() );

	glBindVertexArray(sData.vao);

	glDrawArrays(GL_LINES, 0, dataSky.size()/3 ); //un point est représenté par 3 points

	shaderSkyPerson->unuse();

	draw_text(prj, nav);
}

//a optimiser
void SkyPerson::draw_text(const Projector *prj,const Navigator *nav)
{
	if (((dataSky.size()/3)==198) || ((dataSky.size()/3)==396))
		//double alpha = 0.f; // Il me faut la premiere valeur en alpha de personal.txt ou personeq.txt selon
		for (int i=-9; i<10; i++) {
			std::ostringstream oss;
			//création des positions de points dans pt3,pt4
			Utility::spheToRect(aperson-0.31415926,(i*10)*3.15926/180.0, pt3);
			Utility::spheToRect(aperson-0.31415926+0.01,(i*10)*3.15926/180.0, pt4);
			if (((prj->*proj_func)(pt3, pt1)) && ((prj->*proj_func)(pt4, pt2))) { //test si pt3,pt4 est affichable et transmet à pt1,pt2 sa position
				double angle;
				const double dx = pt1[0]-pt2[0];
				const double dy = pt1[1]-pt2[1];
				const double dq = dx*dx+dy*dy;
				const double d = sqrt(dq);
				angle = acos((pt1[1]-pt2[1])/d);
				if ( pt1[0] < pt2[0] ) angle *= -1;
				if (abs(i)==9) angle += 3.1415926;
				Mat4f MVP = prj->getMatProjectionOrtho2D();
				//suite de transformations de position à partir des coordonnées de pt1
				Mat4f TRANSFO= Mat4f::translation( Vec3f(pt1[0],pt1[1],0) );
				TRANSFO = TRANSFO*Mat4f::rotation( Vec3f(0,0,-1), 3.1415926/2.0-angle);
				//oss << pt1[0] << " " << pt2[0] << pt1[1] << " " << pt2[1];
				oss << i*10 << "°";
				font->print(2,-2,oss.str(), color, MVP*TRANSFO ,1,1);
				oss.clear();
			}
		}
}
