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

#include "body_trace.hpp"
#include "utility.hpp"
#include "projector.hpp"
#include "navigator.hpp"
#include <string>

#include "fmath.hpp"

#include "perf_debug.hpp"

using namespace std;


BodyTrace::BodyTrace()
{
	for(int i= 0; i<NB_MAX_LIST; i++) {
		bodyData[i].size=0;
		bodyData[i].old_punt[0]=0.0;
		bodyData[i].old_punt[1]=0.0;
		bodyData[i].color= Vec3f(1.f,0.f,0.f);
		//~ bodyData[i].color[0]=1.;
		//~ bodyData[i].color[1]=0.;
		//~ bodyData[i].color[2]=0.;
		bodyData[i].hide=false;
	}
	bodyData[0].color = Vec3f(1.0,0.0,0.0);
	//~ bodyData[0]..color[0]=1.;
	//~ bodyData[0].color[1]=0.;
	//~ bodyData[0].color[2]=0.;

	bodyData[1].color= Vec3f(0.2f, 0.2f, 0.7f);
	//~ bodyData[1].color[0]=0.2;
	//~ bodyData[1].color[1]=0.2;
	//~ bodyData[1].color[2]=0.7;

	bodyData[2].color= Vec3f(0.5f, 0.9f, 0.4f);
	//~ bodyData[2].color[0]=0.5;
	//~ bodyData[2].color[1]=0.9;
	//~ bodyData[2].color[2]=0.4;

	bodyData[3].color=Vec3f(0.3f, 0.6f, 0.1f);
	//~ bodyData[3].color[0]=0.3;
	//~ bodyData[3].color[1]=0.6;
	//~ bodyData[3].color[2]=0.1;

	bodyData[4].color=Vec3f(0.1f, 0.8f, 0.5f);
	//~ bodyData[4].color[0]=0.1;
	//~ bodyData[4].color[1]=0.8;
	//~ bodyData[4].color[2]=0.5;

	bodyData[5].color= Vec3f(0.f, 1.0f, 0.2f);
	//~ bodyData[5].color[0]=0.;
	//~ bodyData[5].color[1]=1.;
	//~ bodyData[5].color[2]=0.2;

	bodyData[6].color= Vec3f(0.2f, 1.0f, 0.4f);
	//~ bodyData[6].color[0]=0.2;
	//~ bodyData[6].color[1]=1.;
	//~ bodyData[6].color[2]=0.4;

	bodyData[7].color= Vec3f(0.4f, 1.0f, 0.6f);
	//~ bodyData[7].color[0]=0.4;
	//~ bodyData[7].color[1]=1.;
	//~ bodyData[7].color[2]=0.6;

	is_tracing=true;
	currentUsedList=0;

	createShader();
}

BodyTrace::~BodyTrace()
{
	deleteShader();
}

void BodyTrace::hide(int numberList)
{
	if ( numberList > (NB_MAX_LIST-1)) return;
	if (numberList==-1) {
		//-1 mean all the lists
		for(int i=0; i<NB_MAX_LIST; i++)
			bodyData[i].hide= !bodyData[i].hide;
	} else
		bodyData[numberList].hide= !bodyData[numberList].hide;
}

void BodyTrace::createShader()
{
	shaderTrace = new shaderProgram();
	shaderTrace->init("body_trace.vert","body_trace.geom","body_trace.frag");
	shaderTrace->setUniformLocation("Color");
	shaderTrace->setUniformLocation("Mat");

	glGenVertexArrays(1,&trace.vao);
	glBindVertexArray(trace.vao);

	glGenBuffers(1,&trace.color);
	glGenBuffers(1,&trace.pos);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
}

void BodyTrace::deleteShader()
{
	if (shaderTrace)
		delete shaderTrace;
	shaderTrace = nullptr;

	glDeleteBuffers(1,&trace.color);
	glDeleteBuffers(1,&trace.pos);
	glDeleteVertexArrays(1,&trace.vao);
}


void BodyTrace::draw(const Projector *prj,const Navigator *nav)
{
	//~ AutoPerfDebug apd(&pd, "BodyTrace::draw$"); //Debug
	if (!fader.getInterstate()) return;

	StateGL::enable(GL_BLEND);
	StateGL::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Normal transparency mode

	shaderTrace->use();

	for(int l=0; l<currentUsedList+1; l++) {

		if (bodyData[l].size>2 && !bodyData[l].hide) {

			for (int i=0; i < bodyData[l].size; i=i+1) {

				//~ if ((prj->projectLocal(bodyData[l].punts[i], pt1)) && (prj->projectLocal(bodyData[l].punts[i+1], pt2)) ) {
					vecVertex.push_back( bodyData[l].punts[i][0] );
					vecVertex.push_back( bodyData[l].punts[i][1] );
					vecVertex.push_back( bodyData[l].punts[i][2] );
					//~ vecVertex.push_back( pt1[1] );
					//~ vecVertex.push_back( pt2[0] );
					//~ vecVertex.push_back( pt2[1] );
				//~ }
			}

			//tracé en direct de la courbe de ci dessus
			if( vecVertex.size() >=6 ) { // il faut au moins deux points pour le tracé

				shaderTrace->setUniform("Color", bodyData[l].color );
				shaderTrace->setUniform("Mat", prj->getMatLocalToEye() );

				glBindVertexArray(trace.vao);
				glBindBuffer(GL_ARRAY_BUFFER,trace.pos);
				glBufferData(GL_ARRAY_BUFFER, sizeof(float)*vecVertex.size(), vecVertex.data(), GL_DYNAMIC_DRAW);
				glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,NULL);

				glDrawArrays(GL_LINE_STRIP, 0, vecVertex.size()/3);
			}

			//suppression du contenu des data
			vecVertex.clear();

		}
	}
	shaderTrace->unuse();
}

void BodyTrace::addData(const Navigator *nav, double alt, double az)
{
	if (!fader.getInterstate()) return;
	if(!is_tracing) return;

	if (bodyData[currentUsedList].size==0) {
		bodyData[currentUsedList].old_punt[0]=alt;
		bodyData[currentUsedList].old_punt[1]=az;
		Utility::spheToRect(-(az+C_PI),alt,bodyData[currentUsedList].punts[0]);
		bodyData[currentUsedList].size=1;
	} else {
		if ( (abs(alt-bodyData[currentUsedList].old_punt[0])< 0.001) && (abs(az-bodyData[currentUsedList].old_punt[1])< 0.001) ) return;
		if (bodyData[currentUsedList].size==(MAX_POINTS-1)) return;
		bodyData[currentUsedList].old_punt[0]=alt;
		bodyData[currentUsedList].old_punt[1]=az;
		bodyData[currentUsedList].size+=1;
		Utility::spheToRect(-(az+C_PI),alt,bodyData[currentUsedList].punts[bodyData[currentUsedList].size]);
	}
}
